#include "zc/replay.h"
#include "base/qrs.h"
#include "base/version.h"
#include "base/zapp.h"
#include "zc/zc_sys.h"
#include "base/zc_alleg.h"
#include "base/util.h"
#include "zc/zelda.h"
#include <array>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <chrono>
#include <fmt/format.h>
#include <fmt/ranges.h>

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include <xxhash.h>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

using namespace std::chrono_literals;

struct ReplayStep;

static const int ASSERT_SNAPSHOT_BUFFER = 10;
static const int ASSERT_FAILED_EXIT_CODE = 120;
static const int VERSION = 42;

static const std::string ANNOTATION_MARKER = "«";
static const char TypeMeta = 'M';
static const char TypeKeyDown = 'D';
static const char TypeKeyUp = 'U';
static const char TypeComment = 'C';
static const char TypeQuit = 'Q';
static const char TypeCheat = 'X';
static const char TypeQR = 'Y';
static const char TypeRng = 'R';
static const char TypeKeyMap = 'K';
static const char TypeMouse = 'V';
static const char TypeState = 'S';

static ReplayMode mode = ReplayMode::Off;
static int version;
static bool version_use_latest;
static bool debug;
static bool snapshot_all_frames;
static bool exit_when_done;
static bool sync_rng;
static int frame_arg;
static std::filesystem::path replay_path;
static std::filesystem::path output_dir;
static std::vector<std::shared_ptr<ReplayStep>> replay_log;
static std::vector<std::shared_ptr<ReplayStep>> record_log;
static std::map<std::string, std::string> meta_map;
static std::vector<int> snapshot_frames;
static std::vector<int> expected_loadscr_frame_count;
static int loadscr_count;
static int failed_loadscr_count_frame;
static size_t replay_log_current_index;
static size_t replay_log_current_quit_index;
static size_t replay_log_current_state_index;
static size_t assert_current_index;
static size_t manual_takeover_start_index;
static bool has_assert_failed;
static bool has_aborted;
static int failing_frame;
static int last_failing_gfx_frame;
static int current_failing_gfx_segment_start_frame;
static std::vector<int> unexpected_gfx_frames;
static std::vector<std::pair<int, int>> unexpected_gfx_segments;
static std::vector<std::pair<int, int>> unexpected_gfx_segments_limited;
static bool has_rng_desynced;
static bool did_attempt_input_during_replay;
static int frame_count;
static bool previous_control_state[ZC_CONTROL_STATES];
static char previous_keys[KEY_MAX];
static std::vector<zc_randgen *> rngs;
static std::map<int, int> rng_seed_count_this_frame;
static uint32_t prev_gfx_hash;
static bool prev_gfx_hash_was_same;
static int prev_debug_x;
static int prev_debug_y;
static bool gfx_got_mismatch;
static std::array<int, 4> prev_mouse_state;
static std::array<int, 4> current_mouse_state;
static std::chrono::time_point<std::chrono::steady_clock> time_started;
static std::chrono::time_point<std::chrono::system_clock> time_started_system;
static std::chrono::time_point<std::chrono::steady_clock> time_result_saved;

struct FramebufHistoryEntry
{
	BITMAP* bitmap;
	PALETTE pal;
	int frame;
};
static std::array<FramebufHistoryEntry, ASSERT_SNAPSHOT_BUFFER> framebuf_history;
static int framebuf_history_index;

struct ReplayStep
{
    int frame;
    char type;

    ReplayStep(int frame, char type) : frame(frame), type(type)
    {
    }
    virtual void run() = 0;
    virtual std::string print() = 0;
};

struct KeyMapReplayStep : ReplayStep
{
	static const int NumButtons = 14;
	static KeyMapReplayStep current;
	static KeyMapReplayStep stored;

	static KeyMapReplayStep make(int frame)
	{
		KeyMapReplayStep step(frame, {
			DUkey,
			DDkey,
			DLkey,
			DRkey,
			Akey,
			Bkey,
			Skey,
			Lkey,
			Rkey,
			Pkey,
			Exkey1,
			Exkey2,
			Exkey3,
			Exkey4,
		});
		return step;
	}

	std::array<int, KeyMapReplayStep::NumButtons> button_keys;

	KeyMapReplayStep(int frame, std::array<int, KeyMapReplayStep::NumButtons> button_keys) : ReplayStep(frame, TypeKeyMap), button_keys(button_keys)
	{
	}

	int find_button_index_for_key(int key)
	{
		for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
		{
			if (button_keys[i] == key)
				return i;
		}
		return -1;
	}

	void run()
	{
		DUkey = button_keys[0];
		DDkey = button_keys[1];
		DLkey = button_keys[2];
		DRkey = button_keys[3];
		Akey = button_keys[4];
		Bkey = button_keys[5];
		Skey = button_keys[6];
		Lkey = button_keys[7];
		Rkey = button_keys[8];
		Pkey = button_keys[9];
		Exkey1 = button_keys[10];
		Exkey2 = button_keys[11];
		Exkey3 = button_keys[12];
		Exkey4 = button_keys[13];
	}

	std::string print()
	{
		return fmt::format("{} {} {}", type, frame, fmt::join(button_keys, " "));
	}
};
KeyMapReplayStep KeyMapReplayStep::current(0, {});
KeyMapReplayStep KeyMapReplayStep::stored(0, {});

struct KeyReplayStep : public ReplayStep
{
    inline static const char *button_names[] = {
        "Up",
        "Down",
        "Left",
        "Right",
        "A",
        "B",
        "Start",
        "L",
        "R",
        "Map",
        "Ex1",
        "Ex2",
        "Ex3",
        "Ex4",
        "UpA",
        "DownA",
        "LeftA",
        "RightA",
    };

    static int find_index_for_button_name(std::string button_name)
    {
        for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
        {
            if (button_names[i] == button_name)
                return i;
        }
        return -1;
    }

    static int find_index_for_key_name(std::string key_name)
    {
        for (int i = 1; i < KEY_MAX; i++)
        {
            if (_keyboard_common_names[i] == key_name)
                return i;
        }
        return -1;
    }

    int button_index;
	int key_index;

    KeyReplayStep(int frame, int type, int button_index, int key_index) : ReplayStep(frame, type), button_index(button_index), key_index(key_index)
    {
    }

    void run()
    {
        if (button_index != -1)
            raw_control_state[button_index] = type == TypeKeyDown;

        // Set the key/joystick states, in case zscript queries the state of these things.
        if (button_index != -1 && button_index >= 14)
        {
            // TODO zscript allows polling the joystick state directly,
            // so should probably do that. I don't have a gamepad at the moment
            // so can't verify. For now, do nothing. This seems like it would be
            // rare enough to matter that it's ok to skip for now.
        }
        else
        {
            key_current_frame[key_index] = type == TypeKeyDown ? 1 : 0;
        }
    }

	// If associated with a button in the most recent key map step, print as just the button name.
	// Otherwise, print as the key named prefixed with "k ".
    std::string print()
    {
        if (button_index != -1)
            return fmt::format("{} {} {}", type, frame, button_names[button_index]);
        else
            return fmt::format("{} {} k {}", type, frame, _keyboard_common_names[key_index]);
    }
};

struct CommentReplayStep : ReplayStep
{
    std::string comment;

    CommentReplayStep(int frame, std::string comment) : ReplayStep(frame, TypeComment), comment(comment)
    {
    }

    void run()
    {
    }

    std::string print()
    {
        return fmt::format("{} {} {}", type, frame, comment);
    }
};

struct QuitReplayStep : ReplayStep
{
    // 0 is GameFlags GAMEFLAG_TRYQUIT, 1+ refer to qQuit enum.
    int quit_state;

    QuitReplayStep(int frame, int quit_state) : ReplayStep(frame, TypeQuit), quit_state(quit_state)
    {
    }

    void run()
    {
        // During replay, calls to replay_peek_quit handle settig the Quit state.
    }

    std::string print()
    {
        return fmt::format("{} {} {}", type, frame, quit_state);
    }
};

struct CheatReplayStep : ReplayStep
{
    Cheat cheat;
    int arg1, arg2;
    std::string arg3;

    CheatReplayStep(int frame, Cheat cheat, int arg1, int arg2, std::string arg3) : ReplayStep(frame, TypeCheat), cheat(cheat), arg1(arg1), arg2(arg2), arg3(arg3)
    {
    }

    void run()
    {
        // During replay, replay_do_cheats handles enqueuing cheats.
    }

