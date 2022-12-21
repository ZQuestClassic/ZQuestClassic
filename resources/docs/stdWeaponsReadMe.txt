StdWeapons.zh

v.1.0

**Version History**
v1.0
//Ice rod firing sound no longer hardcoded. Use that "use sound" field in item editor. 
//Ditto for direct wand hit trigger flag for Ice Rod.
//Added new function: FullScreenAttack for spells to affect full screen.
//Ether, Quake and Bombos spells completed.
//Simplified setup process for Hookshot, Portal Wand, Ice Rod and Fire Rod scripts, thanks for GetFFCScript function.
//Split apart bow/arrow and wand/fire rod scripts into separate script files.
//Lweapon`s sprite can be rotated using lweapon->Angle. Set Angular boolean to TRUE to turn on this feature.
//New flag: LWF_AUTO_POS_UPDATE. Controls whether lweapon`s position will be updated automatically. It`s turned on by default.
//2 new functions:LweaponUpdate and LweaponUpdatePostWaitframe. They can be used instead of LweaponWaitframe, if you need to carry out
//  Waitframe command outside routine to avoid running it multiple times in the loop, when script handles multiple lweapons via pointer array.
//2 new flags: LWF_REFLECTED for handling reflected weapons and LWF_INVISIBLE for making lweapon invisible.
//Fixed LWF_DRAW_SHADOW flag.
//New function: ReflectOffMirrors for handling reflecting off mirror combos. Ice rod, fire rod and portal rod magic now reflect off mirrors.
//New function: AutoRotateLweaponHitbox for handling hitbox rotation depending on direction. Works only on cardinal directions.
//New sample script: Expanded Cane of Byrna. It showcases ability to handle multiple ghosted lweapons in one script.
//  and fine-rotating for their sprites.
//New sample script: Flamethrower/Golden candle.
//New sample script: Sand Rod. Fires magic that replaces certain combos with another ones and has limited range.
//Fixed particles not being removed when off-screen.
//New function for Melee sub-library - MeleeWeaponDrawAboveHead. 
//Expanded custom sword and hammer scripts with almost full support of tiger scroll techniques.
//Legacy TriggerUpdate functions now use screen`s undercombo as replacement, instead of next in table.
//The secret debug command (draw hitboxes for all ghosted lweapons) now has an option in main header.


v1.5
//Fixed stupid bug that caused simplified secret trigger functions to use combo 0 as replacement combo instead of using screen`s undercombo.
//New sub-library - Custom Enemy Defenses for defining and manipulating custom enemy defenses, like Ice, Lightning etc.
//New sub-library - Enemy Status Effects for affecting enemies hit by lweapons in the way other than damaging and stunning. Like freezing
// or turning into stone.
//Portals from Portal Wand can teleport dropped items in sideview screens.
//New sample script - ICE ROD. It was created for testing custom enemy defenses and status effects.
/!\ Setup instructions have been changed. Read then again, especially step 5 for main header.

v1.4
//Generalized FFC_MISC_TIMER(constant renamed!!)variable usage. Now It can be used to time all kind of stuff.
// Like simplifying ContiniousSFX function so it no longer require to maintain external counter.
//New lweapon flag: LWF_IGNORE_ENEMY_SHIELDS. When set, it allows lweapon to pierce trough enemy shields.
//New sample script: Portal Wand! Almost straight from Aperture Since Labs!
//Various minor fixes.
//Rewritten Boomerang`s steering functionality to accomodate going trough portals created by Portal Wand.

v1.3
//Added functions that detect if lweapon actually hit unprotected enemy, either dealing damage
// or just stunning it.
//Damaging particles can now use defenses for enemy interaction. 
//Fixed ClearParticles function.
//Fixed off-by-1 error in SetLweaponHitboxOffsets function.
//Added Ice Hookshot sample script.

v1.2
//Integrated particles.zh sub-library. Now it requires UpdateLweaponZH function in the
// main loop of global script to handle particles properly.
//Secret triggers no longer removes lweapons instantaneously. Such behavior must be defined in
// lweapon script via new function: HitSecretTrigger. This would allow custom reaction of lweapon on
// hitting secret.
//Revamped and extended BlockedByCombo function. Now you can use "sound" argument to play sound
// when lweapon hits blocker combo. Also this function no longer automatically removes lweapon
// and that behavior must be defined in lweapon script.
//Added SetAngularMovement function. Yikes!
//HitTrigger and TriggerUpdate functions return default value (false for former), 
// if the flag is set at 0. BlockedByCombo also returns false, if combo type argument 
// is set to 0.
//Fixed DrawXOffset so it reads correct value when read outside LweaponWaitframe.
//Added Master Boomerang sample script.


v1.1
//Implemented int NumScriptedLweapons() and int NumScriptedLweaponsOf (int ffc_id)
//Fixed TransformEnemyOnCollision so new NPC won`t be instantly killed after appearance.
//Rewritten GetItemID function.
//ItemPickup function can be set to pick different kinds of items, like special ones.
//Fixed KillRunScript. It now properly cleans up former FFC.
//Original Damage value no longer needed with DamageModifierEnemySpecific function, as this value
//  is stored automatically in lweapon initialization phase.
//Streamlined interaction with enemy defenses so you could just call SetEnemyDefense and go for it
//  as everything related in processed in LweaponWaitframe function.
//Revamped example melee weapons and added new arguments, like animation speed.
//All melee weapon functions now have an extra argument called "aspeed", which controls sequence speed.
//Fixed hitbox auto-rotating for hammers.

v1.0 
//Initial release.

