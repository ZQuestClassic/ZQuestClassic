Z3 User Guide
=============

This is written for experienced users of ZC, and describes the major changes related to the new `z3` region scrolling feature.

"z3" refers to the third Zelda, A Link To The Past, which unlike the original Zelda features playing areas larger than a single screen. To accomplish the same in ZQuest Classic, one had to rewrite the entire game engine and every feature you wanted to use in ZScript, an incredibly daunting task. Now, ZQuest Classic supports z3-style screens via a new features: regions.

Regions
-------

Regions consist of a subset of the screens of a map (not a DMap). A map can contain multiple regions. They are defined in the ``Quest > Regions`` editor. The positive numbers in the grid denote a region ID - neighboring screens with the same region ID are considered to be the same Region. Region IDs have no particular meaning, and can be re-used within the same map (as long as they are not touching a region of the same ID, they make up distinct regions).

Note: current limitations require that regions must be rectangular with no holes.

Even non-scrolling areas of 1x1 screens are technically considered to be a region. When documentation refers to a "scrolling region", it means a region larger than one screen. A "non-scrolling region" is a traditional 1x1 screen area.

There are some specific terms used for the screens within a region:

* ``origin screen``: The top-left screen of the region. This is equal to `Game->CurScreen`
* ``hero screen``: The screen that the player is currently in

The specific screen that is loaded into (via scrolling or warping or whatever) has no special behavior, other than determining which
region is loaded.

Viewport
--------

The traditional viewport for ZC has been 256 pixels wide by 176 pixels tall - or 16x11 combos. This continues to be the viewport when in a non-scrolling region.

In a scrolling region, the viewport pans as the player moves across the region. By default the viewport is centered to draw the player in the middle of the screen, but this is bounded by the edges of the region to prevent showing beyond the current region. This behavior can be customized via scripting.

There is a new option to have an extended height viewport (applicable only when a region is taller than 1 screen). This makes the viewport 232 pixels (or 3.5 combos taller) - the same height as the passive subscreen. This effectively centers the player as if the passive subscreen did not exist. This is controlled by a DMap flag. When using an extended height viewport, the passive subscreen should be configured to have no background color. Without a transparent passive subscreen, it will look very odd. Similarly, if not using an extended height viewport, the passive subscreen should be opaque.

The viewport height is only extended if the DMap flag is on, and if the current region is taller than one screen.

Note: all this is ignoring the fact that the bottom 8 pixels has always and continues to be not shown (so minus 8 pixels in the height for the true visual viewport height).

When in a scrolling region, enemies and their weapons are paused if they are outside the current viewport, with some buffer (48px). However, weapon/npc scripts still run.

Player weapons are considered "out of bounds" when they leave the viewport. Most player weapons are deleted when they go out of bounds (unless `CollDetection` is false), such as arrows. But some weapon types have special behavior: for example, boomerangs just begin returning to the hero.

ZScript
-------

TL;DR
^^^^^

Most scripts can be migrated by simply making the following changes:

* for iterating every combo position, change 176 to `NUM_COMBO_POS`
* for the max FFC id, use `MAX_FFC` (instead of 32 or 128 or whatever)
* to translate x/y to a combo position, only use `ComboAt`
* to check if something is out-of-bounds, change 256 / 176 to `Region->Width` / `Region->Height`
* to check if something is out-of-view, use `Viewport->Contains(int x, int y)` / `Viewport->Contains(sprite s)`
* set `Screen->DrawOrigin` to adjust the origin used by `Screen` drawing functions

Read on for more detail.

Drawing
^^^^^^^

Screen drawing functions (e.g. `Screen->DrawTile`) used to always be relative to the playing field (where the origin ``(0, 0)`` is the pixel below the passive subscreen, on the left). To account for this during scroll transitions, scripts had to use `Game->Scrolling[]` to adjust the coordinates accordingly.

Now, a new variable :ref:`Screen->DrawOrigin<globals_screen_var_draworigin>` determines how coordinates given to these drawing functions are interpreted. See the documentation for more details.

Viewport
^^^^^^^^

The :ref:`Viewport<globals_viewport>` global defines the visible portion of the current region - what the player currently sees. Via scripting, the viewport can be modified to show any part of the region. See the documentation for more details.

Region
^^^^^^

The :ref:`Region<globals_region>` global defines the currently active region. See the documentation for more details.

sprite SpawnScreen
^^^^^^^^^^^^^^^^^^^

