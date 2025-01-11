String Control Codes
====================

The strings defined in ``Quests > Strings`` can contain string control codes (SCCs) that perform special effects. These codes can be inserted manually in the string editor as seen above, or via the SCC wizard inside the string editor.

All valid codes are listed below.

Formatting
----------

Formatting effects on the string being displayed, such as changing the font, text color, or inserting characters or tiles into the message.

* ``\1\cset\color`` : Changes the text color to the specified color
* ``\26\cset\color`` : Changes the shadow color to the specified color
* ``\27\shadow_type`` : Changes the shadow type to the specified style

   Shadow types:

   0. No Shadow
   1. Basic shadow, no text
   2. 'U' shadow, no text
   3. Full outline shadow, no text
   4. '+' outline shadow, no text
   5. 'X' outline shadow, no text
   6. Basic shadow, behind text
   7. 'U' shadow, behind text
   8. Full outline shadow, behind text
   9. '+' outline shadow, behind text
   10. 'X' outline shadow, behind text

* ``\2\speed`` : Changes the text speed. 'speed' is the number of frames between each character, where '0' is instantaneous
* ``\135\font`` : Changes the text font. 'font' is the ID number of the new font to use
* ``\25`` : New Line - breaks to the next line
* ``\22`` : Inserts the save file name as text
* ``\28\tile\cset\width\height\flip`` : Draws a tile block inline with the text, as though it were a custom font character
* ``\32 through \126`` : Inserts the corresponding ASCII character. Ex: ``\91`` shows ``\``
* ``\24\???`` : Unimplemented; reserved for changing portrait

Menu
----

These codes relate to popping up a menu for the player to select a choice from.

* ``\128\tile\cset\width\height\flip`` : Sets the menu cursor, args same as ``\28`` above
* ``\129\pos\up_pos\down_pos\left_pos\right_pos`` : Creates a menu cursor position. 'pos' is the index of this position, while the 4 directional values are the position that will be moved to when that direction is pressed. If a directional pos is the same as 'pos', that direction will be disabled
* ``\130`` : Start running menu. If no ``\129`` codes have been run, the menu instantly exitswith the '0' pos selected
* ``\131\pos\newstring`` : Goto if menu selection. Switches to the new string if 'pos' is the selected menu position

Switch
------

These codes conditionally change to a different string. If you attempt to switch to a string that does not exist, it an empty string is loaded.

* ``\3\register\value\newstring`` : if ``Screen->D[register] >= value``, switches to newstring
* ``\23\dmap\screen\register\value\newstring`` : Same as ``\3``, but for any screen
* ``\4\factor\newstring`` : Randomly has a 1/factor chance to switch to newstring
* ``\5\itemid\newstring`` : Goes to newstring if the specified item is owned
* ``\6\counter\value\newstring`` : Goto newstring if the specified counter is >= value
* ``\7\counter\percent\newstring`` : Goto newstring if the specified counter is at least percent full
* ``\8\levelid\newstring`` : Goto newstring if the triforce from the specified level is owned
* ``\9\numtriforce\newstring`` : Goto newstring if at least numtriforce triforce pieces are owned
* ``\131\pos\newstring`` : Goto if menu selection. Switches to the new string if 'pos' is the selected menu position

Control Mod
-----------

Modifying counter values.

* ``\10\ctr\val`` : Adds 'val' to 'ctr'
* ``\11\ctr\val`` : Subtracts 'val' from 'ctr'
* ``\12\ctr\val`` : Sets 'ctr' to 'val'
* ``\13\ctr\percent`` : Adds 'percent' % to 'ctr'
* ``\14\ctr\percent`` : Subtracts 'percent' % from 'ctr'
* ``\15\ctr\percent`` : Sets 'ctr' to 'percent' % full
* ``\16\itemid`` : Gives the player 'itemid', silently. The item is not held up, nor does its' pickup script run; though its' counter effects will
* ``\17\itemid`` : Takes 'itemid' from the player, silently. Its' pickup counter modifications will be reversed when removed

Misc
----

* ``\18\dmap\screen\x\y\effect\sound`` : Warps the player to 'dmap,screen'

   If x < 0, then y is treated as a return square(0-3 for return squares A-D, 5 for pit warp).

   If x >= 0, then x and y act as destination coordinates.

   Warp effects:

   0. WARPEFFECT_NONE
   1. WARPEFFECT_ZAP
   2. WARPEFFECT_WAVE
   3. WARPEFFECT_INSTANT
   4. WARPEFFECT_OPENWIPE

* ``\19\dmap\screen\register\value`` : Sets ``Screen->D[register] = value`` for the specified screen
* ``\20\sfxid`` : Plays a specified sfx
* ``\21\MIDIid`` : Plays a specified quest midi (cannot play the built-in midis)
* ``\29`` : Immediately exits the current string. If there is a 'next string', it will begin immediately. The prompt to press 'A' to continue will be skipped
* ``\30`` : Pauses the string and prompts the player to press A to continue (as normally occurs at the end of a string)
* ``\132\perm`` : Triggers secrets, which are permanent unless 'perm' is '0'
* ``\133\flag\state`` : Sets a screen state on the current screen
* ``\134\map\screen\flag\state`` : Same as ``\133`` above, for a remote map/screen
* ``\136\generic_script_id\force_redraw`` : Run the specified generic script, in the RunFrozen mode

ZScript
-------

SCCs can also be set via ZScript, but you must understand the way the control codes are encoded: where in the editor you'd write ``\1``, that actually is converted to a byte of value ``1``. The ``%c`` :ref:`sprintf<globals_fun_sprintf>` format code is useful for this.

The following is an example script found in ZC's tests folder, which gives some insight into how to programmatically use SSCs.

.. only:: html

   .. zscript::
      :url: https://raw.githubusercontent.com/ZQuestClassic/ZQuestClassic/refs/heads/main/tests/scripts/playground/auto/scc.zs
