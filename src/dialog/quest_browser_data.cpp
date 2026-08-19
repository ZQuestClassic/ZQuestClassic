#include "dialog/quest_browser_data.h"
#include "dialog/quest_browser.h"
#include "base/version.h"
#include "zc/ffscript.h"
#include "core/misctypes.h"
#include "core/qst.h"
#include "tiles.h"
#include "zalleg/colors.h"
#include "zconfig.h"
#include "zq/zquest.h"
#include <fmt/format.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <sys/stat.h>

#ifndef __EMSCRIPTEN__
#include "base/process_management.h"
#else
#include "base/emscripten_utils.h"
#include <emscripten.h>
#endif

namespace fs = std::filesystem;

static constexpr int32_t MAX_IMPORTED = 5000;
static constexpr int32_t MAX_SCAN_DEPTH = 6;

static std::string normalize_path_impl(std::string const& path)
{
	std::error_code ec;
	fs::path p = fs::absolute(path, ec);
	if (ec)
		return path;
	return p.lexically_normal().string();
}

// A gathered quest file: path plus its recently-opened rank.
using FoundQuest = std::pair<std::string, int32_t>;

// === platform data provider ===
//
// All platform differences live below. Native lists quests straight off the
// disk and checks for updates via zupdater. Web additionally consults the
// quest manifest (see web/main.js): approval filtering, and metadata for
// unfetched quests - 0-byte lazy placeholder files that would download the
// entire quest if read - plus the persistent /local mount as a source.

// What a platform can know about a quest without reading the file.
struct PlatformQuestInfo
{
	bool approved = true;
	bool has_meta = false;
	std::string title, author, zc_version, date;
};

#ifdef __EMSCRIPTEN__

// Under the persistent mount so the cache survives page reloads.
static constexpr char const* CACHE_FILE = "/local/quest_browser.cache";

// Whether quests from the online quest database (the quests/*/rNN tree)
// are listed by default. Even when false, database quests the user has
// opened (the recent-quests list) still show, as do stock files and the
// user's own quests.
static constexpr bool SHOW_QUEST_DATABASE_QUESTS = true;

// One record per gathered path, filled from the quest manifest in a single
// bulk JS call (a round trip per quest would be too costly).
struct WebManifestRecord
{
	uint8_t approved;
	uint8_t has_meta;
	char title[96];
	char author[96];
	char zc_version[32];
	char date[32];
};
static_assert(offsetof(WebManifestRecord, title) == 2);
static_assert(offsetof(WebManifestRecord, author) == 98);
static_assert(offsetof(WebManifestRecord, zc_version) == 194);
static_assert(offsetof(WebManifestRecord, date) == 226);
static_assert(sizeof(WebManifestRecord) == 258);

EM_ASYNC_JS(void, em_gather_manifest_info_, (const char* paths_joined, uintptr_t out, int count, int record_size), {
	const paths = UTF8ToString(paths_joined).split('\n');
	let manifest = {};
	try {
		manifest = await ZC.getQuestManifest();
	} catch (e) {
		console.error('quest manifest unavailable:', e);
	}
	// No regex literal here: EM_JS stringification mangles escaped slashes.
	for (let i = 0; i < count; i++) {
		const rec = out + i * record_size;
		const parts = (paths[i] || "").split('/').filter(p => p.length);
		// Manifest ids are the first three path segments, e.g.
		// "quests/purezc/123". Paths with no manifest entry (like the stock
		// example quests) are not approval-gated.
		let quest = null;
		let approved = 1;
		if (parts[0] === 'quests' && parts.length >= 3) {
			quest = manifest[parts.slice(0, 3).join('/')] || null;
			if (quest)
				approved = ['auto', true].includes(quest.approval) ? 1 : 0;
		}
		HEAPU8[rec] = approved;
		HEAPU8[rec + 1] = quest ? 1 : 0;
		if (quest) {
			// TextEncoder rather than stringToUTF8: runtime helpers are not
			// reliably linked into EM_JS bodies.
			const enc = new TextEncoder();
			const put = (str, ptr, cap) => {
				const bytes = enc.encode(str || "");
				const n = Math.min(bytes.length, cap - 1);
				HEAPU8.set(bytes.subarray(0, n), ptr);
				HEAPU8[ptr + n] = 0;
			};
			const release = (quest.releases || []).find(r => r.name === parts[3]);
			// Authors are objects: [{name: "..."}, ...].
			const authors = (quest.authors || [])
				.map(a => typeof a === 'string' ? a : (a && a.name) || "")
				.filter(s => s.length)
				.join(', ');
			put(quest.name, rec + 2, 96);
			put(authors, rec + 98, 96);
			put(quest.zcVersion, rec + 194, 32);
			put(release && release.date, rec + 226, 32);
		}
	}
});

