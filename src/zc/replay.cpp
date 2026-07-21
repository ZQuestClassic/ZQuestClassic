#include "zc/replay.h"
#include "zc/replay_compat.h"
#include "base/check.h"
#include "core/qrs.h"
#include "base/version.h"
#include "base/zapp.h"
#include "zc/zc_sys.h"
#include "zalleg/render.h"
#include "zalleg/zalleg.h"
#include "base/util.h"
#include "zc/zelda.h"
#include <array>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <charconv>
#include <ctime>
#include <filesystem>
#include <chrono>
#include <variant>
#include <deque>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include "dialog/info.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include <xxhash.h>

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

extern PALETTE* hw_palette;
extern bool update_hw_pal;

using namespace std::chrono_literals;

static const int ASSERT_SNAPSHOT_BUFFER = 10;
static const int ASSERT_FAILED_EXIT_CODE = 120;
static const int VERSION = 62;

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

// Settings configured externally before replay_start (or that persist across
// replays), so they live outside ReplayState and survive its reset.
static bool snapshot_all_frames;
static bool exit_when_done;
static bool batch_when_done;
static bool batch_any_failed;
static std::filesystem::path output_dir;

// Parse an entire string as a base-10 int. Leaves |out| unchanged and returns false
// if the string (after trimming whitespace) is empty or has any non-numeric content.
// Used for file-controlled values so malformed input degrades gracefully instead of
// throwing from std::stoi.
static bool try_parse_int(std::string_view s, int& out)
{
	while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
	while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
	if (s.empty())
		return false;

	int value;
	const char* begin = s.data();
	const char* end = begin + s.size();
	auto [ptr, ec] = std::from_chars(begin, end, value);
	if (ec != std::errc() || ptr != end)
		return false;

	out = value;
	return true;
}

struct FramebufHistoryEntry
{
	BITMAP* bitmap;
	PALETTE pal;
	int frame;
};

// All mutable state for a single replay session. Held in one instance (`rs`) so it
// can be reset in a single assignment (`rs = ReplayState{}`) in replay_start, which
// keeps start/stop reset logic from drifting out of sync. Externally-configured
// settings (output_dir, exit_when_done, snapshot_all_frames) and the step logs
// (replay_log/record_log, whose element type is defined later) live outside this.
struct ReplayState
{
	ReplayMode mode = ReplayMode::Off;
	int version = 0;
	int initial_version = 0;
	bool version_use_latest = false;
	bool debug = false;
	bool sync_rng = false;
	int frame_arg = 0;
	std::filesystem::path replay_path;
	std::map<std::string, std::string> meta_map;
	std::vector<int> snapshot_frames;
	std::vector<int> expected_loadscr_frame_count;
	int loadscr_count = 0;
	int failed_loadscr_count_frame = -1;
	size_t replay_log_current_index = 0;
	size_t replay_log_current_quit_index = 0;
	size_t replay_log_current_state_index = 0;
	size_t assert_current_index = 0;
	bool has_assert_failed = false;
	bool has_aborted = false;
	int failing_frame = -1;
	int last_failing_gfx_frame = -1;
	int current_failing_gfx_segment_start_frame = -1;
	std::vector<int> unexpected_gfx_frames;
	std::vector<std::pair<int, int>> unexpected_gfx_segments;
	std::vector<std::pair<int, int>> unexpected_gfx_segments_limited;
	bool has_rng_desynced = false;
	bool did_attempt_input_during_replay = false;
	int frame_count = 0;
	bool previous_control_state[controls::btnLast] = {};
	char previous_keys[KEY_MAX] = {};
	std::vector<zc_randgen *> rngs;
	// Per-rng count of how many times each rng was seeded this frame, indexed by rng
	// index. 0 means "not seeded this frame".
	std::vector<int> rng_seed_count_this_frame;
	uint32_t prev_gfx_hash = 0;
	bool prev_gfx_hash_was_same = false;
	int prev_debug_x = -1;
	int prev_debug_y = -1;
	bool gfx_got_mismatch = false;
	// Assert mode: the replay log recorded a gfx change for this frame (the
	// expected screen changed, whether or not ours did).
	bool gfx_expected_changed = false;
	std::array<int, 4> prev_mouse_state = {0, 0, 0, 0};
	std::array<int, 4> current_mouse_state = {0, 0, 0, 0};
	std::chrono::time_point<std::chrono::steady_clock> time_started;
	std::chrono::time_point<std::chrono::system_clock> time_started_system;
	std::chrono::time_point<std::chrono::steady_clock> time_result_saved;
	std::optional<ZCVersionCreated> zc_version_created;

	// Memory ownership for zero-copy string_views.
	std::string replay_file_buffer;
	std::deque<std::string> string_arena;

	std::array<FramebufHistoryEntry, ASSERT_SNAPSHOT_BUFFER> framebuf_history = {};
	int framebuf_history_index = 0;
};
static ReplayState rs;

struct ReplayStep
{
    int frame;
    char type;

    ReplayStep(int frame, char type) : frame(frame), type(type)
    {
    }
};

struct KeyMapReplayStep : ReplayStep
{
	static const int NumButtons = 14;
	static KeyMapReplayStep current;
	static KeyMapReplayStep stored;

	static KeyMapReplayStep make(int frame)
	{
		KeyMapReplayStep step(frame, {});
		
		for (int q = 0; q < KeyMapReplayStep::NumButtons; ++q)
			step.button_keys[q] = active_control_scheme->keys[q];
		return step;
	}

	std::array<int, KeyMapReplayStep::NumButtons> button_keys;

	KeyMapReplayStep(int frame, std::array<int, KeyMapReplayStep::NumButtons> button_keys) : ReplayStep(frame, TypeKeyMap), button_keys(button_keys)
	{
	}

	int find_button_index_for_key(int key) const
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
		for (int q = 0; q < KeyMapReplayStep::NumButtons; ++q)
			replay_control_scheme.keys[q] = button_keys[q];
	}

	std::string print() const
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

    static int find_index_for_button_name(std::string_view button_name)
    {
        for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
        {
            if (button_names[i] == button_name)
                return i;
        }
        return -1;
    }

    static int find_index_for_key_name(std::string_view key_name)
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
    std::string print() const
    {
        if (button_index != -1)
            return fmt::format("{} {} {}", type, frame, button_names[button_index]);
        else
            return fmt::format("{} {} k {}", type, frame, _keyboard_common_names[key_index]);
    }
};

struct CommentReplayStep : ReplayStep
{
    std::string_view comment;

    CommentReplayStep(int frame, std::string_view comment) : ReplayStep(frame, TypeComment), comment(comment)
    {
    }

    void run()
    {
    }

    std::string print() const
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

    std::string print() const
    {
        return fmt::format("{} {} {}", type, frame, quit_state);
    }
};

struct CheatReplayStep : ReplayStep
{
    Cheat cheat;
    int arg1, arg2;
    std::string_view arg3;

    CheatReplayStep(int frame, Cheat cheat, int arg1, int arg2, std::string_view arg3) : ReplayStep(frame, TypeCheat), cheat(cheat), arg1(arg1), arg2(arg2), arg3(arg3)
    {
    }

    void run()
    {
        // During replay, replay_do_cheats handles enqueuing cheats.
    }

    std::string print() const
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

    std::string print() const
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

    std::string print() const
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
		rs.current_mouse_state = state;
	}

	std::string print() const
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
		// During replay, the engine calls `replay_get_state` which handles finding the next relevant state step.
	}

	std::string print() const
	{
		return fmt::format("{} {} {} {}", type, frame, (int)state_type, value);
	}
};

struct MetaReplayStep : ReplayStep
{
	std::string_view key;
	std::string_view value;

