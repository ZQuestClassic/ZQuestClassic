// libFuzzer targets for the file formats zplayer reads. Built only with -DWANT_FUZZ=ON;
// see docs/fuzzing.md for how to build and run them.
//
// The target is picked with the ZC_FUZZ env var (ex: ZC_FUZZ=qst), not a command line
// switch: libFuzzer reads the whole command line as its own, and re-runs argv[0] with it for
// -fork/-jobs/-merge, so the target has to reach those child processes some other way.

#ifdef ZC_FUZZ

#include "zc/fuzz.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"
#include "allegro5/allegro_memfile.h"
#include "base/zapp.h"
#include "components/zasm/serialize.h"
#include "components/zasm/table.h"
#include "components/sound/midi.h"
#include "core/misctypes.h"
#include "core/qst.h"
#include "zc/ffscript.h"
#include "zc/replay.h"
#include "zc/scripting/jit/jit.h"
#include "zc/scripting/zasm_check.h"
#include "zc/zasm_optimize.h"
#include "zc/saves.h"
#include "zc/zelda.h"
#include "zalleg/zsys.h"
#include <gme/gme.h>
#include <atomic>
#include <chrono>
#include <csetjmp>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <pthread.h>
#include <thread>
#include <unistd.h>

namespace fs = std::filesystem;

extern "C" int LLVMFuzzerRunDriver(int* argc, char*** argv, int (*callback)(const uint8_t* data, size_t size));

static int (*fuzz_target)(const uint8_t* data, size_t size);
// When the input being run started (steady clock seconds), or 0 between inputs.
static std::atomic<int64_t> fuzz_input_start;

static int64_t steady_seconds()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static int fuzz_callback(const uint8_t* data, size_t size)
{
	fuzz_input_start = steady_seconds();
	int result = fuzz_target(data, size);
	fuzz_input_start = 0;
	return result;
}

// Runs libFuzzer on this thread. Never returns.
//
// libFuzzer's -timeout works by SIGALRM, which it ignores on any thread but the one it runs on.
// The OS delivers the signal to the process's first thread, but the game (and so libFuzzer) runs
// on one Allegro makes, so without this an input that never ends hangs the fuzzer for good. This
// passes the signal on once an input has run for a while; libFuzzer decides if it is a timeout.
[[noreturn]] static void run_fuzzer(int (*target)(const uint8_t* data, size_t size), int* argc, char*** argv)
{
	fuzz_target = target;
	static pthread_t fuzzing_thread = pthread_self();
	std::thread([]{
		for (;;)
		{
			sleep(1);
			int64_t start = fuzz_input_start;
			if (start && steady_seconds() - start >= 2)
				pthread_kill(fuzzing_thread, SIGALRM);
		}
	}).detach();
	exit(LLVMFuzzerRunDriver(argc, argv, fuzz_callback));
}

// Most of the loaders only take a path. Each fuzzer process gets its own file, so -fork and
// -jobs workers don't clobber each other.
static std::string write_input(const uint8_t* data, size_t size, const char* ext)
{
	static std::string base = (fs::temp_directory_path() / fmt::format("zc_fuzz_{}", getpid())).string();
	std::string path = fmt::format("{}.{}", base, ext);
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	out.write((const char*)data, size);
	return path;
}

static int fuzz_qst(const uint8_t* data, size_t size)
{
	std::string path = write_input(data, size, "qst");
	byte skip_flags[] = {0, 0, 0, 0};
	loadquest(path.c_str(), &QHeader, &QMisc, tunes+ZC_MIDI_COUNT, false, skip_flags, false, false, 0xFF);
	return 0;
}

static int fuzz_sav(const uint8_t* data, size_t size)
{
	std::string path = write_input(data, size, "sav");
	save_t save;
	std::string err;
	_load_from_save_file_expect_one(ReadMode::All, path, save, err);
	return 0;
}

static int fuzz_zplay(const uint8_t* data, size_t size)
{
	std::string path = write_input(data, size, "zplay");
	replay_fuzz_load(path);
	return 0;
}