// Folders to list beyond the stock quests folder.
static std::vector<std::string> platform_extra_sources()
{
	// The user's persistent folder (their own quests live here).
	return {"/local"};
}

// Bulk per-path lookup; one entry per input path.
static std::vector<PlatformQuestInfo> platform_query_quests(std::vector<std::string> const& paths)
{
	std::vector<PlatformQuestInfo> out(paths.size());
	if (paths.empty())
		return out;

	std::string joined;
	for (auto const& p : paths)
	{
		joined += p;
		joined += '\n';
	}
	std::vector<WebManifestRecord> records(paths.size());
	em_gather_manifest_info_(joined.c_str(), (uintptr_t)records.data(),
		(int)paths.size(), (int)sizeof(WebManifestRecord));

	for (size_t i = 0; i < paths.size(); i++)
	{
		out[i].approved = records[i].approved;
		out[i].has_meta = records[i].has_meta;
		if (records[i].has_meta)
		{
			out[i].title = records[i].title;
			out[i].author = records[i].author;
			out[i].zc_version = records[i].zc_version;
			out[i].date = records[i].date;
		}
	}
	return out;
}

// The web build updates through the website; nothing to check.
static std::string platform_check_for_update()
{
	return "";
}

// Directory names that look like a quest revision: "r01", "r2", "r1.2".
static bool is_revision_dirname(std::string const& s)
{
	if (s.size() < 2 || (s[0] != 'r' && s[0] != 'R'))
		return false;

	bool any_digit = false;
	for (size_t i = 1; i < s.size(); i++)
	{
		char c = s[i];
		if (isdigit((uint8_t)c))
			any_digit = true;
		else if (c != '.' && c != '_' && c != '-')
			return false;
	}
	return any_digit;
}

// Numeric-aware "less" so r2 < r10.
static bool natural_less(std::string const& a, std::string const& b)
{
	size_t i = 0, j = 0;
	while (i < a.size() && j < b.size())
	{
		if (isdigit((uint8_t)a[i]) && isdigit((uint8_t)b[j]))
		{
			size_t i2 = i, j2 = j;
			while (i2 < a.size() && isdigit((uint8_t)a[i2])) i2++;
			while (j2 < b.size() && isdigit((uint8_t)b[j2])) j2++;
			// Compare the digit runs numerically (ignore leading zeros).
			while (i < i2 - 1 && a[i] == '0') i++;
			while (j < j2 - 1 && b[j] == '0') j++;
			if (i2 - i != j2 - j)
				return i2 - i < j2 - j;
			int cmp = a.compare(i, i2 - i, b, j, j2 - j);
			if (cmp != 0)
				return cmp < 0;
			i = i2;
			j = j2;
		}
		else
		{
			if (a[i] != b[j])
				return a[i] < b[j];
			i++;
			j++;
		}
	}
	return a.size() - i < b.size() - j;
}

