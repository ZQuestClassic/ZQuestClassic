#include "new_qst.h"
#include "new_packfile.h"
#include "base/util.h"
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_memfile.h>
#include <physfs.h>
#include "zc/ffscript.h"
#include "base/version.h"
#include <miniz.h>

using namespace util;
using namespace NewQuest;

#define FP state.fp

size_t new_write_size = 0;

class quest_io_exception : std::exception
{
public:
	const char* what() const noexcept override
	{
		return msg.c_str();
	}
	quest_io_exception(string const& line)
	{
		msg = line;
	}
	template <typename... Args>
	quest_io_exception(fmt::format_string<Args...> s, Args&&... args)
		: quest_io_exception(fmt::format(s, std::forward<Args>(args)...))
	{}
	
	quest_io_exception& add(string const& line)
	{
		msg = fmt::format("{}\n{}", line, msg); // new lines at the start
		return *this;
	}
	template <typename... Args>
	quest_io_exception& add(fmt::format_string<Args...> s, Args&&... args)
	{
		std::string text = fmt::format(s, std::forward<Args>(args)...);
		return add(text);
	}
private:
	string msg;
};
static const quest_io_exception QUEST_INVALID("Error!");

std::map<string, QuestSection*> sections;

void QuestReadState::enter_section(QuestSection const& sec)
{
	exit_section();
	section_name = sec.name;
	fname = fmt::format("{}.section", sec.name);
	fp = al_fopen(fname.c_str(), "rb");
	if (!fp)
		throw quest_io_exception("Section not found, or could not be opened for reading!");
}
void QuestReadState::exit_section()
{
	if (fp)
		al_fclose(fp);
	fp = nullptr;
	s_version = 0;
	section_name.clear();
	fname.clear();
}
void QuestWriteState::enter_section(QuestSection const& sec)
{
	exit_section();
	section_name = sec.name;
	fname = fmt::format("{}.section", sec.name);
	s_version = sec.cur_version;
	
	new_write_size = 0;
}
void QuestWriteState::allocate_section()
{
	buffer_size = new_write_size;
	new_write_size = 0;
	buffer = al_malloc(buffer_size);
	if (!buffer)
		throw quest_io_exception("Out of memory; failed to allocate buffer!");
	fp = al_open_memfile(buffer, buffer_size, "wb");
	if (!fp)
		throw quest_io_exception("Failed to target memory buffer for writing!");
}
void QuestWriteState::exit_section()
{
	if (fp)
		al_fclose(fp);
	fp = nullptr;
	if (buffer)
		al_free(buffer);
	buffer = nullptr;
	buffer_size = 0;
	s_version = 0;
	section_name.clear();
	fname.clear();
}
void QuestWriteState::write_zip()
{
	al_fclose(fp);
	fp = nullptr;
	if (!mz_zip_writer_add_mem(&zip, fname.c_str(), buffer, buffer_size, MZ_BEST_COMPRESSION))
		throw quest_io_exception("Failed to write buffered data to zip output!");
	exit_section();
}

QuestSection::QuestSection(string const& name, word cur_version,
	std::function<void(QuestReadState&)> reader,
	std::function<void(QuestWriteState&)> writer)
	: name(name), cur_version(cur_version), dirty(false),
	reader(reader), writer(writer)
{
	ASSERT(!sections.contains(name)); // section names are unique
	sections[name] = this;
}

void QuestSection::read(QuestReadState& state)
{
	box_out(fmt::format("Reading '{}'", name).c_str());
	try
	{
		state.enter_section(*this);
		
		string read_name;
		if (!new_getcstr(read_name, FP))
			throw quest_io_exception("Failed to read section header!");
		
		if (read_name != name)
			throw quest_io_exception("Found mismatched section header! Found '{}', expected '{}'!", read_name, name);
		
		if (!new_getw(state.s_version, FP))
			throw quest_io_exception("Failed to read section version!");
		
		if (state.s_version > cur_version)
			throw quest_io_exception("Version error; {} version {} is too new, expected {}",
				name, state.s_version, cur_version);
		
		box_out(fmt::format(" v{}...", state.s_version).c_str());
		
		reader(state);
	}
	catch (quest_io_exception& e)
	{
		state.exit_section();
		throw e.add("In section '{}'...", name);
	}
	state.exit_section();
	box_out(" OK.");
	box_eol();
}
void QuestSection::write(QuestWriteState& state)
{
	if (!dirty)
	{
		box_out(fmt::format("Skipping '{}' (no changes)", name).c_str());
		box_eol();
		return;
	}
	box_out(fmt::format("Writing '{}' v{}...", name, cur_version).c_str());
	
	try
	{
		state.enter_section(*this);
		
		for (auto q = 0; q <= 1; ++q)
		{
			if (q == 1) // we have the write size, allocate the buffer and open the memfile
				state.allocate_section();
			
			if (!new_putcstr(name, FP))
				throw quest_io_exception("Failed to write section header!");
			
			if (!new_putw(cur_version, FP))
				throw quest_io_exception("Failed to write section version!");
		
			writer(state);
		}
		if (state.buffer_size != new_write_size) // confirm same size on both passes
			throw quest_io_exception("Section size mismatch; {} != {}", state.buffer_size, new_write_size);
		
		state.write_zip();
	}
	catch (quest_io_exception& e)
	{
		state.exit_section();
		throw e.add("In section '{}'...", name);
	}
	
	box_out(" OK.");
	box_eol();
}