	MetaReplayStep(int frame, std::string_view key, std::string_view value) : ReplayStep(frame, TypeMeta), key(key), value(value)
	{
	}

	void run()
	{
		std::string& value_str = rs.meta_map[std::string(key)] = std::string(value);
		if (key == "version")
			try_parse_int(value_str, rs.version);
	}

	std::string print() const
	{
		return fmt::format("{} {} {} {}", type, frame, key, value);
	}
};

// Define variant and helper functions
using ReplayStepVariant = std::variant<
    CommentReplayStep,
    QuitReplayStep,
    CheatReplayStep,
    QRReplayStep,
    RngReplayStep,
    KeyMapReplayStep,
    KeyReplayStep,
    MouseReplayStep,
    StateReplayStep,
    MetaReplayStep
>;

static int get_frame(const ReplayStepVariant& v)
{
	return std::visit([](const auto& s) { return s.frame; }, v);
}

static char get_type(const ReplayStepVariant& v)
{
	return std::visit([](const auto& s) { return s.type; }, v);
}

static void run_step(ReplayStepVariant& v)
{
	std::visit([](auto& s) { s.run(); }, v);
}

static std::string print_step(const ReplayStepVariant& v)
{
	return std::visit([](const auto& s) { return s.print(); }, v);
}

static std::vector<ReplayStepVariant> replay_log;
static std::vector<ReplayStepVariant> record_log;

static int get_rng_index(zc_randgen *rng)
{
    auto it = std::find(rs.rngs.begin(), rs.rngs.end(), rng);
    if (it == rs.rngs.end())
        return -1;
    return it - rs.rngs.begin();
}

// Find the RNG step associated with the given |rng_index|, starting at |starting_step_index|.
// Only RNG steps for the current |rs.frame_count| are valid.
// If an offset is given, the nth RNG step will be returned (offset = 1 means the first).
static const RngReplayStep *find_rng_step(int rng_index, size_t starting_step_index, const std::vector<ReplayStepVariant> &log, int offset = 1)
{
    ASSERT(offset >= 1);

    int num_seen = 0;
    for (size_t i = starting_step_index; i < log.size(); i++)
    {
        auto& step = log[i];
        if (get_frame(step) != rs.frame_count)
            break;
        if (get_type(step) != TypeRng)
            continue;

        auto& rng_step = std::get<RngReplayStep>(step);
        if (rng_index >= rng_step.start_index && rng_index <= rng_step.end_index)
            num_seen += 1;

        if (num_seen == offset)
            return &rng_step;
    }

    return nullptr;
}

static bool steps_are_equal(const ReplayStepVariant& var1, const ReplayStepVariant& var2)
{
	char type1 = get_type(var1);
	char type2 = get_type(var2);

	if (get_frame(var1) != get_frame(var2) || type1 != type2)
	{
		return false;
	}

	switch (type2)
	{
	case TypeComment:
		return std::get<CommentReplayStep>(var1).comment == std::get<CommentReplayStep>(var2).comment;
	case TypeKeyUp:
	case TypeKeyDown:
		return std::get<KeyReplayStep>(var1).button_index == std::get<KeyReplayStep>(var2).button_index &&
			std::get<KeyReplayStep>(var1).key_index == std::get<KeyReplayStep>(var2).key_index;
	case TypeQuit:
		return std::get<QuitReplayStep>(var1).quit_state == std::get<QuitReplayStep>(var2).quit_state;
	case TypeCheat:
		return std::get<CheatReplayStep>(var1).cheat == std::get<CheatReplayStep>(var2).cheat &&
			std::get<CheatReplayStep>(var1).arg1 == std::get<CheatReplayStep>(var2).arg1 &&
			std::get<CheatReplayStep>(var1).arg2 == std::get<CheatReplayStep>(var2).arg2 &&
			std::get<CheatReplayStep>(var1).arg3 == std::get<CheatReplayStep>(var2).arg3;
	case TypeQR:
		return std::get<QRReplayStep>(var1).qr == std::get<QRReplayStep>(var2).qr &&
			std::get<QRReplayStep>(var1).value == std::get<QRReplayStep>(var2).value;
	case TypeRng:
		return std::get<RngReplayStep>(var1).seed == std::get<RngReplayStep>(var2).seed &&
			std::get<RngReplayStep>(var1).start_index == std::get<RngReplayStep>(var2).start_index &&
			std::get<RngReplayStep>(var1).end_index == std::get<RngReplayStep>(var2).end_index;
	case TypeKeyMap:
		return std::get<KeyMapReplayStep>(var1).button_keys == std::get<KeyMapReplayStep>(var2).button_keys;
	case TypeMouse:
		return std::get<MouseReplayStep>(var1).state == std::get<MouseReplayStep>(var2).state;
	case TypeState:
		return std::get<StateReplayStep>(var1).state_type == std::get<StateReplayStep>(var2).state_type &&
			std::get<StateReplayStep>(var1).value == std::get<StateReplayStep>(var2).value;
	case TypeMeta:
		return std::get<MetaReplayStep>(var1).key == std::get<MetaReplayStep>(var2).key &&
			std::get<MetaReplayStep>(var1).value == std::get<MetaReplayStep>(var2).value;
	}

	return false;
}

static std::filesystem::path get_file_path(std::string suffix)
{
	return output_dir / (rs.replay_path.filename().string() + suffix);
}

// This is for detecting keyboard input during replaying,
// and prompting the user if they wish to end the replay.
static int keyboard_intercept(int c)
{
	int k = c >> 8;
	if (!is_system_key(k))
		rs.did_attempt_input_during_replay = true;

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
	// Reused scratch buffer to convert to a known depth before hashing. Recreate it if
	// the source dimensions change, otherwise the blit would clip and we'd silently hash
	// a stale region.
	static BITMAP* true_bitmap;
	if (true_bitmap && (true_bitmap->w != bitmap->w || true_bitmap->h != bitmap->h))
	{
		destroy_bitmap(true_bitmap);
		true_bitmap = nullptr;
	}
	if (!true_bitmap)
		true_bitmap = create_bitmap_ex(24, bitmap->w, bitmap->h);

	blit(bitmap, true_bitmap, 0, 0, 0, 0, bitmap->w, bitmap->h);
	int depth = bitmap_color_depth(true_bitmap);
	size_t len = (size_t)true_bitmap->w * true_bitmap->h * BYTES_PER_PIXEL(depth);
	return XXH32(true_bitmap->dat, len, 0);
}

static void do_recording_poll()
{
	rs.gfx_got_mismatch = false;

	if (rs.debug && !Quit)
	{
		if (!screenscrolling)
		{
			int x = HeroX().getInt();
			int y = HeroY().getInt();
			if (x != rs.prev_debug_x || y != rs.prev_debug_y)
			{
				replay_step_comment(fmt::format("h {:x} {:x}", x, y));
				rs.prev_debug_x = x;
				rs.prev_debug_y = y;
			}
		}

		uint32_t hash = hash_bitmap(framebuf);
		replay_step_gfx(hash);
	}

	if (rs.version >= 5)
	{
		KeyMapReplayStep new_key_map = KeyMapReplayStep::make(rs.frame_count);
		if (new_key_map.button_keys != KeyMapReplayStep::current.button_keys)
		{
			KeyMapReplayStep::current = new_key_map;
			record_log.emplace_back(std::in_place_type<KeyMapReplayStep>, rs.frame_count, new_key_map.button_keys);
		}
	}

	for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
	{
		bool state = raw_control_state[i];
		if (state == rs.previous_control_state[i])
			continue;

		int key_index = KeyMapReplayStep::current.button_keys[i];
		record_log.emplace_back(std::in_place_type<KeyReplayStep>, rs.frame_count, state ? TypeKeyDown : TypeKeyUp, i, key_index);
		rs.previous_control_state[i] = state;
	}

	if (rs.version >= 5)
	{
		for (int i = 1; i < KEY_MAX; i++)
		{
			char state = key_current_frame[i];
			if (state == rs.previous_keys[i] || i == KEY_ESC)
				continue;
			if (KeyMapReplayStep::current.find_button_index_for_key(i) != -1)
				continue;

			record_log.emplace_back(std::in_place_type<KeyReplayStep>, rs.frame_count, state ? TypeKeyDown : TypeKeyUp, -1, i);
			rs.previous_keys[i] = state;
		}
	}

	if (rs.current_mouse_state != rs.prev_mouse_state)
	{
		record_log.emplace_back(std::in_place_type<MouseReplayStep>, rs.frame_count, rs.current_mouse_state);
		rs.prev_mouse_state = rs.current_mouse_state;
	}
}