**Usage**

The library is used for creating and manipulating Lweapons with FFC`s to make
scripted weapons. WeaponLauncher item script runs FFC script with arguments 
passed to FFC. FFC controls movement, while Lweapon handles appearance and 
interaction with enemies and stuff.

The script usually begins with LweaponInit function that prepares FFC, 
creates main Lweapon and assigns default parameters, like damage, sprite etc.
Instead of default Waitframe, LweaponWaitframe should be used. This function
updates all routinous aspects of Lweapon controlling, such as gravity, updating
position, direction, sprite etc. Instead of direct call of Quit(), use 
TerminateLweapon so the script at the end could thoroughly clean FFC for recycling.

**Setup**

*Main header setup*

1. Set aside 4x4 empty tiles space.
2. Set aside one combo and assign it`s tile to top left corner of tile space
from step 1. Note down ID of that combo.
3. Open stdWeapons.zh and assign constant named LW_FFC_DATA to Id of the combo
from step 2.
4. Check out other constants in stdWeapons.zh, like FFC and Lweapon Misc 
array indexes to make sure there aren`t any conflicts with other scripts.
5. Global script combining: put UpdateLweaponZH() function inside the main loop of your Active
global script and InitLweaponZH() inside the Init global script. 
6. Import and compile the library. It requires std.zh, ffcscript.zh and stdCombos.zh
7. Assign WeaponLauncher item script to an empty slot.


*Individual weapon setup*

Although some weapons require certain specific setup, most simple scripted
weapons set up like this:
1. Most weapons require sprite to render visible. Set up them in
Quest->Graphics->Sprites->Weapons/Misc editor. Weapons with 8-way direction must
have 2 framesets in tile editor, one right after another for cardinal 
and diagonal directions. To have launched weapon looking properly, 
his sprite for orthogonal directions should point to the UP in sprite editor, 
for diagonal sprite - RIGHT-UP. Also make sure the "Animation Frames" value isn`t 
set to 0, or some lweapons will have wrong animation.
2. Import and compile the weapon FFC script. Assign it to FFC script slot.
3. Set up standard item settings as usual, like graphics, pickup settings etc.
4. In script tab, set D0-D6, according to weapon script documentation, D7 to FFC script
slot number. Assign "WeaponLauncher" item script to Action script slot.

*Melee weapon setup*
1. Set up tiles for melee weapon sprite tiles. Refer to custommeleesetup.png image 
for setting up sprite tiles for various melee weapons.
2. In Quest->Graphics->Sprites->Weapons/Misc editor define new sprite and select top-left corner
tile in the setup. Also make sure the "Animation Frames" value is 
set to 1, or lweapon will have wrong animation.
3. Import and compile melee weapon FFC script. 
Assign WeaponLauncher item script and melee weapon FFC script to slots.
4. Define new item for the weapon and set up his parameters as usual. Set D variables according
to documentation, usually including sprite ID from step 2 and FFC script slot in D7. 
Assign WeaponLauncher into Action item script slot. 

** Header Constants**

LW_FFC_DATA - Combo ID that all stdWeapons FFC`s using. Set it to blank.

//lweapon type ID`s. By default they are set to LW_SCRIPT1 for normal lweapons 
//and LW_SCRIPT2 for particles.
LW_GHOSTED - Type of Lweapon to be considered as scripted. Use an unique number between 31 and 40.
LW_PARTICLE - Particle lweapon ID. Set it so it does not conflict with other scripts.

SFX_DEBUG_TERMINATE - SFX used when Lweapon script terminates. Used mostly when debugging.
DEBUG_DRAW_HITBOXES - Draw lweapon hitboxes. 0 - off, 1 - on. 

DEFAULT_STUN_DURATION - Default enemy stun duration, in frames. Used when stunning enemies.

CUSTOM_SPRITE_DRAW_LAYER - Layer used to draw custom Lweapon sprites. The reason why I use manual
drawing of lweapon`s sprite is the fact that defult drawing is quite buggy with big lweapons.

* FFC`s misc variables. Set them to avoid conflicts with other scripts.
FFC_MISC_Z - FFC`s fake Z position. 
FFC_MISC_VZ - FFC`s fake Z-axis velocity.
FFC_MISC_AZ - FFC`s fake Z-axis acceleration.
FFC_MISC_TIMER - //Counter used for timing stuff, like drawing animated and flickering shadows or continious SFX.
			//Goes up every frame, resets once every 1/2 hour. 

* Lweapon Misc variables. Set them to avoid conflicts with other scripts.
LWEAPON_MISC_DEFENSE - Default enemy defense used against this lweapon.
LWEAPON_MISC_ORIGDAMAGE - Original lweapon->Damage value. Used internally, when interacting with enemies that modify damage.
LWEAPON_MISC_HIT_OFFSET_X  - Actual hitbox offsets for scripted Lweapons.
LWEAPON_MISC_HIT_OFFSET_Y - Needed for adjusting hitboxes for lweapons that can move offscreen.
LWEAPON_MISC_ORIGTILE - The original tile index backup. Used internally with 8-way sprite movement
and melee weapon functions.
LWEAPON_MISC_FLAGS - Misc Flags for lweapons, like penetrating and such.
LWEAPON_MISC_FFC_ID - ID of FFC that controls this lweapon.

* Particle Lweapon Misc Variables. Does not conflict with default lweapon->Misc constants, because of different
lweapon ID`s that use those indexes.
LWEAPON_MISC_PARTICLE_VX - "Vertical velocity" misc variable.
LWEAPON_MISC_PARTICLE_VY - "Horizontal velocity" misc variable.
LWEAPON_MISC_PARTICLE_AX - "Horizontal Acceleration" misc variable.
LWEAPON_MISC_PARTICLE_AY - "Vertical Acceleration" misc variable.
LWEAPON_MISC_PARTICLE_TIMEOUT - "Particle Lifespan" misc variable.
LWEAPON_MISC_PARTICLE_AFFECTED_BY_GRAVITY - "Gravity boolean" misc variable. Used in sideview areas.
LWEAPON_MISC_PARTICLE_XPOS - "Particle X position" misc variable.
LWEAPON_MISC_PARTICLE_YPOS - "Particle Y position" misc variable.
LWEAPON_MISC_PARTICLE_DAMAGE_X_OFFSET - Particle hitbox X offset for damaging particles.
LWEAPON_MISC_PARTICLE_DAMAGE_Y_OFFSET - Particle hitbox Y offset for damaging particles.
LWEAPON_MISC_PARTICLE_DAMAGE_ENEMY_DEFENSE - Defense used for enemies that stepped on the particle.