static void read_header(QuestReadState& state)
{
	auto s_version = state.s_version;
	FFCore.quest_format[vHeader] = s_version;
	
	auto* header = state.header;
	*header = zquestheader();
	if (!new_getw(header->zelda_version, FP))
		throw QUEST_INVALID;
	FFCore.quest_format[vZelda] = header->zelda_version;
	if (header->zelda_version > ZELDA_VERSION)
		throw quest_io_exception("Saved program version {} too new; expected {} or older.",
			header->zelda_version, ZELDA_VERSION);
	if (bad_version(header->zelda_version))
		throw quest_io_exception("Saved program version {} too old! Not supported!",
			header->zelda_version);
	if (!new_getc(header->build, FP))
		throw QUEST_INVALID;
	FFCore.quest_format[vBuild] = header->build;
	if (!new_fread(header->pwd_hash, 16, FP))
		throw QUEST_INVALID;
	if (!new_getw(header->internal, FP))
		throw QUEST_INVALID;
	if (!new_getc(header->quest_number, FP))
		throw QUEST_INVALID;
	FFCore.quest_format[qQuestNumber] = header->quest_number;
	if (!new_fread(header->version, 16, FP))
		throw QUEST_INVALID;
	if (!new_fread(header->minver, 16, FP))
		throw QUEST_INVALID;
	if (!new_fread(header->title, sizeof(header->title), FP))
		throw QUEST_INVALID;
	header->title[sizeof(header->title)-1] = 0;
	if (!new_fread(header->author, sizeof(header->author), FP))
		throw QUEST_INVALID;
	header->author[sizeof(header->author)-1] = 0;
	if (!new_getc(header->use_keyfile, FP))
		throw QUEST_INVALID;
	if (!new_fread(header->data_flags, sizeof(header->data_flags), FP))
		throw QUEST_INVALID;
	if (!new_fread(header->templatepath, sizeof(header->templatepath), FP))
		throw QUEST_INVALID;
	if (!new_getl(header->version_major, FP))
		throw QUEST_INVALID;
	if (!new_getl(header->version_minor, FP))
		throw QUEST_INVALID;
	if (!new_getl(header->version_patch, FP))
		throw QUEST_INVALID;
	string version_string;
	if (!new_getcstr(version_string, FP))
		throw QUEST_INVALID;
	strncpy(header->zelda_version_string, version_string.c_str(), sizeof(header->zelda_version_string)-1);
	header->new_version_id_fourth = 0;
	header->new_version_id_alpha = 0;
	header->new_version_id_beta = 0;
	header->new_version_id_gamma = 0;
	header->new_version_id_release = 0;
	if (!new_getw(header->new_version_id_date_year, FP))
		throw QUEST_INVALID;
	if (!new_getc(header->new_version_id_date_month, FP))
		throw QUEST_INVALID;
	if (!new_getc(header->new_version_id_date_day, FP))
		throw QUEST_INVALID;
	if (!new_getc(header->new_version_id_date_hour, FP))
		throw QUEST_INVALID;
	if (!new_getc(header->new_version_id_date_minute, FP))
		throw QUEST_INVALID;
	header->new_version_devsig[0] = 0;
	if (!new_fread(header->new_version_compilername, 256, FP))
		throw QUEST_INVALID;
	if (!new_fread(header->new_version_compilerversion, 256, FP))
		throw QUEST_INVALID;
	string product_name;
	if (!new_getcstr(product_name, FP))
		throw QUEST_INVALID;
	strcpy(header->product_name, product_name.c_str());
	header->compilerid = 1;
	if (!new_getl(header->compilerversionnumber_first, FP))
		throw QUEST_INVALID;
	if (!new_getl(header->compilerversionnumber_second, FP))
		throw QUEST_INVALID;
	if (!new_getl(header->compilerversionnumber_third, FP))
		throw QUEST_INVALID;
	if (!new_getl(header->compilerversionnumber_fourth, FP))
		throw QUEST_INVALID;
	header->developerid = 0;
	header->made_in_module_name[0] = 0;
	string date, time;
	if (!new_getcstr(date, FP))
		throw QUEST_INVALID;
	if (!new_getcstr(time, FP))
		throw QUEST_INVALID;
	strcpy(header->build_datestamp, date.c_str());
	strcpy(header->build_timestamp, time.c_str());
	if (!new_fread(header->build_timezone, 6, FP))
		throw QUEST_INVALID;
	if (!new_getc(header->new_version_is_nightly, FP))
		throw QUEST_INVALID;
}
static void write_header(QuestWriteState& state)
{
	auto const* header = state.header;
	if (!new_putw(header->zelda_version, FP))
		throw QUEST_INVALID;
	if (!new_putc(header->build, FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->pwd_hash, 16, FP))
		throw QUEST_INVALID;
	if (!new_putw(header->internal, FP))
		throw QUEST_INVALID;
	if (!new_putc(header->quest_number, FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->version, 16, FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->minver, 16, FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->title, sizeof(header->title), FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->author, sizeof(header->author), FP))
		throw QUEST_INVALID;
	if (!new_putc(header->use_keyfile, FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->data_flags, sizeof(header->data_flags), FP))
		throw QUEST_INVALID;
	if (!new_fwrite(header->templatepath, sizeof(header->templatepath), FP))
		throw QUEST_INVALID;
	auto version = getVersion();
	if (!new_putl(version.major, FP))
		throw QUEST_INVALID;
	if (!new_putl(version.minor, FP))
		throw QUEST_INVALID;
	if (!new_putl(version.patch, FP))
		throw QUEST_INVALID;
	if (!new_putcstr(version.version_string, FP))
		throw QUEST_INVALID;
	if (!new_putw(BUILDTM_YEAR, FP))
		throw QUEST_INVALID;
	if (!new_putc(BUILDTM_MONTH, FP))
		throw QUEST_INVALID;
	if (!new_putc(BUILDTM_DAY, FP))
		throw QUEST_INVALID;
	if (!new_putc(BUILDTM_HOUR, FP))
		throw QUEST_INVALID;
	if (!new_putc(BUILDTM_MINUTE, FP))
		throw QUEST_INVALID;
	char buf[256] = {0};
	strcpy(buf, COMPILER_NAME);
	if (!new_fwrite(buf, 256, FP))
		throw QUEST_INVALID;
	memset(buf, 0, sizeof(buf));
	#ifdef _MSC_VER
	zc_itoa(_MSC_VER, buf, 10);
	#else
	strcpy(buf, COMPILER_VERSION);
	#endif
	if (!new_fwrite(buf, 256, FP))
		throw QUEST_INVALID;
	if (!new_putcstr("ZQuest Classic", FP))
		throw QUEST_INVALID;
	
	#ifdef _MSC_VER
	if (!new_putl(_MSC_VER / 100, FP))
		throw QUEST_INVALID;
	if (!new_putl(_MSC_VER % 100, FP))
		throw QUEST_INVALID;
	# if _MSC_VER >= 1400
	if (!new_putl(_MSC_FULL_VER % 100000, FP))
		throw QUEST_INVALID;
	# else
	if (!new_putl(_MSC_FULL_VER % 10000, FP))
		throw QUEST_INVALID;
	#endif // _MSC_VER >= 1400
	if (!new_putl(_MSC_BUILD, FP))
		throw QUEST_INVALID;
	#else // _MSC_VER not defined
	if (!new_putl(COMPILER_V_FIRST, FP))
		throw QUEST_INVALID;
	if (!new_putl(COMPILER_V_SECOND, FP))
		throw QUEST_INVALID;
	if (!new_putl(COMPILER_V_THIRD, FP))
		throw QUEST_INVALID;
	if (!new_putl(COMPILER_V_FOURTH, FP))
		throw QUEST_INVALID;
	#endif // ifdef _MSC_VER
	
	if (!new_putcstr(__DATE__, FP))
		throw QUEST_INVALID;
	if (!new_putcstr(__TIME__, FP))
		throw QUEST_INVALID;
	char tzbuf[6] = {0};
	strncpy(tzbuf, __TIMEZONE__, 5);
	if (!new_fwrite(tzbuf, 6, FP))
		throw QUEST_INVALID;
	if (!new_putc(isStableRelease() ? 0 : 1, FP))
		throw QUEST_INVALID;
}
QuestSection header(SEC_HEADER, NV_HEADER, read_header, write_header);

