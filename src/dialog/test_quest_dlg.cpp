#include "base/dmap.h"
#include "base/initdata.h"
#include "dialog/info.h"
#include "test_quest_dlg.h"
#include "alertfunc.h"
#include "base/process_management.h"
#include "zq/zq_class.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "init.h"
#include <fmt/format.h>
#include <filesystem>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

int32_t onSave();
int32_t onSaveAs();
extern char *filepath;
extern bool saved, first_save;

static int32_t test_start_dmap = 0, test_start_screen = 0, test_ret_sqr = 0, test_init_data_val = 0;
static std::vector<std::string> test_init_data;
static std::vector<std::string> test_init_data_names;
static process_killer test_killer;

static const GUI::ListData retsqrlist
{
	{ "A", 0 },
	{ "B", 1 },
	{ "C", 2 },
	{ "D", 3 }
};

static GUI::ListData init_data_list;

int calculate_test_dmap();
void call_testqst_dialog()
{
	if(!first_save) //Require quest file is saved (i.e. not 'Untitled' fresh from File->New)
	{
		onSaveAs();
		if(!first_save)
			return;
	}
	test_start_dmap = calculate_test_dmap();
	test_start_screen = zc_min(0x7F, Map.getCurrScr());
	
	TestQstDialog().show();
}

TestQstDialog::TestQstDialog() : dmap_list(GUI::ZCListData::dmaps(true))
{}

std::shared_ptr<GUI::Widget> TestQstDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	init_data_list = GUI::ListData();
	init_data_list.add("<default>", 0);

	test_init_data.clear();
	test_init_data_names.clear();
	int i = 1;
	std::string qst_cfg_header = qst_cfg_header_from_path(filepath);
	while (strlen(zc_get_config(qst_cfg_header.c_str(), fmt::format("test_init_data_{}", i).c_str(), "")))
	{
		std::string init_data = zc_get_config(qst_cfg_header.c_str(), fmt::format("test_init_data_{}", i).c_str(), "");
		std::string name = zc_get_config(qst_cfg_header.c_str(), fmt::format("test_init_data_{}_name", i).c_str(), "");
		if (name.empty())
			name = fmt::format("Init Data {}", i);
		test_init_data.push_back(std::move(init_data));
		test_init_data_names.push_back(name);
		init_data_list.add(name, i);
		i += 1;
	}

	return Window(
		title = "Test Quest",
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Label(text = "Start DMap:"),
				DropDownList(data = dmap_list,
					fitParent = true,
					selectedValue = test_start_dmap,
					onSelectFunc = [&](int32_t val)
					{
						test_start_dmap = val;
					}
				),
				Label(text = "Start Screen:"),
				TextField(
					fitParent = true, minwidth = 4_em,
					type = GUI::TextField::type::SWAP_BYTE,
					low = 0x00, high = 0x7F, val = test_start_screen,
					swap_type = 1,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						test_start_screen = val;
					}),
				Label(text = "Return Square:"),
				DropDownList(
					maxwidth = 10_em,
					data = retsqrlist,
					selectedValue = test_ret_sqr,
					onSelectFunc = [&](int32_t val)
					{
						test_ret_sqr = val;
					}
				),
				Label(text = "Init Data:"),
				DropDownList(
					maxwidth = 10_em,
					data = init_data_list,
					selectedValue = test_init_data_val,
					onSelectFunc = [&](int32_t val)
					{
						test_init_data_val = val;
					}
				)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "Test",
					minwidth = 90_px,
					onClick = message::OK,
					focused = true),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "Edit Test Init Data",
					minwidth = 90_px,
					onClick = message::EDIT_INIT_DATA),
				Button(
					text = "Create Test Init Data",
					minwidth = 90_px,
					onClick = message::CREATE_INIT_DATA)
			),
			Rows<2>(
				INFOBTN("Save before opening test mode."),
				Checkbox(text = "Auto-Save", fitParent = true,
					checked = zc_get_config("zquest", "test_mode_auto_save", true),
					onToggleFunc = [&](bool state)
					{
						zc_set_config("zquest", "test_mode_auto_save", state);
					}
				),

				INFOBTN("Restart player when qst is saved."),
				Checkbox(text = "Auto-Restart", fitParent = true,
					checked = zc_get_config("zquest", "test_mode_auto_restart", false),
					onToggleFunc = [&](bool state)
					{
						zc_set_config("zquest", "test_mode_auto_restart", state);
					}
				),

				INFOBTN("Save a recording to replays/test_XXXXXXXX.zplay."),
				Checkbox(text = "Record", fitParent = true,
					checked = zc_get_config("zquest", "test_mode_record", false),
					onToggleFunc = [&](bool state)
					{
						zc_set_config("zquest", "test_mode_record", state);
					}
				)
			)
		)
	);
}