* Shadow animation settings. Used when drawing shadows under high placed lweapons.
LWEAPON_SHADOW_TILE - Shadow tile to draw.
LWEAPON_SHADOW_CSET - Cset for shadow drawing.
LWEAPON_SHADOW_NUMFRAMES - Number of frames for shadow animation.
LWEAPON_SHADOW_ASPEED - Delay between shadow animation frame changing, in frames.
LWEAPON_SHADOW_TRANSLUCENT - Shadow translucensy 0 = False, 1 = TRUE
LWEAPON_SHADOW_FLICKER - Shadow flickering 0 = False, 1 = TRUE

* Large shadow settings.
LWEAPON_LARGE_SHADOW_TILE = 27392; // Top-left corner!!
LWEAPON_LARGE_SHADOW_CSET = 7;
LWEAPON_LARGE_SHADOW_FRAMES = 4;
LWEAPON_LARGE_SHADOW_ANIM_SPEED = 8;
LWEAPON_LARGE_SHADOW_MIN_WIDTH = 2; // Lweapons must be at least this wide
LWEAPON_LARGE_SHADOW_MIN_HEIGHT = 2; // and this high to use large shadows.

** Functions**

*Init*

item script WeaponLauncher
* Main weapon launcher item script. 
* D0-D6 are passed to host FFC as arguments (7 arguments). 
* D7 - Script slot for FFC.
* When FFC script is run, it can access item ID that launched this lweapon via this->InitD[7].
* This allows FFC script to obtain itemdata of the said item and use it for his own purposes.


lweapon LweaponInit (ffc f, int type, int damage, int sprite, int sound)
* Initialize FFC position and creates controlled Lweapon at Link`s position, assigning damage and sprite.
*  Type - ID of new lweapon. Use LW_GHOSTED for best results.
*  Damage - self-explanatory.
*  Sprite - sprite used by new weapon. Use Quest->Graphics->Sprites->Weapons/Misc to retrieve sprite ID.
*  Sound - sound to play on launching.

ffc LaunchScriptedLweapon(int Script, int[8] arguments)
* Launch different scripted Lweapon. Returns a pointer to used FFC.
*  Script - FFC script slot number
*  args[8] - pointer to arguments array. It must contain 8 integers. If you have fewer,
*  expand the array to 8 with zeroes.

lweapon CreateNewWeapon (ffc f, int type, int damage, int sprite)
* Creates another Lweapon at FFC`s position without putting FFC at Link`s position.
*  Type - ID of new lweapon.
*  Damage - self-explanatory.
*  Sprite - sprite used by new weapon.

*Update*

bool LweaponWaitframe(ffc f, lweapon l)
* Main Lweapon update function. Must be called instead of original Waitframe, as in additional to
* waiting a frame, it updates basic parameters of lweapon and his master FFC.
* Returns TRUE, if lweapon pointer is still valid, otherwise returns FALSE.

void LweaponWaitframe (ffc f, lweapon l, bool quitoninvalid)
* Unlike previous function, does not return anything and can automatimcally
* terminate the script if lweapon becomes invalid, complete with FFC cleanup.

void LweaponUpdate(ffc f, lweapon l)
void LweaponUpdatePostWaitframe(ffc f, lweapon l)
* This pair of functions is similar to LweaponWaitframe, but don`t have built in Waitframe command. 
* Run Waitframe between those functions if you need to handle multiple lweapons inside one script.

*Flags*

