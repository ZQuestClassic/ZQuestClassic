Script Types
============

.. |frzmode| replace:: :ref:`frozen script mode<frozen_scripts>`

This section describes the various different types of scripts,
and how they differ from one another.

Global Scripts
--------------

.. scrinfo:: Global Script ~Init
	:type: global
	:initd: 0

	Runs when a new save file is first created, for a single frame only.
	All scripts with the `@InitScript()` :ref:`annotation<annotations>`
	as well as a script named `global script Init` will be "merged",
	all being run at this time.

.. scrinfo:: Global Script Active
	:type: global
	:initd: 0
	:split_at: 1

	Starts running when the quest is launched, and continues until the
	script exits. Generally paired with an infinite loop to run forever
	during the entire quest, to run main global logic.

	The script will stop (and restart thereafter) each of a few events:

	- The Hero dies (script will see the Hero's HP as `0` for a single frame first, and can revive them)
	- The player opens the engine 'F6 Menu'
	- Any script calls :ref:`Game->Continue()<globals_game_fun_continue>`, :ref:`Game->End()<globals_game_fun_end>`, or other similar functions that 'continue' or 'exit' the game.

.. scrinfo:: Global Script onExit
	:type: global
	:initd: 0
	:split_at: 1

	.. todo::

		desc

.. scrinfo:: Global Script onSaveLoad
	:type: global
	:initd: 0
	:split_at: 1

	.. todo::

		desc

.. scrinfo:: Global Script onLaunch
	:type: global
	:initd: 0
	:split_at: 1

	.. todo::

		desc

.. scrinfo:: Global Script onContGame
	:type: global
	:initd: 0
	:split_at: 1

	.. todo::

		desc

.. scrinfo:: Global Script onF6Menu
	:type: global
	:initd: 0
	:split_at: 1

	.. todo::

		desc

.. scrinfo:: Global Script onSave
	:type: global
	:initd: 0
	:split_at: 1

	.. todo::

		desc

Hero Scripts
------------

.. todo::

	list hero script slots

Itemdata Scripts
----------------

.. scrinfo:: Itemdata Action Scripts
	:type: itemdata
	:pointer: itemdata
	:initd: 8
	:initd_str: (Shared with the item's Pickup and Sprite scripts)

	Runs when the item it's attached to via the Item Editor is used by the
	Hero. It continues to run until it exits, and the item cannot be used
	again while it's script still runs.

	Incompatible with Itemdata Passive Scripts.

.. scrinfo:: Itemdata Pickup Scripts
	:type: itemdata
	:pointer: itemdata
	:initd: 8
	:initd_str: (Shared with the item's Action/Passive and Itemsprite scripts)

	Runs when the item it's attached to via the Item Editor is picked up
	by the Hero. Script can only run for 1 frame; attempting to use
	`Waitframe` or similar will exit the script.

.. scrinfo:: Itemdata Passive Scripts
	:type: itemdata
	:pointer: itemdata
	:initd: 8
	:initd_str: (Shared with the item's Pickup and Sprite scripts)
	:split_at: 1

	Runs when the item it's attached to via the Item Editor is owned by the
	Hero. It continues to run until it exits, and starts again automatically
	when the Hero respawns.

	Replaces the Itemdata Action Script, if the `Constant Script` flag is
	checked in the Item Editor. Due to this, these are incompatible with
	Itemdata Action Scripts.

Screen-Based Scripts
--------------------

.. scrinfo:: Screen Scripts
	:type: screendata
	:pointer: Screen
	:initd: 8
	:split_at: 1

	Runs starting when the screen it's at is entered (after scrolling
	onto the screen)

	Additionally runs for a single frame during the loading of the screen
	(before scrolling onto it) if the flag ``Run On Screen Init`` is checked.

.. scrinfo:: FFC Scripts
	:type: ffc
	:pointer: ffc
	:initd: 8
	:split_at: 1

	Runs when the FFC it's attached to via the FFC Editor exists
	with a combo set other than '0' (after scrolling onto the screen)

	Additionally runs for a single frame during the loading of the screen
	(before scrolling onto it) if the FFC flag ``Run Script at Screen Init``
	is checked.

.. scrinfo:: Combo Scripts
	:type: combodata
	:pointer: combodata
	:initd: 8
	:split_at: 1

	Runs when the combo it's attached to via the Combo Editor exists,
	on any *enabled* layer (after scrolling onto the screen). The layers
	can be set by enabling the |qrs| ``Combos Run Scripts on Layer #``
	for each layer number.

	Combo Scripts are the only scripts capable of accessing several combodata
	variables, accessible by :ref:`this->X<classes_combodata_var_x>`,
	:ref:`this->Y<classes_combodata_var_y>`,
	:ref:`this->Pos<classes_combodata_var_pos>`,
	:ref:`this->Layer<classes_combodata_var_layer>`. These values all
	are specific to the *combo position on screen* of the currently running
	script, rather than the *combo ID*.

'Sprite' Scripts
----------------

'Sprite' scripts are any script types that refer to a `sprite`\ -typed object,
i.e. `npc`, `itemsprite`, `lweapon`, `eweapon` (`hero` excluded from this section)

.. scrinfo:: Enemy Scripts
	:type: npc
	:pointer: npc
	:initd: 8

	Runs when the npc it's attached to via the Enemy Editor is alive on-screen.

.. scrinfo:: Itemsprite Scripts
	:type: itemsprite
	:pointer: itemsprite
	:initd: 8
	:initd_str: (Shared with the item's Action/Passive and Pickup scripts)

	Runs when the item it's attached to via the Item Editor exists on-screen
	as a collectable object.

.. scrinfo:: LWeapon Scripts
	:type: lweapon
	:pointer: lweapon
	:initd: 8

	Runs when the weapon it's attached to via the Item Editor exists.

.. scrinfo:: EWeapon Scripts
	:type: eweapon
	:pointer: eweapon
	:initd: 8

	Runs when the weapon it's attached to via the Enemy Editor exists.

Subscreen & DMap Scripts
------------------------

.. scrinfo:: Subscreen Scripts
	:type: subscreendata
	:pointer: subscreendata
	:initd: 8

	Runs when the Active Subscreen it's attached to via the Subscreen Editor
	is open.

.. scrinfo:: DMapData Active Scripts
	:type: dmapdata
	:pointer: dmapdata
	:initd: 8

	Runs starting when the DMap it's attached to is entered. Continues
	running during screen scrolling.

.. scrinfo:: DMapData Scripted Active Subscreen
	:type: dmapdata
	:pointer: dmapdata
	:initd: 8
	:initd_str: (Shared with Scripted Passive Subscreen)
	:split_at: 1

	If a script of this slot is assigned for the current dmap, then pressing
	the ``START`` button will no longer open the engine Active Subscreen, but
	will **instead** launch this script, in |frzmode|.

	|qrs| are available to allow the 'DMapData Active Script' and 'DMapData
	Passive Subscreen Script' to run during this script.

	If you trigger a warp during this script to a dmap with a different script,
	that dmap's script will be immediately loaded instead.

.. scrinfo:: DMapData Scripted Passive Subscreen
	:type: dmapdata
	:pointer: dmapdata
	:initd: 8
	:initd_str: (Shared with Scripted Active Subscreen)

	Runs starting when the DMap it's attached to is entered. Continues
	running during screen scrolling. Has a |qr| to allow it to run during
	special timings (wipes/refills) when other scripts normally do not run.

.. scrinfo:: DMapData Scripted Map
	:type: dmapdata
	:pointer: dmapdata
	:initd: 8
	:split_at: 1

	If a script of this slot is assigned for the current dmap, then pressing
	the ``MAP`` button will no longer open the engine Map, but will
	**instead** launch this script, in |frzmode|.

	If you trigger a warp during this script to a dmap with a different script,
	that dmap's script will be immediately loaded instead.

Generic Scripts
---------------

.. todo::

	list generic script run types


.. _frozen_scripts:

'Frozen Mode' scripts
---------------------

When a script runs in a 'Frozen Mode', everything (or, nearly everything) in the
entire engine will be paused. Other scripts will not run either, unless specifically
stated as an exception.