static void save_test_init_data(int i)
{
	std::string qst_cfg_header = qst_cfg_header_from_path(filepath);
	zc_set_config(qst_cfg_header.c_str(), fmt::format("test_init_data_{}", i + 1).c_str(), test_init_data[i].c_str());
	zc_set_config(qst_cfg_header.c_str(), fmt::format("test_init_data_{}_name", i + 1).c_str(), test_init_data_names[i].c_str());
}

bool TestQstDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		// TODO: calling `zinit.clear` should really be done in ctor...
		case message::CREATE_INIT_DATA:
		{
			zinitdata zinit_test = zinit;
			bool old_saved = saved;
			// Only allow configuration for the same stuff visible in the cheat menu in the player.
			doInit(&zinit_test, true);
			saved = old_saved;

			std::string delta = serialize_init_data_delta(&zinit, &zinit_test);

			int i = test_init_data.size();
			test_init_data.push_back(delta);
			test_init_data_names.push_back(fmt::format("Init Data {}", i + 1));
			save_test_init_data(i);
			rerun_dlg = true;
			test_init_data_val = i + 1;
			return true;
		}
		break;

		case message::EDIT_INIT_DATA:
		{
			if (test_init_data_val == 0)
				return false;
			if (test_init_data_val - 1 >= test_init_data.size())
				return false;

			std::string error;
			zinitdata* zinit_test = apply_init_data_delta(&zinit, test_init_data[test_init_data_val - 1], error);
			if (!zinit_test)
			{
				InfoDialog("Error applying init data delta", error).show();
				return false;
			}

			bool old_saved = saved;
			doInit(zinit_test, true);
			saved = old_saved;

			std::string delta = serialize_init_data_delta(&zinit, zinit_test);
			test_init_data[test_init_data_val - 1] = delta;
			save_test_init_data(test_init_data_val - 1);

			delete zinit_test;
			return false;
		}
		break;

		case message::OK:
		{
			if (!saved && zc_get_config("zquest", "test_mode_auto_save", true))
				onSave();

#ifdef __EMSCRIPTEN__
			em_open_test_mode(filepath, test_start_dmap, test_start_screen, test_ret_sqr);
#else
			test_killer.kill();

			bool should_record = zc_get_config("zquest", "test_mode_record", false);
			std::filesystem::path replay_file_dir = std::filesystem::path(zc_get_config("zquest", "replay_file_dir", "replays/")) / "test";
			std::filesystem::create_directory(replay_file_dir);
			auto replay_path = replay_file_dir / fmt::format("test_{}.zplay", std::time(nullptr));

			std::vector<std::string> args = {
				"-test",
				filepath,
				fmt::format("{}", test_start_dmap),
				fmt::format("{}", test_start_screen),
				fmt::format("{}", test_ret_sqr),
			};
			if (test_init_data_val)
			{
				args.push_back("-test-init-data");
				args.push_back(test_init_data[test_init_data_val - 1]);
			}
			if (zc_get_config("zquest", "test_mode_auto_restart", false))
			{
				args.push_back("-test-auto-restart");
			}
			if (should_record)
			{
				args.push_back("-record");
				args.push_back(replay_path.string().c_str());
			}
			test_killer = launch_process(ZPLAYER_FILE, args);
#endif
		}
		return true;
		
		case message::CANCEL:
			return true;
	}
	return false;
}