void LweaponSetFlag(lweapon l, int flag)
void LweaponUnSetFlag(lweapon l, int flag)
void LweaponToggleFlag(lweapon l, int flag)
*  Sets or unsets a flag that controls various aspects of the given lweapon`s behaviour.
* 
*  *LWF_GHOSTED - Checks if lweapon is controlled by script. Always turned on all the time. Mostly used
*   by other scripts to tell apart "ghosted" lweapons and normal ones.
* 
*  *LWF_8WAY - If set, lweapon will use all directions, other than cardinal ones. Will use tiles right
*   after currant ones in tile table for diagonal directions.
* 
*  *LWF_PENETRATING - If set, lweapon will be able to penetrate enemies.
* 
*  *LWF_MOVE_OFFSCREEN - If set, lweapon can move offscreen without disappearing on screen edges. This is done by
*   altering Draw and Hit coordinate offsets, instead of X and Y values during LweaponWaitframe routine.
* 
*  *LWF_DRAW_SHADOW - Controls whether the shadow will be drawn underneath Lweapon sprite. Used only in top-down areas. 
* 
*  *LWF_NO_FALL - If set, lweapon will not be affected by gravity.
* 
*  *LWF_CAN_PICKUP_ITEMS - If set, lweapon can drag around dropped items.
* 
*  *LWF_CAN_INSTA_DELIVER_ITEMS - If set, any dropped item that touched this lweapon will be teleported to
*   Link`s position. Takes priority over LWF_CAN_PICKUP_ITEMS.
* 
*  *LWF_AUTO_DIR_UPDATE - If set, lweapon`s direction will be updated automatically, depending on
*  FFC`s Vx and Vy values.
* 
*  *LWF_AUTO_SPRITE_UPDATE - If set, lweapon`s sprite will be updated automatically, depending on it`s
*  direction. Uses next sprite in the table, if moving in diagonal directions.
* 
*  *LWF_MELEE - Used internally for telling apart melee and projectile lweapons.
* 
*  *LWF_SWING_WHILE_WALKING - Used only with melee weapons. If set, Link can walk while melee weapon is active.
* 
*  *LWF_IGNORE_ENEMY_SHIELDS - If set, lweapon can blow trough directional enemy shields, like Darknut`s one.
*
*  *LWF_AUTO_POS_UPDATE - If set, lweapon`s position will be updated automatically. Algorithm is decided by LWF_MOVE_OFFSCREEN flag.
*
*  *LWF_REFLECTED - Set internally by ReflectOffMirrors function. Used mostly for condition to trigger reflected magic trigger flags.
*
*  *LWF_INVISIBLE - If set, lweapon will become invisible. His shadow will still remain.

bool LweaponFlagIsSet(lweapon l, int flag)
* Returns TRUE, if the given Lweapon flag is set and false otherwise.


*Movement*

void SetStraightMotion4(ffc f, float speed)
* Sets FFC to move in one of cardinal directions at given speed, using Link`s direction.

void SetStraightMotion4(ffc f, int dir, float speed)
* Same as previous function, but in specific direction (not reading Link`s one).

void SetAngularMovement(ffc f, int angle, int speed)
* Sets angular movement of FFC. Angle is measured in degrees.

void HomingUpdate(ffc f, int speed, int targetX, int targetY, bool accel)
* Causes FFC to home on given point. If "accel" boolean is set to TRUE, the function affects FFC`s
* acceleration, instead of velocity. Usually it`s used to target enemies, but it can be set to target any
* point on the screen, like mouse cursor`s position. Run once every frame, when needed.

void SetPolarCoordinates (ffc f, int oX, int oY, float radius, int angle)
* Set FFC`s position using polar coordinate system with given origin point, radius and angle set.
* Best used with circling Lweapons, like Byrna beams. Run once every frame, like HomingUpdate.

void MoveToSolidContactPosition(ffc f, lweapon l)
* Puts FFC at position where it`s Lweapon`s wall collision just recently occured.
* Do you remember the same function in Game Maker?
* Best used right after occurence of collision with solid combos, so after usage, lweapon`s hitbox will be
* pixel-perfectly next to edge of solid combo.

void Bounce (ffc ball, lweapon l, int sound)
* Makes FFC bounce off solid combos by changing FFC`s velocity depending on lweapon`s position relative
*  to solid combos. Don`t call it too often. Otherwise Lweapon can be stuck in a wall.

*Interaction with enemies*

void SetEnemyDefense(lweapon l, int defense)
* Sets enemy defense for automatic checking against this lweapon. By default, ARROW defense is used.
* /!\Make sure no other scripts that works with LW_SCRIPT# lweapons should coexist with
* /!\the scripts that use this function or enemies will react weirdly when hit by script Lweapons.

int GetEnemyDefense( lweapon l)
* Determines enemy defense used against given Lweapon. Also works with non-ghosted lweapons.

bool AnyNPCCollision(lweapon l)
* Returns TRUE, if the given lweapon collides with any NPC on the screen. 
* Ignores enemies whose defense is set to IGNORE or ONE-HIT-KILL(the latter should be already dead anyways).

