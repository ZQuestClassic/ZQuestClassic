Script Types
============

This section describes the various different types of scripts,
and how they differ from one another.

Global Scripts
--------------

.. todo::

	list global script slots

Hero Scripts
------------

.. todo::

	list hero script slots

Item Scripts
------------

.. scrinfo:: Itemdata Action Scripts
	:type: itemdata
	:pointer: itemdata
	:initd: 8
	:initd_str: (Shared with the item's Pickup and Sprite scripts)

	Runs when the item it is attached to via the Item Editor is used by the
	Hero. It continues to run until it exits, and the item cannot be used
	again while it's script still runs.

	Incompatible with Itemdata Passive Scripts.

.. scrinfo:: Itemdata Pickup Scripts
	:type: itemdata
	:pointer: itemdata
	:initd: 8
	:initd_str: (Shared with the item's Action/Passive and Itemsprite scripts)

	Runs when the item it is attached to via the Item Editor is picked up
	by the Hero. Script can only run for 1 frame; attempting to use
	`Waitframe` or similar will exit the script.

.. scrinfo:: Itemdata Passive Scripts
	:type: itemdata
	:pointer: itemdata
	:initd: 8
	:initd_str: (Shared with the item's Pickup and Sprite scripts)

	Runs when the item it is attached to via the Item Editor is owned by the
	Hero. It continues to run until it exits, and starts again automatically
	when the Hero respawns.

	Replaces the Itemdata Action Script, if the `Constant Script` flag is
	checked in the Item Editor. Due to this, these are incompatible with
	Itemdata Action Scripts.

.. scrinfo:: Itemsprite Scripts
	:type: itemsprite
	:pointer: itemsprite
	:initd: 8
	:initd_str: (Shared with the item's Action/Passive and Pickup scripts)

	Runs when the item it is attached to via the Item Editor exists on-screen
	as a collectable 'itemsprite' object.

Misc Scripts
------------

.. scrinfo:: FFC Scripts
	:type: ffc
	:pointer: ffc
	:initd: 8

	Assignable to each of the 128 FFCs available per-screen. Will not run
	if the FFC's combo is '0'.

.. scrinfo:: Subscreen Scripts
	:type: subscreendata
	:pointer: subscreendata
	:initd: 8

	Runs when the Active Subscreen it is attached to is open.

.. todo::

	|wip|