    std::string print()
    {
        std::string cheat_name = cheat_to_string(cheat);
        if (cheat == Cheat::HeroData)
            return fmt::format("{} {} {} {}", type, frame, cheat_name, arg3);
        else if (arg1 == -1)
            return fmt::format("{} {} {}", type, frame, cheat_name);
        else if (arg2 == -1)
            return fmt::format("{} {} {} {}", type, frame, cheat_name, arg1);
        else
            return fmt::format("{} {} {} {} {}", type, frame, cheat_name, arg1, arg2);
    }
};

struct QRReplayStep : ReplayStep
{
    int qr;
    bool value;

    QRReplayStep(int frame, int qr, bool value) : ReplayStep(frame, TypeQR), qr(qr), value(value)
    {
    }

    void run()
    {
        // During replay, replay_do_qrs handles enqueuing qr changes.
    }

    std::string print()
    {
		return fmt::format("{} {} {} {}", type, frame, qr, value ? 1 : 0);
    }
};

struct RngReplayStep : ReplayStep
{
    int start_index;
    int end_index;
    int seed;

    RngReplayStep(int frame, int start_index, int end_index, int seed) : ReplayStep(frame, TypeRng), start_index(start_index), end_index(end_index), seed(seed)
    {
    }

    void run()
    {
        // During replay, calls to replay_set_rng_seed handle seeding the rng based on RngReplayStep.
    }

    std::string print()
    {
        return fmt::format("{} {} {} {} {}", type, frame, start_index, end_index, seed);
    }
};

struct MouseReplayStep : ReplayStep
{
	std::array<int, 4> state;

	MouseReplayStep(int frame, std::array<int, 4> state) : ReplayStep(frame, TypeMouse), state(state)
	{
	}

	void run()
	{
		current_mouse_state = state;
	}

	std::string print()
	{
		int x = state[0];
		int y = state[1];
		int z = state[2];
		int b = state[3];
		if (z == 0 && b == 0)
			return fmt::format("{} {} {} {}", type, frame, x, y);
		else if (b == 0)
			return fmt::format("{} {} {} {} {}", type, frame, x, y, z);
		else
			return fmt::format("{} {} {} {} {} {}", type, frame, x, y, z, b);
	}
};

struct StateReplayStep : ReplayStep
{
	ReplayStateType state_type;
    int value;

	StateReplayStep(int frame, ReplayStateType state_type, int value) : ReplayStep(frame, TypeState), state_type(state_type), value(value)
	{
	}

	void run()
	{
		// current_mouse_state = state;
	}

	std::string print()
	{
		return fmt::format("{} {} {} {}", type, frame, (int)state_type, value);
	}
};

static int get_rng_index(zc_randgen *rng)
{
    auto it = std::find(rngs.begin(), rngs.end(), rng);
    if (it == rngs.end())
        return -1;
    return it - rngs.begin();
}

// Find the RNG step associated with the given |rng_index|, starting at |starting_step_index|.
// Only RNG steps for the current |frame_count| are valid.
// If an offset is given, the nth RNG step will be returned (offset = 1 means the first).
static RngReplayStep *find_rng_step(int rng_index, size_t starting_step_index, const std::vector<std::shared_ptr<ReplayStep>> &log, int offset = 1)
{
    ASSERT(offset >= 1);

    int num_seen = 0;
    for (size_t i = starting_step_index; i < log.size(); i++)
    {
        auto step = log[i];
        if (step->frame != frame_count)
            break;
        if (step->type != TypeRng)
            continue;

        auto rng_step = static_cast<RngReplayStep *>(step.get());
        if (rng_index >= rng_step->start_index && rng_index <= rng_step->end_index)
            num_seen += 1;

        if (num_seen == offset)
            return rng_step;
    }

    return nullptr;
}

static bool steps_are_equal(const ReplayStep* step1, const ReplayStep* step2)
{
	bool are_equal = false;

	if (step1 == nullptr && step2 == nullptr)
	{
		are_equal = true;
	}
	else if (step1 == nullptr || step2 == nullptr || step1->frame != step2->frame || step1->type != step2->type)
	{
		are_equal = false;
	}
	else
		switch (step2->type)
		{
		case TypeComment:
		{
			auto comment_replay_step = static_cast<const CommentReplayStep *>(step1);
			auto comment_record_step = static_cast<const CommentReplayStep *>(step2);
			are_equal = comment_replay_step->comment == comment_record_step->comment;
		}
		break;
		case TypeKeyUp:
		case TypeKeyDown:
		{
			auto key_replay_step = static_cast<const KeyReplayStep *>(step1);
			auto key_record_step = static_cast<const KeyReplayStep *>(step2);
			are_equal =
				key_replay_step->button_index == key_record_step->button_index &&
				key_replay_step->key_index == key_record_step->key_index;
		}
		break;
		case TypeQuit:
		{
			auto quit_replay_step = static_cast<const QuitReplayStep *>(step1);
			auto quit_record_step = static_cast<const QuitReplayStep *>(step2);
			are_equal = quit_replay_step->quit_state == quit_record_step->quit_state;
		}
		break;
		case TypeCheat:
		{
			auto cheat_replay_step = static_cast<const CheatReplayStep *>(step1);
			auto cheat_record_step = static_cast<const CheatReplayStep *>(step2);
			are_equal = cheat_replay_step->cheat == cheat_record_step->cheat &&
				cheat_replay_step->arg1 == cheat_record_step->arg1 &&
				cheat_replay_step->arg2 == cheat_record_step->arg2 &&
				cheat_replay_step->arg3 == cheat_record_step->arg3;
		}
		break;
		case TypeQR:
		{
			auto qr_replay_step = static_cast<const QRReplayStep *>(step1);
			auto qr_record_step = static_cast<const QRReplayStep *>(step2);
			are_equal = qr_replay_step->qr == qr_record_step->qr &&
				qr_replay_step->value == qr_record_step->value;
		}
		break;
		case TypeRng:
		{
			auto rng_replay_step = static_cast<const RngReplayStep *>(step1);
			auto rng_record_step = static_cast<const RngReplayStep *>(step2);
			are_equal =
				rng_replay_step->seed == rng_record_step->seed &&
				rng_replay_step->start_index == rng_record_step->start_index &&
				rng_replay_step->end_index == rng_record_step->end_index;
		}
		break;
		case TypeKeyMap:
		{
			auto map_replay_step = static_cast<const KeyMapReplayStep *>(step1);
			auto map_record_step = static_cast<const KeyMapReplayStep *>(step2);
			are_equal = map_replay_step->button_keys == map_record_step->button_keys;
		}
		break;
		case TypeMouse:
		{
			auto mouse_replay_step = static_cast<const MouseReplayStep *>(step1);
			auto mouse_record_step = static_cast<const MouseReplayStep *>(step2);
			are_equal = mouse_replay_step->state == mouse_record_step->state;
		}
		break;
		case TypeState:
		{
			auto state_replay_step = static_cast<const StateReplayStep *>(step1);
			auto state_record_step = static_cast<const StateReplayStep *>(step2);
			are_equal =
				state_replay_step->state_type == state_record_step->state_type &&
				state_replay_step->value == state_record_step->value;
		}
		break;
		}

	return are_equal;
}

static std::filesystem::path get_file_path(std::string suffix)
{
	return output_dir / (replay_path.filename().string() + suffix);
}

// This is for detecting keyboard input during replaying,
// and prompting the user if they wish to end the replay.
static int keyboard_intercept(int c)
{
	int k = c >> 8;
	if (!is_system_key(k))
		did_attempt_input_during_replay = true;

	// Don't modify the key state at all; this is OK because the
	// game doesn't read directly from `key` anymore–it now reads from
	// `key_current_frame`. Only the system keys use `key` directly.
	return c;
}

static void install_keyboard_handlers()
{
	keyboard_callback = keyboard_intercept;
}

static void uninstall_keyboard_handlers()
{
	keyboard_callback = nullptr;
}

static uint32_t hash_bitmap(BITMAP* bitmap)
{
	static BITMAP* true_bitmap;
	if (!true_bitmap)
		true_bitmap = create_bitmap_ex(24, bitmap->w, bitmap->h);

	blit(bitmap, true_bitmap, 0, 0, 0, 0, bitmap->w, bitmap->h);
	int depth = bitmap_color_depth(true_bitmap);
	size_t len = true_bitmap->w * true_bitmap->h * BYTES_PER_PIXEL(depth);
	return XXH32(true_bitmap->dat, len, 0);
}