bool BlockedByDefense(lweapon l, npc n)
bool BlockedByDefense(lweapon l)
* Returns true if Lweapon is blocked by enemy`s defenses.

bool EnemyTypeCollision(lweapon l, int type)
* Returns TRUE if Lweapon has collided with enemy of specific ID.
* Ignores enemies whose defense is set to IGNORE or ONE-HIT-KILL(the latter should be already dead anyways).

bool BlockedByShield(lweapon l, npc n)
bool BlockedByShield(lweapon l)
* Returns TRUE if lweapon is blocked by NPC`s directional shields, like Darknut`s one.

bool BlockedByEnemy (lweapon l, npc n)
bool BlockedByEnemy(lweapon l)
* Returns TRUE if Enemy has managed to block this Lweapon in any way.

bool HitEnemy(lweapon l, npc n)
* Returns true if Lweapon actually hits given enemy, dealing damage and such. 

npc OnHitEnemy(lweapon l)
* Returns pointer to NPC if the given Lweapon actually hits it, dealing damage and such. 

bool HitEnemy(lweapon l)
* Returns true if Lweapon actually hits any enemy, dealing damage and such.

void OneHitKillEnemySpecific (lweapon l, int enemyid, int sound)
* Causes lweapon to one-hit-kill any enemy of specific kind (like red Pols Voices & arrows).
* Run this every frame.
* Sound - sound to play when one-hit-kill occurs.

void TransformEnemyOnCollision (lweapon l, int enemyid, int newenemyID, bool retainHP, int sound, int defense)
* Transforms any enemy of specific kind, that has been hit by this Lweapon, into another one.
* Example: Gibdo(L1) turns into Stalfos(L1).
* Sound - sound to play when transformation occurs.
* retainHP - Set to TRUE, and new enemy will spawn with the same amount of HP, as previous one.
*  Otherwise the new enemy will appear with full HP.
* defense - Defense entry used against transformation. Cannot be BLOCKed, but can be IGNOREd.

void DestroyEweapons (lweapon l, int sound, int effectsprite)
void DestroyEweapons (lweapon l, int type, int sound, int effectsprite)
* Destroys any Eweapon/Eweapons of specific type that has collided with the current Lweapon.
* Sound - sound to play when eweapon destruction occurs.
* Effectsprite - effect to turn eweapon into. Set to 0 to disable this feature.
* Run this every frame.

void DamageModifierEnemySpecific( lweapon l, int npcid, float modifier)
* Changes damage dealt by Lweapon when hitting specific enemies.
* npcid - ID of the affected NPC.
* modifier - damage multiplier.
* Run this every frame.

void UseEnemyDefense(lweapon l, int npcid, int defense)
* Makes the current LW_SCRIPT lweapon to check against different enemy defenses(NPCD_* constants).
* npcid allows function to be restricted to specific NPC types. Set to 0 to turn this off.
* Run this every frame.
* /!\Make sure no other scripts that works with LW_SCRIPT# lweapons should coexist with
* /!\the scripts that use this function or enemies will react weirdly when hit by script Lweapons.

*Custom Enemy Defenses*
Custom enemy defenses are stored in one massive global array that consists of 10240 elements (20 defense entries 
for all 512 enemy slots in the quest). All functions listed below work with this array. Aside from original 18 
defenses, SetEnemyDefense function can work with this custom defense array, as well as couple of specials. 
Here are valid values for second argument for SetEnemyDefense function.
0-16 - Default defenses from Enemy editor.
18 - Special Defense - Ignore defenses completely.
19 - Special Defense - all enemies can block this lweapon.
20 - Special Defense - any enemy will be stunned if he got hit by  this lweapon.
21-40 - Custom enemy defenses read from global array.

void InitCustomDefenses()
* Initialize custom defenses array. Must be called in the Init global script.

void SetCustomEnemyDefense(int EnemyID, int defense, int NewDefense)
* Sets the given custom defense entry to given enemy.
* EnemyID - ID of enemy in enemy editor.
* defense - custom defense slot (0-19).
* NewDefense - New defense type for custom defense (block, stun, ignore etc.)

int GetCustomEnemyDefense(int EnemyID, int defense)
* Gets custom defense assigned to this NPC.
* EnemyID - ID of enemy in enemy editor.
* defense - custom defense slot (0-19).

void SetDefaultCustomEnemyDefense(int defense, int newdefense)
* Sets default custom defense entry to all enemies in the quest.
* defense - custom defense slot (0-19).
* NewDefense - New defense type for custom defense (block, stun, ignore etc.)

//Next 3 functions are to be used in ghosted enemy scripts (ghost.zh)

void Ghost_StoreCustomDefenses(npc ghost, int storedDefense)
* Stores custom defenses for given NPC into backup array.
* StoredDefense - pointer to afromentioned array. Must be at least 20 integers long.

void Ghost_SetCustomDefenses(npc ghost, int storedDefense)
* Restores custom defenses for given NPC from backup array.
* StoredDefense - pointer to afromentioned array. Must be at least 20 integers long.

void Ghost_SetAllCustomDefenses(npc ghost, int defType)
* Sets all enemy`s custom defenses to given type.
* defType - New defense type for custom defense (block, stun, ignore etc.)

*Enemy Status Effects*

Certain kinds of lweapons, like projectile from Ice Rod, can induce status effects to enemies,
like freezing, turning into stone, blinding, setting on fire etc. Status effects are separate
FFC scripts that work with affected NPC`s. The InduceEnemyStatusEffect function runs separate FFC
script and passes in number as pointer to NPC.

void InduceEnemyStatusEffect(ffc f, int scriptNum, int args, npc n)
* Induces stautus effect to enemy.
* FFC f - ffc that runs the weapon script.
* scriptnum - target FFC script slot.
* Args - arguments array pointer. Max 7 vars.
* NPC n - pointer to affected enemy.

void InduceStatusEffectOnHitEnemy(ffc f, lweapon l, int scriptNum, int args)
* Shortcut for inducing given status effect on any enemy hit by lweapon (defenses and shields are accounted).
* FFC f - ffc that runs the weapon script.
* lweapon encanted to induce status effects on enemies.
* scriptnum - target FFC script slot.
* Args - arguments array pointer. Max 7 vars.

*Interaction with combos*

bool WallCollision8way(lweapon l)
* Returns TRUE if lweapon contacts with solid combos. Fast, but may fail with BigLweapons.

void TriggerUpdate (lweapon l, int flag, int sound, bool perm, int cmb, int SCSet)
void TriggerUpdate (lweapon l, int flag, int sound, bool perm)
* Checks for collision with trigger flags and triggers screen secrets if a collision occurs.
* Uses screen`s undercombo for replacing triggered combo.
* Sound - sound to play on triggering secrets.
* Perm - Set to TRUE, and the secrets will be permanent.
* CMB - Combo used on replacing activated triggers. Set to 0 to use screen`s undercombo, 
* or -1 for changing to next combo in the table.
* SCset - CSet for Combo used on replacing activated triggers. Set to -1 to use screen`s under cset.
* Run this every frame.

