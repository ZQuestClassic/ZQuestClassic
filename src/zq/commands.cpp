#include "zq/commands.h"
#include "core/dmap.h"
#include "base/general.h"
#include "core/qst.h"
#include "base/zapp.h"
#include "dialog/compilezscript.h"
#include "dialog/quest_browser.h"
#include "dialog/quest_browser_data.h"
#include "dialog/quest_rules.h"
#include "zc_list_data.h"
#include "zc/ffscript.h"
#include "zq/package.h"
#include "zq/zq_class.h"
#include "zq/zq_custom.h"
#include "zq/zq_files.h"
#include "zq/zq_tiles.h"
#include "zq/zquest.h"
#include "zalleg/packfile.h"
#include "test_runner/test_runner.h"
#include <fstream>

extern bool is_zq_replay_test;

// Removes the top layer encoding from a quest file. See open_quest_file.
// This has zero impact on the contents of the quest file. There should be no way for this to
// break anything.
static void do_unencrypt_qst_command(const char* input_filename, const char* output_filename)
{
	// If the file is already an unencrypted packfile, there's nothing to do.
	PACKFILE* pf_check = zalleg_pack_fopen_password(input_filename, F_READ_PACKED, "");
	pack_fclose(pf_check);
	if (pf_check) return;

	int32_t error;
	PACKFILE* pf = open_quest_file(&error, input_filename, false);
	PACKFILE* pf2 = zalleg_pack_fopen_password(output_filename, F_WRITE_PACKED, "");
	int c;
	while ((c = pack_getc(pf)) != EOF)
	{
		pack_putc(c, pf2);
	}
	pack_fclose(pf);
	pack_fclose(pf2);
	clear_quest_tmpfile();

	zq_exit(0);
}

// This will remove the PACKFILE compression. Incidentally, it also removes the top encoding layer.
static void do_uncompress_qst_command(const char* input_filename, const char* output_filename)
{
	auto unencrypted_result = try_open_maybe_legacy_encoded_file(input_filename, ENC_STR, nullptr, QH_NEWIDSTR, QH_IDSTR);
	if (unencrypted_result.not_found)
	{
		printf("qst not found\n");
		zq_exit(1);
	}
	if (!unencrypted_result.compressed && !unencrypted_result.encrypted)
	{
		// If the file is already an uncompressed file, there's nothing to do but copy it.
		fs::copy(input_filename, output_filename);
		zq_exit(0);
		return;
	}

	pack_fclose(unencrypted_result.decoded_pf);

	int32_t error;
	PACKFILE* pf = open_quest_file(&error, input_filename, false);
	PACKFILE* pf2 = zalleg_pack_fopen_password(output_filename, F_WRITE, "");
	int c;
	while ((c = pack_getc(pf)) != EOF)
	{
		pack_putc(c, pf2);
	}
	pack_fclose(pf);
	pack_fclose(pf2);
	clear_quest_tmpfile();

	zq_exit(0);
}

// Copy a quest file by loading and resaving, exactly like if the user did it in the UI.
// Note there could be changes introduced in the loading or saving functions. These are
// typically for compatability, but could possibly be a source of bugs.
static void do_copy_qst_command(const char* input_filename, const char* output_filename)
{
	set_headless_mode();

	// We need to init some stuff before loading a quest file will work.
	int fake_errno = 0;
	allegro_errno = &fake_errno;
	get_qst_buffers();

	int ret = load_quest(input_filename, false);
	if (ret)
	{
		zq_exit(ret);
	}

	ret = save_quest(output_filename);
	zq_exit(ret);
}

static std::vector<std::string> wrap_qr_info(std::string info)
{
	static const int max_chars = 140;
	info = info.substr(0, info.find_last_not_of(" \t\n")+1);
	if (info.empty())
		return {};
	if (auto ws_idx = info.find_first_not_of(" \t\n"); ws_idx != std::string::npos)
		info = info.substr(ws_idx);
	if (info.size() <= max_chars && info.find_first_of("\n") == std::string::npos)
		return { info };
	std::vector<std::string> ret;
	
	size_t idx = 0;
	while (idx < info.size())
	{
		while (info[idx] == '\n')
		{
			ret.push_back("");
			++idx;
		}
		idx = info.find_first_not_of(" ", idx); // skip leading ws
		
		size_t newline_idx = info.find_first_of("\n", idx);
		size_t new_idx = idx + max_chars;
		if (newline_idx != std::string::npos)
			new_idx = zc_min(newline_idx, new_idx);
		if (new_idx < info.size())
		{
			for (int ptr = new_idx - idx; ptr >= 0; --ptr)
			{
				char c = info[idx+ptr];
				if (c == ' ' || c == '\n')
					break;
				if (ptr == max_chars) continue;
				new_idx = idx + ptr;
			}
		}
		std::string tmp = info.substr(idx, new_idx-idx);
		tmp = tmp.substr(0, tmp.find_last_not_of(" ")+1); // trim trailing ws
		ret.push_back(tmp);
		// if (info[new_idx] == "\n")
			// ++new_idx;
		idx = new_idx;
	}
	return ret;
}