static bool set_version()
{
	rs.version_use_latest = false;
	if (!rs.meta_map.contains("version"))
	{
		rs.version = 1;
		rs.initial_version = rs.version;
		return true;
	}

	std::string version_str = rs.meta_map.at("version");
	if (version_str == "latest")
	{
		rs.version = VERSION;
		rs.version_use_latest = true;
		rs.initial_version = rs.version;
		return true;
	}

	if (!try_parse_int(version_str, rs.version))
		return false;
	rs.initial_version = rs.version;
	return true;
}

// Returns false if the file could not be opened or was malformed in a way that
// prevents loading. Deeper structural corruption is still caught by CHECK.
static bool load_replay(std::string& buffer, std::map<std::string, std::string>& meta, std::filesystem::path path, bool only_meta = false)
{
    auto start = std::chrono::steady_clock::now();

    meta.clear();

#ifdef __EMSCRIPTEN__
    if (em_is_lazy_file(path))
    {
        em_fetch_file(path);
    }
#endif

    // Read the entire file into memory at once.
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        fprintf(stderr, "could not open replay file: %s\n", path.string().c_str());
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer.assign(size, '\0');
    if (size > 0)
        file.read(buffer.data(), size);

    file.close();

    KeyMapReplayStep key_map = KeyMapReplayStep::make(0);
    bool found_key_map = false;
	// Replays files start with `M` metadata lines. For example: `M key value`.
	// When done with initial metadata, each line after is associated with a specific frame. For example: `C 123 ...`
	// `M` steps also exist that change metadata at runtime (like the replay version) - but these are associated
	// with specific frames. For example: `M 123 key value`.
    bool done_with_initial_meta = false;

    if (size > 0)
        replay_log.reserve(size / 10);

    auto consume_int = [](std::string_view& sv, int& val) -> bool
    {
        while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t'))
            sv.remove_prefix(1);

        if (sv.empty())
            return false;

        int sign = 1;
        if (sv.front() == '-')
        {
            sign = -1;
            sv.remove_prefix(1);
        }

        if (sv.empty() || sv.front() < '0' || sv.front() > '9')
            return false;

        val = 0;
        while (!sv.empty() && sv.front() >= '0' && sv.front() <= '9')
        {
            val = val * 10 + (sv.front() - '0');
            sv.remove_prefix(1);
        }
        val *= sign;
        return true;
    };

    auto consume_string = [](std::string_view& sv, std::string& val) -> bool
    {
        while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t'))
            sv.remove_prefix(1);

        if (sv.empty())
            return false;

        auto space = sv.find_first_of(" \t");
        if (space == std::string_view::npos)
        {
            val = std::string(sv);
            sv = {};
        }
        else
        {
            val = std::string(sv.substr(0, space));
            sv.remove_prefix(space);
        }
        return true;
    };

    std::string_view file_view(buffer);

    while (!file_view.empty())
    {
        auto newline_pos = file_view.find('\n');
        std::string_view line = file_view.substr(0, newline_pos);

        if (newline_pos != std::string_view::npos)
            file_view.remove_prefix(newline_pos + 1);
        else
            file_view = {};

        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);

        if (line.empty())
            continue;

        // First find '\xC2' instead of the 2-byte std::string ANNOTATION_MARKER.
        auto annotation_pos = line.find('\xC2');
        if (annotation_pos != std::string_view::npos && annotation_pos + 1 < line.size() && line[annotation_pos + 1] == (char)0xAB)
            line = line.substr(0, annotation_pos);

        if (!line.empty() && line.front() == ' ')
            line.remove_prefix(1);
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
            line.remove_suffix(1);

        if (line.empty())
            continue;

        char type = line.front();
        line.remove_prefix(1);

        int frame = 0;
        if (type != TypeMeta || done_with_initial_meta)
            consume_int(line, frame);

        if (!done_with_initial_meta && type != TypeMeta)
        {
            if (only_meta)
                return true;

            done_with_initial_meta = true;
            if (!set_version())
            {
                fprintf(stderr, "invalid version in replay file: %s\n", path.string().c_str());
                return false;
            }
            if (rs.version < 5)
                KeyMapReplayStep::current = key_map;
        }

        if (type == TypeMeta)
        {
            std::string key;
            consume_string(line, key);

            if (!line.empty() && line.front() == ' ')
                line.remove_prefix(1);

            std::string value(line);

            if (!done_with_initial_meta)
            {
                CHECK(meta.find(key) == meta.end());
                CHECK(annotation_pos == std::string_view::npos);
                meta[key] = value;
            }
            else
            {
                rs.string_arena.push_back(key);
                std::string_view key_view = rs.string_arena.back();
                rs.string_arena.push_back(value);
                std::string_view value_view = rs.string_arena.back();
                replay_log.emplace_back(std::in_place_type<MetaReplayStep>, frame, key_view, value_view);
            }
        }
        else if (type == TypeComment)
        {
            if (!line.empty() && line.front() == ' ')
                line.remove_prefix(1);

            replay_log.emplace_back(std::in_place_type<CommentReplayStep>, frame, line);
        }
        else if (type == TypeQuit)
        {
            int quit_state;
            consume_int(line, quit_state);
            replay_log.emplace_back(std::in_place_type<QuitReplayStep>, frame, quit_state);
        }
        else if (type == TypeCheat)
        {
            Cheat cheat;
            int arg1 = -1, arg2 = -1;
            std::string_view arg3;

            std::string cheat_name;
            consume_string(line, cheat_name);
            cheat = cheat_from_string(cheat_name);
            CHECK(cheat > Cheat::None && cheat < Cheat::Last);

            if (cheat == Cheat::HeroData)
            {
                if (!line.empty() && line.front() == ' ')
                    line.remove_prefix(1);
                arg3 = line;
            }
            else
            {
                if (!consume_int(line, arg1))
                    arg1 = -1;
                if (!consume_int(line, arg2))
                    arg2 = -1;
            }
            replay_log.emplace_back(std::in_place_type<CheatReplayStep>, frame, (Cheat)cheat, arg1, arg2, arg3);
        }
        else if (type == TypeQR)
        {
            int qr;
            int value_int = 0;

            consume_int(line, qr);
            consume_int(line, value_int);

            CHECK(qr >= 0 && qr < qr_MAX);

            replay_log.emplace_back(std::in_place_type<QRReplayStep>, frame, qr, value_int != 0);
        }
        else if (type == TypeRng)
        {
            int start_index, end_index, seed;
            consume_int(line, start_index);
            consume_int(line, end_index);
            consume_int(line, seed);
            CHECK(start_index <= end_index);
            replay_log.emplace_back(std::in_place_type<RngReplayStep>, frame, start_index, end_index, seed);
        }
        else if (type == TypeKeyMap)
        {
            CHECK(rs.version >= 5);
            std::array<int, KeyMapReplayStep::NumButtons> keys;
            for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
                consume_int(line, keys[i]);

            replay_log.emplace_back(std::in_place_type<KeyMapReplayStep>, frame, keys);
            if (!found_key_map)
            {
                KeyMapReplayStep::current = KeyMapReplayStep(frame, keys);
                found_key_map = true;
            }
            key_map = KeyMapReplayStep(frame, keys);
        }
        else if (type == TypeKeyUp || type == TypeKeyDown)
        {
            if (rs.version >= 5)
            {
                CHECK(found_key_map);
            }

            if (!line.empty() && line.front() == ' ')
                line.remove_prefix(1);

            std::string_view text = line;

            int button_index;
            int key_index;
            if (text.rfind("k ", 0) == 0)
            {
                button_index = -1;
                key_index = KeyReplayStep::find_index_for_key_name(text.substr(2));
                if (key_index == -1)
                    fprintf(stderr, "unknown key %.*s\n", (int)text.size() - 2, text.data() + 2);
                CHECK(key_index != -1);
            }
            else
            {
                button_index = KeyReplayStep::find_index_for_button_name(text);
                if (button_index == -1)
                    fprintf(stderr, "unknown button %.*s\n", (int)text.size(), text.data());
                CHECK(button_index != -1);
                key_index = key_map.button_keys[button_index];
            }

            replay_log.emplace_back(std::in_place_type<KeyReplayStep>, frame, type, button_index, key_index);
        }
        else if (type == TypeMouse)
        {
            int x, y, z = 0, b = 0;
            consume_int(line, x);
            consume_int(line, y);
            consume_int(line, z);
            consume_int(line, b);

            std::array<int, 4> state = {x, y, z, b};
            replay_log.emplace_back(std::in_place_type<MouseReplayStep>, frame, state);
        }
        else if (type == TypeState)
        {
            int state_type, value;
            consume_int(line, state_type);
            consume_int(line, value);
            replay_log.emplace_back(std::in_place_type<StateReplayStep>, frame, (ReplayStateType)state_type, value);
        }

        if (rs.frame_arg != -1 && replay_log.size() && get_frame(replay_log.back()) > rs.frame_arg)
        {
            replay_log.pop_back();
            break;
        }
    }

    rs.replay_log_current_index = 0;
    rs.replay_log_current_quit_index = 0;
    rs.replay_log_current_state_index = 0;
    if (!set_version())
    {
        fprintf(stderr, "invalid version in replay file: %s\n", path.string().c_str());
        return false;
    }
    rs.debug = replay_get_meta_bool("debug");
    rs.sync_rng = replay_get_meta_bool("sync_rng");

    if (rs.mode == ReplayMode::Assert || rs.mode == ReplayMode::Update)
    {
        rs.expected_loadscr_frame_count.clear();
        for (auto& step : replay_log)
        {
            if (get_type(step) != TypeComment) continue;

            auto& comment_step = std::get<CommentReplayStep>(step);
            if (comment_step.comment.rfind("scr=", 0) == 0)
                rs.expected_loadscr_frame_count.push_back(get_frame(step));
        }
    }

    int32_t load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    zprint2("Time to load replay: %d ms\n", load_ms);
    return true;
}

