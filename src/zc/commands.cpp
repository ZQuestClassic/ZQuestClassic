#include "zc/commands.h"
#include "base/misctypes.h"
#include "base/qst.h"
#include "base/zapp.h"
#include "zc/jit.h"
#include "zc/replay_upload.h"
#include "zc/saves.h"
#include "zc/script_debug.h"
#include "zc/zasm_optimize.h"
#include "zc/zasm_pipeline.h"
#include "zc/zc_sys.h"
#include "zc/zelda.h"

void do_load_and_quit_command(const char* quest_path, bool jit_precompile)
{
	byte skip_flags[] = {0, 0, 0, 0};
	int ret = loadquest(quest_path,&QHeader,&QMisc,tunes+ZC_MIDI_COUNT,false,skip_flags,true,false,0xFF);
	if (ret)
		exit(ret);

	strcpy(qstpath, quest_path);
	printf("Hash: %s\n", QHeader.hash().c_str());

	if (jit_precompile)
		jit_set_enabled(true);
	zasm_pipeline_init(true);

	exit(0);
}

void do_extract_zasm_command(const char* quest_path)
{
	byte skip_flags[] = {0, 0, 0, 0};
	int ret = loadquest(quest_path,&QHeader,&QMisc,tunes+ZC_MIDI_COUNT,false,skip_flags,false,false,0xFF);
	if (ret)
		exit(ret);

	DEBUG_PRINT_TO_FILE = true;
	strcpy(qstpath, quest_path);
	bool top_functions = true;
	bool generate_yielder = get_flag_bool("-extract-zasm-yielder").value_or(false);
	if (get_flag_bool("-jit").value_or(false))
		jit_set_enabled(true);
	zasm_pipeline_init(true);
	zasm_for_every_script(true, [&](zasm_script* script){
		ScriptDebugHandle h(script, ScriptDebugHandle::OutputSplit::ByScript, script->name);
		h.print(zasm_to_string(script, top_functions, generate_yielder).c_str());
	});

	exit(0);
}

void zplayer_handle_commands()
{
	int test_zc_arg = zapp_check_switch("-test-zc");
	if (test_zc_arg)
	{
		bool success = true;
		if (!saves_test())
		{
			success = false;
			printf("saves_test failed\n");
		}
		if (!zasm_optimize_test())
		{
			success = false;
			printf("zasm_optimize_test failed\n");
		}
		if (success)
			printf("all tests passed\n");
		exit(success ? 0 : 1);
	}

	if (zapp_check_switch("-upload-replays"))
	{
#ifdef HAS_CURL
		replay_upload();
		exit(0);
#else
		exit(1);
#endif
	}

	int only_arg = zapp_check_switch("-only", {"qst"});
	if (only_arg)
	{
		fs::path only_path = fs::current_path() / zapp_get_arg_string(only_arg + 1);
		only_qstpath = only_path.string();
		if (only_path.extension() != ".qst")
			Z_error_fatal("-only value must be a qst file, but got: %s\n", only_qstpath.c_str());
		if (!fs::exists(only_path))
			Z_error_fatal("Could not find file: %s\n", only_qstpath.c_str());
	}

	int test_opt_zasm_arg = zapp_check_switch("-test-optimize-zasm", {"qst"});
	if (test_opt_zasm_arg)
	{
		std::string path = zapp_get_arg_string(test_opt_zasm_arg + 1);
		zasm_optimize_run_for_file(path);
		exit(0);
	}

	int load_and_quit_arg = zapp_check_switch("-load-and-quit", {"qst"});
	if (load_and_quit_arg > 0)
	{
		std::string path = zapp_get_arg_string(load_and_quit_arg + 1);
		bool jit_precompile = zapp_check_switch("-jit-precompile");
		do_load_and_quit_command(path.c_str(), jit_precompile);
	}

	int extract_zasm_arg = zapp_check_switch("-extract-zasm", {"qst"});
	if (extract_zasm_arg > 0)
	{
		std::string path = zapp_get_arg_string(extract_zasm_arg + 1);
		do_extract_zasm_command(path.c_str());
	}

	int create_save_arg = zapp_check_switch("-create-save", {"qst"});
	if (create_save_arg)
	{
		set_headless_mode();

		std::string path = zapp_get_arg_string(create_save_arg + 1);

		gamedata* new_game = new gamedata();
		new_game->header.name = "hiro";
		new_game->header.qstpath = path;
		new_game->set_maxlife(3*16);
		new_game->set_life(3*16);
		new_game->set_maxbombs(8);
		new_game->set_continue_dmap(0);
		new_game->set_continue_scrn(0xFF);

		if (auto r = saves_create_slot(new_game); !r)
			Z_error_fatal("failed to create save file: %s\n", r.error().c_str());

		exit(0);
	}

	if (zapp_check_switch("-no_console"))
		console_enabled = false;
}
