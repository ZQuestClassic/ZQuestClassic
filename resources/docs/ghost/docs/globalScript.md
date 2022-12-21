# Global script functions

Many features of ghost.zh require global script support. If you don't want
to use the provided `GhostZHActiveScript`, you just need to add a few functions
into whatever global script you do use.

## Combined

The simpler option is to use these three functions. These provide all
functionality and will continue to work if more functions are ever added
in the future.

void StartGhostZH()
:   Call this before the global script's main loop.

void UpdateGhostZH1()
:   Call this in the global script's main loop before `Waitdraw()`.

void UpdateGhostZH2()
:   Call this in the global script's main loop after `Waitdraw()`.

## Separate

These are the functions used internally by those listed above. Calling these
individually lets you leave out functionality you don't need, but it's possible
a future ghost.zh update will require updating the script.

void InitializeGhostZHData()
:   Initializes internal counters and data.

    Call before the main loop of the global script.

void UpdateGhostZHData1()
void UpdateGhostZHData2()
:   Updates counters and internal data.

    Call `UpdateGhostZHData1()` in the main loop before `Waitdraw()` and
    `UpdateGhostZHData2()` after `Waitdraw()`.

void UpdateEWeapon(eweapon wpn)
:   This must be called on eweapons each frame to enable the special behaviors
    provided by ghost.zh.
    
    Call in the main loop before `Waitdraw()`.

void UpdateEWeapons()
:   Runs `UpdateEWeapon()` on all eweapons onscreen.
    
    Call in the main loop before `Waitdraw()`.

void CleanUpGhostFFCs()
:   Hides enemy FFCs when Link dies. If `__GH_USE_DRAWCOMBO` is enabled, this function does nothing.
    
    Call in the main loop before `Waitdraw()` and after anything that would save
    Link from dying when `Link->HP<=0`.

void DrawGhostFFCs()
:   Draws enemy combos if `__GH_USE_DRAWCOMBO` is enabled and draws additional
    combos regardless. If `__GH_USE_DRAWCOMBO` is disabled and no scripts use
    `Ghost_AddCombo()`, this function does nothing.
    
    Call in the main loop after `Waitdraw()`.

void AutoGhost()
:   Detects AutoGhost enemies and sets up FFCs for them.
    
    Call in the main loop after `Waitdraw()`.
