#ifndef REPLAY_H_
#define REPLAY_H_

#include "base/random.h"
#include "zc/cheats.h"
#include <string>
#include <filesystem>

// Records the button input state, RNG seeds, and interesting events to a .zplay file,
// and play it back while optionally asserting that everything happens the same way
// on every frame.
//
// See docs/replays.md for an overview.
//
// See tests/run_replay_tests.py for the test runner, and instructions for the CLI.

const std::string REPLAY_EXTENSION = ".zplay";

enum ReplayMode
{
    Off,
    Replay,
    Record,
    Assert,
    Update,
};

std::string replay_mode_to_string(ReplayMode mode);
std::map<std::string, std::string> replay_load_meta(std::filesystem::path path);
void replay_start(ReplayMode mode_, std::filesystem::path path, int frame);
void replay_continue(std::filesystem::path path);
void replay_poll();
bool replay_add_snapshot_frame(std::string frames_shorthand);
void replay_peek_quit();
void replay_peek_input();
void replay_do_cheats();
void replay_do_qrs();
bool replay_is_assert_done();
void replay_forget_input();
void replay_stop(bool aborted = false);
void replay_quit();
void replay_save();
void replay_save(std::filesystem::path path);

void replay_step_comment(std::string comment);
void replay_step_comment_loadscr(int screen);
void replay_step_gfx(uint32_t gfx_hash);
void replay_step_quit(int quit_state);
void replay_step_cheat(Cheat cheat, int arg1, int arg2, std::string arg3);
void replay_step_qr(int qr, bool value);

void replay_set_meta(std::string key, std::string value);
void replay_set_meta(std::string key, int value);
void replay_set_meta_bool(std::string key, bool value);
void replay_delete_meta(std::string key);
std::string replay_get_meta_str(std::string key);
std::string replay_get_meta_str(std::string key, std::string defaultValue);
int replay_get_meta_int(std::string key);
int replay_get_meta_int(std::string key, int defaultValue);
bool replay_get_meta_bool(std::string key);
bool replay_has_meta(std::string key);

ReplayMode replay_get_mode();
int replay_get_version();
// Return true if replay mode is active, and the current version is between [min/max) (inclusive/exclusive).
// If `max` is -1, returns true for all versions greater than or equal to min; and when replay mode is not active.
bool replay_version_check(int min, int max = -1);
int replay_get_frame();
void replay_set_output_dir(std::filesystem::path dir);
std::filesystem::path replay_get_replay_path();
std::string replay_get_buttons_string();
bool replay_is_active();
void replay_set_debug(bool enable_debug);
bool replay_is_debug();
void replay_set_snapshot_all_frames(bool snapshot_all_frames);
bool replay_is_snapshot_all_frames();
void replay_set_sync_rng(bool enable);
bool replay_is_replaying();
bool replay_is_recording();
void replay_set_frame_arg(int frame);
void replay_enable_exit_when_done();

size_t replay_register_rng(zc_randgen *rng);
void replay_set_rng_seed(zc_randgen *rng, int seed);
void replay_sync_rng();

int replay_get_mouse(int index);
void replay_set_mouse(int index, int value);

enum ReplayStateType
{
	MusicPosition = 1,
	SfxPosition,
};

int replay_get_state(ReplayStateType state_type, int fn());

#endif
