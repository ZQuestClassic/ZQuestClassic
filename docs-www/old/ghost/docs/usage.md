# General usage

## Enemy scripts

A ghost.zh script should begin by calling one of the `Ghost_Init` functions to
find the enemy the FFC should be associated with and set up internal data.
Afterward, `Ghost_Waitframe()` should be used instead of `Waitframe()`.
This is necessary to keep data consistent and to provide standard enemy behavior,
such as reacting to damage. Most of the standard enemy data is manipulated by
using the global variables and functions [here](enemyScript.md), but some things
- particularly defenses and draw offsets - must still be accessed via the npc pointer.

The enemy used can be of any type and with any settings. Its HP, damage, and
defenses will be used normally. Its regular tiles and animation will be used
if the FFC is set to use `GH_INVISIBLE_COMBO`; otherwise, it will be made
invisible. Other enemy data can be used as you see fit.

There are also a number of functions used to create and control eweapons. They
can be assigned a number of simple movements and can be made to vanish and
spawn additional eweapons. These behaviors require support from the global script.

For scripted enemies to be susceptible to clocks, `StartClock()` and `UpdateClock()`
must be called in the global script, and the included item script
`GhostZHClockScript` must be set as any clock items' pickup script, with D0 set
to match the clock's duration.

To prevent conflicts with ghost.zh, scripts should avoid using
`npc->Misc[__GHI_NPC_DATA]` and `eweapon->Misc[__EWI_FLAGS]`. Both indices
are set to 15 by default.

Functions, variables, and constants whose names start with `__` are for internal
use and may be changed or removed in future versions without warning.

## AutoGhost

With appropriately written scripts, it is possible to place scripted enemies
without any manual FFC setup. The `AutoGhost()` function will scan the screen for
enemies that should be handled by scripts and set up FFCs automatically when
it finds them. `AutoGhost()` should come after `WaitDraw()` in the global script.

An AutoGhost FFC script's run function should take an enemy ID as argument D0,
which should be passed to the `Ghost_InitAutoGhost` function. `AutoGhost()` will not set
any other arguments, but they can be used if your script runs additional
instances of the same script.

The corresponding enemy needs to be set up to identify the script and combo
it will use, as described [here](autoGhost.md#setting-the-script-and-combo).