// The first byte picks the format, the rest is the file.
static int fuzz_music(const uint8_t* data, size_t size)
{
	if (size < 1)
		return -1;

	uint8_t kind = data[0] % 4;
	data++;
	size--;

	switch (kind)
	{
		// Allegro's acodec: the same decoders zcmusic streams .ogg/.mp3 through, and that
		// decode the .ogg sfx embedded in quest files.
		case 0:
		case 1:
		{
			ALLEGRO_FILE* f = al_open_memfile((void*)data, size, "rb");
			ALLEGRO_SAMPLE* sample = al_load_sample_f(f, kind == 0 ? ".ogg" : ".mp3");
			if (sample)
				al_destroy_sample(sample);
			al_fclose(f);
			break;
		}

		// GME: .spc .gbs .vgm .gym .nsf. It identifies the format from the header.
		case 2:
		{
			Music_Emu* emu = nullptr;
			if (!gme_open_data(data, size, &emu, 44100) && emu)
			{
				int tracks = std::min(gme_track_count(emu), 4);
				for (int i = 0; i < tracks; i++)
				{
					if (gme_start_track(emu, i))
						continue;
					short buf[2048];
					for (int j = 0; j < 8; j++)
						gme_play(emu, std::size(buf), buf);
				}
			}
			gme_delete(emu);
			break;
		}

		// Allegro legacy MIDI loading (what readtunes and the music folder use), then
		// get_midi_info, which walks every track to work out the song length.
		case 3:
		{
			std::string path = write_input(data, size, "mid");
			if (MIDI* midi = load_midi(path.c_str()))
			{
				midi_info mi;
				get_midi_info(midi, &mi);
				destroy_midi(midi);
			}
			break;
		}
	}

	return 0;
}

// ZC_FUZZ=unpack zplayer <in> <out>: writes a .qst or .sav file's payload with the compression
// and encoding layers taken off. Mutations on compressed bytes are mostly wasted, so the corpus
// is stored this way (the loaders accept both).
static void unpack(const char* in, const char* out)
{
	const char* save_header = "ZQuest Classic Save File";
	const char* old_save_header = "Zelda Classic Save File";
	PACKFILE* pf = nullptr;
	if (fs::path(in).extension() == ".sav")
	{
		// The same steps as load_from_save_file.
		pf = try_open_maybe_legacy_encoded_file(in, save_header, old_save_header, "SVGM", nullptr).decoded_pf;
		std::string tmp = write_input(nullptr, 0, "tmp");
		if (!pf && (!decode_file_007(in, tmp.c_str(), save_header, ENC_METHOD_MAX-1, false, "") ||
			!decode_file_007(in, tmp.c_str(), old_save_header, ENC_METHOD_MAX-1, false, "")))
			pf = pack_fopen(tmp.c_str(), F_READ_PACKED);
	}
	else
	{
		int32_t err = 0;
		pf = open_quest_file(&err, in, false);
	}
	if (!pf)
	{
		fprintf(stderr, "Could not open %s\n", in);
		exit(1);
	}

	std::ofstream f(out, std::ios::binary | std::ios::trunc);
	char buf[4096];
	long n;
	while ((n = pack_fread(buf, sizeof(buf), pf)) > 0)
		f.write(buf, n);
	pack_fclose(pf);
	exit(0);
}

// ZASM: each input decodes to a script, which runs in a live game (playground.qst) through the
// interpreter, then through the optimizer and the JIT. Quests carry raw ZASM that is not
// validated beyond its jump targets, so any command with any arguments can reach both.

namespace {

// Reads the input front to back; zeroes once it runs out.
struct ByteReader
{
	const uint8_t* data;
	size_t size;

	bool empty() const { return size == 0; }

	uint8_t u8()
	{
		if (!size)
			return 0;
		size--;
		return *data++;
	}