// Curates the gathered list for the quest database's revisioned layout
// (<quest>/<r01..rNN>/<file>.qst, one directory per revision) - a web-only
// concept; native folders list as-is.
static void platform_curate_found(std::vector<FoundQuest>& found)
{
	if (!SHOW_QUEST_DATABASE_QUESTS)
	{
		// Database quests are hidden by default; the ones the user has
		// opened (anything with a recent-quests rank) still show, exactly
		// as opened - no revision collapsing.
		std::erase_if(found, [](FoundQuest const& fr) {
			if (fr.second != INT32_MAX)
				return false;
			std::string path = normalize_path_impl(fr.first);
			if (path.rfind("/quests/", 0) != 0)
				return false;
			return is_revision_dirname(fs::path(path).parent_path().filename().string());
		});
		return;
	}

	// Collapse revisions: only each quest's latest revision is listed.
	if (found.size() < 2)
		return;

	auto split = [](std::string const& raw, std::string& gp, std::string& pname) {
		fs::path p = normalize_path_impl(raw);
		pname = p.parent_path().filename().string();
		gp = p.parent_path().parent_path().string();
	};

	// Quest dir -> latest revision-named subdir with a gathered qst.
	std::map<std::string, std::string> latest_rev;
	for (auto const& [raw, rank] : found)
	{
		std::string gp, pname;
		split(raw, gp, pname);
		if (!is_revision_dirname(pname))
			continue;

		auto [it, fresh] = latest_rev.try_emplace(gp, pname);
		if (!fresh && natural_less(it->second, pname))
			it->second = pname;
	}

	std::erase_if(found, [&](auto const& fr) {
		std::string gp, pname;
		split(fr.first, gp, pname);
		if (!is_revision_dirname(pname))
			return false;
		auto it = latest_rev.find(gp);
		return it != latest_rev.end() && it->second != pname;
	});
}

#else // !__EMSCRIPTEN__

static constexpr char const* CACHE_FILE = "quest_browser.cache";

static std::vector<std::string> platform_extra_sources()
{
	return {};
}

static std::vector<PlatformQuestInfo> platform_query_quests(std::vector<std::string> const& paths)
{
	// Everything on disk is listable as-is.
	return std::vector<PlatformQuestInfo>(paths.size());
}

// Revisioned quest layouts are a web quest-database concept.
static void platform_curate_found(std::vector<FoundQuest>&)
{
}

// Returns the new version string the first time an update is found, else "".
static std::string platform_check_for_update()
{
	static bool checked = false;
	if (checked || !zc_get_config(quest_browser::CFG_SECTION, "check_for_updates", 1))
		return "";

	checked = true;

	std::string output;
	if (!run_and_get_output(locate_zapp_file(ZUPDATER_FILE), {"-headless", "-print-next-release"}, output))
		return "";

	auto output_map = parse_output_map(output);
	if (!output_map.contains("tag_name"))
		return "";

	std::string version = output_map["tag_name"];
	if (version == getVersionString())
		return "";

	return version;
}

#endif

// === end platform data provider ===