void do_dev_qrs_zscript_command(std::string const& fname)
{
	// Names come from the table generated from qrs.h; the gaps in the enum have none and
	// are skipped below.
	static const std::map<int, std::string> qr_const_names = []{
		std::map<int, std::string> names;
		for (int q = 0; q < qr_MAX; ++q)
			if (const char* name = qr_name(q))
				names.emplace(q, name);
		return names;
	}();
	static std::map<std::string, std::string> qr_compat_names = {
		{ "qr_WEAPONS_EXTRA_FRAME", qr_const_names.at(qr_WEAPONS_EXTRA_DEATH_FRAME)}
	};
	auto const& rule_templates = GUI::ZCListData::rule_templates_list();
	GUI::ListData qrs = combinedQRList() + combinedZSRList();
	qrs.valsort();
	
	std::ostringstream missing_qrs;
	qrs.filter([&](GUI::ListItem& ref)
		{
			if (!qr_const_names.contains(ref.value))
				missing_qrs << ref.value << " [" << ref.text << "]\n";
			auto info = ref.info;
			auto data = parse_hint_text(info);
			for (auto& val : data.hints[INFOHINT_T_QR])
				if (!qrs.hasKey(val) && !qr_const_names.contains(val))
					missing_qrs << val << " [Unknown; from '" << ref.text << "'s info hint]\n";
			return true;
		});
	
	auto missing_str = missing_qrs.str();
	if (!missing_str.empty())
	{
		printf("error: missing qr constants!\n%s\n", missing_str.c_str());
		zq_exit(1);
	}

	std::ofstream file(fname);
	if (file.fail())
	{
		printf("error outputting file '%s'\n", fname.c_str());
		zq_exit(1);
	}
	
	file << "// Quest rules. Used to toggle legacy behavior and other quest-wide options.\n";
	file << "enum QR\n{\n";
	bool did_skip = false, first = true;
	const string indent = "\t";
	for (int q = 0; q < qr_MAX; ++q)
	{
		if (!qr_const_names.contains(q))
		{
			did_skip = true;
			continue; // unused
		}
		string info, name;
		if (qrs.hasKey(q))
		{
			auto const& ref = qrs.accessItem(q);
			info = ref.info;
			name = fmt::format("'{}'", ref.text);
		}
		else info = "@deprecated";
		
		auto const data = parse_hint_text(info);
		
		vector<string> info_lines = wrap_qr_info(info);
		
		for (auto& [ty, vals] : data.hints)
		{
			if (vals.empty()) continue;
			switch (ty)
			{
				case INFOHINT_T_QR:
					info_lines.emplace_back();
					info_lines.push_back("Related QRs:");
					
					for (auto& val : vals)
						info_lines.push_back(fmt::format("- [{}|{}]", qr_const_names.at(val), qrs.findText(val)));
					break;
				case INFOHINT_T_RULETMPL:
					info_lines.emplace_back();
					info_lines.push_back("Related Rule Templates:");
					for (auto& val : vals)
						info_lines.push_back(fmt::format("- {}", rule_templates.findText(val)));
					break;
			}
		}
		
		if (!name.empty())
		{
			if (info_lines.empty())
				info_lines.push_back(name);
			else
			{
				info_lines.insert(info_lines.begin(), "");
				info_lines.insert(info_lines.begin(), name);
			}
		}
		
		bool multiline_info = info_lines.size() > 1;
		
		if (multiline_info)
		{
			if (!first)
				file << "\n";
			for (string const& line : info_lines)
			{
				if (line.empty())
					file << indent << "//\n";
				else
					file << indent << "// " << line << "\n";
			}
		}
		file << indent << qr_const_names.at(q);
		if (did_skip)
		{
			file << " = ";
			if (!(q % 8))
				file << (q/8) << " * 8";
			else file << q;
		}
		file << ",";
		
		if (multiline_info || info_lines.empty())
			file << "\n";
		else file << " // " << info_lines[0] << "\n";
		
		did_skip = false;
		first = false;
	}
	file << indent << "qr_MAX";
	if (did_skip)
		file << " = " << qr_MAX;
	file << "\n};\n\n";
	
	file << "// Old names for old script compat\n";
	for(auto [old_name, new_name] : qr_compat_names)
		file << "const QR " << old_name << " = " << new_name << ";\n";
	file << "\n";

	file.close();

	zq_exit(0);
}