All sprites (such as `ffc`, `npc`, `lweapon`, `eweapon`, `itemsprite` etc.) have a `SpawnScreen` variable, which is the screen the sprite was created on. This variable does not update as the sprite moves around a region.

Functionality specific to a screen (like screen flags, secrets, etc.) correspond to a sprite's `SpawnScreen`.

Game->HeroScreen
^^^^^^^^^^^^^^^^

This variable updates as the player moves around within a region.

`mapdata` and `Screen`
^^^^^^^^^^^^^^^^^^^^^^

The :ref:`Screen<globals_screen>` global and the :ref:`mapdata<classes_mapdata>` class are similiar in that they both deal with screens. However, `Screen` only references the currently active screens, and `mapdata` may reference active screens, scrolling screens, or the canonical map screens.

By default, :ref:`Screen<globals_screen>` reads from the origin screen. Some script types (screendata, ffc, npc, eweapon, lweapon, itemsprite, combo) operate on the screen that spawned the script. Player scripts operate on the screen the player is currently in (`Game->HeroScreen`).

* `Screen->ComboD[pos]` can return any combo in the current region. Before regions, `pos` (which stands for combo position) could be `0-175`. The same is true with regions, but the range is `0` to `NUM_COMBO_POS` (exclusive), where `NUM_COMBO_POS` is equal to 176 multiplied by the number of screens in a region.
* The above is also true for:
* * `Screen->ComboC[pos]`
* * `Screen->ComboF[pos]`
* * `Screen->ComboI[pos]`
* * `Screen->ComboT[pos]`
* * `Screen->ComboS[pos]`
* * `Screen->ComboE[pos]`
* `Screen->LoadFFC(id)` can return any FFC within the current region. `id` can be `1-MAX_FFC`
* For everything else on `Screen->`, it accesses just a single screen

Before regions, to iterate every combo on a screen you loop between 0 and 176 (exclusive). With regions, the upper value is instead `NUM_COMBO_POS` - when not in a region, this value is 176. To make scripts compatible with regions, replace 176 with `NUM_COMBO_POS`.

To get the correct value of `pos` for a given `x` and `y` coordinate, you can still use `ComboAt(x, y)`. This will use the current region to determine the combo position.

To access other screens of the current region, use `mapdata`. There is `Game->LoadMapData(map, screen)`, `Game->LoadTempScreen(layer)`, and `Game->LoadScrollingScreen(layer)`. These all return a `mapdata`:

* `Game->LoadMapData(map, screen)`: Returns a handle that accesses canonical screens. If the player is currently on this screen, no changes will be observed until the screen is reloaded. Modifications will not persist after saving/continue.
* `Game->LoadTempScreen(layer)`, `Game->LoadTempScreen(layer, screen)`: Returns a handle that accesses a currently loaded screen at the given layer. Modifications to the temporary screen will not persist when the player leaves the region.
* `Game->LoadTempScreenForComboPos(layer)`, `Game->LoadTempScreenForComboPos(layer, pos)`: Similar to above, but returns the temp screen for the given given combo position (the result of `ComboAt(x, y)`).
* `Game->LoadScrollingScreen(layer)`, `Game->LoadScrollingScreen(layer, screen)`: Returns a handle that accesses a temporary screen from the region that the player is scrolling away from, at the given layer. This is only valid during screen scrolling.

`mapdata` only ever refers to a single screen.

FFCs
^^^^

Each individual screen within a region may have up to 128 FFCs. The number used to uniquely identify an FFC in the current region is :ref:`ffc::ID<classes_ffc_var_id>`. The maximum FFC ID for the currently loaded region is `MAX_FFC`.

To load the FFC with the specified ID, use `Screen->LoadFFC(int ffc_id)`.

To load the FFC at a specific index for a given screen, use `Screen->LoadFFC(int screen, int index)`.

Region-related functions
^^^^^^^^^^^^^^^^^^^^^^^^

New functions in `std` (see :doc:`/zscript/libs/std/std_functions` for details):

* :ref:`int RegionRelativeScreenX(int scr)<libs_std_fun_regionrelativescreenx>`
* :ref:`int RegionRelativeScreenY(int scr)<libs_std_fun_regionrelativescreeny>`
* :ref:`int RegionWorldOffsetX(int scr)<libs_std_fun_regionworldoffsetx>`
* :ref:`int RegionWorldOffsetY(int scr)<libs_std_fun_regionworldoffsety>`
* :ref:`bool RegionIncludesScreen(int scr)<libs_std_fun_regionincludesscreen>`
