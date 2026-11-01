Combo trigger options
=====================

.. TODO: generate this page from src/zq/dialog/combo_trigger_editor.cpp, which has every option's
   tab, label and help text, so it can't drift from the editor. Until then, update it by hand when
   the Combo Trigger Editor changes.

Every option in the Combo Trigger Editor, grouped by the editor tab it's in. For how they fit
together, read :doc:`/tutorials/combo_triggers` first.

The **Kind** column says what each option does to the trigger:

* **Cause**: fires the trigger. A trigger needs at least one.
* **Condition**: must be true for the trigger to do anything, but never fires it.
* **Effect**: happens when the trigger fires.
* **Weapon effect**: happens to the weapon that fired the trigger.
* **Setting**: a value or switch that other options on the trigger use.

Options that only some versions have are marked with the versions that have them, like
:since:`3.0`. In 2.55 a combo has a single trigger, edited in the Combo Editor's ``Triggers`` tab,
and the options are spread across different tabs (see
:ref:`Differences in 2.55 <combo-triggers-differences-in-2-55>`). A few also have different names
there: ``Invert Prox. Req`` is ``Invert Proximity Req``,
and ``Spawned Item Pickup`` is four checkboxes (``Hold Up Item``, ``Time Out Item``,
``Item Triggers Secrets`` and ``Can Hook Item``).

The trigger's ``Label`` :since:`3.0`, at the top of the editor, names it in the Combo Editor's
trigger list. A ``P`` button next to a field opens a list to pick its values from.

Weapons
-------

LWeapons
^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - Sword, Sword Beam, Boomerang, Bomb Boom, Super Bomb Boom, Placed Bomb, Placed Super Bomb,
       Arrow, Fire, Whistle, Bait, Wand, Magic, Wind, Refl. Magic, Refl. Fireball, Refl. Rock,
       Refl. Arrow, Refl. Fire, Refl. Fire 2, Hammer, Hookshot, Sparkle, Byrna, Refl. Beam, Stomp,
       Thrown, Quake Hammer, S. Quake Hammer, Custom Weapon 1-10
     - Cause
     - Fires when a player weapon (LWeapon) of this type touches the combo.
   * - ``Min Level (Applies to all)``
     - Condition
     - The LWeapon's level must be at least this. Applies to every LWeapon type above.
   * - ``Max level instead``
     - Setting
     - Makes ``Min Level`` a maximum: the LWeapon's level must be at most this.

EWeapons
^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - Fireball, Arrow, Boomerang, Sword, Rock, Magic, Bomb Blast, SBomb Blast, Lit Bomb, Lit SBomb,
       Fire Trail, Flame, Wind, Flame 2, Custom Weapon 1-10
     - Cause
     - Fires when an enemy weapon (EWeapon) of this type touches the combo.

Other (weapons)
^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Normal Fire``, ``Strong Fire``, ``Magic Fire``, ``Divine Fire``
     - Cause
     - Fires when a weapon (player or enemy) burning with this strength of fire touches the combo.
       Much of the engine's own fire, like a candle's flame, always counts as ``Normal Fire``. The
       quest rule ``'Burning' triggers don't count as 'weapon triggers'`` changes how these work
       (see :ref:`Weapons <combo-triggers-weapons>`).
   * - ``Only Grounded Weapons``
     - Condition
     - Only weapons on the ground (Z = 0) count. Has no effect in sideview.