static void do_compile_command(std::string path, int mode)
{
	is_zq_replay_test = true;
	set_headless_mode();
	FFCore.initIncludePaths();

	int load_ret = load_quest(path.c_str(), false);
	bool success = load_ret == qe_OK;
	if (!success)
	{
		printf("Failed to load quest: %d\n", load_ret);
		zq_exit(1);
	}

	success = do_compile_and_slots(mode, false);
	if (!success)
	{
		printf("Failed to compile\n");
		zq_exit(1);
	}

	success = save_quest(path.c_str()) == 0;
	if (!success)
	{
		printf("Failed to save quest\n");
		zq_exit(1);
	}

	zq_exit(0);
}

static bool partial_load_test(std::string test_dir)
{
	auto classic_path = fs::path(test_dir) / "replays/classic_1st/classic_1st.qst";
	int ret = load_quest(classic_path.string().c_str(), false);
	if (ret)
	{
		printf("failed to load classic_1st.qst: ret == %d\n", ret);
		return false;
	}

	int cont = DMaps[0].cont;

	// Skip same stuff as used in zq_tiles.cpp for grabbing tiles from a qst.
	byte skip_flags[4];
	for (int i=0; i<skip_max; ++i)
		set_bit(skip_flags,i,1);
	set_bit(skip_flags,skip_tiles,0);
	set_bit(skip_flags,skip_header,0);
	zquestheader tempheader{};
	auto ptux_path = fs::path(test_dir) / "quests/PTUX.qst";
	ret = loadquest(ptux_path.string().c_str(), &tempheader, &QMisc, customtunes, false, skip_flags);

	if (ret)
	{
		printf("failed to load PTUX.qst: ret == %d\n", ret);
		return false;
	}

	if (DMaps[0].cont != cont)
	{
		printf("unexpected modification: DMaps[0].cont == %d, should be %d\n", DMaps[0].cont, cont);
		return false;
	}

	// The quest browser reads other quests' headers and icons while a quest
	// is open. Neither may leak the other quest's globals into the editor:
	// a leaked map_count made every layer look invalid.
	word prev_map_count = map_count;
	byte prev_quest_rules[QUESTRULES_NEW_SIZE];
	memcpy(prev_quest_rules, quest_rules, sizeof(prev_quest_rules));
	quest_browser::Entry entry;
	entry.path = ptux_path.string();
	if (!quest_browser::scan_meta(entry))
	{
		printf("failed to scan PTUX.qst metadata\n");
		return false;
	}
	if (!quest_browser::scan_icon(entry))
	{
		printf("failed to scan PTUX.qst icon\n");
		return false;
	}
	if (map_count != prev_map_count)
	{
		printf("unexpected modification: map_count == %d, should be %d\n", map_count, prev_map_count);
		return false;
	}
	if (memcmp(quest_rules, prev_quest_rules, sizeof(prev_quest_rules)) != 0)
	{
		printf("unexpected modification: quest rules changed\n");
		return false;
	}

	// Quest > Defaults reads a section out of the default template while a
	// quest is open; its header must not stick either.
	int32_t prev_format[versiontypesLAST];
	memcpy(prev_format, FFCore.quest_format, sizeof(prev_format));
	if (!init_combos(true, &header))
	{
		printf("failed to reset combos from the template\n");
		return false;
	}
	if (map_count != prev_map_count)
	{
		printf("unexpected modification after default combos: map_count == %d, should be %d\n", map_count, prev_map_count);
		return false;
	}
	if (memcmp(quest_rules, prev_quest_rules, sizeof(prev_quest_rules)) != 0)
	{
		printf("unexpected modification after default combos: quest rules changed\n");
		return false;
	}
	if (memcmp(FFCore.quest_format, prev_format, sizeof(prev_format)) != 0)
	{
		printf("unexpected modification after default combos: quest format changed\n");
		return false;
	}

	// TODO should run replay. Currently, resaving classic_1st.qst fails its replay (see test_save in test_zeditor.py)

	return true;
}