///
/// Quest Reading
///

static void _load_new_quest_int(QuestReadState& state)
{
	al_set_physfs_file_interface();
	if (!PHYSFS_mount(state.quest_path, "", 0))
		throw quest_io_exception("Failed to mount file {}", state.quest_path);
	
	try
	{
		header.read(state); // ensure the header reads first
		
		if (state.flags & qstload_print_meta)
			print_quest_metadata(*state.header, state.quest_path, 0xFF);
		
		//{ Version Warning
		if (state.flags & qstload_versionwarn)
		{
			int32_t vercmp = state.header->compareVer();
			int32_t astatecmp = compare(int32_t(state.header->getAlphaState()), getAlphaState());
			int32_t avercmp = compare(state.header->getAlphaVer(), 0);
			if(vercmp > 0 || (!vercmp && (astatecmp > 0 || (!astatecmp && avercmp > 0))))
			{
				enter_sys_pal();
				bool r = alert_confirm("Quest saved in newer version",
					"This quest was last saved in a newer version of ZQuest."
					" Attempting to load this quest may not work correctly; to"
					" avoid issues, try loading this quest in at least '" + std::string(state.header->getVerStr()) + "'"
					"\n\nWould you like to continue loading anyway? (Not recommended)");
				exit_sys_pal();
				if(!r)
					throw quest_io_exception("Version too new; user chose to cancel");
			}
		}
		
		for (auto [secname, ptr] : sections)
		{
			if (ptr == &header) continue;
			ptr->read(state);
		}
	}
	catch(quest_io_exception& e)
	{
		if (!PHYSFS_unmount(state.quest_path))
			e.add("[UNMOUNT FAILURE]");
		throw e;
	}
	if (!PHYSFS_unmount(state.quest_path))
		throw quest_io_exception("[UNMOUNT FAILURE]");
}
bool load_new_quest(const char *filename, zquestheader *header, miscQdata *misc,
	zctune *tune_list, quest_load_flags flags, dword tileset_flags)
{
	// In CI, builds are cached for replay tests, which can result in their build dates being earlier than what it would be locally.
	// So to avoid a more-recently updated .qst file from hitting the "last saved in a newer version" prompt, we disable in CI.
	if (is_ci())
		flags &= ~qstload_versionwarn;
	
	string relpath = relativize_path(filename);
	QuestReadState state
	{
		.quest_abs_path = filename,
		.quest_path = relpath.c_str(),
		.header = header,
		.misc = misc,
		.tune_list = tune_list,
		.flags = flags,
		.tileset_flags = tileset_flags,
	};
	char const* quest_name = get_filename(filename);
	zapp_reporting_add_breadcrumb("load_new_quest", quest_name);
	zapp_reporting_set_tag("qst.filename", quest_name);
	
	box_start(1, "Loading Quest", get_zc_font(font_lfont), font, true);
	box_out("Loading Quest...");
	box_eol();
	box_eol();
	
	string err_str;
	bool err = false;
	ALLEGRO_STATE old_state;
	al_store_state(&old_state, ALLEGRO_STATE_NEW_FILE_INTERFACE);
	auto start = std::chrono::steady_clock::now();
	zprint2("Loading '.quest': %s\n", filename);
	try
	{
		_load_new_quest_int(state);
	}
	catch (quest_io_exception& e)
	{
		err = true;
		err_str = e.what();
		if (err_str.empty())
			err_str = "UNKNOWN ERROR!";
	}
	int32_t load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
	zprint2("Time to load '.quest': %d ms\n", load_ms);
	al_restore_state(&old_state);
	if (err)
		zprint2("Error: %s\n", err_str.c_str());
	if (state.flags & qstload_show_progress)
	{
		if(err)
		{
			box_eol();
			box_out("-- Error loading quest file! --");
			box_eol();
			box_out(err_str.c_str());
			box_end(true);
		}
		else box_end(false);
	}

	zapp_reporting_set_tag("qst.author", header->author);
	zapp_reporting_set_tag("qst.title", header->title);
	zapp_reporting_set_tag("qst.zc_version", header->getVerStr());

	return !err;
}

