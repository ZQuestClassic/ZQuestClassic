#ifndef ZC_FUZZ_H_
#define ZC_FUZZ_H_

#ifdef ZC_FUZZ

// True when the ZC_FUZZ env var names a fuzz target.
bool zplayer_fuzz_requested();
// Hands the process over to libFuzzer. Never returns.
void zplayer_fuzz_main();

#endif

#endif
