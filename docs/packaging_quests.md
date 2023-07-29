# Packaging Quests

This is currently Windows only.

With ZQuest's `File -> Export -> Package` you can generate a standalone app bundle that plays just your quest.
This gives you the option to distribute your quest in a way that allows people to play without
grabbing any other files, or even needing to select a file when starting a save slot.

All following examples assume a qst file `game.qst` with a title "My Game" (as set in the quest header). The package would look like this:

```
- My Game
	- My Game.exe
	- data/
		- zelda.exe
		- zc_args.txt
		- game.qst
		- ... all support files ...
```

This is located in the `packages` folder of the root installation folder.

Double clicking `My Game.exe` launches `./data/zelda.exe` with the CLI
switches listed in `zc_args.txt`. By default, the export tool provides
these switches: `-only game.qst -notitle`. You can set whatever else
you like. For example, if you want to skip the save file screen entirely,
you can use `-standalone` instead of `-only`.

Every time a package export is done in ZQuest, the folder is created
from scratch.

To include additional files, make a folder `./packages/My Game_extra`
and place whatever your quest needs (music, images, `zc_args.txt`,
`zc.cfg`, etc). The export tool will copy this folder into the package
folder at the end, which allows you to overwrite whatever you want.

## Icons

If a PNG is present at `My Game_extra/icon.png`, it will be used as the icon for the exe.
Only PNG is supported, and it must be 32bpp ARGB.