///
/// Quest Writing
///
static void write_keyfiles(QuestWriteState& state)
{
	if(state.header->use_keyfile && state.header->dirty_password)
	{
		char const* kfname = state.quest_path;
		char keyfilename[2048]={0};
		zprint2("Writing key files for '%s'\n", kfname);
		
		char temp_pw[QSTPWD_LEN] = {0};
		uint ind = 0;
		for(char const* ext : {"key","zpwd","zcheat"})
		{
			replace_extension(keyfilename, kfname, ext, 2047);
			PACKFILE *fp = pack_fopen_password(keyfilename, F_WRITE, "");
			char msg[80] = {0};
			sprintf(msg, "ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!");
			msg[78]=13;
			msg[79]=10;
			pfwrite(msg, 80, fp);
			p_iputw(state.header->zelda_version, fp);
			p_putc(state.header->build, fp);
			char const* pwd = state.header->password;
			if(ind == 2) //.zcheat, hashed pwd
			{
				char hashmap = 'Z';
				hashmap += 'Q';
				hashmap += 'U';
				hashmap += 'E';
				hashmap += 'S';
				hashmap += 'T';
				for ( int q = 0; q < QSTPWD_LEN; ++q )
				{
					temp_pw[q] = state.header->password[q];
					temp_pw[q] += hashmap;
				}
				pwd = temp_pw;
			}
			pfwrite(pwd, strlen(pwd), fp);
			pack_fclose(fp);
			++ind;
		}
	}
}
static void _save_new_quest_int(QuestWriteState& state)
{
	box_start(1, "Saving Quest", get_zc_font(font_lfont), font, true);
	box_out("Saving Quest...");
	box_eol();
	box_eol();
	
	string tmp_filename = util::create_temp_file_path(state.quest_path);
	string abs_tmp_filename = derelativize_path(tmp_filename);
	try
	{
		PHYSFS_setWriteDir(".");
		
		memset(&state.zip, 0, sizeof(mz_zip_archive));
		
		if (!mz_zip_writer_init_file(&state.zip, tmp_filename.c_str(), 0))
			throw quest_io_exception("Failed to create file!");
		
		// Data will be written out to memory buffers via `al_open_memfile`
		header.write(state); // ensure the header outputs first
		for (auto [secname, ptr] : sections)
		{
			if (ptr == &header) continue;
			ptr->write(state);
		}
		
		if (!mz_zip_writer_finalize_archive(&state.zip))
			throw quest_io_exception("Failed to finalize zip archive!");
		if (!mz_zip_writer_end(&state.zip))
			throw quest_io_exception("Failed to end zip writer!");
		
		write_keyfiles(state);
		
		// Move file to destination at end, to avoid issues with file being unavailable to test mode.
		std::error_code ec;
		fs::rename(abs_tmp_filename, state.quest_abs_path, ec);
		if (ec)
			throw quest_io_exception("File rename error: ", std::strerror(ec.value()));
	}
	catch (quest_io_exception& e)
	{
		std::error_code ec;
		fs::remove(abs_tmp_filename, ec);
		throw e;
	}
	
#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif
}
bool save_new_quest(const char* filename, zquestheader* header, miscQdata const* misc,
	zctune const* tune_list, bool saveall)
{
	string relpath = relativize_path(filename);
	QuestWriteState state
	{
		.quest_abs_path = filename,
		.quest_path = relpath.c_str(),
		.header = header,
		.misc = misc,
		.tune_list = tune_list,
	};
	
	if (saveall)
		for (auto [name, ptr] : sections)
			ptr->dirty = true;
	string err_str;
	bool err = false;
	ALLEGRO_STATE old_state;
	al_store_state(&old_state, ALLEGRO_STATE_NEW_FILE_INTERFACE);
	try
	{
		_save_new_quest_int(state);
		strncpy(header->zelda_version_string, getVersionString(), sizeof(header->zelda_version_string)-1);
	}
	catch (quest_io_exception& e)
	{
		err = true;
		err_str = e.what();
		if (err_str.empty())
			err_str = "UNKNOWN ERROR!";
	}
	al_restore_state(&old_state);
	
	fake_pack_writing = false;
	if(err)
	{
		box_out("-- Error saving quest file! --");
		box_eol();
		box_out(err_str.c_str());
	}
	else
	{
		for (auto [name, ptr] : sections)
			ptr->dirty = false;
	}
	box_end(err);
	return !err;
}

void mark_section_dirty(string const& id, bool dirty)
{
	sections[id]->dirty = dirty;
}