static void save_replay(std::string filename, const std::vector<ReplayStepVariant> &log)
{
    // Compare addresses since record_log and replay_log are statically assigned via std::move if roundtrip checking applies
    bool insert_baseline_annotations =
        rs.has_assert_failed &&
        filename.find(".roundtrip") != std::string::npos &&
        &log == &record_log;

    std::time_t ct = std::time(0);
    replay_set_meta("time_updated", strtok(ctime(&ct), "\n"));
    replay_set_meta("zc_version_updated", getVersionString());
    if (rs.version_use_latest)
        replay_set_meta("version", "latest");
    else
        replay_set_meta("version", rs.initial_version);
    replay_set_meta("frames", rs.frame_count);
	replay_set_meta("length", log.size());

    std::ofstream out(filename, std::ios::binary);
    for (auto it : rs.meta_map)
        out << fmt::format("{} {} {}", TypeMeta, it.first, it.second) << '\n';
    for (size_t i = 0; i < log.size(); i++)
    {
        auto& step = log[i];
        std::string step_as_string = print_step(step);
        out << step_as_string;
        if (insert_baseline_annotations && i < replay_log.size())
        {
            auto& replay_log_step = replay_log[i];
            if (!steps_are_equal(step, replay_log_step))
                out << std::string(std::max(0, 60 - (int)step_as_string.size()), ' ')
                    << ANNOTATION_MARKER
                    << ' '
                    << print_step(replay_log_step);
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
            auto& replay_log_step = replay_log[i];
            out << std::string(60, ' ')
                << ANNOTATION_MARKER
                << ' '
                << print_step(replay_log_step)
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
		int second = it.second == -1 ? rs.frame_count : it.second;
		if (first != second)
			parts.push_back(fmt::format("{}-{}", first, second));
		else
			parts.push_back(fmt::format("{}", first));
	}
	return fmt::format("{}", fmt::join(parts, " "));
}

static void save_result(bool stopped = false, bool changed = false)
{
	rs.time_result_saved = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = rs.time_result_saved - rs.time_started;
	int elapsed_ms = elapsed.count();
	std::time_t time_started_c = std::chrono::system_clock::to_time_t(rs.time_started_system);
	int fps = elapsed_ms ? (double)rs.frame_count / elapsed_ms * 1000 : 0;

	std::ofstream out(get_file_path(".result.txt"), std::ios::binary);

	out << fmt::format("replay: {}", rs.replay_path.string()) << '\n';
	out << fmt::format("mode: {}", replay_mode_to_string(rs.mode)) << '\n';
	out << fmt::format("time: {}", strtok(ctime(&time_started_c), "\n")) << '\n';
	out << fmt::format("duration: {}", elapsed_ms) << '\n';
	out << fmt::format("zc_version: {}", getVersionString()) << '\n';
	if (replay_is_replaying() && !replay_log.empty())
		out << fmt::format("replay_log_frames: {}", get_frame(replay_log.back())) << '\n';
	if (replay_is_recording() && !record_log.empty())
		out << fmt::format("record_log_frames: {}", get_frame(record_log.back())) << '\n';
	out << fmt::format("frame: {}", rs.frame_count) << '\n';
	out << fmt::format("fps: {}", fps) << '\n';
	if (stopped || rs.has_assert_failed)
	{
		bool success = stopped && !rs.has_aborted && !rs.has_assert_failed && !rs.has_rng_desynced && rs.failed_loadscr_count_frame == -1;
		out << fmt::format("success: {}", success) << '\n';
	}
	if (rs.has_rng_desynced)
		out << fmt::format("rng_desync: {}", rs.has_rng_desynced) << '\n';
	if (rs.has_assert_failed)
		out << fmt::format("failing_frame: {}", rs.failing_frame) << '\n';
	if (rs.mode == ReplayMode::Update)
		out << fmt::format("changed: {}", changed) << '\n';
	if (rs.has_assert_failed && !rs.unexpected_gfx_frames.empty())
		out << fmt::format("unexpected_gfx_frames: {}", fmt::join(rs.unexpected_gfx_frames, ", ")) << '\n';
	if (rs.has_assert_failed && !rs.unexpected_gfx_segments.empty())
		out << fmt::format("unexpected_gfx_segments: {}", segments_to_string(rs.unexpected_gfx_segments)) << '\n';
	if (rs.has_assert_failed && !rs.unexpected_gfx_segments_limited.empty())
		out << fmt::format("unexpected_gfx_segments_limited: {}", segments_to_string(rs.unexpected_gfx_segments_limited)) << '\n';
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
		rs.unexpected_gfx_frames.push_back(frame);
}

static void save_history_snapshots()
{
	for (auto& entry : rs.framebuf_history)
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
    while (rs.replay_log_current_index < replay_log.size() && get_frame(replay_log[rs.replay_log_current_index]) == rs.frame_count)
    {
        if (rs.version >= 6)
        {
            char type = get_type(replay_log[rs.replay_log_current_index]);
            if (type != TypeKeyDown && type != TypeKeyUp && type != TypeMouse && type != TypeMeta)
                run_step(replay_log[rs.replay_log_current_index]);
        }
        else
        {
            run_step(replay_log[rs.replay_log_current_index]);
        }
        rs.replay_log_current_index += 1;
    }

    rs.replay_log_current_quit_index = rs.replay_log_current_index;
    rs.replay_log_current_state_index = rs.replay_log_current_index;
}

static void check_assert()
{
    // Only print the very first difference. When replay_stop is called,
    // the program will exit with a status code based on this bool. If
    // asserts have failed, a ".zplay.roundtrip" file will be written
    // for comparison.
    if (rs.has_assert_failed)
        return;

    while (rs.assert_current_index < replay_log.size() && get_frame(replay_log[rs.assert_current_index]) <= rs.frame_count)
    {
        if (rs.assert_current_index >= record_log.size())
            break;

        auto& replay_step = replay_log[rs.assert_current_index];
        auto& record_step = record_log[rs.assert_current_index];
        if (!steps_are_equal(replay_step, record_step))
        {
            rs.has_assert_failed = true;
            rs.failing_frame = get_frame(record_step);
            int line_number = rs.assert_current_index + rs.meta_map.size() + 1;
            std::string error = fmt::format("<{}> expected:\n\t{}\nbut got:\n\t{}", line_number,
                                            print_step(replay_step), print_step(record_step));
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
                InfoDialog("Assert", error).show();
                exit_sys_pal();
            }
            break;
        }

        rs.assert_current_index++;
    }
}