Weapon Effects
^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Kill Triggering Weapon``
     - Weapon effect
     - Destroys the weapon. Doesn't work on swords or hammers.
   * - ``Once Per Weapon`` :since:`3.0, 2.55.18+`
     - Weapon effect
     - Once a weapon has fired the trigger, that weapon doesn't fire it again. The combo's other
       triggers aren't affected (see :ref:`Weapons <combo-triggers-weapons>`).
   * - ``Ignite Weapon (Normal)``, ``(Strong)``, ``(Magic)``, ``(Divine)``
     - Weapon effect
     - Makes the weapon burn with that strength of fire, so it fires triggers with the matching
       burning cause.
   * - ``...Unignite Instead`` :since:`3.0`
     - Setting
     - The ``Ignite Weapon`` options put the weapon's fire out instead.
   * - ``Weapon Separate Triggers``
     - Setting
     - When a weapon fires the trigger, only the weapon effects happen, and the trigger's
       conditions and ``Cooldown`` don't apply to them. Everything else only happens for the
       trigger's other causes.

Other
-----

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Btn: Top``, ``Btn: Bottom``, ``Btn: Left``, ``Btn: Right``
     - Cause
     - Fires when the player presses one of the ``Buttons`` while standing against the combo from
       that side.
   * - ``Buttons``
     - Setting
     - Which buttons the ``Btn:`` causes use.
   * - ``Button Prompt``
     - Setting
     - A combo shown near the player when they're in position to press the button.
   * - ``Fail Prompt`` :since:`3.0`
     - Setting
     - The prompt shown when the trigger's conditions fail.
   * - ``Prompt Xoffset``, ``Prompt Yoffset``
     - Setting
     - Where the prompt is drawn, relative to the player.
   * - ``LightBeam``
     - Setting
     - Which light beam ``Light On->`` and ``Light Off->`` react to: 0 for any beam, or 1-32 for
       only that one.
   * - ``Timer``
     - Cause
     - Fires the trigger every *n* frames, until the combo changes.
   * - ``Push Time`` :since:`3.0`
     - Setting
     - How many frames the player has to push before ``Push->`` fires. 2.55 always uses 8.
   * - ``Step->``
     - Cause
     - Fires every frame the player stands on the combo.
   * - ``Step-> (Sensitive)``
     - Cause
     - Like ``Step->``, but even one pixel of overlap counts.
   * - ``Light On->``
     - Cause
     - Fires every frame a light beam matching ``LightBeam`` hits the combo.
   * - ``Light Off->``
     - Cause
     - Fires every frame no matching light beam hits the combo.
   * - ``Lens On->``
     - Cause
     - Fires every frame a Lens of Truth item with ``Triggers Lens Trigflag`` checked is in use.
   * - ``Lens Off->``
     - Cause
     - Fires every frame no such lens is in use.
   * - ``Push->``
     - Cause
     - Fires when the player pushes against the combo, and again every ``Push Time`` frames while
       they keep pushing.
   * - ``Pushed->``
     - Cause
     - Fires when the combo, as a pushblock, finishes moving into its new position.
   * - ``Dive->``, ``Dive-> (Sensitive)``
     - Cause
     - Fires every frame the player dives on the combo. With the sensitive version, even one pixel
       of overlap counts.
   * - ``Swim->``, ``Swim-> (Sensitive)`` :since:`3.0`
     - Cause
     - Fires every frame the player swims on the combo. With the sensitive version, even one pixel
       of overlap counts.
   * - ``Screen Loads->`` :since:`3.0`
     - Cause
     - Fires when the screen (or region) loads, after its LevelStates and ExStates are applied.
   * - ``Screen Unloads->`` :since:`3.0`
     - Cause
     - Fires when the screen (or region) unloads. For a warp this is after the warp effect, and for
       scrolling it's before the scroll starts.
   * - ``Always Triggered``
     - Cause
     - Fires every frame. Only on layer 0 and FFCs, unless the ``Auto Combos Work On ...`` quest
       rules are on.
   * - ``Shutter->``
     - Cause
     - Fires when the screen's shutters open.
   * - ``Secrets->``
     - Cause
     - Fires when the screen's secrets are triggered.
   * - ``ComboType Causes->`` :since:`3.0`
     - Cause
     - Fires when the combo type's own event happens, such as the player falling into a pitfall or
       drowning in liquid. Grayed out for combo types that don't have one.
   * - ``Land Here->`` :since:`3.0`
     - Cause
     - Fires when the player lands on the combo from a jump.
   * - ``Land Anywhere->`` :since:`3.0`
     - Cause
     - Fires when the player lands from a jump anywhere on the screen.
   * - ``Block (Same Layer)->`` :since:`3.0`
     - Cause
     - Fires when a pushblock is about to settle on top of this combo, replacing it.
   * - ``Block (Any Layer)->`` :since:`3.0`
     - Cause
     - Fires when a pushblock is about to settle at this combo's position, on any layer.
   * - ``Enemies->``
     - Cause
     - Fires every frame once all the screen's enemies are defeated.