static void do_recording_poll()
{
	gfx_got_mismatch = false;

	if (debug && !Quit)
	{
		if (!screenscrolling)
		{
			int x = HeroX().getInt();
			int y = HeroY().getInt();
			if (x != prev_debug_x || y != prev_debug_y)
			{
				replay_step_comment(fmt::format("h {:x} {:x}", x, y));
				prev_debug_x = x;
				prev_debug_y = y;
			}
		}

		uint32_t hash = hash_bitmap(framebuf);
		replay_step_gfx(hash);
	}

	if (version >= 5)
	{
		KeyMapReplayStep new_key_map = KeyMapReplayStep::make(frame_count);
		if (new_key_map.button_keys != KeyMapReplayStep::current.button_keys)
		{
			KeyMapReplayStep::current = new_key_map;
			record_log.push_back(std::make_shared<KeyMapReplayStep>(frame_count, new_key_map.button_keys));
		}
	}

	for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
	{
		bool state = raw_control_state[i];
		if (state == previous_control_state[i])
			continue;

		int key_index = KeyMapReplayStep::current.button_keys[i];
		record_log.push_back(std::make_shared<KeyReplayStep>(frame_count, state ? TypeKeyDown : TypeKeyUp, i, key_index));
		previous_control_state[i] = state;
	}

	if (version >= 5)
	{
		for (int i = 1; i < KEY_MAX; i++)
		{
			char state = key_current_frame[i];
			if (state == previous_keys[i] || i == KEY_ESC)
				continue;
			if (KeyMapReplayStep::current.find_button_index_for_key(i) != -1)
			    continue;

			record_log.push_back(std::make_shared<KeyReplayStep>(frame_count, state ? TypeKeyDown : TypeKeyUp, -1, i));
			previous_keys[i] = state;
		}
	}

	if (current_mouse_state != prev_mouse_state)
	{
		record_log.push_back(std::make_shared<MouseReplayStep>(frame_count, current_mouse_state));
		prev_mouse_state = current_mouse_state;
	}
}

static void set_version()
{
	version_use_latest = false;
	if (!meta_map.contains("version"))
	{
		version = 1;
		return;
	}

	std::string version_str = meta_map.at("version");
	if (version_str == "latest")
	{
		version = VERSION;
		version_use_latest = true;
		return;
	}

	version = std::stoi(version_str);
}

static void load_replay(std::map<std::string, std::string>& meta_map, std::filesystem::path path, bool only_meta = false)
{
	meta_map.clear();

#ifdef __EMSCRIPTEN__
    if (em_is_lazy_file(path))
    {
        em_fetch_file(path);
    }
#endif

    std::ifstream file(path);

    if (!file.is_open())
    {
        fprintf(stderr, "could not open file: %s\n", path.string().c_str());
        abort();
    }

    KeyMapReplayStep key_map = KeyMapReplayStep::make(0);
    bool found_key_map = false;

    bool done_with_meta = false;
    std::string line;
    while (util::portable_get_line(file, line))
    {
        if (line.empty())
            continue;

        // Annotations are always discarded.
        std::string annotation;
        auto annotation_pos = line.find(ANNOTATION_MARKER, 0);
        if (annotation_pos != std::string::npos)
        {
            annotation = line.substr(annotation_pos + 1);
            line = line.substr(0, annotation_pos);
        }

        util::trimstr(line);
        std::istringstream iss(line);
        char type;
        int frame;

        iss >> type;
        iss.ignore(1);

        if (type != TypeMeta)
        {
            iss >> frame;
            iss.ignore(1);
        }

        if (!done_with_meta && type != TypeMeta)
        {
            if (only_meta)
                return;

            done_with_meta = true;
            set_version();
            if (version < 5)
                KeyMapReplayStep::current = key_map;
        }

        if (type == TypeMeta)
        {
            ASSERT(!done_with_meta);

            std::string key;
            std::string value;
            iss >> key;
            iss.ignore(1);
            util::portable_get_line(iss, value);

            ASSERT(meta_map.find(key) == meta_map.end());
            ASSERT(annotation_pos == std::string::npos);
            meta_map[key] = value;
        }
        else if (type == TypeComment)
        {
            std::string comment;
            util::portable_get_line(iss, comment);
            replay_log.push_back(std::make_shared<CommentReplayStep>(frame, comment));
        }
        else if (type == TypeQuit)
        {
            int quit_state;
            iss >> quit_state;
            replay_log.push_back(std::make_shared<QuitReplayStep>(frame, quit_state));
        }
        else if (type == TypeCheat)
        {
            Cheat cheat;
            int arg1 = -1, arg2 = -1;
            std::string arg3;

            std::string cheat_name;
            iss >> cheat_name;
            cheat = cheat_from_string(cheat_name);
            ASSERT(cheat > Cheat::None && cheat < Cheat::Last);

            if (cheat == Cheat::HeroData)
            {
                iss.ignore(1);
                util::portable_get_line(iss, arg3);
            }
            else
            {
                if (!(iss >> arg1))
                    arg1 = -1;
                if (!(iss >> arg2))
                    arg2 = -1;
            }
            replay_log.push_back(std::make_shared<CheatReplayStep>(frame, (Cheat)cheat, arg1, arg2, arg3));
        }
        else if (type == TypeQR)
        {
            int qr;
            bool value;

            iss >> qr;
            iss >> value;

            ASSERT(qr >= 0 && qr < qr_MAX);

            replay_log.push_back(std::make_shared<QRReplayStep>(frame, qr, value));
        }
        else if (type == TypeRng)
        {
            int start_index, end_index, seed;
            iss >> start_index;
            iss >> end_index;
            iss >> seed;
            ASSERT(start_index <= end_index);
            replay_log.push_back(std::make_shared<RngReplayStep>(frame, start_index, end_index, seed));
        }
        else if (type == TypeKeyMap)
        {
			ASSERT(version >= 5);
            std::array<int, KeyMapReplayStep::NumButtons> keys;
            for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
            {
                iss >> keys[i];
            }
            replay_log.push_back(std::make_shared<KeyMapReplayStep>(frame, keys));
            if (!found_key_map)
            {
                KeyMapReplayStep::current = KeyMapReplayStep(frame, keys);
                found_key_map = true;
            }
            key_map = KeyMapReplayStep(frame, keys);
        }
        else if (type == TypeKeyUp || type == TypeKeyDown)
        {
            if (version >= 5)
                ASSERT(found_key_map);

            std::string text;
            util::portable_get_line(iss, text);

            int button_index;
            int key_index;
            if (text.rfind("k ", 0) == 0)
            {
                button_index = -1;
                key_index = KeyReplayStep::find_index_for_key_name(text.substr(2));
                if (key_index == -1)
                    fprintf(stderr, "unknown key %s\n", text.substr(2).c_str());
                ASSERT(key_index != -1);
            }
            else
            {
                button_index = KeyReplayStep::find_index_for_button_name(text);
                if (button_index == -1)
                    fprintf(stderr, "unknown button %s\n", text.c_str());
                ASSERT(button_index != -1);
                key_index = key_map.button_keys[button_index];
            }

            replay_log.push_back(std::make_shared<KeyReplayStep>(frame, type, button_index, key_index));
        }
        else if (type == TypeMouse)
        {
            int x, y, z, b;
            iss >> x;
            iss >> y;
            if (!(iss >> z))
                z = 0;
            if (!(iss >> b))
                b = 0;
            std::array<int, 4> state = {x, y, z, b};
            replay_log.push_back(std::make_shared<MouseReplayStep>(frame, state));
        }
        else if (type == TypeState)
        {
            int state_type, value;
            iss >> state_type;
            iss >> value;
            replay_log.push_back(std::make_shared<StateReplayStep>(frame, (ReplayStateType)state_type, value));
        }

        if (frame_arg != -1 && replay_log.size() && replay_log.back()->frame > frame_arg && mode != ReplayMode::Update)
        {
            replay_log.pop_back();
            break;
        }
    }

    file.close();
    replay_log_current_index = 0;
    replay_log_current_quit_index = 0;
    replay_log_current_state_index = 0;
    set_version();
    debug = replay_get_meta_bool("debug");
    sync_rng = replay_get_meta_bool("sync_rng");

    if (mode == ReplayMode::Assert || mode == ReplayMode::Update)
    {
        expected_loadscr_frame_count.clear();
        for (auto step : replay_log)
        {
            if (step->type != TypeComment) continue;

            auto comment_step = static_cast<CommentReplayStep *>(step.get());
            if (comment_step->comment.rfind("scr=", 0) == 0)
                expected_loadscr_frame_count.push_back(step->frame);
        }
    }
}