void zeditor_handle_commands()
{
	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	if (test_zc_arg)
	{
		std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

		set_headless_mode();

		bool verbose = zapp_check_switch("-verbose") || zapp_check_switch("-v");
		bool success = true;

		// TODO: convert to TestRunner.
		if (!partial_load_test(test_dir))
		{
			success = false;
			printf("partial_load_test failed\n");
		}

		extern TestResults test_zquest(bool);
		extern void set_zquest_test_dir(std::string const&);
		set_zquest_test_dir(test_dir);
		if (!run_tests(test_zquest, "test_zquest", verbose)) success = false;

		if (success)
			printf("all tests passed\n");
		else
			printf("tests failed\n");
		zq_exit(success ? 0 : 1);
	}

	int dev_qrs_arg = zapp_check_switch("-dev-qrs-zscript", {"filename"});
	if (dev_qrs_arg)
	{
		std::string fname = zapp_get_arg_string(dev_qrs_arg + 1);
		do_dev_qrs_zscript_command(fname);
	}

	int package_arg = zapp_check_switch("-package", {"qst", "package name"});
	if (package_arg)
	{
		std::string input_filename = zapp_get_arg_string(package_arg + 1);
		std::string package_name = zapp_get_arg_string(package_arg + 2);
		if (auto error = package_create(input_filename, package_name))
			Z_error_fatal("%s\n", error->c_str());
		zq_exit(0);
	}

	int copy_qst_arg = zapp_check_switch("-copy-qst", {"input", "output"});
	if (copy_qst_arg)
	{
		std::string input_filename = zapp_get_arg_string(copy_qst_arg + 1);
		std::string output_filename = zapp_get_arg_string(copy_qst_arg + 2);
		do_copy_qst_command(input_filename.c_str(), output_filename.c_str());
	}

	int unencrypt_qst_arg = zapp_check_switch("-unencrypt-qst", {"input", "output"});
	if (unencrypt_qst_arg)
	{
		std::string input_filename = zapp_get_arg_string(unencrypt_qst_arg + 1);
		std::string output_filename = zapp_get_arg_string(unencrypt_qst_arg + 2);
		do_unencrypt_qst_command(input_filename.c_str(), output_filename.c_str());
	}

	int uncompress_qst_arg = zapp_check_switch("-uncompress-qst", {"input", "output"});
	if (uncompress_qst_arg)
	{
		std::string input_filename = zapp_get_arg_string(uncompress_qst_arg + 1);
		std::string output_filename = zapp_get_arg_string(uncompress_qst_arg + 2);
		do_uncompress_qst_command(input_filename.c_str(), output_filename.c_str());
	}

	int quick_assign_arg = zapp_check_switch("-quick-assign", {"qst"});
	if (quick_assign_arg > 0)
	{
		std::string path = zapp_get_arg_string(quick_assign_arg + 1);
		do_compile_command(path, 1);
	}

	int smart_assign_arg = zapp_check_switch("-smart-assign", {"qst"});
	if (smart_assign_arg > 0)
	{
		std::string path = zapp_get_arg_string(smart_assign_arg + 1);
		do_compile_command(path, 2);
	}

	int export_strings_arg = zapp_check_switch("-export-strings", {"input.qst", "output.tsv"});
	if (export_strings_arg > 0)
	{
		is_zq_replay_test = true;
		set_headless_mode();

		std::string input_filename = zapp_get_arg_string(export_strings_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_strings_arg + 2);

		int load_ret = load_quest(input_filename.c_str(), false);
		bool success = load_ret == qe_OK;
		if (!success)
		{
			printf("Failed to load quest: %d\n", load_ret);
			zq_exit(1);
		}

		success = save_strings_tsv(output_filename.c_str());
		if (!success)
		{
			printf("Failed to export strings\n");
			zq_exit(1);
		}

		zq_exit(0);
	}

	int import_strings_arg = zapp_check_switch("-import-strings", {"qst", "input.tsv"});
	if (import_strings_arg > 0)
	{
		is_zq_replay_test = true;
		set_headless_mode();

		std::string qst = zapp_get_arg_string(import_strings_arg + 1);
		std::string input_tsv = zapp_get_arg_string(import_strings_arg + 2);

		int load_ret = load_quest(qst.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		try
		{
			parse_strings_tsv(util::read_text_file(input_tsv));
		}
		catch (std::exception& ex)
		{
			printf("Failed to import strings: %s\n", ex.what());
			zq_exit(1);
		}

		if (save_quest(qst.c_str()) != 0) { printf("Failed to save quest\n"); zq_exit(1); }
		zq_exit(0);
	}

	int export_tiles_arg = zapp_check_switch("-export-tiles", {"input.qst", "output.ztileset", "start_page", "page_count"});
	if (export_tiles_arg > 0)
	{
		set_headless_mode();
		std::string input_filename = zapp_get_arg_string(export_tiles_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_tiles_arg + 2);
		int start_page = zapp_get_arg_int(export_tiles_arg + 3);
		int page_count = zapp_get_arg_int(export_tiles_arg + 4);

		int load_ret = load_quest(input_filename.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(output_filename.c_str(), F_WRITE, "");
		if (!f) { printf("Failed to open output file\n"); zq_exit(1); }
		int ret = writetilefile(f, start_page * TILES_PER_PAGE, page_count * TILES_PER_PAGE);
		pack_fclose(f);
		if (!ret) { printf("Failed to export tiles\n"); zq_exit(1); }
		zq_exit(0);
	}

	int import_tiles_arg = zapp_check_switch("-import-tiles", {"qst", "input.ztileset", "dest_page"});
	if (import_tiles_arg > 0)
	{
		set_headless_mode();
		std::string qst = zapp_get_arg_string(import_tiles_arg + 1);
		std::string input_tileset = zapp_get_arg_string(import_tiles_arg + 2);
		int dest_page = zapp_get_arg_int(import_tiles_arg + 3);

		int load_ret = load_quest(qst.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(input_tileset.c_str(), F_READ, "");
		if (!f) { printf("Failed to open tileset file\n"); zq_exit(1); }
		int ret = readtilefile_to_location(f, dest_page * TILES_PER_PAGE);
		pack_fclose(f);
		if (!ret) { printf("Failed to import tiles\n"); zq_exit(1); }

		if (save_quest(qst.c_str()) != 0) { printf("Failed to save quest\n"); zq_exit(1); }
		zq_exit(0);
	}

	int export_guys_arg = zapp_check_switch("-export-guys", {"input.qst", "output.guy"});
	if (export_guys_arg > 0)
	{
		set_headless_mode();
		std::string input_filename = zapp_get_arg_string(export_guys_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_guys_arg + 2);

		int load_ret = load_quest(input_filename.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		if (!save_guys(output_filename.c_str())) { printf("Failed to export guys\n"); zq_exit(1); }
		zq_exit(0);
	}

	int import_guys_arg = zapp_check_switch("-import-guys", {"qst", "input.guy"});
	if (import_guys_arg > 0)
	{
		set_headless_mode();
		std::string qst = zapp_get_arg_string(import_guys_arg + 1);
		std::string input_guys = zapp_get_arg_string(import_guys_arg + 2);

		int load_ret = load_quest(qst.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		if (!load_guys(input_guys.c_str())) { printf("Failed to import guys\n"); zq_exit(1); }

		if (save_quest(qst.c_str()) != 0) { printf("Failed to save quest\n"); zq_exit(1); }
		zq_exit(0);
	}

	int export_npc_arg = zapp_check_switch("-export-npc", {"input.qst", "output.znpc", "npc_index"});
	if (export_npc_arg > 0)
	{
		set_headless_mode();
		std::string input_filename = zapp_get_arg_string(export_npc_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_npc_arg + 2);
		int npc_index = zapp_get_arg_int(export_npc_arg + 3);

		int load_ret = load_quest(input_filename.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(output_filename.c_str(), F_WRITE, "");
		if (!f) { printf("Failed to open output file\n"); zq_exit(1); }
		int ret = writeonenpc(f, npc_index);
		pack_fclose(f);
		if (!ret) { printf("Failed to export npc\n"); zq_exit(1); }
		zq_exit(0);
	}

	int import_npc_arg = zapp_check_switch("-import-npc", {"qst", "input.znpc", "npc_index"});
	if (import_npc_arg > 0)
	{
		set_headless_mode();
		std::string qst = zapp_get_arg_string(import_npc_arg + 1);
		std::string input_npc = zapp_get_arg_string(import_npc_arg + 2);
		int npc_index = zapp_get_arg_int(import_npc_arg + 3);

		int load_ret = load_quest(qst.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(input_npc.c_str(), F_READ, "");
		if (!f) { printf("Failed to open npc file\n"); zq_exit(1); }
		int ret = readonenpc(f, npc_index);
		pack_fclose(f);
		if (!ret) { printf("Failed to import npc\n"); zq_exit(1); }

		if (save_quest(qst.c_str()) != 0) { printf("Failed to save quest\n"); zq_exit(1); }
		zq_exit(0);
	}

	int export_doorset_arg = zapp_check_switch("-export-doorset", {"input.qst", "output.zdoors", "index"});
	if (export_doorset_arg > 0)
	{
		set_headless_mode();
		std::string input_filename = zapp_get_arg_string(export_doorset_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_doorset_arg + 2);
		int index = zapp_get_arg_int(export_doorset_arg + 3);

		int load_ret = load_quest(input_filename.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(output_filename.c_str(), F_WRITE, "");
		if (!f) { printf("Failed to open output file\n"); zq_exit(1); }
		int ret = writeonezdoorset(f, index);
		pack_fclose(f);
		if (!ret) { printf("Failed to export doorset\n"); zq_exit(1); }
		zq_exit(0);
	}

	int import_doorset_arg = zapp_check_switch("-import-doorset", {"qst", "input.zdoors", "index"});
	if (import_doorset_arg > 0)
	{
		set_headless_mode();
		std::string qst = zapp_get_arg_string(import_doorset_arg + 1);
		std::string input_doorset = zapp_get_arg_string(import_doorset_arg + 2);
		int index = zapp_get_arg_int(import_doorset_arg + 3);

		int load_ret = load_quest(qst.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(input_doorset.c_str(), F_READ, "");
		if (!f) { printf("Failed to open doorset file\n"); zq_exit(1); }
		int ret = readonezdoorset(f, index);
		pack_fclose(f);
		if (!ret) { printf("Failed to import doorset\n"); zq_exit(1); }

		if (save_quest(qst.c_str()) != 0) { printf("Failed to save quest\n"); zq_exit(1); }
		zq_exit(0);
	}

	int export_combo_arg = zapp_check_switch("-export-combo", {"input.qst", "output.zcombo", "start_combo", "count"});
	if (export_combo_arg > 0)
	{
		set_headless_mode();
		std::string input_filename = zapp_get_arg_string(export_combo_arg + 1);
		std::string output_filename = zapp_get_arg_string(export_combo_arg + 2);
		int start_combo = zapp_get_arg_int(export_combo_arg + 3);
		int count = zapp_get_arg_int(export_combo_arg + 4);

		int load_ret = load_quest(input_filename.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(output_filename.c_str(), F_WRITE, "");
		if (!f) { printf("Failed to open output file\n"); zq_exit(1); }
		int ret = writecombofile(f, start_combo, count);
		pack_fclose(f);
		if (!ret) { printf("Failed to export combo\n"); zq_exit(1); }
		zq_exit(0);
	}

	int import_combo_arg = zapp_check_switch("-import-combo", {"qst", "input.zcombo", "start_combo"});
	if (import_combo_arg > 0)
	{
		set_headless_mode();
		std::string qst = zapp_get_arg_string(import_combo_arg + 1);
		std::string input_combo = zapp_get_arg_string(import_combo_arg + 2);
		int start_combo = zapp_get_arg_int(import_combo_arg + 3);

		int load_ret = load_quest(qst.c_str(), false);
		if (load_ret != qe_OK) { printf("Failed to load quest: %d\n", load_ret); zq_exit(1); }

		PACKFILE *f = zalleg_pack_fopen_password(input_combo.c_str(), F_READ, "");
		if (!f) { printf("Failed to open combo file\n"); zq_exit(1); }
		int ret = readcombofile_to_location(f, start_combo, 0, 0);
		pack_fclose(f);
		if (!ret) { printf("Failed to import combo\n"); zq_exit(1); }

		if (save_quest(qst.c_str()) != 0) { printf("Failed to save quest\n"); zq_exit(1); }
		zq_exit(0);
	}
}