Effects
-------

Misc
^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Music`` :since:`3.0`
     - Effect
     - Plays this music. ``(None)`` does nothing, ``(Play Screen/DMap Music)`` plays the screen's
       or DMap's music, and ``(Stop Music)`` stops the music.
   * - ``Set Music Refresh`` :since:`3.0`
     - Effect
     - Changes when the engine next tries to replay the screen's music. The mode goes back to
       ``Screen`` after the next refresh.
   * - ``Combo Change``
     - Effect
     - Changes the combo by this much: 1 is the next combo, -1 the previous one.
   * - ``Absolute`` (Combo Change) :since:`3.0`
     - Setting
     - ``Combo Change`` is a combo ID to change into, instead of an amount.
   * - ``CSet Change``
     - Effect
     - Changes the combo's CSet by this much.
   * - ``Absolute`` (CSet Change) :since:`3.0`
     - Setting
     - ``CSet Change`` is a CSet to change to, instead of an amount.
   * - ``Triggers Secrets``
     - Effect
     - Triggers the screen's secrets. They're permanent if the DMap and screen allow it and the
       screen's ``Temporary Secrets`` flag isn't checked.
   * - ``Toggle Darkness`` :since:`3.0`
     - Effect
     - Toggles whether the screen is dark, until the player leaves it.
   * - ``Reset Anim``
     - Effect
     - Restarts the combo's animation. If the combo changed, the new combo's animation restarts.
   * - ``Cancel Further Triggers`` :since:`3.0`
     - Effect
     - The combo's triggers below this one don't fire for this cause.
   * - ``->ComboType Effects``
     - Effect
     - Does what the combo type does on its own, like opening a chest or showing a sign's
       string. Grayed out for combo types that don't have an effect.
   * - ``Kill Enemies``
     - Effect
     - Kills every enemy on the screen, like the "kill all enemies" item.
   * - ``Clear Enemies``
     - Effect
     - Removes every enemy from the screen.
   * - ``Clear LWeapons``
     - Effect
     - Removes every player weapon from the screen.
   * - ``Clear EWeapons``
     - Effect
     - Removes every enemy weapon from the screen.
   * - ``RunFrozen``
     - Effect
     - Runs this generic script in its "Frozen" mode (see ``genericdata->RunFrozen()``).
   * - ``Trigger String`` :since:`3.0`
     - Effect
     - Shows this message string.
   * - ``Fail String`` :since:`3.0`
     - Effect
     - Shows this message string when the trigger fires but its conditions fail.
   * - ``Set Gravity`` :since:`3.0`
     - Effect
     - Sets the screen's gravity and terminal velocity to ``Gravity`` and
       ``Terminal Velocity``.
   * - ``Revert Gravity`` :since:`3.0`
     - Effect
     - Puts the screen's gravity back to its editor settings.
   * - ``Gravity``, ``Terminal Velocity`` :since:`3.0`
     - Setting
     - The values ``Set Gravity`` uses.

