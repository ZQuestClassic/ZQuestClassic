.. _frame_timings:

Frame Timings
=============

.. |Waitdraw| replace:: :ref:`Waitdraw()<globals_fun_waitdraw>`
.. |WaitTo| replace:: :ref:`WaitTo()<globals_fun_waitto>`
.. |ScriptTiming| replace:: :ref:`ScriptTiming<globals_enum_scripttiming>`

Every frame of gameplay, the engine runs its own systems and each type of
:ref:`script<zslang_scripts>` in a fixed order. This order matters whenever one
script reads a value that the engine (or another script) writes during the same
frame, and it is what |Waitdraw| and |WaitTo| let you position a script around.

The list below is the order for a normal frame of gameplay. Screen scrolling,
warping, and :ref:`frozen scripts<frozen_scripts>` each use their own loop.
The ``SCR_TIMING_`` name on each step is the |ScriptTiming| value that a
passively-running :ref:`generic script<classes_genericdata>` can wait for with
|WaitTo|; a passive generic script gets a chance to run at every one of these
points.

#. ``SCR_TIMING_START_FRAME``: Script draws queued last frame are cleared.
#. ``SCR_TIMING_POST_COMBO_ANIM``: Combos animate.
#. ``SCR_TIMING_POST_POLL_INPUT``: Controller and keyboard input is read.
#. ``SCR_TIMING_POST_FFCS``: FFCs update. This first runs the Screen script for
   each screen in the region, then the script of every FFC (in FFC ID order),
   then moves the FFCs.
#. ``SCR_TIMING_POST_GLOBAL_ACTIVE``: The Global Active script runs, up to its
   |Waitdraw|.
#. ``SCR_TIMING_POST_PLAYER_ACTIVE``: The Hero Active script runs, up to its
   |Waitdraw|.
#. ``SCR_TIMING_POST_DMAPDATA_ACTIVE``: The DMap Active script runs, up to its
   |Waitdraw|.
#. ``SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN``: The DMap Scripted Passive
   Subscreen script runs, up to its |Waitdraw|.
#. ``SCR_TIMING_POST_COMBOSCRIPT``: Combo scripts run, layer by layer, position
   by position.
#. ``SCR_TIMING_POST_PUSHBLOCK``: Moving push blocks update.
#. ``SCR_TIMING_POST_ITEMSPRITE_SCRIPT``: Itemsprite scripts run.
#. ``SCR_TIMING_POST_ITEMSPRITE_ANIMATE``: Items animate (including conveyors
   moving them).
#. ``SCR_TIMING_POST_NPC_ANIMATE``: Enemies animate. Each enemy's npc script
   runs as part of that enemy's update.
#. ``SCR_TIMING_POST_EWPN_ANIMATE``: EWeapons animate.
#. ``SCR_TIMING_POST_EWPN_SCRIPT``: EWeapon scripts run.
#. ``SCR_TIMING_POST_OLD_ITEMDATA_SCRIPT``: ``HitBy[]`` arrays are cleared for
   the frame. If the compatibility |qr| ``qr_OLD_ITEMDATA_SCRIPT_TIMING`` is on,
   itemdata scripts run here instead of later.
#. ``SCR_TIMING_POST_PLAYER_ANIMATE``: The Hero animates: movement, collision
   with enemies and weapons, picking up items (which runs Pickup scripts), and
   anything triggered by that such as warps, scrolling, or opening the
   subscreen.
#. ``SCR_TIMING_POST_NEW_ITEMDATA_SCRIPT``: Itemdata (Action / Passive) scripts
   run.
#. ``SCR_TIMING_POST_CASTING``: Magic casting animations update.
#. ``SCR_TIMING_POST_LWPN_ANIMATE``: LWeapons animate. Each lweapon's script
   runs as part of that weapon's update.
#. ``SCR_TIMING_POST_DECOPARTICLE_ANIMATE``: Decorations and particles animate.
#. ``SCR_TIMING_POST_COLLISIONS_PALETTECYCLE``: The hookshot updates, collisions
   are checked, whistle-dried water refills, and the palette cycles.
#. ``SCR_TIMING_WAITDRAW``: The Waitdraw point.

   Every script that called |Waitdraw| this frame now resumes, in this order,
   with a timing after each group:

   - ``SCR_TIMING_POST_GLOBAL_WAITDRAW``: Global Active
   - ``SCR_TIMING_POST_PLAYER_WAITDRAW``: Hero Active
   - ``SCR_TIMING_POST_DMAPDATA_ACTIVE_WAITDRAW``: DMap Active
   - ``SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN_WAITDRAW``: DMap Scripted Passive Subscreen
   - ``SCR_TIMING_POST_SCREEN_WAITDRAW``: Screen scripts
   - ``SCR_TIMING_POST_FFC_WAITDRAW``: FFC scripts
   - ``SCR_TIMING_POST_COMBO_WAITDRAW``: Combo scripts
   - ``SCR_TIMING_POST_ITEM_WAITDRAW``: Itemdata scripts
   - ``SCR_TIMING_POST_NPC_WAITDRAW``: NPC scripts
   - ``SCR_TIMING_POST_EWPN_WAITDRAW``: EWeapon scripts
   - ``SCR_TIMING_POST_LWPN_WAITDRAW``: LWeapon scripts
   - ``SCR_TIMING_POST_ITEMSPRITE_WAITDRAW``: Itemsprite scripts

#. ``SCR_TIMING_PRE_DRAW``, ``SCR_TIMING_POST_DRAW``: The screen is drawn, with
   the first timing running just before and the second just after. All script
   draws queued during the frame are rendered here.
#. ``SCR_TIMING_POST_STRINGS``: Message strings advance.
#. ``SCR_TIMING_END_FRAME``: End of frame.

.. note::

	Steps 10 through 22 (push blocks through collisions) are skipped while the
	screen is frozen: while a message string is displayed with the |qr|
	``Messages Freeze All Action`` on, or while a ``Screen Freeze`` combo is on
	the screen. (During a message freeze, enemies flagged to ignore it still
	animate, and the item and npc animate timings still fire.)

	Individual systems can also be paused from script with
	:ref:`Game->Suspend[]<globals_game_var_suspend>`.

.. tip::

	Because the Global Active script runs *before* enemies, weapons, and the
	Hero update, values such as the Hero's position and direction read there
	are from the *previous* frame's update. Read them after |Waitdraw| to see
	this frame's final values before they are drawn.

Game start
----------

When a game is launched, the start-up scripts run before the first frame of
gameplay (``Global Script ~Init``, ``Hero Script Init``, ``Global Script
onSaveLoad``, ``Global Script onLaunch``, ``Global Script onContGame``). See
:doc:`/zscript/lang/script_types` for when each of these runs.