static void maybe_take_snapshot()
{
	if (rs.mode == ReplayMode::Assert)
	{
		if (rs.current_failing_gfx_segment_start_frame != -1)
		{
			// Limit how many snapshots are saved in the same gfx failure segment.
			if (rs.frame_count - rs.current_failing_gfx_segment_start_frame > 60*5)
			{
				if (rs.unexpected_gfx_segments_limited.back().second == -1)
					rs.unexpected_gfx_segments_limited.back().second = rs.frame_count - 1;
			}
			// Save when our screen changed, but also when the baseline's screen
			// was expected to change - so a static-but-wrong screen still
			// produces frames the compare report can pair against the baseline.
			// The segment's first frame always saves.
			else if (!rs.prev_gfx_hash_was_same || rs.gfx_expected_changed
				|| rs.current_failing_gfx_segment_start_frame == rs.frame_count)
				save_snapshot(framebuf, RAMpal, rs.frame_count, rs.gfx_got_mismatch);
			return;
		}
		else if (rs.last_failing_gfx_frame != -1 && rs.frame_count - rs.last_failing_gfx_frame <= 10)
		{
			// Save a few frames after the last failing gfx, for context.
			if (!rs.prev_gfx_hash_was_same || rs.gfx_expected_changed)
				save_snapshot(framebuf, RAMpal, rs.frame_count, rs.gfx_got_mismatch);
			return;
		}
	}

	auto it = std::find(rs.snapshot_frames.begin(), rs.snapshot_frames.end(), rs.frame_count);
	if (!snapshot_all_frames && !rs.gfx_got_mismatch && it == rs.snapshot_frames.end())
	{
		if (rs.mode == ReplayMode::Assert && !rs.prev_gfx_hash_was_same)
		{
			blit(framebuf, rs.framebuf_history[rs.framebuf_history_index].bitmap, 0, 0, 0, 0, framebuf->w, framebuf->h);
			rs.framebuf_history[rs.framebuf_history_index].frame = rs.frame_count;
			memcpy(rs.framebuf_history[rs.framebuf_history_index].pal, RAMpal, PAL_SIZE*sizeof(RGB));
			rs.framebuf_history_index = (rs.framebuf_history_index + 1) % rs.framebuf_history.size();
		}
		return;
	}

	save_snapshot(framebuf, RAMpal, rs.frame_count, rs.gfx_got_mismatch);
}

static void fail_replay(std::string error)
{
	int line_number = rs.replay_log_current_index + rs.meta_map.size() + 1;
	std::string err_out = fmt::format("<{}> {}! stopping replay\nframe {}", line_number, error, rs.frame_count);
	fprintf(stderr, "%s\n", err_out.c_str());

	if (rs.failing_frame == -1)
	{
		rs.failing_frame = rs.frame_count;
	}

	if (!exit_when_done)
	{
		enter_sys_pal();
		InfoDialog(replay_mode_to_string(rs.mode), err_out).show();
		exit_sys_pal();
	}

	save_history_snapshots();
	replay_stop();
}

std::string replay_mode_to_string(ReplayMode m)
{
	switch (m)
	{
		case ReplayMode::Off: return "off";
		case ReplayMode::Replay: return "replay";
		case ReplayMode::Record: return "record";
		case ReplayMode::Assert: return "assert";
		case ReplayMode::Update: return "update";
	}
	return "unknown";
}

std::map<std::string, std::string> replay_load_meta(std::filesystem::path path)
{
	std::string buffer;
	std::map<std::string, std::string> meta;
	bool only_meta = true;
	load_replay(buffer, meta, path, only_meta);
	return meta;
}

static void peek_meta_steps()
{
	if (!replay_is_replaying())
		return;

	size_t i = rs.replay_log_current_index;
	while (i < replay_log.size() && get_frame(replay_log[i]) == rs.frame_count)
	{
		if (get_type(replay_log[i]) == TypeMeta)
		{
			auto& meta_step = std::get<MetaReplayStep>(replay_log[i]);
			if (replay_is_recording())
				replay_step_meta(std::string(meta_step.key), std::string(meta_step.value));
			else
				run_step(replay_log[i]);
		}
		i++;
	}
}

bool replay_start(ReplayMode mode_, std::filesystem::path path, int frame)
{
    ASSERT(rs.mode == ReplayMode::Off);
    rs = ReplayState{};
    rs.mode = mode_;
    rs.time_started = std::chrono::steady_clock::now();
    rs.time_started_system = std::chrono::system_clock::now();
    rs.replay_path = path;
    rs.frame_arg = frame;
    refresh_control_scheme();
    if (output_dir.empty()) output_dir = rs.replay_path.parent_path();
    // replay_log/record_log live outside ReplayState, so clear them explicitly.
    replay_log.clear();
    record_log.clear();
    replay_forget_input();

    switch (rs.mode)
    {
    case ReplayMode::Off:
        return false;
    case ReplayMode::Record:
    {
        rs.version = VERSION;
		rs.initial_version = VERSION;
        std::time_t ct = std::time(0);
        replay_set_meta("time_created", strtok(ctime(&ct), "\n"));
        replay_set_meta("zc_version_created", getVersionString());
        KeyMapReplayStep::current = KeyMapReplayStep::make(0);
        break;
    }
    case ReplayMode::Replay:
    case ReplayMode::Assert:
    case ReplayMode::Update:
        if (!load_replay(rs.replay_file_buffer, rs.meta_map, rs.replay_path))
        {
            rs.mode = ReplayMode::Off;
            return false;
        }
        break;
    }

    if (rs.mode == ReplayMode::Assert)
    {
        for (int i = 0; i < rs.framebuf_history.size(); i++)
        {
            rs.framebuf_history[i].bitmap = create_bitmap_ex(8, framebuf->w, framebuf->h);
            rs.framebuf_history[i].frame = -1;
        }
    }

    if (replay_is_replaying())
    {
        ASSERT(!keyboard_callback);
        install_keyboard_handlers();
        KeyMapReplayStep::stored = KeyMapReplayStep::make(0);
    }

    if (replay_is_recording() && rs.version >= 5)
    {
        record_log.emplace_back(std::in_place_type<KeyMapReplayStep>, 0, KeyMapReplayStep::current.button_keys);
    }

    peek_meta_steps();

    save_result();

    replay_compat_setup_zc_maths();
    return true;
}