Player
^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Dest X``, ``Dest Y``, ``Dest Z`` :since:`3.0`
     - Effect
     - Moves the player by this much along that axis.
   * - ``Set X (Abs)``, ``Set Y (Abs)``, ``Set Z (Abs)`` :since:`3.0`
     - Setting
     - The matching ``Dest`` value is the exact position to put the player at, not an amount to
       move them. The player is teleported, with no collision checks.
   * - ``Set X (Rel)``, ``Set Y (Rel)`` :since:`3.0`
     - Setting
     - The matching ``Dest`` value is a position relative to this combo (for an FFC, the center of
       its hitbox).
   * - ``Player Bounce`` :since:`3.0`
     - Effect
     - Makes the player jump with this force. Negative values make them fall.
   * - ``Player Dir`` :since:`3.0`
     - Effect
     - Turns the player to face this direction.
   * - ``Sword Jinx``, ``Item Jinx``, ``Shield Jinx``, ``Stun``, ``Bunny`` :since:`3.0`
     - Effect
     - Sets how long the player has this status: -2 does nothing, -1 lasts until cured, 0 cures
       it, and any other value lasts that many frames.
   * - ``Force Ice Physics`` :since:`3.0`
     - Effect
     - Applies this ``Icy Floor`` combo's physics to the player. Combo 0 stops forcing ice
       physics.
   * - ``Force Vx:``, ``Force Vy:`` :since:`3.0`
     - Effect
     - Sets the player's ice physics velocity.
   * - ``Respawn Here`` :since:`3.0`
     - Effect
     - Makes this combo's position the player's hazard respawn point. Until ``Reset Respawn``, the
       respawn point no longer follows the safe ground the player stands on.
   * - ``Reset Respawn`` :since:`3.0`
     - Effect
     - Lets the hazard respawn point follow safe ground again.

SFX
^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``SFX``
     - Effect
     - Plays this sound.
   * - ``Pan`` :since:`3.0`
     - Setting
     - -1 pans the sound by the combo's position on the screen. 0-255 sets the pan, with 128
       centered.
   * - ``Volume`` :since:`3.0`
     - Setting
     - Percentage of the sound's normal volume.
   * - ``Frequency`` :since:`3.0`
     - Setting
     - Frequency in Hz to play the sound at, or -1 for its normal frequency.
   * - ``Loop`` :since:`3.0`
     - Setting
     - The sound loops until something stops it.
   * - ``No Restart`` :since:`3.0`
     - Setting
     - If the sound is already playing, keep it playing (updating its pan, volume and frequency)
       instead of starting it over.
   * - ``Kill`` :since:`3.0`
     - Setting
     - Stops the sound instead of playing it.

Conditions
----------

General
^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Cooldown``
     - Setting
     - After firing, the trigger can't fire again for this many frames. Its weapon effects still
       happen. Unlike a failed condition, it doesn't show the ``Fail String`` or ``Fail Prompt``.
   * - ``Chance`` (*n* ``in`` *m*) :since:`3.0`
     - Condition
     - The trigger only fires *n* out of *m* times it otherwise would, using the game's random
       number generator.
   * - ``Req. On Screen`` :since:`3.0`
     - Condition
     - The combo must be at least partly inside the viewport, extended on each side by
       ``Viewport Range``.
   * - ``Req. Off Screen`` :since:`3.0`
     - Condition
     - The combo must be entirely outside the viewport, extended on each side by
       ``Viewport Range``. With ``Req. On Screen`` also checked, the trigger never fires.
   * - ``Viewport Range`` :since:`3.0`
     - Setting
     - How far past the viewport's edges ``Req. On Screen`` and ``Req. Off Screen`` look. Positive
       values reach further off screen.
   * - ``Req. Darkness`` :since:`3.0`
     - Condition
     - The screen must be dark. The quest rule ``Inverted Dark Combo Triggers`` swaps this and
       ``Req. No Darkness``.
   * - ``Req. No Darkness`` :since:`3.0`
     - Condition
     - The screen must not be dark.
   * - ``Req. Player Standing`` :since:`3.0`
     - Condition
     - The player must be standing on the ground. Works in sideview too.
   * - ``Req. Player Not Standing`` :since:`3.0`
     - Condition
     - The player must not be standing on the ground.
   * - ``Map View Only`` :since:`3.0`
     - Condition
     - The trigger only fires in the map view (the spacebar map).
   * - ``Not Map View`` :since:`3.0`
     - Condition
     - The trigger never fires in the map view.