	uint32_t u32()
	{
		uint32_t v = 0;
		for (int i = 0; i < 4; i++)
			v |= u8() << (8 * i);
		return v;
	}
};

constexpr int fuzz_global_slot = GLOBAL_SCRIPT_ONSAVE;
// The JIT finds compiled scripts by id, so this must not be one the quest uses.
constexpr zasm_script_id fuzz_zasm_script_id = 0xFFFF;
constexpr size_t max_fuzz_instructions = 256;
// Jumps only go forward, so every script finishes - except that a call returns behind itself,
// so each call can double how much runs. Capping them keeps that small.
constexpr int max_fuzz_calls = 6;

std::vector<int> fuzz_commands;
std::vector<int> fuzz_registers;
script_data* fuzz_script_data;
jmp_buf fuzz_invalid_zasm_jmp;
bool fuzz_invalid_zasm;

bool is_fuzzable_command(const script_command& sc)
{
	if (sc.flags & ARGFL_UNIMPL)
		return false;

	// Their jump targets are in a literal or a register, so they could jump backward and loop
	// forever. (RETURN pops its target off the stack.)
	if (sc.command == GOTOR || sc.command == GOTOTABLE || sc.command == GOTORANGES || sc.command == RETURN)
		return false;

	// Only for tests (a quest that uses it outside of test mode stops with an error). Its debug
	// asserts check the engine's refcounts, which scripts here are free to mark however they like.
	if (sc.command == GC)
		return false;

	// Touch the file system or network, end the game, or run whole frames (fades, menus, a
	// nested frozen generic script) - none of which a single script run should do here.
	std::string_view name = sc.name;
	for (auto bad : {"FILE", "DIRECTORY", "DIREXISTS", "WEBSOCKET", "SAVE", "READBITMAP",
		"WRITEBITMAP", "GAMEEND", "GAMEEXIT", "GAMERELOAD", "GAMECONTINUE", "SHOWF6SCREEN",
		"RUNGENFRZSCR", "BREAKPOINT", "SUBDATA_OPEN", "WAVYIN", "WAVYOUT", "ZAPIN", "ZAPOUT",
		"OPENWIPE", "CLOSEWIPE", "FXWAVY", "FXZAP"})
	{
		if (name.find(bad) != std::string_view::npos)
			return false;
	}
	return true;
}

void init_fuzz_zasm()
{
	for (int i = 0; i < NUMCOMMANDS; i++)
	{
		if (auto sc = get_script_command(i); sc && is_fuzzable_command(*sc))
			fuzz_commands.push_back(i);
	}
	for (int i = 0; i < NUMVARIABLES; i++)
	{
		// Not PC: writing it is a jump backward, so a loop that never ends.
		if (get_script_variable(i) && i != PC)
			fuzz_registers.push_back(i);
	}

	// Borrow the metadata (compiler version etc.) of a real script, so the engine treats these
	// like scripts from a modern quest.
	fuzz_script_data = new script_data(ScriptType::Global, fuzz_global_slot);
	for (auto sd : globalscripts)
	{
		if (sd && sd->valid())
		{
			fuzz_script_data->meta = sd->meta;
			break;
		}
	}
}

int32_t fuzz_register(ByteReader& in)
{
	uint8_t b = in.u8();
	if (b < 160)
		return D(b % 8);
	if (b < 250)
		return fuzz_registers[in.u32() % fuzz_registers.size()];
	return (int32_t)in.u32();
}

int32_t fuzz_value(ByteReader& in)
{
	uint8_t b = in.u8();
	if (b < 128)
		return (b % 32 - 8) * 10000;
	if (b < 192)
		return b % 64;
	return (int32_t)in.u32();
}

std::shared_ptr<zasm_script> build_fuzz_script(const uint8_t* data, size_t size)
{
	ByteReader in{data, size};
	std::vector<ffscript> zasm;
	zasm_literals literals;
	int calls = 0;

	// Decoded first, so jump targets can be fixed up once the length is known.
	struct Jump { size_t pc; uint32_t r; };
	std::vector<Jump> jumps;

	while (!in.empty() && zasm.size() < max_fuzz_instructions)
	{
		uint16_t sel = in.u8() | in.u8() << 8;
		int command;
		// Mostly real commands, but anything a quest file can hold is fair game.
		if (sel < 0xFF00)
			command = fuzz_commands[sel % fuzz_commands.size()];
		else
			command = (sel & 0xFF) + NUMCOMMANDS;

		ffscript op(command);
		auto sc = get_script_command(command);
		if (sc)
		{
			if (command == CALLFUNC && ++calls > max_fuzz_calls)
				continue;

			int32_t* args[] = {&op.arg1, &op.arg2, &op.arg3};
			for (int i = 0; i < sc->args; i++)
			{
				if (i == 0 && ((sc->flags & ARGFL_GOTO) || command == CALLFUNC))
					jumps.push_back({zasm.size(), in.u32()});
				else if (sc->is_register(i))
					*args[i] = fuzz_register(in);
				else if (sc->arg_type[i] == ARGTY::COMPARE_OP)
					*args[i] = in.u8() % 8;
				else
					*args[i] = fuzz_value(in);
			}

			// LOOP's target isn't marked as a jump, so it isn't made to go forward, and it doesn't
			// count its register down: it could loop forever. (Dropped here and not from
			// fuzz_commands, so the corpus still decodes to the same commands.)
			if (command == LOOP)
				op = ffscript(NOP);

			if (sc->arr_type)
			{
				zasm_literal literal;
				uint8_t len = in.u8() % 24;
				if (sc->arr_type & 1)
				{
					auto& str = literal.str.emplace();
					for (int i = 0; i < len; i++)
						str.push_back((char)in.u8());
				}
				else
				{
					auto& vec = literal.vec.emplace();
					for (int i = 0; i < len; i++)
						vec.push_back(fuzz_value(in));
				}
				literals.set(op, std::move(literal));
			}
		}
		zasm.push_back(op);
	}

	if (zasm.empty())
		return nullptr;

	// Forward only: somewhere after the jump, up to and including the terminator.
	for (auto [pc, r] : jumps)
		zasm[pc].arg1 = pc + 1 + r % (zasm.size() - pc);
	zasm.emplace_back(0xFFFF);

	if (zasm_find_invalid_jump(zasm, literals))
		return nullptr;

	return std::make_shared<zasm_script>(fuzz_zasm_script_id, "fuzz", std::move(zasm), std::move(literals));
}

// Returns false if the script did something only ZASM the compiler never emits can (see ZASM_CHECK).
bool run_fuzz_script(std::shared_ptr<zasm_script> zs, bool optimize_and_jit)
{
	fuzz_script_data->zasm_script = zs;
	fuzz_script_data->pc = 0;
	fuzz_script_data->end_pc = zs->size;
	zs->script_datas = {fuzz_script_data};

	JittedScript* j_script = nullptr;
	fuzz_invalid_zasm = false;
	FFCore.reset_script_engine_data(ScriptType::Global, fuzz_global_slot);
	auto& data = get_script_engine_data(ScriptType::Global, fuzz_global_slot);
	if (optimize_and_jit)
	{
		if (zasm_optimize_is_enabled())
			zasm_optimize_script(zs.get());
		j_script = jit_compile_script(zs.get());
		if (j_script)
			data.j_instance = std::shared_ptr<JittedScriptInstance>(jit_create_script_impl(fuzz_script_data, &data.ref, j_script));
	}

	script_data* prev = globalscripts[fuzz_global_slot];
	globalscripts[fuzz_global_slot] = fuzz_script_data;
	// A jump rather than an exception, which could not pass through jitted code (it has no unwind
	// info). It skips destructors on the way, which only a fuzzing build can put up with.
	bool valid = true;
	if (fuzz_invalid_zasm)
		valid = false;
	else if (setjmp(fuzz_invalid_zasm_jmp) == 0)
		run_script(ScriptType::Global, fuzz_global_slot, fuzz_global_slot);
	else
		valid = false;
	// Set while running too, where ending the run on the spot isn't safe (ex: in the middle of a GC).
	if (fuzz_invalid_zasm)
		valid = false;
	globalscripts[fuzz_global_slot] = prev;

	// Releasing what the script held is where references it forged show up.
	FFCore.reset_script_engine_data(ScriptType::Global, fuzz_global_slot);
	if (fuzz_invalid_zasm)
		valid = false;
	if (j_script)
		jit_release(j_script);
	fuzz_script_data->zasm_script = nullptr;
	return valid;
}

} // namespace