void replay_continue(std::filesystem::path path)
{
    ASSERT(rs.mode == ReplayMode::Off);
    rs = ReplayState{};
    rs.mode = ReplayMode::Record;
    rs.frame_arg = -1;
    rs.replay_path = path;
    replay_forget_input();
    if (!load_replay(rs.replay_file_buffer, rs.meta_map, rs.replay_path) || replay_log.empty())
    {
        rs.mode = ReplayMode::Off;
        return;
    }

	for (auto& step : replay_log)
	{
		if (get_type(step) == TypeMeta)
			run_step(step);
	}

    record_log = std::move(replay_log);
    rs.frame_count = get_frame(record_log.back()) + 1;

	// Not certain if really old replays (prior to TypeKeyMap) can be updated.
	if (rs.version >= 5 && rs.version != VERSION)
		replay_step_meta("version", fmt::format("{}", VERSION));

	replay_compat_setup_zc_maths();
}

void replay_poll()
{
    if (rs.mode == ReplayMode::Off)
        return;

    if (rs.did_attempt_input_during_replay && replay_is_replaying())
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
		if (alert_confirm("Replay", "Would you like to halt the replay and take back control?"))
		{
			replay_quit();
			exit_sys_pal();
			return;
		}
		exit_sys_pal();

		// exit_sys_pal only schedules the palette swap for the next render, but the
		// gfx hash below converts the frame via the currently applied palette - so
		// apply it now, else this frame hashes with the system palette.
		if (hw_palette)
		{
			zc_set_palette(*hw_palette);
			update_hw_pal = false;
		}

        rs.did_attempt_input_during_replay = false;
        install_keyboard_handlers();
        for (int i = 0; i < KEY_MAX; i++)
        {
            _key[i] = key_copy[i];
            key[i] = key_copy[i];
        }
        for (int i = 0; i < controls::btnLast; i++)
            down_control_states[i] = down_states[i];
    }

    if (rs.frame_arg != -1 && rs.frame_arg <= rs.frame_count && replay_is_replaying())
    {
        if (rs.mode != ReplayMode::Assert)
        {
            replay_forget_input();
            replay_stop();
            if (!is_headless())
            {
                Throttlefps = true;
                Paused = true;
                enter_sys_pal();
                InfoDialog("Recording", "Replaying stopped at requested frame").show();
                exit_sys_pal();
            }
        }
    }

    switch (rs.mode)
    {
    case ReplayMode::Off:
        return;
    case ReplayMode::Record:
        do_recording_poll();
        break;
    case ReplayMode::Replay:
        do_replaying_poll();
        if (rs.replay_log_current_index == replay_log.size())
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
        if (rs.frame_count == get_frame(replay_log.back()))
            replay_stop();
        break;
    }

    if (rs.mode == ReplayMode::Assert && rs.gfx_got_mismatch)
    {
        save_history_snapshots();
    }
    maybe_take_snapshot();

    static bool save_every_frame = get_flag_bool("-replay-save-result-every-frame").value_or(false);
    bool should_save_result = save_every_frame
        || rs.frame_count == 0
        || std::chrono::steady_clock::now() - rs.time_result_saved > 1s;
    if (should_save_result)
        save_result();

    if (rs.mode == ReplayMode::Assert || rs.mode == ReplayMode::Record)
    {
        if (rs.frame_arg != -1 && rs.frame_arg == rs.frame_count)
        {
            if (rs.mode == ReplayMode::Record) replay_save();

            replay_forget_input();
            replay_stop();
            if (!is_headless())
            {
                Throttlefps = true;
                Paused = true;
                enter_sys_pal();
                InfoDialog(replay_mode_to_string(rs.mode), "Stopped at requested frame").show();
                exit_sys_pal();
            }
            return;
        }
    }

    if (rs.mode == ReplayMode::Assert)
    {
        if (rs.replay_log_current_index == rs.assert_current_index && rs.assert_current_index == replay_log.size())
        {
            replay_stop();
            return;
        }

        if (rs.frame_count > get_frame(replay_log.back()))
        {
            replay_stop();
            return;
        }

        // If loadscr is not called when expected, the rest of the replay is not going to go well and may possible
        // loop forever. Only record 60 more frames at this point.
        if (rs.has_assert_failed && rs.failed_loadscr_count_frame == -1)
        {
            int expected_frame = rs.loadscr_count + 1 >= rs.expected_loadscr_frame_count.size() ?
                get_frame(replay_log.back()) :
                rs.expected_loadscr_frame_count[rs.loadscr_count + 1];
            if (rs.frame_count > expected_frame)
                rs.failed_loadscr_count_frame = rs.frame_count;
        }
        if (rs.has_assert_failed && rs.failed_loadscr_count_frame != -1 && rs.frame_count - rs.failed_loadscr_count_frame > 60)
        {
            replay_stop();
            return;
        }
    }

    // Updating should never modify when screens are loaded.
    if (rs.mode == ReplayMode::Update)
    {
        int expected_frame = rs.loadscr_count + 1 >= rs.expected_loadscr_frame_count.size() ?
            get_frame(replay_log.back()) :
            rs.expected_loadscr_frame_count[rs.loadscr_count + 1];
        if (rs.frame_count > expected_frame)
        {
            rs.failed_loadscr_count_frame = rs.frame_count;
            replay_stop();
            return;
        }
    }

    std::fill(rs.rng_seed_count_this_frame.begin(), rs.rng_seed_count_this_frame.end(), 0);
    rs.frame_count++;
	peek_meta_steps();
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

	rs.snapshot_frames.insert(rs.snapshot_frames.end(), frames.begin(), frames.end());
	return true;
}

void replay_peek_quit()
{
    int i = rs.replay_log_current_quit_index;
    while (i < replay_log.size() && get_frame(replay_log[i]) == rs.frame_count)
    {
        if (get_type(replay_log[i]) == TypeQuit)
        {
            auto& quit_replay_step = std::get<QuitReplayStep>(replay_log[i]);
            if (quit_replay_step.quit_state == 0)
                GameFlags |= GAMEFLAG_TRYQUIT;
            else
                Quit = quit_replay_step.quit_state;
            rs.replay_log_current_quit_index = i + 1;
            break;
        }
        i++;
    }
}

void replay_peek_input()
{
    size_t i = rs.replay_log_current_index;
    while (i < replay_log.size() && get_frame(replay_log[i]) == rs.frame_count)
    {
        char type = get_type(replay_log[i]);
        if (type == TypeKeyDown || type == TypeKeyUp || type == TypeMouse)
        {
            run_step(replay_log[i]);
        }
        i++;
    }
}

void replay_do_cheats()
{
    size_t i = rs.replay_log_current_index;
    while (i < replay_log.size() && get_frame(replay_log[i]) == rs.frame_count)
    {
        if (get_type(replay_log[i]) == TypeCheat)
        {
            auto& cheat_replay_step = std::get<CheatReplayStep>(replay_log[i]);
            cheats_enqueue(cheat_replay_step.cheat, cheat_replay_step.arg1, cheat_replay_step.arg2, std::string(cheat_replay_step.arg3));
        }
        i++;
    }
}

void replay_do_qrs()
{
    size_t i = rs.replay_log_current_index;
    while (i < replay_log.size() && get_frame(replay_log[i]) == rs.frame_count)
    {
        if (get_type(replay_log[i]) == TypeQR)
        {
            auto& qr_replay_step = std::get<QRReplayStep>(replay_log[i]);
            enqueue_qr_change(qr_replay_step.qr, qr_replay_step.value);
        }
        i++;
    }
}