static void save_replay(std::string filename, const std::vector<std::shared_ptr<ReplayStep>> &log)
{
	bool insert_baseline_annotations =
		has_assert_failed &&
		filename.find(".roundtrip") != std::string::npos &&
		log == record_log;

    std::time_t ct = std::time(0);
    replay_set_meta("time_updated", strtok(ctime(&ct), "\n"));
    replay_set_meta("zc_version_updated", getVersionString());
    if (version_use_latest)
        replay_set_meta("version", "latest");
    else
        replay_set_meta("version", version);
    replay_set_meta("frames", frame_count);
	replay_set_meta("length", log.size());

    std::ofstream out(filename, std::ios::binary);
    for (auto it : meta_map)
        out << fmt::format("{} {} {}", TypeMeta, it.first, it.second) << '\n';
    for (size_t i = 0; i < log.size(); i++)
    {
        auto step = log[i];
        std::string step_as_string = step->print();
        out << step_as_string;
        if (insert_baseline_annotations && i < replay_log.size())
        {
            auto replay_log_step = replay_log[i];
            if (!steps_are_equal(step.get(), replay_log_step.get()))
                out << std::string(std::max(0, 60 - (int)step_as_string.size()), ' ')
                    << ANNOTATION_MARKER
                    << ' '
                    << replay_log_step->print();
        }
        out << '\n';
    }
    if (insert_baseline_annotations && replay_log.size() > log.size())
    {
        size_t num_extra = replay_log.size() - log.size();
        size_t num_extra_to_print_limit = 20;
        size_t num_extra_to_print = std::min(num_extra, num_extra_to_print_limit);
        for (size_t i = log.size(); i < log.size() + num_extra_to_print; i++)
        {
            auto replay_log_step = replay_log[i];
            out << std::string(60, ' ')
                << ANNOTATION_MARKER
                << ' '
                << replay_log_step->print()
                << '\n';
        }
        if (num_extra > num_extra_to_print)
        {
            out << std::string(60, ' ')
                << ANNOTATION_MARKER
                << ' '
                << fmt::format("{} more ...", num_extra - num_extra_to_print)
                << '\n';
        }
    }
    out.close();
}

static std::string segments_to_string(const std::vector<std::pair<int, int>>& segments)
{
	std::vector<std::string> parts;
	for (auto it : segments)
	{
		int first = it.first;
		int second = it.second == -1 ? frame_count : it.second;
		if (first != second)
			parts.push_back(fmt::format("{}-{}", first, second));
		else
			parts.push_back(fmt::format("{}", first));
	}
	return fmt::format("{}", fmt::join(parts, " "));
}

static void save_result(bool stopped = false, bool changed = false)
{
	time_result_saved = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = time_result_saved - time_started;
	int elapsed_ms = elapsed.count();
	std::time_t time_started_c = std::chrono::system_clock::to_time_t(time_started_system);
	int fps = elapsed_ms ? (double)frame_count / elapsed_ms * 1000 : 0;

	std::ofstream out(get_file_path(".result.txt"), std::ios::binary);

	out << fmt::format("replay: {}", replay_path.string()) << '\n';
	out << fmt::format("mode: {}", replay_mode_to_string(mode)) << '\n';
	out << fmt::format("time: {}", strtok(ctime(&time_started_c), "\n")) << '\n';
	out << fmt::format("duration: {}", elapsed_ms) << '\n';
	out << fmt::format("zc_version: {}", getVersionString()) << '\n';
	if (replay_is_replaying() && !replay_log.empty())
		out << fmt::format("replay_log_frames: {}", replay_log.back()->frame) << '\n';
	if (replay_is_recording() && !record_log.empty())
		out << fmt::format("record_log_frames: {}", record_log.back()->frame) << '\n';
	out << fmt::format("frame: {}", frame_count) << '\n';
	out << fmt::format("fps: {}", fps) << '\n';
	if (stopped || has_assert_failed)
	{
		bool success = stopped && !has_aborted && !has_assert_failed && !has_rng_desynced && failed_loadscr_count_frame == -1;
		out << fmt::format("success: {}", success) << '\n';
	}
	if (has_rng_desynced)
		out << fmt::format("rng_desync: {}", has_rng_desynced) << '\n';
	if (has_assert_failed)
		out << fmt::format("failing_frame: {}", failing_frame) << '\n';
	if (mode == ReplayMode::Update)
		out << fmt::format("changed: {}", changed) << '\n';
	if (has_assert_failed && !unexpected_gfx_frames.empty())
		out << fmt::format("unexpected_gfx_frames: {}", fmt::join(unexpected_gfx_frames, ", ")) << '\n';
	if (has_assert_failed && !unexpected_gfx_segments.empty())
		out << fmt::format("unexpected_gfx_segments: {}", segments_to_string(unexpected_gfx_segments)) << '\n';
	if (has_assert_failed && !unexpected_gfx_segments_limited.empty())
		out << fmt::format("unexpected_gfx_segments_limited: {}", segments_to_string(unexpected_gfx_segments_limited)) << '\n';
    out << fmt::format("stopped: {}", stopped) << '\n';

	out.close();
}

static void save_snapshot(BITMAP* bitmap, PALETTE pal, int frame, bool was_unexpected)
{
	std::string suffix = fmt::format(".{}", frame);
	if (was_unexpected)
		suffix += "-unexpected";
	suffix += ".png";
	auto img_path = get_file_path(suffix);
	// TODO: fmt::print crashes in Visual Studio IDE...
	fprintf(stdout, "%s\n", fmt::format("Saving bitmap: {}", img_path.string()).c_str());
	save_bitmap(img_path.string().c_str(), bitmap, pal);
	if (was_unexpected)
		unexpected_gfx_frames.push_back(frame);
}

static void save_history_snapshots()
{
	for (auto& entry : framebuf_history)
	{
		if (entry.frame > 0)
		{
			save_snapshot(entry.bitmap, entry.pal, entry.frame, false);
			entry.frame = -1;
		}
	}
}

static void do_replaying_poll()
{
    while (replay_log_current_index < replay_log.size() && replay_log[replay_log_current_index]->frame == frame_count)
    {
        if (version >= 6)
        {
            char type = replay_log[replay_log_current_index]->type;
            if (type != TypeKeyDown && type != TypeKeyUp && type != TypeMouse)
                replay_log[replay_log_current_index]->run();
        }
        else
        {
            replay_log[replay_log_current_index]->run();
        }
        replay_log_current_index += 1;
    }

    replay_log_current_quit_index = replay_log_current_index;
    replay_log_current_state_index = replay_log_current_index;
}

static void check_assert()
{
    // Only print the very first difference. When replay_stop is called,
    // the program will exit with a status code based on this bool. If
    // asserts have failed, a ".zplay.roundtrip" file will be written
    // for comparison.
    if (has_assert_failed)
        return;

    while (assert_current_index < replay_log.size() && replay_log[assert_current_index]->frame <= frame_count)
    {
        if (assert_current_index >= record_log.size())
            break;

        auto replay_step = replay_log[assert_current_index];
        auto record_step = record_log[assert_current_index];
        if (!steps_are_equal(replay_step.get(), record_step.get()))
        {
            has_assert_failed = true;
            failing_frame = record_step->frame;
            int line_number = assert_current_index + meta_map.size() + 1;
            std::string error = fmt::format("<{}> expected:\n\t{}\nbut got:\n\t{}", line_number,
                                            replay_step->print(), record_step->print());
            fprintf(stderr, "%s\n", error.c_str());
            replay_save(get_file_path(".roundtrip"));

            static bool fail_instant = get_flag_bool("-replay-fail-assert-instant").value_or(false);
            if (fail_instant)
            {
                replay_stop();
                return;
            }
            if (!exit_when_done)
            {
                enter_sys_pal();
                jwin_auto_alert("Assert", error.c_str(), 150, 8, "OK (keep replaying)", NULL, 13, 27, get_zc_font(font_lfont));
                exit_sys_pal();
            }
            break;
        }

        assert_current_index++;
    }
}

static size_t old_start_of_next_screen_index;
static bool stored_control_state[KeyMapReplayStep::NumButtons];

