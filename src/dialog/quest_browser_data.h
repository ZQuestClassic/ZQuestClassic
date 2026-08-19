#ifndef ZC_DIALOG_QUEST_BROWSER_DATA_H_
#define ZC_DIALOG_QUEST_BROWSER_DATA_H_

#include <cstdint>
#include <string>
#include <vector>

// Data layer for the quest browser: gathering the quest list from every
// source, the metadata cache, and per-quest metadata/icon extraction. All
// platform (web vs native) differences live in the implementation;
// quest_browser.cpp is UI only.
namespace quest_browser
{

constexpr char const* CFG_SECTION = "quest_browser";

struct Entry
{
	std::string path; // absolute, normalized
	uint64_t mtime = 0;
	uint64_t size = 0;
	std::string title, author, zc_version;
	// Sort key for the ZC-version sort; roughly newest-first across old and
	// new versioning.
	uint64_t version_key = 0;
	std::vector<uint8_t> icon_rgba; // 16x16 RGBA; empty = none
	bool meta_scanned = false;
	bool meta_ok = false;
	bool icon_scanned = false;
	// Metadata was supplied by the platform provider (web quest manifest)
	// rather than read from the file; date_str replaces the mtime display.
	bool from_manifest = false;
	std::string date_str;
	int32_t recent_rank = INT32_MAX;
};

// The entry table. Rebuilt by gather(); persists across dialog reshows.
std::vector<Entry>& entries();
bool gathered();
// Makes the next QuestBrowserDialog re-gather.
void invalidate();

// Rebuilds the entry table: recent quests, the quests folder, imported
// files/folders, and platform sources, minus quests the platform filters
// out and old revisions. One bulk platform query; no quest files are read.
void gather();

// Reads title/author/version from the quest file's header.
bool scan_meta(Entry& e);
// Extracts the quest's save-select icon via a partial load.
// Both are safe to call with a quest loaded in the editor.
bool scan_icon(Entry& e);

// Scan results are cached keyed by path+mtime+size.
void mark_cache_dirty();
void flush_cache();
// flush_cache plus config flush and (web) filesystem sync, so recents,
// imports, and the cache survive a page reload. Call when leaving the
// browser or after recording an opened quest.
void persist();

// Adds a file or folder to the imported list. Returns true if it was new.
bool add_imported_path(std::string const& raw);

std::string normalize_path(std::string const& path);
std::string format_date(uint64_t mtime);
// Stand-in name for untitled quests: the path relative to the quests
// folder, or just the filename.
std::string display_name_for_path(std::string const& path);
// Returns the new version string the first time an update is found, else "".
std::string check_for_update();

}

#endif