bool replay_is_assert_done()
{
	return rs.mode == ReplayMode::Assert && (rs.assert_current_index == replay_log.size() || rs.frame_count >= get_frame(replay_log.back()));
}

void replay_forget_input()
{
    if (rs.mode == ReplayMode::Off)
        return;

    for (int i = 0; i < KEY_MAX; i++)
    {
        _key[i] = 0;
        key[i] = 0;
        key_current_frame[i] = 0;
    }
    for (int i = 0; i < KeyMapReplayStep::NumButtons; i++)
        rs.previous_control_state[i] = raw_control_state[i] = false;
    for (int i = 0; i < KEY_MAX; i++)
        rs.previous_keys[i] = false;
}

void replay_stop(bool aborted)
{
    if (rs.mode == ReplayMode::Off)
        return;

	rs.has_aborted = aborted;

    if (replay_is_replaying())
    {
        keyboard_callback = nullptr;
        replay_forget_input();
        KeyMapReplayStep::stored.run();
    }

    if (rs.mode == ReplayMode::Assert && !aborted)
    {
        check_assert();
        bool log_size_mismatch = replay_log.size() != record_log.size();
        rs.has_assert_failed |= log_size_mismatch || rs.has_rng_desynced;
        if (rs.has_assert_failed)
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
			rs.mode = ReplayMode::Off;
			zc_exit(rs.has_assert_failed ? ASSERT_FAILED_EXIT_CODE : 0);
        }
        else if (rs.has_assert_failed && !batch_when_done)
        {
            enter_sys_pal();
            InfoDialog("Assert", "Replay has stopped, and the assert failed.").show();
            exit_sys_pal();
			Paused = true;
        }
    }

    if (rs.mode == ReplayMode::Update && !aborted)
    {
        bool should_save;
        if (rs.failed_loadscr_count_frame != -1)
        {
            should_save = false;

            if (exit_when_done)
            {
                save_result(true);
                rs.mode = ReplayMode::Off;
                zc_exit(ASSERT_FAILED_EXIT_CODE);
            }
            else
            {
                enter_sys_pal();
                InfoDialog("Update", "Failed to update replay as there was a non-graphical change.").show();
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
                if (!steps_are_equal(replay_log[i], record_log[i]))
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

    for (int i = 0; i < rs.framebuf_history.size(); i++)
    {
        if (rs.framebuf_history[i].bitmap)
		{
            destroy_bitmap(rs.framebuf_history[i].bitmap);
			rs.framebuf_history[i].bitmap = nullptr;
		}
    }

    rs.mode = ReplayMode::Off;
	refresh_control_scheme();
	replay_compat_setup_zc_maths();
    rs.frame_count = 0;
    replay_log.clear();
    rs.rngs.clear();
    rs.meta_map.clear();
    rs.replay_file_buffer.clear();
    rs.string_arena.clear();

	if (exit_when_done)
	{
		zc_exit(rs.has_rng_desynced ? 1 : 0);
	}
	else if (batch_when_done && !aborted)
	{
		batch_any_failed |= rs.has_assert_failed || rs.has_rng_desynced;
		if (!replay_batch_load_next())
			replay_batch_exit();
	}
}

void replay_quit()
{
    bool aborted = true;
    replay_stop(aborted);
}

void replay_save()
{
    replay_save(rs.replay_path);
}

void replay_save(std::filesystem::path path)
{
    save_replay(path.string(), record_log);
}

void replay_step_comment(std::string comment)
{
    if (replay_is_recording())
    {
        util::trimstr_trailing(comment);
        rs.string_arena.push_back(std::move(comment));
        record_log.emplace_back(std::in_place_type<CommentReplayStep>, rs.frame_count, rs.string_arena.back());
        // Not necessary to call this here, but helps to halt the program exactly when an unexpected
        // comment occurs instead of at the next call to replay_poll.
        if (rs.mode == ReplayMode::Assert)
            check_assert();
    }
}

void replay_step_comment_loadscr(int screen)
{
	replay_step_comment(fmt::format("scr={}", screen));
	rs.loadscr_count += 1;
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

	if (rs.mode == ReplayMode::Assert)
	{
		std::string expected_gfx_comment;
		auto it_start = std::lower_bound(replay_log.begin(), replay_log.end(), rs.frame_count,
			[](auto& step, const int value) {
				return get_frame(step) < value;
			});
		for (auto it = it_start; it < replay_log.end(); it++)
		{
			if (get_frame(*it) != rs.frame_count)
				break;
			if (get_type(*it) == TypeComment)
			{
				auto& comment_step = std::get<CommentReplayStep>(*it);
				if (comment_step.comment.rfind("g ", 0) == 0)
				{
					expected_gfx_comment = comment_step.comment;
					break;
				}
			}
		}

		rs.gfx_expected_changed = !expected_gfx_comment.empty();

		if (expected_gfx_comment.empty() && gfx_hash != rs.prev_gfx_hash)
			rs.gfx_got_mismatch = true;
		if (!expected_gfx_comment.empty() && expected_gfx_comment != gfx_comment)
			rs.gfx_got_mismatch = true;
		if (rs.gfx_got_mismatch)
			rs.last_failing_gfx_frame = rs.frame_count;
		// A change on either side can start or end a failure segment. Checking
		// only our own hash would never even open a segment when the failure is
		// a static-but-wrong screen (frozen black) against a changing baseline.
		if (gfx_hash != rs.prev_gfx_hash || rs.gfx_expected_changed)
		{
			if (rs.gfx_got_mismatch && rs.current_failing_gfx_segment_start_frame == -1)
			{
				rs.current_failing_gfx_segment_start_frame = rs.frame_count;
				rs.unexpected_gfx_segments.push_back({rs.frame_count, -1});
				rs.unexpected_gfx_segments_limited.push_back({rs.frame_count, -1});
			}
			else if (!rs.gfx_got_mismatch && rs.current_failing_gfx_segment_start_frame != -1)
			{
				rs.current_failing_gfx_segment_start_frame = -1;
				if (!rs.unexpected_gfx_segments.empty())
					rs.unexpected_gfx_segments.back().second = rs.frame_count - 1;
				if (!rs.unexpected_gfx_segments_limited.empty() && rs.unexpected_gfx_segments_limited.back().second == -1)
					rs.unexpected_gfx_segments_limited.back().second = rs.frame_count - 1;
			}
		}
	}

	// Skip if last invocation was the same value.
	rs.prev_gfx_hash_was_same = gfx_hash == rs.prev_gfx_hash;
	if (!rs.prev_gfx_hash_was_same)
	{
		replay_step_comment(gfx_comment);
		rs.prev_gfx_hash = gfx_hash;
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

    rs.meta_map[key] = value;
}

void replay_set_meta(std::string key, int value)
{
    if (replay_is_active())
        rs.meta_map[key] = fmt::format("{}", value);
}

void replay_set_meta_bool(std::string key, bool value)
{
    if (value)
        replay_set_meta(key, "true");
    else
        rs.meta_map.erase(key);
}

void replay_delete_meta(std::string key)
{
	rs.meta_map.erase(key);
}

static std::string get_meta_raw_value(std::string key)
{
    auto it = rs.meta_map.find(key);
    if (it == rs.meta_map.end())
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
    int value = 0;
    try_parse_int(get_meta_raw_value(key), value);
    return value;
}

int replay_get_meta_int(std::string key, int defaultValue)
{
    int value = defaultValue;
    try_parse_int(get_meta_raw_value(key), value);
    return value;
}

bool replay_get_meta_bool(std::string key)
{
    return get_meta_raw_value(key) == "true";
}

bool replay_has_meta(std::string key)
{
    return rs.meta_map.find(key) != rs.meta_map.end();
}

void replay_step_quit(int quit_state)
{
	if (replay_is_recording())
		record_log.emplace_back(std::in_place_type<QuitReplayStep>, rs.frame_count, quit_state);
}

void replay_step_cheat(Cheat cheat, int arg1, int arg2, std::string arg3)
{
	if (replay_is_recording())
	{
		rs.string_arena.push_back(std::move(arg3));
		record_log.emplace_back(std::in_place_type<CheatReplayStep>, rs.frame_count, cheat, arg1, arg2, rs.string_arena.back());
	}
}

void replay_step_qr(int qr, bool value)
{
	if (replay_is_recording())
		record_log.emplace_back(std::in_place_type<QRReplayStep>, rs.frame_count, qr, value);
}

void replay_step_meta(std::string key, std::string value)
{
	if (!replay_is_recording())
		return;

	// Nothing else is supported right now.
	CHECK(key == "version");

	rs.string_arena.push_back(key);
	std::string_view key_view = rs.string_arena.back();
	rs.string_arena.push_back(value);
	std::string_view value_view = rs.string_arena.back();
	record_log.emplace_back(std::in_place_type<MetaReplayStep>, rs.frame_count, key_view, value_view);
	std::get<MetaReplayStep>(record_log.back()).run();
}

ReplayMode replay_get_mode()
{
    return rs.mode;
}

int replay_get_version()
{
    return rs.version;
}

ZCVersionCreated replay_get_zc_version_created()
{
	if (rs.zc_version_created.has_value())
		return *rs.zc_version_created;

	ZCVersionCreated result{};
	std::string str = replay_get_meta_str("zc_version_created");

	// zc_version_created was first added only after zc_version_updated, so fall back to that if not
	// present.
	if (str.empty())
		str = replay_get_meta_str("zc_version_updated");

	if (str.starts_with("2.55-alpha"))
	{
		result = {.well_formed = true, .major = 2, .minor = 55};
	}
	else if (!str.empty())
	{
		std::vector<std::string> parts;
		util::split(str, parts, '.');
		if (parts.size() >= 3)
		{
			errno = 0;
			int major = std::strtol(parts[0].data(), nullptr, 10);
			int minor = std::strtol(parts[1].data(), nullptr, 10);
			int patch = std::strtol(parts[2].data(), nullptr, 10);
			if (!errno)
				result = {.well_formed = true, .major = major, .minor = minor, .patch = patch};
		}
	}

	rs.zc_version_created = result;
	return result;
}

bool replay_version_check(int min, int max)
{
	if (!replay_is_active())
	{
		return max == -1;
	}

	if (max == -1)
	{
		return rs.version >= min;
	}

    return rs.version >= min && rs.version < max;
}

int replay_get_frame()
{
    return rs.frame_count;
}

void replay_set_output_dir(std::filesystem::path dir)
{
	output_dir = dir;
}

std::filesystem::path replay_get_replay_path()
{
    return rs.replay_path;
}

std::string replay_get_buttons_string()
{
    std::string text;
    text += fmt::format("{} ", rs.frame_count);
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
    return rs.mode != ReplayMode::Off;
}

void replay_set_debug(bool enable_debug)
{
    rs.debug = enable_debug;
    replay_set_meta_bool("debug", rs.debug);
}

bool replay_is_debug()
{
    return rs.mode != ReplayMode::Off && rs.debug;
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
    rs.sync_rng = enable;
    replay_set_meta_bool("sync_rng", rs.sync_rng);
}

bool replay_is_replaying()
{
    return rs.mode == ReplayMode::Replay || rs.mode == ReplayMode::Assert || rs.mode == ReplayMode::Update;
}

bool replay_is_recording()
{
    return rs.mode == ReplayMode::Record || rs.mode == ReplayMode::Assert || rs.mode == ReplayMode::Update;
}

void replay_enable_exit_when_done()
{
	exit_when_done = true;
}

void replay_enable_batch_when_done()
{
	batch_when_done = true;
}

bool replay_batch_any_failed()
{
	return batch_any_failed;
}

void replay_batch_note_failure()
{
	batch_any_failed = true;
}

void replay_batch_exit()
{
	zc_exit(batch_any_failed ? ASSERT_FAILED_EXIT_CODE : 0);
}

size_t replay_register_rng(zc_randgen *rng)
{
    if (std::find(rs.rngs.begin(), rs.rngs.end(), rng) != rs.rngs.end())
        return get_rng_index(rng);

    rs.rngs.push_back(rng);
    return rs.rngs.size() - 1;
}

void replay_set_rng_seed(zc_randgen *rng, int seed)
{
    ASSERT(rs.mode != ReplayMode::Off);

    int index = get_rng_index(rng);
    ASSERT(index != -1);

    if ((size_t)index >= rs.rng_seed_count_this_frame.size())
        rs.rng_seed_count_this_frame.resize(index + 1, 0);
    int seed_count = rs.rng_seed_count_this_frame[index] + 1;
    rs.rng_seed_count_this_frame[index] = seed_count;

    if (replay_is_replaying())
    {
        const RngReplayStep *rng_step = find_rng_step(index, rs.replay_log_current_index, replay_log, seed_count);
        if (rng_step)
        {
            seed = rng_step->seed;
        }
        // Only OK to be missing if in update mode.
        else if (rs.mode != ReplayMode::Update)
        {
            rs.has_rng_desynced = true;
            fail_replay("rng desync");
        }
    }

    if (replay_is_recording())
    {
        bool did_extend = false;
        if (!record_log.empty() && get_type(record_log.back()) == TypeRng && get_frame(record_log.back()) == rs.frame_count)
        {
            auto& rng_step = std::get<RngReplayStep>(record_log.back());
            if (rng_step.seed == seed)
            {
                if (rng_step.start_index == index + 1)
                {
                    rng_step.start_index = index;
                    did_extend = true;
                }
                if (rng_step.end_index == index - 1)
                {
                    rng_step.end_index = index;
                    did_extend = true;
                }
            }
        }

        if (!did_extend)
            record_log.emplace_back(std::in_place_type<RngReplayStep>, rs.frame_count, index, index, seed);
    }

    rng->seed(seed);
}

void replay_sync_rng()
{
    if (!rs.sync_rng)
        return;

    int seed = time(0);
    for (size_t i = 0; i < rs.rngs.size(); i++)
    {
        // Only reset the rs.rngs that haven't been updated this frame.
        if (i < rs.rng_seed_count_this_frame.size() && rs.rng_seed_count_this_frame[i] != 0)
            continue;

        replay_set_rng_seed(rs.rngs[i], seed);
        if (rs.mode == ReplayMode::Off)
            return;
    }

    global_frame = 0;
}

int replay_get_mouse(int index)
{
	return rs.current_mouse_state[index];
}

void replay_set_mouse(int index, int value)
{
	rs.current_mouse_state[index] = value;
}


int replay_get_state(ReplayStateType state_type, int fn())
{
	int value = 0;
	if (replay_is_replaying())
	{
		value = 0;
		while (rs.replay_log_current_state_index < replay_log.size() && get_frame(replay_log[rs.replay_log_current_state_index]) == rs.frame_count)
		{
			size_t i = rs.replay_log_current_state_index;
			rs.replay_log_current_state_index += 1;

			if (get_type(replay_log[i]) == TypeState)
			{
				auto& state_replay_step = std::get<StateReplayStep>(replay_log[i]);
				if (state_replay_step.state_type == state_type)
					value = state_replay_step.value;
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
		record_log.emplace_back(std::in_place_type<StateReplayStep>, rs.frame_count, state_type, value);
	}

	return value;
}