namespace quest_browser
{

namespace
{

std::vector<Entry> the_entries;
bool entries_initialized = false;
bool cache_dirty = false;

// The stock quests folder, shared with the player (zc.cfg [zeldadx]
// quest_dir; users can point it elsewhere).
std::string quest_folder()
{
	return zc_get_config("zeldadx", "quest_dir", "quests", App::zelda);
}

bool stat_file(std::string const& path, uint64_t& mtime, uint64_t& size)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	mtime = (uint64_t)st.st_mtime;
	size = (uint64_t)st.st_size;
	return true;
}

// Titles/authors are hand-typed into fixed-size header fields: trim the ends
// and collapse internal whitespace runs to a single space.
std::string clean_meta_text(std::string const& s)
{
	std::string out;
	out.reserve(s.size());
	bool in_space = true; // drops leading whitespace
	for (char c : s)
	{
		if (isspace((uint8_t)c))
		{
			in_space = true;
		}
		else
		{
			if (in_space && !out.empty())
				out += ' ';
			out += c;
			in_space = false;
		}
	}

	return out;
}

std::string hex_encode(std::vector<uint8_t> const& data)
{
	static char const* digits = "0123456789abcdef";
	std::string out;
	out.reserve(data.size() * 2);
	for (uint8_t b : data)
	{
		out += digits[b >> 4];
		out += digits[b & 15];
	}
	return out;
}

std::vector<uint8_t> hex_decode(std::string_view s)
{
	auto val = [](char c) -> int {
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		return -1;
	};
	std::vector<uint8_t> out;
	if (s.size() % 2)
		return out;

	out.reserve(s.size() / 2);
	for (size_t i = 0; i < s.size(); i += 2)
	{
		int hi = val(s[i]), lo = val(s[i + 1]);
		if (hi < 0 || lo < 0)
			return {};

		out.push_back((uint8_t)((hi << 4) | lo));
	}

	return out;
}

// Sort key for a version string like "1.90" or "2.55" - comparable with
// the legacy branch of version_key_of (2.55 -> 0x255).
uint64_t version_key_from_string(std::string const& s)
{
	int maj = 0, min = 0;
	if (sscanf(s.c_str(), "%d.%d", &maj, &min) < 1)
		return 0;
	int bcd = ((min / 10) << 4) | (min % 10);
	return ((uint64_t)(((maj & 0xF) << 8) | bcd)) << 8;
}

uint64_t version_key_of(zquestheader const& h)
{
	// Semver-era quests sort above all legacy ones. Within semver, order by
	// the triple, then by build date - prereleases and nightlies share a
	// triple (e.g. every 3.0.0 prerelease is "3.0.0"), so the date is what
	// actually distinguishes them. Legacy: internal version word + build.
	if (h.version_major > 0)
	{
		uint64_t date_min =
			((((uint64_t)h.new_version_id_date_year * 12 + h.new_version_id_date_month) * 31
				+ h.new_version_id_date_day) * 24 + h.new_version_id_date_hour) * 60
			+ h.new_version_id_date_minute;
		return (1ull << 62)
			| ((uint64_t)(h.version_major & 0xFF) << 54)
			| ((uint64_t)(h.version_minor & 0xFF) << 46)
			| ((uint64_t)(h.version_patch & 0xFF) << 38)
			| (date_min & ((1ull << 38) - 1));
	}
	return ((uint64_t)(word)h.zelda_version << 8) | h.build;
}

// === imported paths config ===

std::vector<std::string> load_imported_paths()
{
	std::vector<std::string> paths;
	int count = zc_get_config(CFG_SECTION, "imported_count", 0);
	count = std::clamp(count, 0, MAX_IMPORTED);
	for (int i = 0; i < count; i++)
	{
		std::string key = fmt::format("imported_{}", i);
		char const* str = zc_get_config(CFG_SECTION, key.c_str(), "");
		if (str && str[0])
			paths.push_back(str);
	}
	return paths;
}

void save_imported_paths(std::vector<std::string> const& paths)
{
	zc_set_config(CFG_SECTION, "imported_count", (int32_t)paths.size());
	for (int i = 0; i < (int32_t)paths.size(); i++)
	{
		std::string key = fmt::format("imported_{}", i);
		zc_set_config(CFG_SECTION, key.c_str(), paths[i].c_str());
	}
}

// === metadata cache ===

// Bump to discard caches written by older code (bake or sort-key changes).
constexpr char const* CACHE_HEADER = "qbcache 1";

void load_cache(std::map<std::string, Entry>& out)
{
	std::ifstream f(CACHE_FILE);
	if (!f)
		return;

	std::string line;
	if (!std::getline(f, line) || line != CACHE_HEADER)
		return;

	while (std::getline(f, line))
	{
		std::vector<std::string> fields;
		std::stringstream ss(line);
		std::string field;
		while (std::getline(ss, field, '\t'))
			fields.push_back(field);
		if (fields.size() != 8)
			continue;

		Entry e;
		e.path = fields[0];
		e.mtime = strtoull(fields[1].c_str(), nullptr, 10);
		e.size = strtoull(fields[2].c_str(), nullptr, 10);
		// Cleaned on load too, covering cache entries written before
		// titles were cleaned at scan time.
		e.title = clean_meta_text(fields[3]);
		e.author = clean_meta_text(fields[4]);
		e.zc_version = fields[5];
		e.version_key = strtoull(fields[6].c_str(), nullptr, 10);
		if (fields[7] != "-")
		{
			e.icon_rgba = hex_decode(fields[7]);
			if (e.icon_rgba.size() != 16 * 16 * 4)
				e.icon_rgba.clear();
		}
		e.meta_scanned = e.meta_ok = e.icon_scanned = true;
		out[e.path] = std::move(e);
	}
}

void save_cache()
{
	std::ofstream f(CACHE_FILE, std::ios::trunc);
	if (!f)
		return;

	f << CACHE_HEADER << '\n';
	for (auto const& e : the_entries)
	{
		if (!e.meta_scanned || !e.meta_ok)
			continue;
		// Manifest-sourced entries are free to reconstruct every gather.
		if (e.from_manifest)
			continue;

		// Note: titles/authors went through clean_meta_text, so the fields
		// can't contain the tabs/newlines that would break this format.
		f << e.path << '\t' << e.mtime << '\t' << e.size << '\t'
		  << e.title << '\t' << e.author << '\t'
		  << e.zc_version << '\t' << e.version_key << '\t'
		  << (e.icon_rgba.empty() ? "-" : hex_encode(e.icon_rgba)) << '\n';
	}
}

// Bakes an icon tile (from the current newtilebuf/colordata globals) into
// 16x16 RGBA, the same way the player's save-select icon is produced (see
// update_icon in zc/saves.cpp). `t` is the quest's Misc.icons[0].
void bake_icon(Entry& e, int32_t t)
{
	e.icon_rgba.clear();

	if (t < 0 || t >= NEWMAXTILES)
		t = 0;
	int32_t tileind = t ? t : 28;

	// In-memory 4-bit tiles are unpacked: one pixel per byte, values 0-15.
	if (newtilebuf[tileind].format != tf4Bit || !newtilebuf[tileind].data)
		return;
	byte const* data = newtilebuf[tileind].data;
	byte const* pal = colordata + CSET(t ? pSprite(spICON1) : 6) * 3;

	std::vector<uint8_t> rgba(16 * 16 * 4, 0);
	bool any = false;
	for (int i = 0; i < 256; i++)
	{
		byte v = data[i] & 15;
		if (!v)
			continue; // color 0 = transparent

		any = true;
		uint8_t* px = &rgba[i * 4];
		// colordata components are 8-bit: readcolordata upscales pre-v6
		// section data through _rgb_scale_6 at load.
		px[0] = pal[v * 3 + 0];
		px[1] = pal[v * 3 + 1];
		px[2] = pal[v * 3 + 2];
		px[3] = 255;
	}

	if (any)
		e.icon_rgba = std::move(rgba);
}

// === gathering ===

// Quest backups live next to the quest in "<name>.qst.backups" folders
// (Windows: "backups.<name>.qst") - see create_path_for_backup in
// zq_class.cpp. The browser never lists them.
bool is_backup_folder_name(std::string const& name)
{
	return name.starts_with("backups.") || name.ends_with(".backups");
}

bool in_backup_folder(std::string const& path)
{
	for (auto const& part : fs::path(path).parent_path())
	{
		if (is_backup_folder_name(part.string()))
			return true;
	}
	return false;
}

void gather_folder(std::string const& folder, std::vector<std::string>& out, int32_t depth)
{
	if (depth > MAX_SCAN_DEPTH || (int32_t)out.size() >= MAX_IMPORTED)
		return;

	std::error_code ec;
	fs::directory_iterator it(folder, ec), end;
	while (!ec && it != end)
	{
		if ((int32_t)out.size() >= MAX_IMPORTED)
			return;
		auto const& ent = *it;
		std::string name = ent.path().filename().string();
		if (ent.is_directory(ec))
		{
			if (!name.empty() && name[0] != '.' && !is_backup_folder_name(name))
				gather_folder(ent.path().string(), out, depth + 1);
		}
		else if (ent.path().extension() == ".qst")
			out.push_back(ent.path().string());
		ec.clear();
		it.increment(ec);
	}
}

} // namespace

