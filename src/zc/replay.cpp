#include "replay.h"
#include "zc_sys.h"
#include "base/zc_alleg.h"
#include "zelda.h"
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <fmt/format.h>

struct ReplayStep;

static const int ASSERT_FAILED_EXIT_CODE = 120;

static const char TypeMeta = 'M';
static const char TypeButtonDown = 'D';
static const char TypeButtonUp = 'U';
static const char TypeComment = 'C';
static const char TypeQuit = 'Q';
static const char TypeCheat = 'X';
static const char TypeRng = 'R';

static ReplayMode mode = ReplayMode::Off;
static bool debug;
static bool sync_rng;
static int frame_arg;
static std::string filename;
static std::vector<std::shared_ptr<ReplayStep>> replay_log;
static std::vector<std::shared_ptr<ReplayStep>> record_log;
static std::map<std::string, std::string> meta_map;
static size_t replay_log_current_index;
static size_t assert_current_index;
static size_t manual_takeover_start_index;
static bool has_assert_failed;
static bool did_attempt_input_during_replay;
static int frame_count;
static bool previous_control_state[ZC_CONTROL_STATES];
static std::vector<zc_randgen *> rngs;
static uint32_t prev_gfx_hash;

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

struct ButtonReplayStep : public ReplayStep
{
    inline static int button_keys[ZC_CONTROL_STATES];

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

    static void load_keys()
    {
        // TODO: block changing controls during replay
        button_keys[btnUp] = DUkey;
        button_keys[btnDown] = DDkey;
        button_keys[btnLeft] = DLkey;
        button_keys[btnRight] = DRkey;
        button_keys[btnA] = Akey;
        button_keys[btnB] = Bkey;
        button_keys[btnS] = Skey;
        button_keys[btnL] = Lkey;
        button_keys[btnR] = Rkey;
        button_keys[btnP] = Pkey;
        button_keys[btnEx1] = Exkey1;
        button_keys[btnEx2] = Exkey2;
        button_keys[btnEx3] = Exkey3;
        button_keys[btnEx4] = Exkey4;
    }

    static int find_index_for_button_name(std::string button_name)
    {
        for (int i = 0; i < ZC_CONTROL_STATES; i++)
        {
            if (button_names[i] == button_name)
                return i;
        }
        return -1;
    }

    int button_index;

    ButtonReplayStep(int frame, int type, int button_index) : ReplayStep(frame, type), button_index(button_index)
    {
    }

    void run()
    {
        raw_control_state[button_index] = type == TypeButtonDown;

        // Set the key/joystick states directly, in case the engine does readkey
        // directly, or zscript queries the state of these things.
        if (button_index < 14)
        {
            key[button_keys[button_index]] = type == TypeButtonDown ? 1 : 0;
        }
        else
        {
            // TODO zscript allows polling the joystick state directly,
            // so should probably do that. I don't have a gamepad at the moment
            // so can't verify. For now, do nothing. This seems like it would be
            // rare enough to matter that it's ok to skip for now.
        }
    }