static void start_manual_takeover()
{
    manual_takeover_start_index = replay_log_current_index;
    old_start_of_next_screen_index = -1;
    for (size_t i = manual_takeover_start_index; i < replay_log.size(); i++)
    {
        if (replay_log[i]->type != TypeComment)
            continue;

        auto comment_step = static_cast<CommentReplayStep *>(replay_log[i].get());
        if (comment_step->comment.rfind("scr=", 0) != 0 && comment_step->comment.rfind("dmap=", 0) != 0)
            continue;

        old_start_of_next_screen_index = i;
        break;
    }
    // TODO: support updating the very last screen.
    ASSERT(old_start_of_next_screen_index != -1);

    // Calculate what the button state is at the beginning of the next screen.
    // The state will be restored to this after the manual takeover is done.
    for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
        stored_control_state[i] = raw_control_state[i];
    for (size_t i = manual_takeover_start_index; i < old_start_of_next_screen_index; i++)
    {
        if (replay_log[i]->type != TypeKeyDown && replay_log[i]->type != TypeKeyUp)
            continue;

        auto key_step = static_cast<KeyReplayStep *>(replay_log[i].get());
        stored_control_state[key_step->key_index] = key_step->type == TypeKeyDown;
    }

    // Avoid unexpected input when manual takeover starts, which can be awkward to play.
	for (int i = 0; i < KEY_MAX; i++)
	{
		_key[i] = 0;
		key[i] = 0;
	}

    mode = ReplayMode::ManualTakeover;
    uninstall_keyboard_handlers();
    Throttlefps = true;
    Paused = true;
}

static void maybe_take_snapshot()
{
	if (mode == ReplayMode::Assert)
	{
		if (current_failing_gfx_segment_start_frame != -1)
		{
			// Limit how many snapshots are saved in the same gfx failure segment.
			if (frame_count - current_failing_gfx_segment_start_frame > 60*5)
			{
				if (unexpected_gfx_segments_limited.back().second == -1)
					unexpected_gfx_segments_limited.back().second = frame_count - 1;
			}
			else if (!prev_gfx_hash_was_same)
				save_snapshot(framebuf, RAMpal, frame_count, gfx_got_mismatch);
			return;
		}
		else if (last_failing_gfx_frame != -1 && frame_count - last_failing_gfx_frame <= 10)
		{
			// Save a few frames after the last failing gfx, for context.
			if (!prev_gfx_hash_was_same)
				save_snapshot(framebuf, RAMpal, frame_count, gfx_got_mismatch);
			return;
		}
	}

	auto it = std::find(snapshot_frames.begin(), snapshot_frames.end(), frame_count);
	if (!snapshot_all_frames && !gfx_got_mismatch && it == snapshot_frames.end())
	{
		if (mode == ReplayMode::Assert && !prev_gfx_hash_was_same)
		{
			blit(framebuf, framebuf_history[framebuf_history_index].bitmap, 0, 0, 0, 0, framebuf->w, framebuf->h);
			framebuf_history[framebuf_history_index].frame = frame_count;
			memcpy(framebuf_history[framebuf_history_index].pal, RAMpal, PAL_SIZE*sizeof(RGB));
			framebuf_history_index = (framebuf_history_index + 1) % framebuf_history.size();
		}
		return;
	}

	save_snapshot(framebuf, RAMpal, frame_count, gfx_got_mismatch);
}