std::vector<Entry>& entries()
{
	return the_entries;
}

bool gathered()
{
	return entries_initialized;
}

void invalidate()
{
	entries_initialized = false;
}

// The web build's persistent mount only stores what has been synced; a
// page reload discards everything else. No-op natively.
static void persist_fs()
{
#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif
}

void mark_cache_dirty()
{
	cache_dirty = true;
}

void flush_cache()
{
	if (!cache_dirty)
		return;

	save_cache();
	cache_dirty = false;
	persist_fs();
}

void persist()
{
	flush_cache();
	// Config holds the recent-quest list, imported paths, and browser
	// settings; nothing else flushes it until app exit.
	flush_config_file();
	persist_fs();
}

std::string normalize_path(std::string const& path)
{
	return normalize_path_impl(path);
}

std::string format_date(uint64_t mtime)
{
	time_t t = (time_t)mtime;
	tm* lt = localtime(&t);
	if (!lt)
		return "";

	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", lt);
	return buf;
}

// Adds a file or folder to the imported list. Returns true if it was new.
bool add_imported_path(std::string const& raw)
{
	std::string path = normalize_path(raw);
	auto paths = load_imported_paths();
	for (auto const& p : paths)
	{
		if (normalize_path(p) == path)
			return false;
	}

	if ((int32_t)paths.size() >= MAX_IMPORTED)
		return false;

	paths.push_back(path);
	save_imported_paths(paths);
	return true;
}