    std::string print()
    {
        return fmt::format("{} {} {}", type, frame, button_names[button_index]);
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

    CheatReplayStep(int frame, Cheat cheat, int arg1, int arg2) : ReplayStep(frame, TypeCheat), cheat(cheat), arg1(arg1), arg2(arg2)
    {
    }

    void run()
    {
        cheats_execute(cheat, arg1, arg2);
    }

    std::string print()
    {
        std::string cheat_name = cheat_to_string(cheat);
        if (arg1 == -1)
            return fmt::format("{} {} {}", type, frame, cheat_name);
        else if (arg2 == -1)
            return fmt::format("{} {} {} {}", type, frame, cheat_name, arg1);
        else
            return fmt::format("{} {} {} {} {}", type, frame, cheat_name, arg1, arg2);
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

static int get_rng_index(zc_randgen *rng)
{
    auto it = std::find(rngs.begin(), rngs.end(), rng);
    if (it == rngs.end())
        return -1;
    return it - rngs.begin();
}

static RngReplayStep *find_rng_step(int rng_index, size_t starting_step_index, const std::vector<std::shared_ptr<ReplayStep>> &log)
{
    RngReplayStep *result = nullptr;

    for (size_t i = starting_step_index; i < log.size(); i++)
    {
        auto step = log[i];
        if (step->frame != frame_count)
            break;
        if (step->type != TypeRng)
            continue;

        auto rng_step = static_cast<RngReplayStep *>(step.get());
        if (rng_index >= rng_step->start_index && rng_index <= rng_step->end_index)
            result = rng_step;
    }

    return result;
}

static bool is_Fkey(int k)
{
    switch (k)
    {
    case KEY_F1:
    case KEY_F2:
    case KEY_F3:
    case KEY_F4:
    case KEY_F5:
    case KEY_F6:
    case KEY_F7:
    case KEY_F8:
    case KEY_F9:
    case KEY_F10:
    case KEY_F11:
    case KEY_F12:
        return true;
    }

    return false;
}

// This is for ignoring keyboard input during recording playback,
// and prompting the user if they wish to end the replay.
static int keyboard_intercept(int key)
{
    if (is_Fkey(key >> 8))
        return key;
    did_attempt_input_during_replay = true;
    return 0;
}

static void start_recording()
{
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
        previous_control_state[i] = false;
    record_log.clear();
}

static void do_recording_poll()
{
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
    {
        bool state = raw_control_state[i];
        if (state == previous_control_state[i])
            continue;

        record_log.push_back(std::make_shared<ButtonReplayStep>(frame_count, state ? TypeButtonDown : TypeButtonUp, i));
        previous_control_state[i] = state;
    }
}

// https://stackoverflow.com/a/6089413/2788187
static std::istream &portable_get_line(std::istream &is, std::string &t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;)
    {
        int c = sb->sbumpc();
        switch (c)
        {
        case '\n':
            return is;
        case '\r':
            if (sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if (t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

static void load_replay(std::string filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        fprintf(stderr, "could not open file: %s\n", filename.c_str());
        exit(1);
    }

    bool done_with_meta = false;
    std::string line;
    while (portable_get_line(file, line))
    {
        if (line.empty())
            continue;

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

        if (!done_with_meta)
            done_with_meta = type != TypeMeta;

        if (type == TypeMeta)
        {
            ASSERT(!done_with_meta);

            std::string key;
            std::string value;
            iss >> key;
            iss.ignore(1);
            portable_get_line(iss, value);

            ASSERT(meta_map.find(key) == meta_map.end());
            meta_map[key] = value;
        }
        else if (type == TypeComment)
        {
            std::string comment;
            portable_get_line(iss, comment);
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
            int arg1, arg2;

            std::string cheat_name;
            iss >> cheat_name;
            cheat = cheat_from_string(cheat_name);
            ASSERT(cheat > Cheat::None && cheat < Cheat::Last);

            if (!(iss >> arg1))
                arg1 = -1;
            if (!(iss >> arg2))
                arg2 = -1;
            replay_log.push_back(std::make_shared<CheatReplayStep>(frame, (Cheat)cheat, arg1, arg2));
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
        else if (type == TypeButtonUp || type == TypeButtonDown)
        {
            std::string button_name;
            portable_get_line(iss, button_name);
            int button = ButtonReplayStep::find_index_for_button_name(button_name);
            ASSERT(button != -1);
            if (button == -1)
                continue;

            iss >> button;
            replay_log.push_back(std::make_shared<ButtonReplayStep>(frame, type, button));
        }
    }

    file.close();
    replay_log_current_index = 0;
    debug = replay_get_meta_bool("debug");
    sync_rng = replay_get_meta_bool("sync_rng");
}

static void save_replay(std::string filename, const std::vector<std::shared_ptr<ReplayStep>> &log)
{
    std::time_t ct = std::time(0);
    replay_set_meta("time_updated", strtok(ctime(&ct), "\n"));

    std::ofstream out(filename);
    for (auto it : meta_map)
        out << fmt::format("{} {} {}", TypeMeta, it.first, it.second) << '\n';
    for (auto it : log)
        out << it->print() << '\n';
    out.close();
}

static void do_replaying_poll()
{
    while (replay_log_current_index < replay_log.size() && replay_log[replay_log_current_index]->frame == frame_count)
    {
        replay_log[replay_log_current_index]->run();
        replay_log_current_index += 1;
    }
}

static void check_assert()
{
    // Only print the very first difference. When replay_stop is called,
    // the program will exit with a status code based on this bool. If
    // asserts have failed, a ".zplay.roundtrip" file will be written
    // for comparison.
    if (has_assert_failed)
        return;

    size_t replay_log_size = frame_arg == -1 ? replay_log.size() : frame_arg;
    while (assert_current_index < replay_log_size && replay_log[assert_current_index]->frame <= frame_count)
    {
        if (assert_current_index >= record_log.size())
            break;

        auto replay_step = replay_log[assert_current_index];
        auto record_step = record_log[assert_current_index];
        bool are_equal = true;

        if (replay_step->frame != record_step->frame || replay_step->type != record_step->type)
        {
            are_equal = false;
        }
        else
            switch (record_step->type)
            {
            case TypeComment:
            {
                auto comment_replay_step = static_cast<CommentReplayStep *>(replay_step.get());
                auto comment_record_step = static_cast<CommentReplayStep *>(record_step.get());
                are_equal = comment_replay_step->comment == comment_record_step->comment;
            }
            break;
            case TypeButtonUp:
            case TypeButtonDown:
            {
                auto button_replay_step = static_cast<ButtonReplayStep *>(replay_step.get());
                auto button_record_step = static_cast<ButtonReplayStep *>(record_step.get());
                are_equal = button_replay_step->button_index == button_record_step->button_index;
            }
            break;
            case TypeQuit:
            {
                auto quit_replay_step = static_cast<QuitReplayStep *>(replay_step.get());
                auto quit_record_step = static_cast<QuitReplayStep *>(record_step.get());
                are_equal = quit_replay_step->quit_state == quit_record_step->quit_state;
            }
            break;
            case TypeCheat:
            {
                auto cheat_replay_step = static_cast<CheatReplayStep *>(replay_step.get());
                auto cheat_record_step = static_cast<CheatReplayStep *>(record_step.get());
                are_equal = cheat_replay_step->cheat == cheat_record_step->cheat;
            }
            break;
            case TypeRng:
            {
                auto rng_replay_step = static_cast<RngReplayStep *>(replay_step.get());
                auto rng_record_step = static_cast<RngReplayStep *>(record_step.get());
                are_equal =
                    rng_replay_step->seed == rng_record_step->seed &&
                    rng_replay_step->start_index == rng_record_step->start_index &&
                    rng_replay_step->end_index == rng_record_step->end_index;
            }
            break;
            }

        if (!are_equal)
        {
            has_assert_failed = true;
            int line_number = assert_current_index + meta_map.size() + 1;
            std::string error = fmt::format("<{}> expected:\n\t{}\nbut got:\n\t{}", line_number,
                                              replay_step->print(), record_step->print());
            fprintf(stderr, "%s\n", error.c_str());
            replay_save(filename + ".roundtrip");
            // Paused = true;
            break;
        }

        assert_current_index++;
    }
}

static size_t old_start_of_next_screen_index;
static bool stored_control_state[ZC_CONTROL_STATES];

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
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
        stored_control_state[i] = raw_control_state[i];
    for (size_t i = manual_takeover_start_index; i < old_start_of_next_screen_index; i++)
    {
        if (replay_log[i]->type != TypeButtonDown && replay_log[i]->type != TypeButtonUp)
            continue;

        auto button_step = static_cast<ButtonReplayStep *>(replay_log[i].get());
        stored_control_state[button_step->button_index] = button_step->type == TypeButtonDown;
    }

    // Avoid unexpected input when manual takeover starts, which can be awkward to play.
    for (int i = 0; i < KEY_MAX; i++)
        key[i] = 0;

    mode = ReplayMode::ManualTakeover;
    keyboard_callback = nullptr;
    Throttlefps = true;
    Paused = true;
}

void replay_start(ReplayMode mode_, std::string filename_)
{
    ASSERT(mode == ReplayMode::Off);
    ASSERT(mode_ != ReplayMode::Off && mode_ != ReplayMode::ManualTakeover);
    mode = mode_;
    debug = false;
    sync_rng = false;
    did_attempt_input_during_replay = false;
    has_assert_failed = false;
    filename = filename_;
    manual_takeover_start_index = assert_current_index = replay_log_current_index = frame_count = 0;
    frame_arg = -1;
    prev_gfx_hash = 0;
    ButtonReplayStep::load_keys();

    switch (mode)
    {
    case ReplayMode::Off:
    case ReplayMode::ManualTakeover:
        return;
    case ReplayMode::Record:
    {
        std::time_t ct = std::time(0);
        replay_set_meta("time_created", strtok(ctime(&ct), "\n"));
        replay_set_meta("version", 2);
        start_recording();
        break;
    }
    case ReplayMode::Replay:
        load_replay(filename);
        break;
    case ReplayMode::Assert:
    case ReplayMode::Update:
        load_replay(filename);
        start_recording();
        break;
    }

    if (replay_is_replaying())
    {
        ASSERT(!keyboard_callback);
        keyboard_callback = keyboard_intercept;
    }
}

void replay_continue(std::string filename_)
{
    ASSERT(mode == ReplayMode::Off);
    mode = ReplayMode::Record;
    filename = filename_;
    ButtonReplayStep::load_keys();
    load_replay(filename);
    record_log = replay_log;
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
        previous_control_state[i] = false;
    frame_count = record_log.back()->frame + 1;
}

void replay_poll()
{
    if (mode == ReplayMode::Off)
        return;

    ASSERT(locking_keys);

    if (did_attempt_input_during_replay && replay_is_replaying())
    {
        int key_copy[KEY_MAX];
        bool down_states[controls::btnLast];
        for (int i = 0; i < controls::btnLast; i++)
            down_states[i] = down_control_states[i];
        for (int i = 0; i < KEY_MAX; i++)
        {
            key_copy[i] = key[i];
            key[i] = 0;
        }
        keyboard_callback = nullptr;
        locking_keys = false;
		
		enter_sys_pal();
		if (jwin_alert("Replay",
					   "Would you like to halt the replay and",
					   "take back control?",
					   "",
					   "Yes", "No", 'y', 'n', lfont) == 1)
		{
			replay_quit();
			exit_sys_pal();
			return;
		}
		exit_sys_pal();

        did_attempt_input_during_replay = false;
        locking_keys = true;
        keyboard_callback = keyboard_intercept;
        for (int i = 0; i < KEY_MAX; i++)
            key[i] = key_copy[i];
        for (int i = 0; i < controls::btnLast; i++)
            down_control_states[i] = down_states[i];
    }

    if (frame_arg != -1 && frame_arg == frame_count)
    {
        if (mode == ReplayMode::Update)
        {
            start_manual_takeover();
            jwin_alert("Recording", "Re-recording until new screen is loaded", NULL, NULL, "OK", NULL, 13, 27, lfont);
        }
        else
        {
            Throttlefps = true;
            Paused = true;
            replay_forget_input();
            replay_stop();
            jwin_alert("Recording", "Replaying stopped at requested frame", NULL, NULL, "OK", NULL, 13, 27, lfont);
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
        if (replay_log_current_index == replay_log.size() && assert_current_index == replay_log.size())
            replay_stop();
        if (has_assert_failed && frame_count - replay_log[assert_current_index]->frame > 60*60)
            replay_stop();
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

    frame_count++;
}

void replay_peek_quit()
{
    int i = replay_log_current_index;
    while (i < replay_log.size() && replay_log[i]->frame == frame_count)
    {
        if (replay_log[i]->type == TypeQuit)
        {
            auto quit_replay_step = static_cast<QuitReplayStep *>(replay_log[i].get());
            if (quit_replay_step->quit_state == 0)
                GameFlags |= GAMEFLAG_TRYQUIT;
            else
                Quit = quit_replay_step->quit_state;
            break;
        }
        i++;
    }
}

bool replay_is_assert_done()
{
    return mode == ReplayMode::Assert && (has_assert_failed || assert_current_index == replay_log.size());
}

void replay_forget_input()
{
    if (mode == ReplayMode::Off)
        return;

    for (int i = 0; i < KEY_MAX; i++)
        key[i] = 0;
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
        previous_control_state[i] = raw_control_state[i] = false;
}

void replay_stop()
{
    if (mode == ReplayMode::Off)
        return;

    if (replay_is_replaying())
        keyboard_callback = nullptr;

    if (mode == ReplayMode::Assert)
    {
        check_assert();
        bool log_size_mismatch = replay_log.size() != record_log.size();
        if (frame_arg == -1)
            has_assert_failed |= log_size_mismatch;
        if (has_assert_failed)
        {
            replay_save(filename + ".roundtrip");
        }
        if (log_size_mismatch)
        {
            fprintf(stderr, "replay_log size is %zu but record_log size is %zu\n", replay_log.size(), record_log.size());
        }
        exit(has_assert_failed ? ASSERT_FAILED_EXIT_CODE : 0);
    }

    if (mode == ReplayMode::Update)
    {
        replay_save();
        exit(0);
    }

    mode = ReplayMode::Off;
    frame_count = 0;
    replay_log.clear();
    rngs.clear();
    meta_map.clear();
}

void replay_quit()
{
    if (mode == ReplayMode::Assert || mode == ReplayMode::Update)
        mode = ReplayMode::Replay;
    replay_stop();
}

void replay_save()
{
    replay_save(filename);
}

void replay_save(std::string filename)
{
    save_replay(filename, record_log);
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
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
        raw_control_state[i] = stored_control_state[i];

    // Insert some button steps to make the replay_log match what the record_log will have inserted on the next
    // call to do_recording_poll.
    // This is the same as do_recording_poll, but without setting the previous control state variable.
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
    {
        bool state = raw_control_state[i];
        if (state == previous_control_state[i])
            continue;

        restore_log.push_back(std::make_shared<ButtonReplayStep>(frame_count, state ? TypeButtonDown : TypeButtonUp, i));
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

    save_replay(filename, replay_log);
    replay_log_current_index = record_log.size();
    mode = ReplayMode::Update;
    keyboard_callback = keyboard_intercept;
    frame_arg = -1;
    jwin_alert("Recording", "Done re-recording, resuming replay from here", NULL, NULL, "OK", NULL, 13, 27, lfont);

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
    if (replay_is_active())
    {
        record_log.push_back(std::make_shared<CommentReplayStep>(frame_count, comment));
        // Not necessary to call this here, but helps to halt the program exactly when an unexpected
        // comment occurs instead of at the next call to replay_poll.
        if (mode == ReplayMode::Assert)
            check_assert();
    }
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
    while (value > 0)
    {
        T remainder = value % 91;
        value /= 91;
        result.insert(result.begin(), alphabet[remainder]);
    }
    return result;
}

void replay_step_gfx(uint32_t gfx_hash)
{
    // Skip if last invocation was the same value.
    if (gfx_hash == prev_gfx_hash)
        return;

    prev_gfx_hash = gfx_hash;
    // 16 bits should be enough entropy to detect visual regressions.
    // Using uint16_t reduces .zplay by ~7%.
    replay_step_comment(int_to_basE91((uint16_t)gfx_hash));

    // Note: I tried a simple queue cache to remember the last N hashes and use shorthand
    // for repeats (ex: gfx ^2), but even with a huge memory of 16777216 hashes the
    // savings was never more than 2%, so not worth it.
}

void replay_set_meta(std::string key, std::string value)
{
    if (key == "qst")
        std::replace_if(
            value.begin(), value.end(),
            [](std::string::value_type v)
            {
                return v == '\\';
            },
            '/');

    if (replay_is_active())
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

void replay_step_quit(int quit_state)
{
    record_log.push_back(std::make_shared<QuitReplayStep>(frame_count, quit_state));
}

void replay_step_cheat(Cheat cheat, int arg1, int arg2)
{
    record_log.push_back(std::make_shared<CheatReplayStep>(frame_count, cheat, arg1, arg2));
}

ReplayMode replay_get_mode()
{
    return mode;
}

std::string replay_get_filename()
{
    return filename;
}

std::string replay_get_buttons_string()
{
    std::string text;
    text += fmt::format("{} ", frame_count);
    for (int i = 0; i < ZC_CONTROL_STATES; i++)
    {
        if (raw_control_state[i])
        {
            if (!text.empty())
                text += ' ';
            text += ButtonReplayStep::button_names[i];
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

void replay_set_sync_rng(bool enable)
{
    sync_rng = enable;
    replay_set_meta_bool("sync_rng", sync_rng);
}

bool replay_is_replaying()
{
    return mode == ReplayMode::Replay || mode == ReplayMode::Assert || mode == ReplayMode::Update;
}

void replay_set_frame_arg(int frame)
{
    if (frame == -1)
        frame = replay_log.front()->frame + 1;
    else
        frame_arg = frame;
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

    if (replay_is_replaying())
    {
        RngReplayStep *rng_step = find_rng_step(index, replay_log_current_index, replay_log);
        if (rng_step)
        {
            seed = rng_step->seed;
        }
        // Only OK to be missing if in update mode.
        else if (mode != ReplayMode::Update)
        {
            int line_number = replay_log_current_index + meta_map.size() + 1;
            fprintf(stderr, "<%d> rng desync\n", line_number);
            if (mode == ReplayMode::Assert)
            {
                replay_stop();
            }
            ASSERT(false);
        }
    }

    bool did_extend = false;
    if (!record_log.empty() && record_log.back()->type == TypeRng && record_log.back()->frame == frame_count)
    {
        auto rng_step = static_cast<RngReplayStep *>(record_log.back().get());
        if (rng_step->seed == seed && rng_step->end_index == index - 1)
        {
            rng_step->end_index = index;
            did_extend = true;
        }
    }

    if (!did_extend)
        record_log.push_back(std::make_shared<RngReplayStep>(frame_count, index, index, seed));

    rng->seed(seed);
}

void replay_sync_rng()
{
    if (!sync_rng)
        return;

    // Only reset the rngs that haven't been updated this frame.
    size_t first_step_index_for_frame = 0;
    bool found_first_step = false;
    if (!record_log.empty())
    {
        size_t i = record_log.size() - 1;
        while (i > 0 && record_log[i]->frame == frame_count)
        {
            found_first_step = true;
            first_step_index_for_frame = i;
            i--;
        }
    }

    int seed = time(0);
    for (size_t i = 0; i < rngs.size(); i++)
    {
        if (found_first_step && find_rng_step(i, first_step_index_for_frame, record_log))
            continue;
        replay_set_rng_seed(rngs[i], seed);
    }

    frame = 0;
}
