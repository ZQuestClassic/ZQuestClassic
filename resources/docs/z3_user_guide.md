# Z3

"z3" refers to the third Zelda, A Link To The Past, which unlike the original Zelda features playing areas larger than a single screen. To accomplish the same in ZQuest Classic, one had to rewrite the entire game engine and every feature you wanted to use in ZScript, an incredibly daunting task. Now, ZQuest Classic supports z3-style screens via a new features: regions.

## Regions

Regions consist of a subset of the screens of a Map. Regions are associated with a DMap, and a DMap can have multiple regions within. You can define regions in the DMap Editor. The numbers in the grid denote a region ID - screens with the same region ID are considered to be the same Region. Region IDs have no particular meaning within the engine, and can be re-used within the same DMap (as long as they are not touching a region of the same ID, they will be considered distinct).

Note: current limitations require that regions 1) must be rectangular with no holes and 2) can only span a single map.

## Viewport

The traditional viewport for ZQuest has been 256 pixels wide by 176 pixels tall - or 16x11 combos.

In a region, the viewport pans across the region. By default the viewport centers the player in the middle, but this is bounded by the edges of the border. This behavior can be customized in via scripting.

For a region taller than one screen, there is the option to have an extended height viewport. This makes the viewport 232 pixels (or 3.5 combos) taller - the same height as the passive subscreen. This effectively centers the player as if the passive subscreen did not exist. This is controlled by a DMap flag. When using an extended height viewport, the passive subscreen should be configured to have no background color. Without a transparent passive subscreen, it will look very odd. Similarly, if not using an extended height viewport, the passive subscreen should be opaque.

The viewport height will only be extended if the DMap flag is on, and if the current region is taller than one screen.

Note: all this is ignoring the fact that the bottom 8 pixels has always and continues to be not shown.

## ZScript

TODO