std::string display_name_for_path(std::string const& path)
{
	static std::string const prefix = []() {
		std::string p = normalize_path(quest_folder());
		p += (char)fs::path::preferred_separator;
		return p;
	}();

	if (path.size() > prefix.size() && path.compare(0, prefix.size(), prefix) == 0)
	{
		// A deeply nested relative path won't fit the title column either.
		std::string rel = fs::path(path.substr(prefix.size())).generic_string();
		if (rel.size() <= 40)
			return rel;
	}

	return fs::path(path).filename().string();
}

std::string check_for_update()
{
	return platform_check_for_update();
}

bool scan_meta(Entry& e)
{
	auto quest_load_guard = make_partial_quest_load_guard();

	int32_t error = 0;
	PACKFILE* f = open_quest_file(&error, e.path.c_str(), false);
	if (!f)
		return false;

	zquestheader h{};
	int32_t ret = readheader(f, &h, 0);
	pack_fclose(f);
	if (ret != 0)
		return false;

	e.title = clean_meta_text(h.title);
	e.author = clean_meta_text(h.author);
	e.zc_version = h.getVerStr();
	e.version_key = version_key_of(h);
	return true;
}

// Partial-loads just the sections needed for the icon (tiles, csets, game
// icons). Safe while a quest is loaded in the editor: the guard restores
// every global the load touches.
bool scan_icon(Entry& e)
{
	byte skip_flags[4];
	memset(skip_flags, 0xFF, sizeof(skip_flags));
	set_bit(skip_flags, skip_header, 0);
	set_bit(skip_flags, skip_tiles, 0);
	set_bit(skip_flags, skip_csets, 0);
	// readcolordata's internal legacy skip check tests skip_colors, not
	// skip_csets - with it set, the cset section is read but discarded.
	set_bit(skip_flags, skip_colors, 0);
	set_bit(skip_flags, skip_icons, 0);

	auto quest_load_guard = make_partial_quest_load_guard();

	miscQdata temp_misc = QMisc;
	zquestheader h{};
	int32_t ret = loadquest(e.path.c_str(), &h, &temp_misc, customtunes, false, skip_flags, 0, false);

	bool ok = ret == qe_OK;
	if (ok)
		bake_icon(e, temp_misc.icons[0]);

	clear_tiles(grabtilebuf);
	return ok;
}