static void fail_replay(std::string error)
{
	int line_number = replay_log_current_index + meta_map.size() + 1;
	std::string error1 = fmt::format("<{}> {}! stopping replay", line_number, error);
	std::string error2 = fmt::format("frame {}", frame_count);
	fprintf(stderr, "%s\n", error1.c_str());
	fprintf(stderr, "%s\n", error2.c_str());

	if (failing_frame == -1)
	{
		failing_frame = frame_count;
	}

	if (!exit_when_done)
	{
		enter_sys_pal();
		jwin_alert(replay_mode_to_string(mode).c_str(), error1.c_str(), error2.c_str(), NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
		exit_sys_pal();
	}

	save_history_snapshots();
	replay_stop();
}

std::string replay_mode_to_string(ReplayMode mode)
{
	switch (mode)
	{
		case ReplayMode::Off: return "off";
		case ReplayMode::Replay: return "replay";
		case ReplayMode::Record: return "record";
		case ReplayMode::Assert: return "assert";
		case ReplayMode::Update: return "update";
		case ReplayMode::ManualTakeover: return "manual_takeover";
	}
	return "unknown";
}

std::map<std::string, std::string> replay_load_meta(std::filesystem::path path)
{
	std::map<std::string, std::string> meta_map;
	bool only_meta = true;
	load_replay(meta_map, path, only_meta);
	return meta_map;
}

void replay_start(ReplayMode mode_, std::filesystem::path path, int frame)
{
    ASSERT(mode == ReplayMode::Off);
    ASSERT(mode_ != ReplayMode::Off && mode_ != ReplayMode::ManualTakeover);
    time_started = std::chrono::steady_clock::now();
    time_started_system = std::chrono::system_clock::now();
    mode = mode_;
    debug = false;
    sync_rng = false;
    did_attempt_input_during_replay = false;
    has_assert_failed = false;
    has_aborted = false;
    failing_frame = -1;
    last_failing_gfx_frame = -1;
    current_failing_gfx_segment_start_frame = -1;
    unexpected_gfx_frames.clear();
    unexpected_gfx_segments.clear();
    unexpected_gfx_segments_limited.clear();
    loadscr_count = 0;
    failed_loadscr_count_frame = -1;
    has_rng_desynced = false;
    gfx_got_mismatch = false;
    replay_path = path;
    if (output_dir.empty()) output_dir = replay_path.parent_path();
    manual_takeover_start_index = assert_current_index = replay_log_current_index = frame_count = 0;
    frame_arg = frame;
    prev_gfx_hash = 0;
    prev_gfx_hash_was_same = false;
    prev_debug_x = prev_debug_y = -1;
    prev_mouse_state = {0, 0, 0, 0};
    current_mouse_state = {0, 0, 0, 0};
    replay_log.clear();
    record_log.clear();
	snapshot_frames.clear();
    framebuf_history_index = 0;
    version_use_latest = false;
    replay_forget_input();

    switch (mode)
    {
    case ReplayMode::Off:
    case ReplayMode::ManualTakeover:
        return;
    case ReplayMode::Record:
    {
        version = VERSION;
        std::time_t ct = std::time(0);
        replay_set_meta("time_created", strtok(ctime(&ct), "\n"));
        replay_set_meta("zc_version_created", getVersionString());
        KeyMapReplayStep::current = KeyMapReplayStep::make(0);
        break;
    }
    case ReplayMode::Replay:
    case ReplayMode::Assert:
    case ReplayMode::Update:
        load_replay(meta_map, replay_path);
        break;
    }

    if (mode == ReplayMode::Assert)
    {
        for (int i = 0; i < framebuf_history.size(); i++)
        {
            framebuf_history[i].bitmap = create_bitmap_ex(8, framebuf->w, framebuf->h);
            framebuf_history[i].frame = -1;
        }
    }

    if (replay_is_replaying())
    {
        ASSERT(!keyboard_callback);
        install_keyboard_handlers();
        KeyMapReplayStep::stored = KeyMapReplayStep::make(0);
    }

    if (replay_is_recording() && version >= 5)
    {
        record_log.push_back(std::make_shared<KeyMapReplayStep>(0, KeyMapReplayStep::current.button_keys));
    }

    save_result();
}

void replay_continue(std::filesystem::path path)
{
    ASSERT(mode == ReplayMode::Off);
    mode = ReplayMode::Record;
    frame_arg = -1;
    has_aborted = false;
    prev_mouse_state = {0, 0, 0, 0};
    current_mouse_state = {0, 0, 0, 0};
    replay_forget_input();
    replay_path = path;
    load_replay(meta_map, replay_path);
    record_log = replay_log;
    frame_count = record_log.back()->frame + 1;
}

void replay_poll()
{
    if (mode == ReplayMode::Off)
        return;

    if (did_attempt_input_during_replay && replay_is_replaying())
    {
        int key_copy[KEY_MAX];
        bool down_states[controls::btnLast];
        for (int i = 0; i < controls::btnLast; i++)
            down_states[i] = down_control_states[i];
        for (int i = 0; i < KEY_MAX; i++)
        {
            key_copy[i] = key[i];
            _key[i] = 0;
            key[i] = 0;
        }
        uninstall_keyboard_handlers();
		
		enter_sys_pal();
		if (jwin_alert("Replay",
					   "Would you like to halt the replay and",
					   "take back control?",
					   "",
					   "Yes", "No", 'y', 'n', get_zc_font(font_lfont)) == 1)
		{
			replay_quit();
			exit_sys_pal();
			return;
		}
		exit_sys_pal();

        did_attempt_input_during_replay = false;
        install_keyboard_handlers();
        for (int i = 0; i < KEY_MAX; i++)
        {
            _key[i] = key_copy[i];
            key[i] = key_copy[i];
        }
        for (int i = 0; i < controls::btnLast; i++)
            down_control_states[i] = down_states[i];
    }

    if (frame_arg != -1 && frame_arg <= frame_count && replay_is_replaying())
    {
        if (mode == ReplayMode::Update)
        {
            start_manual_takeover();
            enter_sys_pal();
            jwin_alert("Recording", "Re-recording until new screen is loaded", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
            exit_sys_pal();
        }
        else if (mode != ReplayMode::Assert)
        {
            Throttlefps = true;
            Paused = true;
            replay_forget_input();
            replay_stop();
            enter_sys_pal();
            jwin_alert("Recording", "Replaying stopped at requested frame", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
            exit_sys_pal();
        }
    }

    switch (mode)
    {
    case ReplayMode::Off:
        return;
    case ReplayMode::Record:
        do_recording_poll();
        break;
    case ReplayMode::Replay:
        do_replaying_poll();
        if (replay_log_current_index == replay_log.size())
            replay_stop();
        break;
    case ReplayMode::Assert:
        do_replaying_poll();
        do_recording_poll();
        check_assert();
        break;
    case ReplayMode::Update:
        do_replaying_poll();
        do_recording_poll();
        if (frame_count == replay_log.back()->frame)
            replay_stop();
        break;
    case ReplayMode::ManualTakeover:
        do_recording_poll();
        break;
    }

    if (mode == ReplayMode::Assert && gfx_got_mismatch)
    {
        save_history_snapshots();
    }
    maybe_take_snapshot();

    static bool save_every_frame = get_flag_bool("-replay-save-result-every-frame").value_or(false);
    bool should_save_result = save_every_frame
        || frame_count == 0
        || std::chrono::steady_clock::now() - time_result_saved > 1s;
    if (should_save_result)
        save_result();

    if (mode == ReplayMode::Assert || mode == ReplayMode::Record)
    {
        if (frame_arg != -1 && frame_arg == frame_count)
        {
            if (mode == ReplayMode::Record) replay_save();

            Throttlefps = true;
            Paused = true;
            replay_forget_input();
            replay_stop();
            enter_sys_pal();
            jwin_alert(replay_mode_to_string(mode).c_str(), "Stopped at requested frame", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
            exit_sys_pal();
            return;
        }
    }

    if (mode == ReplayMode::Assert)
    {
        if (replay_log_current_index == assert_current_index && assert_current_index == replay_log.size())
        {
            replay_stop();
            return;
        }

        if (frame_count > replay_log.back()->frame)
        {
            replay_stop();
            return;
        }

        // If loadscr is not called when expected, the rest of the replay is not going to go well and may possible
        // loop forever. Only record 60 more frames at this point.
        if (has_assert_failed && failed_loadscr_count_frame == -1)
        {
            int expected_frame = loadscr_count + 1 >= expected_loadscr_frame_count.size() ?
                replay_log.back()->frame :
                expected_loadscr_frame_count[loadscr_count + 1];
            if (frame_count > expected_frame)
                failed_loadscr_count_frame = frame_count;
        }
        if (has_assert_failed && failed_loadscr_count_frame != -1 && frame_count - failed_loadscr_count_frame > 60)
        {
            replay_stop();
            return;
        }
    }

    // Updating should never modify when screens are loaded.
    if (mode == ReplayMode::Update)
    {
        int expected_frame = loadscr_count + 1 >= expected_loadscr_frame_count.size() ?
            replay_log.back()->frame :
            expected_loadscr_frame_count[loadscr_count + 1];
        if (frame_count > expected_frame)
        {
            failed_loadscr_count_frame = frame_count;
            replay_stop();
            return;
        }
    }

    rng_seed_count_this_frame.clear();
    frame_count++;
}

// example: 0 3 4-10 45
bool replay_add_snapshot_frame(std::string frames_shorthand)
{
	std::vector<int> frames;
	bool in_number = false;
	bool in_range = false;
	size_t cur_start_index = 0;

	for (size_t i = 0; i <= frames_shorthand.size(); i++)
	{
		char c = i == frames_shorthand.size() ? ' ' : frames_shorthand[i];
		if (c == ' ')
		{
			if (!in_number)
				continue;

			errno = 0;
			int as_int = std::strtol(frames_shorthand.data() + cur_start_index, nullptr, 10);
			if (errno)
				return false;

			if (in_range)
			{
				int from = frames.back();
				if (from >= as_int)
					return false;

				for (int i = from + 1; i <= as_int; i++)
					frames.push_back(i);
			}
			else
			{
				frames.push_back(as_int);
			}
			in_number = in_range = false;
		}
		else if (std::isdigit(c))
		{
			if (!in_number)
			{
				in_number = true;
				cur_start_index = i;
			}
		}
		else if (c == '-')
		{
			if (!in_number)
				return false;

			errno = 0;
			int as_int = std::strtol(frames_shorthand.data() + cur_start_index, nullptr, 10);
			if (errno)
				return false;

			frames.push_back(as_int);
			in_number = false;
			in_range = true;
		}
		else
		{
			return false;
		}
	}

	if (frames.size() > 100000)
		return false;

	snapshot_frames.insert(snapshot_frames.end(), frames.begin(), frames.end());
	return true;
}

void replay_peek_quit()
{
    int i = replay_log_current_quit_index;
    while (i < replay_log.size() && replay_log[i]->frame == frame_count)
    {
        if (replay_log[i]->type == TypeQuit)
        {
            auto quit_replay_step = static_cast<QuitReplayStep *>(replay_log[i].get());
            if (quit_replay_step->quit_state == 0)
                GameFlags |= GAMEFLAG_TRYQUIT;
            else
                Quit = quit_replay_step->quit_state;
            replay_log_current_quit_index = i + 1;
            break;
        }
        i++;
    }
}

void replay_peek_input()
{
    size_t i = replay_log_current_index;
    while (i < replay_log.size() && replay_log[i]->frame == frame_count)
    {
        char type = replay_log[i]->type;
        if (type == TypeKeyDown || type == TypeKeyUp || type == TypeMouse)
        {
            replay_log[i]->run();
        }
        i++;
    }
}

void replay_do_cheats()
{
    size_t i = replay_log_current_index;
    while (i < replay_log.size() && replay_log[i]->frame == frame_count)
    {
        if (replay_log[i]->type == TypeCheat)
        {
            auto cheat_replay_step = static_cast<CheatReplayStep *>(replay_log[i].get());
            cheats_enqueue(cheat_replay_step->cheat, cheat_replay_step->arg1, cheat_replay_step->arg2, cheat_replay_step->arg3);
        }
        i++;
    }
}

void replay_do_qrs()
{
    size_t i = replay_log_current_index;
    while (i < replay_log.size() && replay_log[i]->frame == frame_count)
    {
        if (replay_log[i]->type == TypeQR)
        {
            auto qr_replay_step = static_cast<QRReplayStep *>(replay_log[i].get());
            enqueue_qr_change(qr_replay_step->qr, qr_replay_step->value);
        }
        i++;
    }
}

bool replay_is_assert_done()
{
    return mode == ReplayMode::Assert && (assert_current_index == replay_log.size() || frame_count >= replay_log.back()->frame);
}

void replay_forget_input()
{
    if (mode == ReplayMode::Off)
        return;

    for (int i = 0; i < KEY_MAX; i++)
    {
        _key[i] = 0;
        key[i] = 0;
        key_current_frame[i] = 0;
    }
    for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
        previous_control_state[i] = raw_control_state[i] = false;
    for (int i = 0; i < KEY_MAX; i++)
        previous_keys[i] = false;
}

void replay_stop(bool aborted)
{
    if (mode == ReplayMode::Off)
        return;

	has_aborted = aborted;

    if (replay_is_replaying())
    {
        keyboard_callback = nullptr;
        replay_forget_input();
        KeyMapReplayStep::stored.run();
    }

    if (mode == ReplayMode::Assert && !aborted)
    {
        check_assert();
        bool log_size_mismatch = replay_log.size() != record_log.size();
        has_assert_failed |= log_size_mismatch || has_rng_desynced;
        if (has_assert_failed)
        {
            replay_save(get_file_path(".roundtrip"));
        }
        if (log_size_mismatch)
        {
            fprintf(stderr, "replay_log size is %zu but record_log size is %zu\n", replay_log.size(), record_log.size());
        }

        save_result(true);
        if (exit_when_done)
        {
			mode = ReplayMode::Off;
			zc_exit(has_assert_failed ? ASSERT_FAILED_EXIT_CODE : 0);
        }
        else if (has_assert_failed)
        {
            enter_sys_pal();
            jwin_alert("Assert", "Replay has stopped, and the assert failed.", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
            exit_sys_pal();
			Paused = true;
        }
    }

    if (mode == ReplayMode::Update && !aborted)
    {
        bool should_save;
        if (failed_loadscr_count_frame != -1)
        {
            should_save = false;

            if (exit_when_done)
            {
                save_result(true);
                mode = ReplayMode::Off;
                zc_exit(ASSERT_FAILED_EXIT_CODE);
            }
            else
            {
                enter_sys_pal();
                jwin_alert("Update", "Failed to update replay as there was a non-graphical change.", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
                exit_sys_pal();
            }
        }
        else if (replay_log.size() != record_log.size())
        {
            should_save = true;
        }
        else
        {
            should_save = false;
            for (size_t i = 0; i < replay_log.size(); i++)
            {
                if (!steps_are_equal(replay_log[i].get(), record_log[i].get()))
                {
                    should_save = true;
                    break;
                }
            }
        }

        if (should_save)
        {
            replay_save();
            save_result(true, true);
        }
        else
        {
            save_result(true);
        }
    }
    else
    {
        save_result(true);
    }

    for (int i = 0; i < framebuf_history.size(); i++)
    {
        if (framebuf_history[i].bitmap)
		{
            destroy_bitmap(framebuf_history[i].bitmap);
			framebuf_history[i].bitmap = nullptr;
		}
    }

    mode = ReplayMode::Off;
    frame_count = 0;
    replay_log.clear();
    rngs.clear();
    meta_map.clear();

	if (exit_when_done)
	{
		zc_exit(has_rng_desynced ? 1 : 0);
	}
}

void replay_quit()
{
    bool aborted = true;
    replay_stop(aborted);
}

void replay_save()
{
    replay_save(replay_path);
}

void replay_save(std::filesystem::path path)
{
    save_replay(path.string(), record_log);
}

void replay_stop_manual_takeover()
{
    ASSERT(mode == ReplayMode::ManualTakeover);

    // Update the replay log to account for the newly added steps.
    int old_frame_duration = replay_log[old_start_of_next_screen_index]->frame - frame_arg;
    int new_frame_duration = frame_count - frame_arg;
    int frame_delta = new_frame_duration - old_frame_duration;
    for (size_t i = old_start_of_next_screen_index; i < replay_log.size(); i++)
    {
        replay_log[i]->frame += frame_delta;
    }

    // Restore button state.
    std::vector<std::shared_ptr<ReplayStep>> restore_log;
    for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
        raw_control_state[i] = stored_control_state[i];

    // Insert some button steps to make the replay_log match what the record_log will have inserted on the next
    // call to do_recording_poll.
    // This is the same as do_recording_poll, but without setting the previous control state variable.
    for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
    {
        bool state = raw_control_state[i];
        if (state == previous_control_state[i])
            continue;

        int key_index = KeyMapReplayStep::current.button_keys[i];
        restore_log.push_back(std::make_shared<KeyReplayStep>(frame_count, state ? TypeKeyDown : TypeKeyUp, i, key_index));
    }

    int num_steps_before = replay_log.size();
    // TODO: technically these should be inserted at the end of this frame's steps (button steps are always at the end),
    // but even then they could be out-of-order from how the record log would write them (since button steps are written
    // in a specific order, this is additive, and we aren't taking into account the presence of existing button steps on this frame).
    // But whatever. This is just so we can do save_replay on the replay_log to write _something_ when the player is done,
    // in case the recording needs to have multiple screens updated (just have to repeat this manual takeover once for every screen,
    // picking up from the previous replay_log saved at the end of this function).
    replay_log.insert(replay_log.begin() + old_start_of_next_screen_index, restore_log.begin(), restore_log.end());
    // Erase the old steps.
    replay_log.erase(replay_log.begin() + manual_takeover_start_index, replay_log.begin() + old_start_of_next_screen_index);
    // Insert the new steps.
    replay_log.insert(replay_log.begin() + manual_takeover_start_index, record_log.begin() + manual_takeover_start_index, record_log.end());
    int steps_delta = replay_log.size() - num_steps_before;

    save_replay(replay_path.string(), replay_log);
    replay_log_current_index = record_log.size();
    mode = ReplayMode::Update;
    install_keyboard_handlers();
    frame_arg = -1;
    enter_sys_pal();
    jwin_alert("Recording", "Done re-recording, resuming replay from here", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
    exit_sys_pal();

    // TODO currently just for manually debugging this system. Instead, should somehow enable assert mode when going back to ::Update.
    bool DEBUG_MANUAL_OVERRIDE = false;
    if (DEBUG_MANUAL_OVERRIDE)
    {
        // Skip the assertion index to the first step after the mark of the new frame, to skip over the injected out-of-order button steps.
        assert_current_index = old_start_of_next_screen_index + steps_delta;
        for (size_t i = assert_current_index; i < replay_log.size(); i++)
        {
            if (replay_log[i]->frame != frame_count)
            {
                assert_current_index = i;
                break;
            }
        }
        // assert_current_index = record_log.size() - 1; // Should be this, but can't b/c out-of-order reason from big comment above.
        mode = ReplayMode::Assert;
    }
}

void replay_step_comment(std::string comment)
{
    if (replay_is_recording())
    {
        util::trimstr(comment);
        record_log.push_back(std::make_shared<CommentReplayStep>(frame_count, comment));
        // Not necessary to call this here, but helps to halt the program exactly when an unexpected
        // comment occurs instead of at the next call to replay_poll.
        if (mode == ReplayMode::Assert)
            check_assert();
    }
}

void replay_step_comment_loadscr(int screen)
{
	replay_step_comment(fmt::format("scr={}", screen));
	loadscr_count += 1;
}

// https://base91.sourceforge.net/
// The maximum number of digits this can generate:
//     uint64_t = 10
//     uint32_t = 5
//     uint16_t = 3
//     uint8_t  = 2
template <typename T>
std::string int_to_basE91(T value)
{
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&()*+,./:;<=>?@[]^_`{|}~\"";
    std::string result;
    do {
        T remainder = value % 91;
        value /= 91;
        result.insert(result.begin(), alphabet[remainder]);
    } while (value > 0);
    return result;
}

void replay_step_gfx(uint32_t gfx_hash)
{
	// 16 bits should be enough entropy to detect visual regressions.
	// Using uint16_t reduces .zplay by ~7%.
	std::string gfx_comment = fmt::format("g {}", int_to_basE91((uint16_t)gfx_hash));

	if (mode == ReplayMode::Assert)
	{
		std::string expected_gfx_comment;
		auto it_start = std::lower_bound(replay_log.begin(), replay_log.end(), frame_count,
			[](auto step, const int value) {
				return step->frame < value;
			});
		for (auto it = it_start; it < replay_log.end(); it++)
		{
			if (it->get()->frame != frame_count)
				break;
			if (it->get()->type == TypeComment)
			{
				auto comment_step = static_cast<const CommentReplayStep *>(it->get());
				if (comment_step->comment.rfind("g ", 0) == 0)
				{
					expected_gfx_comment = comment_step->comment;
					break;
				}
			}
		}

		if (expected_gfx_comment.empty() && gfx_hash != prev_gfx_hash)
			gfx_got_mismatch = true;
		if (!expected_gfx_comment.empty() && expected_gfx_comment != gfx_comment)
			gfx_got_mismatch = true;
		if (gfx_got_mismatch)
			last_failing_gfx_frame = frame_count;
		if (gfx_hash != prev_gfx_hash)
		{
			if (gfx_got_mismatch && current_failing_gfx_segment_start_frame == -1)
			{
				current_failing_gfx_segment_start_frame = frame_count;
				unexpected_gfx_segments.push_back({frame_count, -1});
				unexpected_gfx_segments_limited.push_back({frame_count, -1});
			}
			else if (!gfx_got_mismatch && current_failing_gfx_segment_start_frame != -1)
			{
				current_failing_gfx_segment_start_frame = -1;
				if (!unexpected_gfx_segments.empty())
					unexpected_gfx_segments.back().second = frame_count - 1;
				if (!unexpected_gfx_segments_limited.empty() && unexpected_gfx_segments_limited.back().second == -1)
					unexpected_gfx_segments_limited.back().second = frame_count - 1;
			}
		}
	}

	// Skip if last invocation was the same value.
	prev_gfx_hash_was_same = gfx_hash == prev_gfx_hash;
	if (!prev_gfx_hash_was_same)
	{
		replay_step_comment(gfx_comment);
		prev_gfx_hash = gfx_hash;
	}

	// Note: I tried a simple queue cache to remember the last N hashes and use shorthand
	// for repeats (ex: gfx ^2), but even with a huge memory of 16777216 hashes the
	// savings was never more than 2%, so not worth it.
}

void replay_set_meta(std::string key, std::string value)
{
    if (!replay_is_active())
        return;

    if (key == "qst")
        std::replace_if(
            value.begin(), value.end(),
            [](std::string::value_type v)
            {
                return v == '\\';
            },
            '/');

    meta_map[key] = value;
}

void replay_set_meta(std::string key, int value)
{
    if (replay_is_active())
        meta_map[key] = fmt::format("{}", value);
}

void replay_set_meta_bool(std::string key, bool value)
{
    if (value)
        replay_set_meta(key, "true");
    else
        meta_map.erase(key);
}

void replay_delete_meta(std::string key)
{
	meta_map.erase(key);
}

static std::string get_meta_raw_value(std::string key)
{
    auto it = meta_map.find(key);
    if (it == meta_map.end())
        return "";
    return it->second;
}

std::string replay_get_meta_str(std::string key)
{
    return get_meta_raw_value(key);
}

std::string replay_get_meta_str(std::string key, std::string defaultValue)
{
	std::string raw = get_meta_raw_value(key);
	if (raw.empty()) return defaultValue;
	return raw;
}

int replay_get_meta_int(std::string key)
{
    return std::stoi(get_meta_raw_value(key).c_str());
}

int replay_get_meta_int(std::string key, int defaultValue)
{
    std::string raw = get_meta_raw_value(key);
    if (raw.empty()) return defaultValue;
    return std::stoi(raw.c_str());
}

bool replay_get_meta_bool(std::string key)
{
    return get_meta_raw_value(key) == "true";
}

bool replay_has_meta(std::string key)
{
    return meta_map.find(key) != meta_map.end();
}

void replay_step_quit(int quit_state)
{
    if (replay_is_recording())
        record_log.push_back(std::make_shared<QuitReplayStep>(frame_count, quit_state));
}

void replay_step_cheat(Cheat cheat, int arg1, int arg2, std::string arg3)
{
    if (replay_is_recording())
        record_log.push_back(std::make_shared<CheatReplayStep>(frame_count, cheat, arg1, arg2, arg3));
}

void replay_step_qr(int qr, bool value)
{
    if (replay_is_recording())
        record_log.push_back(std::make_shared<QRReplayStep>(frame_count, qr, value));
}

ReplayMode replay_get_mode()
{
    return mode;
}

int replay_get_version()
{
    return version;
}

bool replay_version_check(int min, int max)
{
	if (!replay_is_active())
	{
		return max == -1;
	}

	if (max == -1)
	{
		return version >= min;
	}

    return version >= min && version < max;
}

int replay_get_frame()
{
    return frame_count;
}

void replay_set_output_dir(std::filesystem::path dir)
{
	output_dir = dir;
}

std::filesystem::path replay_get_replay_path()
{
    return replay_path;
}

std::string replay_get_buttons_string()
{
    std::string text;
    text += fmt::format("{} ", frame_count);
    for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
    {
        if (raw_control_state[i])
        {
            if (!text.empty())
                text += ' ';
            text += KeyReplayStep::button_names[i];
        }
    }
    return text;
}

bool replay_is_active()
{
    return mode != ReplayMode::Off;
}

void replay_set_debug(bool enable_debug)
{
    debug = enable_debug;
    replay_set_meta_bool("debug", debug);
}

bool replay_is_debug()
{
    return mode != ReplayMode::Off && debug;
}

void replay_set_snapshot_all_frames(bool enable_snapshot_all_frames)
{
	snapshot_all_frames = enable_snapshot_all_frames;
}

bool replay_is_snapshot_all_frames()
{
	return snapshot_all_frames;
}

void replay_set_sync_rng(bool enable)
{
    sync_rng = enable;
    replay_set_meta_bool("sync_rng", sync_rng);
}

bool replay_is_replaying()
{
    return mode == ReplayMode::Replay || mode == ReplayMode::Assert || mode == ReplayMode::Update;
}

bool replay_is_recording()
{
    return mode == ReplayMode::Record || mode == ReplayMode::Assert || mode == ReplayMode::Update || mode == ReplayMode::ManualTakeover;
}

void replay_set_frame_arg(int frame)
{
    if (frame == -1)
        frame = replay_log.front()->frame + 1;
    else
        frame_arg = frame;
}

void replay_enable_exit_when_done()
{
	exit_when_done = true;
}

size_t replay_register_rng(zc_randgen *rng)
{
    if (std::find(rngs.begin(), rngs.end(), rng) != rngs.end())
        return get_rng_index(rng);

    rngs.push_back(rng);
    return rngs.size() - 1;
}

void replay_set_rng_seed(zc_randgen *rng, int seed)
{
    ASSERT(mode != ReplayMode::Off);

    int index = get_rng_index(rng);
    ASSERT(index != -1);

    int seed_count = 0;
    {
        auto it = rng_seed_count_this_frame.find(index);
        if (it != rng_seed_count_this_frame.end())
            seed_count = it->second;
    }
    seed_count += 1;
    rng_seed_count_this_frame[index] = seed_count;

    if (replay_is_replaying())
    {
        RngReplayStep *rng_step = find_rng_step(index, replay_log_current_index, replay_log, seed_count);
        if (rng_step)
        {
            seed = rng_step->seed;
        }
        // Only OK to be missing if in update mode.
        else if (mode != ReplayMode::Update)
        {
            has_rng_desynced = true;
            fail_replay("rng desync");
        }
    }

    if (replay_is_recording())
    {
        bool did_extend = false;
        if (!record_log.empty() && record_log.back()->type == TypeRng && record_log.back()->frame == frame_count)
        {
            auto rng_step = static_cast<RngReplayStep *>(record_log.back().get());
            if (rng_step->seed == seed)
            {
                if (rng_step->start_index == index + 1)
                {
                    rng_step->start_index = index;
                    did_extend = true;
                }
                if (rng_step->end_index == index - 1)
                {
                    rng_step->end_index = index;
                    did_extend = true;
                }
            }
        }

        if (!did_extend)
            record_log.push_back(std::make_shared<RngReplayStep>(frame_count, index, index, seed));
    }

    rng->seed(seed);
}

void replay_sync_rng()
{
    if (!sync_rng)
        return;

    int seed = time(0);
    for (size_t i = 0; i < rngs.size(); i++)
    {
        // Only reset the rngs that haven't been updated this frame.
        if (rng_seed_count_this_frame.find(i) != rng_seed_count_this_frame.end())
            continue;

        replay_set_rng_seed(rngs[i], seed);
        if (mode == ReplayMode::Off)
            return;
    }

    frame = 0;
}

int replay_get_mouse(int index)
{
	return current_mouse_state[index];
}

void replay_set_mouse(int index, int value)
{
	current_mouse_state[index] = value;
}


int replay_get_state(ReplayStateType state_type, int fn())
{
	int value = 0;
	if (replay_is_replaying())
	{
		value = 0;
		while (replay_log_current_state_index < replay_log.size() && replay_log[replay_log_current_state_index]->frame == frame_count)
		{
			size_t i = replay_log_current_state_index;
			replay_log_current_state_index += 1;

			if (replay_log[i]->type == TypeState)
			{
				auto state_replay_step = static_cast<StateReplayStep *>(replay_log[i].get());
				if (state_replay_step->state_type == state_type)
					value = state_replay_step->value;
				else
					fail_replay(fmt::format("state desync: {}", (int)state_type));
				break;
			}
		}
	}
	else
	{
		value = fn();
	}

	if (replay_is_recording())
	{
		record_log.push_back(std::make_shared<StateReplayStep>(frame_count, state_type, value));
	}

	return value;
}