bool HitSecretTrigger (lweapon l, int flag, int sound, bool perm, int cmb, int SCSet)
bool HitSecretTrigger (lweapon l, int flag, int sound, bool perm)
* Same, as TriggerUpdate, but returns whether lweapon just triggered secret flag or not.

bool BlockedByCombo (lweapon l, int type, int sound)
bool BlockedByCombo (lweapon l, int type)
* Detects if lweapon hits blocker combo type.
* sound - sound to play on hitting blocker combo.

void ReplaceCombosOnCollision( lweapon l, int type, int newcombo, int newcset, int sound)
* Replaces any combos of specific type when touched by Lweapon
* newcombo - ID of the combo to replace with.
* newcset - Cset to use for replacement combos.
* Sound - sound to play on combo replacement.

void ReplaceFlaggedCombos( lweapon l, int flag, int newcombo, int newcset, int sound, bool retainflag)
* Same as above but with flags.
* newcombo - ID of the combo to replace with.
* newcset - Cset to use for replacement combos.
* Sound - sound to play on combo replacement.
* retainflag - if set to FALSE, the combo flag that triggered collision will be removed.

void DestroyCombosOnCollision( lweapon l, int type, int sound)
* Destroys combos of specific type on collision by replacing them with screen`s undercombo. 
* Sound - sound to play on destruction.

void ChangeToNextOnCollision( lweapon l, int type, int newcset, int sound)
* Destroys combos of specific type on on collision by replacing them with next combo in the table.
* Sound - sound to play on destruction.

int ReflectOffMirrors(int oldcmb, ffc f, lweapon l)
* Makes lweapon reflect off mirror combos.
* oldcmb - musi be set to function`s return value to avoid repeatedly hitting the same mirror.

*End of life cycle*

lweapon ChangeLweapon(ffc f, lweapon l, int type, int damage, int sprite)
* Turns Lweapon into another one.

void KillExplosionMultiple (ffc f, lweapon l, int damage, bool super, int numbooms, int spread, int delay)
* Causes Lweapon to explode multiple times like destroyed ghosted boss.
* Super - Set to TRUE for super-bomb-like blast radius.
* Numbooms - number of explosions to create.
* Spread - Maximum explosion distance away from source lweapon.
* Delay - delay between explosions, in frames.

void KillExplosion (ffc f, lweapon l, int damage, bool super)
* A simpler version of the above function that creates one single explosion.
* Super - Set to TRUE for super-bomb-like blast radius.

void KillMolotov( ffc f, lweapon l, int damage, int numflames, int spread)
* Cause Lweapon to burst into flames, ala Molotov Cocktail.
* numflames - Number of fire lweapons to produce.
* spread - fire spread radius.

void KillSpawnItem(ffc f, lweapon l, int itemID)
* Turns lweapon into dropped item.

void KillRunScript(ffc f, lweapon l, int d0, int d1, int d2, int d3, int d4,int d5, int d6, int d7, int scriptNum)
* Starts running another FFC script. Removes Lweapon if successful. Uses separate FFC.

void TerminateLweapon (ffc f, lweapon l)
* Removes the scripted lweapon and terminates his master script. Use this instead of usual
* direct call of Quit() as it also clears FFC and prepares it for being reused.

*Melee weapons*
* The almost fully automated sub-library for making lweapons act like swords, hammers, wands etc.
* After performing the action (stab, slash, pound), lweapon is not removed so LweaponWaitframe
* does not terminate the script and allow it to execute some more code, like launching sword beam.
* Always use MeleeweaponEndStrike to terminate melee lweapon.

void MeleeWeaponStab(ffc f, lweapon l, int origtile, int delay, int offset, int triggerflag, 
 int combodestroy, int enemydefense, int combodestroysfx, int aspeed)
* Performs melee weapon stab, like spear, or sword w/o slash ability.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.
* triggerflag - flag, that triggers secrets, when hit by this lweapon. 0 to disable.
* combodestroy - type of combo to be destroyed by this lweapon. 0 to disable.
* enemydefense - enemy defense to use against this lweapon.
* combodestroysfx - sound to play when this lweapon destroys combos.
* aspeed - stab speed. Lower value-faster.

void MeleeWeaponSlash90(ffc f, lweapon l, int origtile, int delay, int offset, int triggerflag, 
 int combodestroy, int enemydefense, int combodestroysfx, int aspeed)
* Performs melee weapon slash, like sword with Flip Right Facing Slash quest rule on.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.
* triggerflag - flag, that triggers secrets, when hit by this lweapon. 0 to disable.
* combodestroy - type of combo to be destroyed by this lweapon. 0 to disable.
* enemydefense - enemy defense to use against this lweapon.
* combodestroysfx - sound to play when this lweapon destroys combos.
* aspeed - slash speed. Lower value-faster.

void MeleeWeaponSlash90Alt(ffc f, lweapon l, int origtile, int delay, int offset, int triggerflag, 
 int combodestroy, int enemydefense, int combodestroysfx, int aspeed)
* Same as MeleeWeaponSlash90, but with Flip Right Facing Slash quest rule turned off.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.
* triggerflag - flag, that triggers secrets, when hit by this lweapon. 0 to disable.
* combodestroy - type of combo to be destroyed by this lweapon. 0 to disable.
* enemydefense - enemy defense to use against this lweapon.
* combodestroysfx - sound to play when this lweapon destroys combos.
* aspeed - slash speed. Lower value-faster.