Player Position
^^^^^^^^^^^^^^^

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Player X``, ``Player Y`` :since:`3.0`
     - Setting
     - The values the ``Req X`` and ``Req Y`` options compare against.
   * - ``Req X >=``, ``Req Y >=`` :since:`3.0`
     - Condition
     - The player's X (or Y) must be at least ``Player X`` (or ``Player Y``).
   * - ``Req X <=``, ``Req Y <=`` :since:`3.0`
     - Condition
     - The player's X (or Y) must be at most ``Player X`` (or ``Player Y``).
   * - ``...Relative`` (X and Y) :since:`3.0`
     - Setting
     - ``Player X`` or ``Player Y`` is relative to where the player would be if centered on the
       combo.
   * - ``Player Z`` :since:`3.0`
     - Condition
     - The player's Z must be at least this.
   * - ``Invert Z Req`` :since:`3.0`
     - Setting
     - The player's Z must be below ``Player Z``.
   * - ``Player Jump`` :since:`3.0`
     - Setting
     - The value the ``Req Jump`` options compare against.
   * - ``Req Jump >=`` :since:`3.0`
     - Condition
     - The player's jump value must be at least ``Player Jump``.
   * - ``Req Jump <=`` :since:`3.0`
     - Condition
     - The player's jump value must be at most ``Player Jump``.
   * - ``Proximity``
     - Condition
     - The player must be within this many pixels of the combo. It applies even when a copycat
       fires the trigger, so put it on the trigger that starts a copycat chain.
   * - ``Invert Prox. Req``
     - Setting
     - The player must be farther away than ``Proximity``.
   * - ``Use S. Hitbox`` :since:`3.0`
     - Setting
     - ``Proximity`` measures from the player's solidity hitbox, which is nearer their feet. This
       only makes a difference when the player's ``Large Hitbox`` is off.
   * - ``Square Area`` :since:`3.0`
     - Setting
     - ``Proximity`` checks a square instead of a circle.
   * - ``Player Facing`` :since:`3.0`
     - Condition
     - The player must be facing one of these directions.

Counter / Item / Enemy
----------------------

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Counter``, ``Amount``
     - Setting
     - The counter and amount the counter options use.
   * - ``Require >=``
     - Condition
     - The counter must have at least ``Amount``.
   * - ``Require <``
     - Condition
     - The counter must have less than ``Amount``.
   * - ``...Is Percent`` :since:`3.0`
     - Setting
     - ``Amount`` is a percentage of the counter's maximum.
   * - ``Apply Discount`` :since:`3.0`
     - Setting
     - ``Amount`` is discounted by the player's Wealth Medal item (or whatever the quest calls
       that item class), like shop prices.
   * - ``Consume Amount``
     - Effect
     - Takes ``Amount`` from the counter, if it has enough. A negative amount adds to it.
   * - ``...Gradual`` :since:`3.0`
     - Setting
     - ``Consume Amount`` drains or fills the counter gradually instead of all at once.
   * - ``Consume w/o trig``
     - Setting
     - ``Consume Amount`` happens even when the counter conditions fail or the trigger is on
       ``Cooldown``, every time a cause tries to fire the trigger. Conditions checked before the
       counter (``Req Item``, ``Proximity``, states and so on) still stop it.
   * - ``Partial Consume`` :since:`3.0`
     - Setting
     - ``Consume Amount`` happens even if the counter doesn't have enough.
   * - ``Spawn Enemy``
     - Effect
     - Spawns this enemy.
   * - ``Trigger ExState after enemy kill``
     - Setting
     - The trigger's ``ExState`` is set when the spawned enemy dies, not when the trigger fires.
   * - ``Req Item``
     - Condition
     - The player must have this item.
   * - ``Invert Item Req``
     - Setting
     - The player must *not* have ``Req Item``.
   * - ``Consume Item Req``
     - Effect
     - Takes ``Req Item`` away when the trigger fires.
   * - ``Spawn Item``
     - Effect
     - Spawns this item. A negative value drops from that dropset instead.
   * - ``Spawned Item Pickup``
     - Setting
     - Pickup flags for the spawned item.
   * - ``Spawns Special Item``
     - Setting
     - The spawned item is the screen's special item, so it doesn't spawn again once collected.
   * - ``Spawned Item auto-collects``
     - Setting
     - The player collects the spawned item right away.
   * - ``Trigger ExState after item pickup``
     - Setting
     - The trigger's ``ExState`` is set when the spawned item is picked up, not when the trigger
       fires.