void fuzz_reject_invalid_zasm()
{
	longjmp(fuzz_invalid_zasm_jmp, 1);
}

void fuzz_mark_invalid_zasm()
{
	fuzz_invalid_zasm = true;
}

static int fuzz_zasm(const uint8_t* data, size_t size)
{
	auto zs = build_fuzz_script(data, size);
	if (!zs)
		return -1;

	// ZC_FUZZ_PRINT=1 shows what an input decodes to (ex: when reproducing a crash).
	static bool print = getenv("ZC_FUZZ_PRINT");
	if (print)
	{
		for (pc_t pc = 0; pc < zs->size; pc++)
			fprintf(stderr, "%u: %s\n", pc, zasm_op_to_string(zs->zasm[pc], zs->literals).c_str());
	}

	static bool jit_enabled = jit_is_enabled();
	jit_set_enabled(false);
	bool valid = run_fuzz_script(zs, false);
	jit_set_enabled(jit_enabled);
	// A second script, since the optimizer rewrites it in place.
	if (valid && jit_enabled)
		valid = run_fuzz_script(build_fuzz_script(data, size), true);
	// Inputs that aren't valid ZASM stay out of the corpus.
	return valid ? 0 : -1;
}

// Starts a game of ZC_FUZZ_ZASM_QST (playground.qst), then runs libFuzzer from inside the
// first frame of it. Never returns.
static void start_zasm_fuzzer()
{
	const char* qst = getenv("ZC_FUZZ_ZASM_QST");
	if (!qst || !fs::exists(qst))
	{
		fprintf(stderr, "ZC_FUZZ_ZASM_QST must be the path to a quest (tests/replays/playground/playground.qst)\n");
		exit(1);
	}

	// A replay with nothing in it but a far-off end is the simplest way to start a game headless.
	std::string replay = write_input(nullptr, 0, "zplay");
	{
		std::ofstream out(replay);
		out << "M qst " << fs::absolute(qst).string() << "\n";
		out << "M starting_dmap 0\nM starting_scr 0\nM test_mode true\nM version latest\n";
		out << "M frames 100000000\n";
		out << "K 0 84 85 82 83 26 24 67 17 23 75 1 19 4 3\n";
		out << "Q 100000000 3\n";
	}

	// libFuzzer gets the real command line; the game sees one that compiles scripts with the JIT,
	// synchronously.
	static int fuzzer_argc = zapp_get_argc();
	static char** fuzzer_argv = zapp_get_argv();
	static const char* game_args[] = {fuzzer_argv[0], "-jit", "-jit-precompile", "-jit-threads", "0"};
	zapp_replace_args(std::size(game_args), const_cast<char**>(game_args));

	extern std::function<void()> test_update;
	test_update = [](){
		test_update = nullptr;
		init_fuzz_zasm();
		run_fuzzer(fuzz_zasm, &fuzzer_argc, &fuzzer_argv);
	};

	load_replay_file_deferred(ReplayMode::Replay, replay);
	init_and_run_main_zplayer_loop();
	fprintf(stderr, "The game ended before the fuzzer started\n");
	exit(1);
}

