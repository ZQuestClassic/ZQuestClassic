# Z3

"z3" refers to the third Zelda, A Link To The Past, which unlike the original Zelda features playing areas larger than a single screen. To accomplish the same in ZQuest Classic, one had to rewrite the entire game engine and every feature you wanted to use in ZScript, an incredibly daunting task. Now, ZQuest Classic supports z3-style screens via a new features: regions.

## Regions

Regions consist of a subset of the screens of a Map. Regions are associated with a DMap, and a DMap can have multiple regions within. You can define regions in the DMap Editor. The numbers in the grid denote a region ID - screens with the same region ID are considered to be the same Region. Region IDs have no particular meaning within the engine, and can be re-used within the same DMap (as long as they are not touching a region of the same ID, they will be considered distinct).

Note: current limitations require that regions 1) must be rectangular with no holes and 2) can only span a single map.

There are some specific terms used for the screens within a region:

* `origin screen`: The top-left screen of the region
* `hero screen`: The screen that the player is currently in

The specific screen that is loaded into (via scrolling or warping or whatever) has no special behavior, other than determining what
region is loaded.

## Viewport

The traditional viewport for ZQuest has been 256 pixels wide by 176 pixels tall - or 16x11 combos.

In a region, the viewport pans across the region. By default the viewport centers the player in the middle, but this is bounded by the edges of the border. This behavior can be customized via scripting.

For a region taller than one screen, there is the option to have an extended height viewport. This makes the viewport 232 pixels (or 3.5 combos) taller - the same height as the passive subscreen. This effectively centers the player as if the passive subscreen did not exist. This is controlled by a DMap flag. When using an extended height viewport, the passive subscreen should be configured to have no background color. Without a transparent passive subscreen, it will look very odd. Similarly, if not using an extended height viewport, the passive subscreen should be opaque.

The viewport height will only be extended if the DMap flag is on, and if the current region is taller than one screen.

Note: all this is ignoring the fact that the bottom 8 pixels has always and continues to be not shown.

## ZScript

TODO

### MapData and ScreenData

First of all, the `Screen->` methods all operate on the origin screen, and cannot be changed. Before regions that would just be the only screen that is loaded. With regions, it is the top-left screen of the region. In general these methods have been surpassed by `mapdata`, so if you are scripting with Regions you should entirely avoid `Screen->` for reading/writing to map screens.

There is `Game->LoadMapData(map, screen)`, `Game->LoadTempScreen(layer)`, and `Game->LoadScrollingScreen(layer)`. These all return a `mapdata`.

* `Game->LoadMapData(map, screen)`: Returns a handle that accesses canonical screens. If the player is currently on this screen, no changes will be observed until the screen is reloaded. Modifications will not persist after saving/continue.
* `Game->LoadTempScreen(layer)`: Returns a handle that accesses the currently loaded screen at the given layer. When the player enters a screen, the canonical screen is copied to a temporary screen, such that any modifications to the temporary screen will not persist when the player leaves the screen.
* `Game->LoadScrollingScreen(layer)`: Returns a handle that accesses the temporary screens that the player is scrolling away from, at the given layer. This is only valid during screen scrolling.

With a `mapdata` from a temporary or scrolling screen, methods such as `mapdata->ComboD[pos]` behave slightly differently than before regions. Before, `pos` (which stands for combo position) is a `0-175` value indexes into the individual combos of a screen. The same is true with regions,  but:

* the range is `0-(the number of screen in a region)*176 - 1`
* the term `pos` is now `rpos`, or "region combo position"

To get the correct value of `rpos` for a given `x` and `y` coordinate, you can still use `ComboAt(x, y)`.

If you were iterating between 0 and 175 to hit every combo in a `mapdata`, you can use the new `Region->MaxRpos` or `Region->NumRpos`. The only difference between these two is that `Region->MaxRpos` is one less than `Region->NumRpos`. If not in a region, these values are just 175 and 176 respectively. Changing your code to use these instead of 175/176 will make your scripts compatible with regions.

(NOT YET IMPLEMENTED): FFCs

### Region-related functions

New functions in `std` (see `std_functions.zh` for details):

```c
int RegionRelativeScreenX(int scr);
int RegionRelativeScreenY(int scr);
int RegionWorldOffsetX(int scr);
int RegionWorldOffsetY(int scr);
bool RegionIncludesScreen(int scr);
int RegionGetFFCId(int scr, int index);
```