void MeleeWeaponSlash180(ffc f, lweapon l, int origtile, int delay, int offset, int triggerflag, 
 int combodestroy, int enemydefense, int combodestroysfx, int aspeed)
* Performs 180 degree melee weapon slash, like swordin Link to the past.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.
* triggerflag - flag, that triggers secrets, when hit by this lweapon. 0 to disable.
* combodestroy - type of combo to be destroyed by this lweapon. 0 to disable.
* enemydefense - enemy defense to use against this lweapon.
* combodestroysfx - sound to play when this lweapon destroys combos. 
* aspeed - slash speed. Lower value-faster.

void MeleeWeaponSlash360(ffc f, lweapon l, int origtile, int delay, int offset, int triggerflag, 
 int combodestroy, int enemydefense, int combodestroysfx, int aspeed)
* Performs full 360 degree spin attack with the weapon. If you want something, like Hurricane spin,
* call this function multiple times in a row, like in for() loop.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.
* triggerflag - flag, that triggers secrets, when hit by this lweapon. 0 to disable.
* combodestroy - type of combo to be destroyed by this lweapon. 0 to disable.
* enemydefense - enemy defense to use against this lweapon.
* combodestroysfx - sound to play when this lweapon destroys combos. 
* aspeed - slash speed. Lower value-faster.

void MeleeWeaponPound(ffc f, lweapon l, int origtile, int lag, int delay, int offset, 
int triggerflag, bool breakshield, int combodestroy, int enemydefense, int poundsfx, int combodestroysfx, int aspeed)
* Performs mele weapon Pound, like hammer.
* Performs full 360 degree spin attack with the weapon. If you want something, like Hurricane spin,
* call this function multiple times in a row, like in for() loop.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* lag - time between Link drawing lweapon (and holding it upwards) and slamming it into the ground, in frames. 
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.
* triggerflag - flag, that triggers secrets, when hit by this lweapon. 0 to disable.
* combodestroy - type of combo to be destroyed by this lweapon. 0 to disable.
* enemydefense - enemy defense to use against this lweapon.
* combodestroysfx - sound to play when this lweapon destroys combos.
* aspeed - action speed. Lower value-faster.

void MeleeWeaponDraw(ffc f, lweapon l, int origtile, int delay, int offset)
* Performs lweapon drawing, like a bow, a gun, or wand. Best used for projectile shooting weapons.
* Collision is disabled for this lweapon.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.

MeleeWeaponDrawAboveHead(ffc f, lweapon l, int origtile, int delay, int offset)
* Same as previous function, but above Link`s head, like charging Quake Hammer.
* Collision is disabled for this lweapon.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.


void MeleeWeaponEndStrike(ffc f, lweapon l, int origtile, int delay, int offset, int aspeed)
* Performs animation of Link putting weapon back into his pocket and terminates the script,
* complete with FFC cleanup. 
* Collision is disabled for this lweapon.
* origtile - the top-left tile in the tile setup for animation
*  Usually you should pass in lweapon->Misc[LWEAPON_MISC_ORIGTILE] (see "Library Constants") here.
* delay - wait time between last frame of melee weapon sequence and function returning.
* offset - How far Lweapon is away from Link, in pixels.

*Particles*
* Particles are little animated images created by main lweapon, in specific functions listed below
* are called. Particles have coodinates, velocity, acceleration and lifespan. Classical examples 
* of particles are hammer impact dust, silver&golden arrow sparkles and Magic/Fire boomerang sparkles.
* The fire boomerang`s sparkles have special ability to damage enemies that stepped on them.

lweapon CreateParticle (int x, int y, int sprite, int ax, int ay, int vx, int vy, int lifespan, bool grav)
* Creates a particle.
* x, y - starting coordinates
* sprite - sprite used by prticle from Weapons/Misc animation data
* vx, vy - initial velocity, horizontal and vertical
* ax, ay - acceleration, horizontal and vertical
* lifespan - delay between creating particle and it disappearing. Set to -2 for full animation 
* sequence(Aspeed*Numfraes)
* grav - whether the particle is affected by gravity in sideview areas or not.

lweapon CreateParticleAdvanced( int x, int y, int numframes, int aspeed, int origtile,
int cset, int flashcset, int ax, int ay, int vx, int vy, int lifespan, bool grav)
* Andvanced version of particle creating. Use it if you are running out of sprite slots 
* in Weapons/Misc animation data.
* x, y - starting coordinates
* origtile - original tile for first frame of animation
* numframes - nuber of frames in animation
* aspeed - delay between frame changing, in frames
* cset - Cset used by particle
* flashcset - second Cset used for flashing. Match to "cset" for no flashing
* vx, vy - initial velocity, horizontal and vertical
* ax, ay - acceleration, horizontal and vertical
* lifespan - delay between creating particle and it disappearing. Set to -2 for full animation 
* sequence(Aspeed*Numfraes)
* grav - whether the particle is affected by gravity in sideview areas or not.

void SetParticleAngularMovement(lweapon anim, int angle, int speed)
* Set angular motion of particle. Angle is measured in degrees. Speed is measured in pixels per frame.

void BigParticle (lweapon anim, int tilewidth, int tileheight)
* Expands particle size.

