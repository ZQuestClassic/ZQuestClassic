#ifndef ZC_FUZZ_H_
#define ZC_FUZZ_H_

#ifdef ZC_FUZZ

#include <functional>

// True when the ZC_FUZZ env var names a fuzz target.
bool zplayer_fuzz_requested();
// Called first thing in main. Keeps the command line for libFuzzer, and for targets that run in a
// game (zasm) swaps in the one the game should see.
void zplayer_fuzz_setup_args(int* argc, char*** argv);
// True if the target runs inside a game, so startup carries on as usual (see fuzz_frame_hook).
bool zplayer_fuzz_needs_game();
// Hands the process over to libFuzzer, for targets that don't need a game. Never returns.
[[noreturn]] void zplayer_fuzz_main();
// If set, the game loop calls this at the start of each frame.
extern std::function<void()> fuzz_frame_hook;

#endif

#endif