void gather()
{
	std::map<std::string, Entry> cache;
	load_cache(cache);

	// Recent quests, in order (rank doubles as the recently-opened sort key).
	std::vector<std::pair<std::string, int32_t>> found;
	for (int i = 0; i < 10; i++)
	{
		std::string key = fmt::format("rec_qst_{}", i);
		char const* str = zc_get_config("recent", key.c_str(), "");
		if (str && str[0] && strcmp(str, "---") != 0)
			found.emplace_back(str, i);
	}

	// Everything in the quests folder (shipped next to the app by default).
	{
		std::vector<std::string> qsts;
		gather_folder(quest_folder(), qsts, 0);
		for (auto const& q : qsts)
			found.emplace_back(q, INT32_MAX);
	}

	// Imported files and folders.
	for (auto const& p : load_imported_paths())
	{
		std::error_code ec;
		if (fs::is_directory(p, ec))
		{
			std::vector<std::string> qsts;
			gather_folder(p, qsts, 0);
			for (auto const& q : qsts)
				found.emplace_back(q, INT32_MAX);
		}
		else
		{
			found.emplace_back(p, INT32_MAX);
		}
	}

	// Platform-specific sources (e.g. the web build's persistent /local).
	for (auto const& src : platform_extra_sources())
	{
		std::vector<std::string> qsts;
		gather_folder(src, qsts, 0);
		for (auto const& q : qsts)
			found.emplace_back(q, INT32_MAX);
	}

	// One bulk platform query: approval filtering, plus any metadata the
	// platform already knows (path -> info for those).
	std::map<std::string, PlatformQuestInfo> quest_info;
	if (!found.empty())
	{
		std::vector<std::string> norm;
		norm.reserve(found.size());
		for (auto const& [raw, rank] : found)
			norm.push_back(normalize_path(raw));

		auto infos = platform_query_quests(norm);
		std::vector<std::pair<std::string, int32_t>> kept;
		kept.reserve(found.size());
		for (size_t i = 0; i < found.size(); i++)
		{
			if (!infos[i].approved)
				continue;
			if (infos[i].has_meta)
				quest_info.emplace(norm[i], std::move(infos[i]));
			kept.push_back(std::move(found[i]));
		}
		found = std::move(kept);
	}

	// Web quest-database curation (revision collapsing / default visibility).
	platform_curate_found(found);

	the_entries.clear();
	std::map<std::string, size_t> index_of;
	for (auto const& [raw, rank] : found)
	{
		std::string path = normalize_path(raw);

		// Covers recent/imported paths pointing inside a backup folder.
		if (in_backup_folder(path))
			continue;

		auto it = index_of.find(path);
		if (it != index_of.end())
		{
			the_entries[it->second].recent_rank =
				std::min(the_entries[it->second].recent_rank, rank);
			continue;
		}

		uint64_t mtime, size;
		if (!stat_file(path, mtime, size))
			continue; // missing file

		Entry e;
		auto cit = cache.find(path);
		auto wit = quest_info.find(path);
		if (size == 0 && cit != cache.end())
		{
			// A previously-fetched quest whose file is a 0-byte placeholder
			// again (the web build after a page reload): the cache's scanned
			// data - real icon and version - beats the manifest's, and its
			// stored mtime/size keep the date display and cache key stable.
			e = cit->second;
			mtime = e.mtime;
			size = e.size;
		}
		else if (size == 0 && wit != quest_info.end())
		{
			// An unfetched quest (0-byte lazy placeholder): use the
			// platform-supplied metadata instead of reading the file, which
			// would download the entire quest.
			auto const& info = wit->second;
			e.title = clean_meta_text(info.title);
			e.author = clean_meta_text(info.author);
			e.zc_version = info.zc_version;
			e.version_key = version_key_from_string(info.zc_version);
			e.date_str = info.date;
			e.from_manifest = true;
			e.meta_scanned = e.meta_ok = true;
			e.icon_scanned = true; // no icon without downloading the quest
			mtime = 0; // placeholder mtimes are meaningless
		}
		else
		{
			if (cit != cache.end() && cit->second.mtime == mtime && cit->second.size == size)
				e = cit->second;
			else
				cache_dirty = true;
		}
		e.path = path;
		e.mtime = mtime;
		e.size = size;
		e.recent_rank = rank;

		index_of[path] = the_entries.size();
		the_entries.push_back(std::move(e));
	}

	entries_initialized = true;
}

} // namespace quest_browser

void quest_browser_record_loaded_quest(char const* path)
{
	using namespace quest_browser;

	if (!path || !path[0])
		return;

	std::string npath = normalize_path(path);
	uint64_t mtime, size;
	if (!stat_file(npath, mtime, size))
		return;

	Entry* entry = nullptr;
	for (auto& e : entries())
	{
		if (e.path == npath)
			entry = &e;
	}

	if (!entry)
	{
		entries().emplace_back();
		entry = &entries().back();
		entry->path = npath;
	}

	// The full quest is in memory: metadata and icon are free.
	entry->mtime = mtime;
	entry->size = size;
	entry->title = clean_meta_text(header.title);
	entry->author = clean_meta_text(header.author);
	entry->zc_version = header.getVerStr();
	entry->version_key = version_key_of(header);
	entry->meta_scanned = entry->meta_ok = true;
	bake_icon(*entry, QMisc.icons[0]);
	entry->icon_scanned = true;
	mark_cache_dirty();
	// Also persists the recent-quest config this open just updated.
	persist();
}
