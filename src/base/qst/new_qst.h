#pragma once

#include "base/qst.h"
#include "base/misctypes.h"

void mark_section_dirty(string const& id, bool dirty = true);

#define SEC_HEADER "quest_header"
#define NV_HEADER  1

enum quest_load_flags : uint8_t
{
	qstload_none          = 0x00,
	qstload_show_progress = 0x01,
	qstload_print_meta    = 0x02,
	qstload_versionwarn   = 0x04,
};

struct QuestSection;

struct QuestSectionState
{
	word s_version = 0;
	string section_name;
	string fname;
	
	void clear();
	void load(QuestSection const& sec);
};

struct QuestReadState
{
	QuestSectionState sec_state;
	char const* quest_abs_path = nullptr;
	char const* quest_path = nullptr;
	ALLEGRO_FILE* fp = nullptr;
	
	zquestheader* header = nullptr;
	miscQdata* misc = nullptr;
	zctune* tune_list = nullptr;
	
	quest_load_flags flags = qstload_none;
	dword tileset_flags = 0;
};
struct QuestWriteState
{
	QuestSectionState sec_state;
	char const* quest_abs_path = nullptr;
	char const* quest_path = nullptr;
	ALLEGRO_FILE* fp = nullptr;
	
	zquestheader const* header = nullptr;
	miscQdata const* misc = nullptr;
	zctune const* tune_list = nullptr;
};

struct QuestSection
{
	const string name;
	const word cur_version;
	bool dirty;
	
	QuestSection(string const& name, word cur_version,
		std::function<void(QuestReadState&)> reader,
		std::function<void(QuestWriteState&)> writer);
	
	void read(QuestReadState& state);
	void write(QuestWriteState& state);
private:
	const std::function<void(QuestReadState&)> reader;
	const std::function<void(QuestWriteState&)> writer;
};

bool load_new_quest(const char *filename, zquestheader *header, miscQdata *misc,
	zctune *tune_list, quest_load_flags flags, dword tileset_flags);
bool save_new_quest(const char* filename, zquestheader* header, miscQdata const* misc,
	zctune const* tune_list, bool saveall = false);