bool zplayer_fuzz_requested()
{
	return getenv("ZC_FUZZ") != nullptr;
}

void zplayer_fuzz_main()
{
	std::string target = getenv("ZC_FUZZ");
	int (*callback)(const uint8_t* data, size_t size) = nullptr;
	if (target == "unpack")
	{
		if (zapp_get_argc() < 3)
		{
			fprintf(stderr, "Usage: ZC_FUZZ=unpack zplayer <in> <out>\n");
			exit(1);
		}
		unpack(zapp_get_argv()[1], zapp_get_argv()[2]);
	}
	else if (target == "qst")
		callback = fuzz_qst;
	else if (target == "sav")
		callback = fuzz_sav;
	else if (target == "zplay")
		callback = fuzz_zplay;
	else if (target == "music")
	{
		// Headless skips installing audio, but samples need it.
		if (!al_install_audio() || !al_init_acodec_addon())
		{
			fprintf(stderr, "Could not install audio\n");
			exit(1);
		}
		callback = fuzz_music;
	}
	else if (target == "zasm")
		start_zasm_fuzzer();
	else
	{
		fprintf(stderr, "Unknown ZC_FUZZ target: %s\n", target.c_str());
		fprintf(stderr, "Expected one of: qst, sav, zplay, music, zasm\n");
		exit(1);
	}

	int argc = zapp_get_argc();
	char** argv = zapp_get_argv();
	run_fuzzer(callback, &argc, &argv);
}

#endif