States
------

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``ExState``
     - Effect
     - Sets this ExState (0-31) of the screen. While it's set, the combo reapplies its change when
       the screen loads (see :ref:`States <combo-triggers-states>`).
   * - ``Clear ExState on trigger`` :since:`3.0`
     - Setting
     - Unsets ``ExState`` instead.
   * - ``ExDoor Dir``, ``ExDoor Index``
     - Effect
     - Sets this extra door state, and the matching one on the neighboring screen. Works like
       ``ExState``.
   * - ``Clear ExDoor on trigger`` :since:`3.0`
     - Setting
     - Unsets the ExDoor state instead.
   * - ``Copycat``
     - Cause and effect
     - Combos with the same ``Copycat`` number fire together (see
       :ref:`Copycats <combo-triggers-copycats>`).
   * - ``Copycat doesn't trigger this`` :since:`3.0`
     - Setting
     - Other combos' copycats don't fire this trigger. It still fires them.
   * - ``LevelState``, ``GlobalState``
     - Setting
     - Which LevelState (0-31) or GlobalState (0-255) the state options use.
   * - ``LevelState->``, ``GlobalState->``
     - Cause
     - Fires when the state is toggled, and when the player enters a screen while it's on.
   * - ``->LevelState``, ``->GlobalState``
     - Effect
     - Toggles the state.
   * - ``GlobalState Timer``
     - Setting
     - If above 0, ``->GlobalState`` turns the state on for this many frames instead of toggling it.
   * - ``Contributes To TrigGroup``
     - Setting
     - Counts this combo toward ``Trigger Group`` (see
       :ref:`Trigger groups <combo-triggers-trigger-groups>`).
   * - ``Trigger Group``, ``Trigger Group Val``
     - Setting
     - The group (0-255) and the value the ``TrigGroup`` causes compare its count with.
   * - ``TrigGroup Less->``
     - Cause
     - Fires every frame the group's count is less than ``Trigger Group Val``.
   * - ``TrigGroup Greater->``
     - Cause
     - Fires every frame the group's count is greater than ``Trigger Group Val``.
   * - ``Copycat`` (Large Combos) :since:`3.0`
     - Setting
     - The large-combo copycat number the two options below use.
   * - ``->Large Copycat`` :since:`3.0`
     - Effect
     - Fires the ``Large Copycat->`` triggers with the same number on the other combos of this
       large combo.
   * - ``Large Copycat->`` :since:`3.0`
     - Cause
     - Fires when another combo of this large combo fires a ``->Large Copycat`` trigger with the
       same number.
   * - ``Req States`` (GlobalState Conditions) :since:`3.0`
     - Condition
     - These GlobalStates must all be on. Like ``Unreq States``, it isn't checked for a combo
       outside the current region when ``Trig DMap Level`` is -1.
   * - ``Unreq States`` (GlobalState Conditions) :since:`3.0`
     - Condition
     - These GlobalStates must all be off.

