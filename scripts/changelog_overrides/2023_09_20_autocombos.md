section 6bfafa3630a03b2e402b0eba81b6938925b59071 Relational Combos Update
The old "Relational" and "Dungeon Carving" modes have been replaced by a new drawing mode that can handle various relational drawing behaviors. Alongside this, favorite combos list has been expanded and improved to make designing screens faster than ever before.

## Autocombos
The Autocombo, similar to the "Alias" and "Combo Pool" drawing modes before it, adds a list of user defined patterns which are used for relational drawing. Each autocombo has a set of combos used for the pattern, a combo for erasing from the pattern with right click, and a display combo which affects how it will appear in the editor for user convenience. They can also auto generate a pattern from a default arrangement of combos like how the older drawing modes worked, but do not need use combos placed in that order.

Unlike most other features, autocombos can read and modify combos on adjacent screens with the "Cross Screens" flag. 

Autocombo behavior can be selected from the following 9 types:
* The "Basic Relational" type is a standard relational mode with 4-directional tile awareness.
* The "Relational" type works like the old relational drawing mode with 8-directional tile awareness. It also has a flag to use the old combo layout when generating.
* The "Flat Mountain" type is the simplest of the set, with only 6 tiles representing the corners, sides, and top.
* The "Pancake Mountain" type is for drawing flat topped mountains, formed by drawing the edges and then extruding the side faces downwards.
* The "Fence" type is similar to "Pancake Mountain" but without the combos for extruding sides.
* The "Dungeon Carving" type works like the old dungeon carving drawing mode, drawing 2 tile high dungeon walls. It also has a flag to use the old combo layout when generating.
* The "Complex Mountain" type has you fill in the top layer of a mountain and then fills out the bottoms, backs, and sides with a 3/4 perspective. Unlike the other sets, this one is imperfect and may require some manual correction, but it makes using the hardest mountain sets in common usage far easier.
* The "Positional Tiling" type lets you form a rectangular pattern that it then tiles based on X/Y position within the map. With shift + click you can set where the top-left corner of the pattern is located.
* The "Replace" type lets you set an arbitrary set of before and after combos to replace when drawing. Instead of erasing, right click will reverse the process.

## Favorite Combos Expansion
The favorite combos list has also been greatly expanded. It now allows for 9 pages of combos. Right clicking the page selector buttons will create a drop-down allowing you to jump to any page.

It can also now hold entries from any of the four drawing modes. When selecting an entry from a different drawing mode, the current mode will automatically update to match. This should make switching from relationally drawing a screen's layout to placing down large objects as aliases to adding finer details a breeze. Each of the three special drawing modes will have icons drawn over their favorite combo entries, which can be disabled with the "Show Favorite Combo Modes" option in `Ect->Options...`. 

Lastly, adding favorite combos now respects the AutoBrush setting, making adding large blocks of single combos to the page far quicker.
=end