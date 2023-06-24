# Z3-Style Screens

"z3" refers to the third Zelda, A Link To The Past, which unlike the original Zelda features playing areas larger than a single screen's worth of combos. To accomplish the same in ZQuest Classic, one had to rewrite the entire game engine and every feature you wanted to use in ZScript, an incredibly daunting task. Now, ZQuest Classic supports z3-style screens via a new features: Regions.

## Regions

Regions consist of a subset of the screens of a Map. Regions are associated with a DMap, and a DMap can have multiple Regions within. You can define Regions in the DMap Editor. The numbers in the grid denote a Region ID - screens with the same Region ID are considered to be the same Region. Region IDs have no particular meaning within the engine, and can be re-used within the same DMap (as long as they are not touching a Region of the same ID, they will be considered distinct).

Note: current limitations require that Regions 1) must be rectangular and 2) can only span a single Map.

## ZScript