Graphics
--------

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Tint Palette R/G/B`` :since:`3.0`
     - Effect
     - Tints the palette, like the script ``Graphics->Tint()``. Tints add up.
   * - ``->ClearTint`` :since:`3.0`
     - Effect
     - Clears all tint, like ``Graphics->ClearTint()``. Runs before ``Tint Palette``.
   * - ``Load Level Palette`` :since:`3.0`
     - Effect
     - Loads this level palette over the current one. -1 does nothing, -2 loads the screen's own
       palette.
   * - ``Load Boss Palette`` :since:`3.0`
     - Effect
     - Loads this extra sprite palette (0-29) into the boss CSet (CSet 14). -1 does nothing.
   * - ``Quake`` :since:`3.0`
     - Effect
     - Shakes the screen for this many frames. -1 does nothing.
   * - ``Wavy`` :since:`3.0`
     - Effect
     - Makes the screen wavy for this many frames. -1 does nothing.

Level / Screen Based
--------------------

.. list-table::
   :header-rows: 1
   :widths: 25 15 60

   * - Option
     - Kind
     - What it does
   * - ``Trig DMap Level`` :since:`3.0`
     - Setting
     - The level the Level Items options and the LevelState conditions use. -1 is the current
       DMap's level.
   * - ``Req Items`` (Level Items) :since:`3.0`
     - Setting
     - The level items (map, compass, boss key, ...) the options below use.
   * - ``Require All`` :since:`3.0`
     - Condition
     - The level must have all of ``Req Items``.
   * - ``Require Not All`` :since:`3.0`
     - Condition
     - The level must be missing at least one of ``Req Items``.
   * - ``->Set`` (Level Items) :since:`3.0`
     - Effect
     - Gives the level ``Req Items``. With ``->Unset`` also checked, toggles them.
   * - ``->Unset`` (Level Items) :since:`3.0`
     - Effect
     - Takes ``Req Items`` away from the level.
   * - ``Req States`` (LevelState Conditions) :since:`3.0`
     - Condition
     - These LevelStates of ``Trig DMap Level`` must all be on.
   * - ``Unreq States`` (LevelState Conditions) :since:`3.0`
     - Condition
     - These LevelStates of ``Trig DMap Level`` must all be off.
   * - ``Trig Map / Screen`` :since:`3.0`
     - Setting
     - The screen the screen state, ExState and ExDoor conditions check. Map 0 is the combo's
       own screen.
   * - ``Req States`` (Screen State / ExState) :since:`3.0`
     - Condition
     - These screen states (secrets, item, ...) must all be set.
   * - ``Unreq States`` (Screen State / ExState) :since:`3.0`
     - Condition
     - These screen states must all be unset.
   * - ``Req ExStates`` :since:`3.0`
     - Condition
     - These ExStates must all be set.
   * - ``Unreq ExStates`` :since:`3.0`
     - Condition
     - These ExStates must all be unset.
   * - ``Req`` (Screen ExDoors) :since:`3.0`
     - Condition
     - These ExDoor states, picked per direction, must all be set.
   * - ``Unreq`` (Screen ExDoors) :since:`3.0`
     - Condition
     - These ExDoor states must all be unset.
   * - ``State Number`` (Position States) :since:`3.0`
     - Setting
     - Which of the combo position's 8 states (0-7) the options below use.
   * - ``Require`` (Position States) :since:`3.0`
     - Condition
     - The position state must be set. With ``Require Not`` also checked, the trigger never fires.
   * - ``Require Not`` (Position States) :since:`3.0`
     - Condition
     - The position state must not be set.
   * - ``Cause`` (Position States) :since:`3.0`
     - Cause
     - While the position state is set, the combo reapplies its change, like an ``ExState``.
   * - ``->Set`` (Position States) :since:`3.0`
     - Effect
     - Sets the position state. With ``->Unset`` also checked, toggles it.
   * - ``->Unset`` (Position States) :since:`3.0`
     - Effect
     - Unsets the position state.