void SetAsDamagingParticle(lweapon anim, int dam, int HitXoffset, int HitYoffset, 
int HitWidth, int Hitheight, int defense)
* Sets particle as Damaging particle that deals damage to all NPC`s when stepped on.
* dam - damage
* HitXoffset, HitYoffset - hitbox offsets, left and top. 
* HitWidth, Hitheight - hitbox width and height.
* defense - defense used for interaction with enemies

void ClearParticles()
* Removes all particles from screen.

*Misc functions*

bool AmmoManager(int counter, int cost, int infiniteitem, bool drain)
bool AmmoManager(int counter1, int cost1, int infiniteitem1, int counter2, int cost2, int infiniteitem2,
 bool drain)
bool AmmoManager(int counter1, int cost1, int infiniteitem1, int counter2, int cost2, int infiniteitem2,
 int counter3, int cost3, int infiniteitem3, bool drain)
bool AmmoManager(int counter1, int cost1, int infiniteitem1, int counter2, int cost2, int infiniteitem2,
 int counter3, int cost3, int infiniteitem3, int counter4, int cost4, int infiniteitem4, bool drain)
* Returns true if Link has enough ammo (also subtracting cost) or has an item that allows infinite ammo.
* Usually placed in the init phase of the script before Lweapon initialization.
* Call ClearFFC(this) then Quit() if this function returns "false" (not enough ammo).
* Or execute something cruel, like weapon backfiring. :-)
* counter - Counter used as fuel. 
* cost - Amount to subtract each time the function is called.
* infiniteitem - if Link has this item in his inventory, the check, and ammo cost payup, will be completely 
* bypassed.
* drain - if set to TRUE, the counter updates gradually.
* Overloaded to check up to 4 different counters with different 
* costs and presense of "infinite counter" items.

void AmmoManager(ffc f, int counter, int cost, int infiniteitem, bool drain)
void AmmoManager(ffc f, int counter1, int cost1, int infiniteitem1, int counter2, int cost2, 
 int infiniteitem2, bool drain)
bool AmmoManager(int counter1, int cost1, int infiniteitem1, int counter2, int cost2, int infiniteitem2,
 int counter3, int cost3, int infiniteitem3, bool drain)
bool AmmoManager(int counter1, int cost1, int infiniteitem1, int counter2, int cost2, int infiniteitem2,
 int counter3, int cost3, int infiniteitem3, int counter4, int cost4, int infiniteitem4, bool drain)
* Same as previous functions, but simply quits the script when not enough ammo, 
* complete with FFC cleanup.


int NumScriptedLweapons (ffc f)
* Returns total number of ffc`s that run the same script as the given one, which is also counted.
int NumScriptedLweapons()
* Returns the number of lweapons running a stdWeapons.zh script, as denoted by their l->Misc[INDEX}.
int NumScriptedLweaponsOf (int scriptID)
* Returns total number of ffc`s that running a specific script.

CheckMaxLweapons(ffc f, int maxlweapons)
* Checks against maximum number of allowed scripted lweapons running given FFC`s script.
* Clears and quits automatically, if the limit is exceeded.
* Usually placed in the init phase of the script before Lweapon initialization.

bool MaxLweaponsReached(ffc f, int maxlweapons)
* Checks against maximum number of allowed scripted lweapons running given FFC`s script.
* Unlike previous function, it just returns whether limit is reached or not.
 
bool InsideScreen (lweapon l)
* Returns TRUE, if lweapon is inside screen boundaries. 

bool OnScreenEdge(lweapon lw)
* Returns TRUE, if lweapon is touching screen boundaries.

void BigLweapon(lweapon lw, int Xsize, int Ysize)
* Sets size of the given lweapon, in tiles.

void ContiniousSFX(ffc f, int delay, int sound)
* Run this function every frame and lweapon will emit sounds continuously.

void ItemPickup (lweapon l, bool instant, int PickFlags)
void ItemPickup (lweapon l, bool instant)
* Causes Lweapon to drag dropped items around or instantly deliver items straight to Link.
  Called automatically on LweaponWaitframe, if LWF_CAN_PICKUP_ITEMS or 
  LWF_CAN_INSTADELIVER_ITEMS lweapon flag is set.
  Uses item->Pickup flags to control which items to pickup. By default (and automatically inside
  LweaponWaitframe) By default, it works for only items dropped by dead enemies.

GetLweaponHitboxOffsets(lweapon l, int OFFSETS)
* Loads lweapon hitbox offsets into given array. Minimum 4 ints.

void SetLweaponHitboxOffsets(lweapon l, int top, int bottom, int left, int right)
* Sets Lweapon Hitbox offset parameters. Values between 0 and 64.

void AutoRotateLweaponHitbox (lweapon l, int OrigHitboxData)
* When run every frame, this function sets lweapon hitbox rotation dynamically
* OrighitboxData needed to be stored prior to using this function.

int GetMasterFFCID(lweapon l)
* Obtains ID of the FFC that controls this scripted lweapon. Non-ghosted lweapon returns -1.

int GetItemID(lweapon l)
* Obtains ID of the item that launched this scripted lweapon. Non-ghosted lweapon returns -1.

void GetLweaponHitboxOffsets(lweapon l, int[] OFFSETS)
* Loads lweapon hitbox offsets into given array. Minimum size of input array is 4 units.

void FullScreenAttack(ffc f, lweapon l, int flashcolor, int duration,int defense, int trigger)
* Performs full-screen attack ala LTTP medallions.
* flashcolor - color of fullscreen flashing
* duration - duration of attack
* defense - enemy defense used against this piece of code
* trigger - if at least 1 combo flag with specific ID is present on the screen, secrets pop open.


*Debug functions*
void DrawLweaponHitbox(lweapon l)
* Draws Lweapon`s interaction hitbox. Used for debugging.

void LweaponDebugValue(int number, int value)
* Draws any value of your choise at the top of the screen. Ripped from stdExtra.zh
* number - affects Y position of drawn number.
* value - the target variable to draw.