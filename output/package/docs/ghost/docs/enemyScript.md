# Enemy variables and functions

## Global variables

Many aspects of the enemy are controlled by these global variables. They're set
by a `Ghost_Init` function and kept consistent between frames by the
`Ghost_Waitframe` functions.

float Ghost_X
float Ghost_Y
float Ghost_Z
:   The X, Y and Z position of the enemy.

float Ghost_Vx
float Ghost_Vy
float Ghost_Jump
:   The X, Y, and Z velocity of the enemy.

    `Ghost_Vx` and `Ghost_Vy` are handled by `Ghost_MoveXY()`, so they will respect
    solidity, water, pits, and screen edges.

float Ghost_Ax
float Ghost_Ay
:   The X and Y acceleration of the enemy.

int Ghost_Dir
:   The direction the enemy is facing.

int Ghost_Data
int Ghost_CSet
:   The current combo and CSet of the enemy.
    
    If `Ghost_InitAutoGhost()` is used, `Ghost_CSet` will be set to the enemy's CSet.
    All other initializion functions will use the FFC's CSet.
    
    If the `GHF_4WAY` or `GHF_8WAY` flags are used, `Ghost_Data` should always be set to
    the upward-facing combo.

int Ghost_TileWidth
int Ghost_TileHeight
:   The width and height of the enemy in tiles, which must be between 1 and 4.

    !!! caution
        If these variables are set, the change will not fully take effect until
        the next Waitframe. It's better to use `Ghost_SetSize()` instead and
        consider these variables read-only.

int Ghost_HP
:   The enemy's current HP.

## Initialization

Call one of the init functions at the start of the script. It will set
the ffc's and npc's properties and initialize the global variables.
Normally, `ghost->OriginalTile` will be set to `GH_BLANK_TILE` to make the enemy
invisible, `ghost->Extend` will be set to 3, and `Ghost_TileWidth` and
`Ghost_TileHeight` will be set to the size of the FFC. These will not happen
if the FFC's combo is `GH_INVISIBLE_COMBO`, or, in the case of AutoGhost
enemies, if that's the combo they're set to use.

void Ghost_Init(ffc this, npc ghost)
:   The basic initialization function. Provide your own npc.

npc Ghost_InitCreate(ffc this, int enemyID)
:   This will create an npc of type `enemyID` and return a pointer to it.

npc Ghost_InitWait(ffc this, int enemyIndex, bool useEnemyPos)
:   This function will select an enemy on the screen as the ghost.
    
    `enemyIndex` is the number to pass to `Screen->LoadNPC()`, normally the npc's
    position in the screen's enemy list.
    
    If `useEnemyPos` is true, the FFC will be moved to the enemy's position.
    Otherwise, the FFC's current position will be used.
    
    The function will only wait 4 frames for the enemy to appear to minimize
    the possibility of incorrectly using an enemy that is spawned later on. If
    the expected enemy does not appear in that time, it will call `Quit()`.

npc Ghost_InitWait2(ffc this, int enemyID, bool useEnemyPos)
:   Similar to `Ghost_InitWait()`, except this loads the first enemy of type
    `enemyID` that is not already in use by another ghost.zh script. This is
    especially helpful if enemies are present other than those placed by
    **Screen > Enemies**, since you can't always be certain what index an enemy
    will be assigned.

npc Ghost_InitSpawn(ffc this, int enemyID)
:   This will create the ghosted enemy in a random location.

npc Ghost_InitAutoGhost(ffc this, int enemyID)
:   This should only be used with a script that is meant to be set up
    automatically by the `AutoGhost()` function. Similar to `Ghost_InitWait2()`,
    this will load the first unused enemy of the given type. The enemy must
    already be on the screen when the function is called, or it will log
    an error and call `Quit()`.

## Flags

There are a number of flags you can apply to an enemy to control its behavior
and appearance. This is typically done just after initialization, but flags
can be changed at any time while the enemy is alive.

Note that these flags cannot be combined with bitwise OR.

void Ghost_SetFlag(int flag)
void Ghost_UnsetFlag(int flag)
:   Set or unset a flag that controls details of the enemy's behavior.
    
    Valid arguments are:

    GHF_KNOCKBACK
    :   The enemy can be knocked back when hit.

    GHF_KNOCKBACK_4WAY
    :   The enemy will be knocked back when hit, even if its direction and
        Link's are not suitably aligned.
        
        !!! note
            It is not necessary to use `GHF_KNOCKBACK` along with this flag.

    GHF_REDUCED_KNOCKBACK
    :   The enemy will be knocked back 2 pixels per frame instead of 4.
        
        !!! note
            It is not necessary to use `GHF_KNOCKBACK` along with this flag.

    GHF_STUN
    :   Stunning will be handled automatically. `Ghost_Waitframe` functions will not
        return while the enemy is stunned.

    GHF_CLOCK
    :   The enemy will be affected by clocks. `Ghost_Waitframe` functions will not
        return while a clock is active.

    GHF_NO_FALL
    :   `ghost->Jump` will be set to 0 each frame and `Ghost_Jump` will be ignored.
        If `GHF_NO_FALL` is not used, `Ghost_CanMove(DIR_UP)` will always be false
        on sideview screens.

    GHF_SET_DIRECTION
    :   The enemy's direction will automatically be set based on which way
        it moves. Unless `GHF_8WAY` is set, this will only use up, down, left,
        and right.

    GHF_SET_OVERLAY
    :   Set or unset the **Draw Over** flag each frame based on Z position. The
        height at which it changes is determined by `GH_DRAW_OVER_THRESHOLD`.

    GHF_FAKE_Z
    :   `Ghost_Z` and `Ghost_Jump` will affect the enemy's position on the Y axis
        rather than Z. A shadow will be drawn under it according to the
        `GH_SHADOW_` settings.
        
        !!! note
            If the global setting `GH_FAKE_Z` is enabled, all Z movement
            will behave this way, and this flag will have no effect.

    GHF_4WAY
    :   Change the FFC's combo based on `Ghost_Dir`. This requires a particular
        setup. There must be four consecutive combos in the list, one for each
        direction: up, down, left, and right, in order. Set `Ghost_Data` to the
        upward-facing combo.

    GHF_8WAY
    :   Set the FFC's combo based on direction, inclusing diagonals. This will
        also cause `GHF_SET_DIRECTION` to use all eight directions. The combo
        order is up, down, left, right, up-left, up-right, down-left, down-right.

    GHF_MOVE_OFFSCREEN
    :   Movement functions will not stop the enemy at the screen edges.

    GHF_NORMAL
    :   Combines `GHF_KNOCKBACK`, `GHF_STUN`, `GHF_CLOCK`, and `GHF_SET_DIRECTION`.

    GHF_IGNORE_SOLIDITY
    :   The enemy can move over solid combos, as long as they are not pits or water.
        
        !!! note
            This and the following walkability control flags only affect movement
            via the `Ghost_Move` functions and `Ghost_Vx` and `Ghost_Vy`.
            If `Ghost_X` and `Ghost_Y` are set directly, the enemy can move anywhere.

    GHF_IGNORE_WATER
    :   The enemy can move over water, regardless of solidity.

    GHF_IGNORE_PITS
    :   The enemy can move over direct warps, regardless of solidity.

    GHF_IGNORE_ALL_TERRAIN
    :   Combines `GHF_IGNORE_SOLIDITY`, `GHF_IGNORE_WATER`, and `GHF_IGNORE_PITS`.

    GHF_WATER_ONLY
    :   The enemy can only move in water (including shallow water). This overrides
        the above terrain flags.

    GHF_DEEP_WATER_ONLY
    :   The enemy can only move in deep water. This overrides the above
        terrain flags.

    GHF_FULL_TILE_MOVEMENT
    :   The enemy will not walk onto a tile unless the entire tile is walkable.
        This is particularly useful for enemies using walking functions along
        with other movement.

    GHF_FLYING_ENEMY
    :   The enemy will be blocked by the **No Flying Enemies** combo type.
        If this flag is not set, it will be blocked by **No Ground Enemies** if
        `Ghost_Z==0` or **No Jumping Enemies** if `Ghost_Z>0`.
        
        !!! note
            This flag does not imply any others that a flying enemy would likely use,
            such as `GHF_IGNORE_PITS` or `GHF_NO_FALL`. Those must be set separately.

    GHF_IGNORE_NO_ENEMY
    :   The enemy will ignore all enemy blocking flags and combos.

    GHF_STATIC_SHADOW
    :   The enemy's shadow will not animate. This only applies if Z movement is
        faked due to either by `GH_FAKE_Z` or `GHF_FAKE_Z`.

void Ghost_ClearFlags()
:   Unsets all flags.

bool Ghost_FlagIsSet(int flag)
:   Returns true if the given flag is set and false if not.

## Waitframe

After the enemy is initialized and until it dies, use the `Ghost_Waitframe`
functions below instead of `Waitframe()` to keep the global variables
consistent and maintain the illusion of a real enemy. Failure to do so may
cause the global variables to become incorrect, completely breaking your script.

bool Ghost_Waitframe(ffc this, npc ghost, bool clearOnDeath, bool quitOnDeath)
:   This is a replacement function for `Waitframe()`. In addition to waiting a frame,
    it will handle the necessary routine updates. These include setting the npc's
    and ffc's positions and movement and dealing with clocks, stunning, flashing,
    knockback, and death. If the enemy is stunned or frozen by a clock, or if
    scripts are suspended, `Ghost_Waitframe()` will not return until the enemy dies
    or the blocking condition ends.
    
    If `clearOnDeath` is true, when the npc dies, `this->Data` will be set to 0,
    `Ghost_ClearAttachedFFCs()` will be called, and the npc will be moved so that
    its death animation and dropped item are centered.
    
    If `quitOnDeath` is true, `Quit()` will be called when the npc dies.
    If it's false, the function will return true if the npc is alive and false otherwise.

bool Ghost_Waitframe2(ffc this, npc ghost, bool clearOnDeath, bool quitOnDeath)
:   Like the above, except this ignores the position and movement variables and
    uses the npc's instead. Use this if you want to use built-in enemy movement.

void Ghost_WaitframeLight(ffc this, npc ghost)
:   Minimal waitframe function. Aside from waiting a frame, this only keeps the
    global variables consistent, positions attached FFCs, and sets up drawing.
    If scripts are suspended, this function will not return until they are resumed.

bool Ghost_Waitframes(ffc this, npc ghost, bool clearOnDeath, bool quitOnDeath, int numFrames)
bool Ghost_Waitframes2(ffc this, npc ghost, bool clearOnDeath, bool quitOnDeath, int numFrames)
void Ghost_WaitframesLight(ffc this, npc ghost, int numFrames)
:   These simply call the corresponding `Ghost_Waitframe` function `numFrames` times.

void Ghost_Waitframe(ffc this, npc ghost)
void Ghost_Waitframe2(ffc this, npc ghost)
void Ghost_Waitframes(ffc this, npc ghost, int numFrames)
void Ghost_Waitframes2(ffc this, npc ghost, int numFrames)
:   Simpler variants of the Waitframe functions. These are equivalent to calling
    the regular functions with `clearOnDeath` and `quitOnDeath` both true. The
    script will quit if the enemy dies, so there is no return value from these.

bool Ghost_Waitframe(ffc this, npc ghost, int deathAnimation, bool quitOnDeath)
bool Ghost_Waitframe2(ffc this, npc ghost, int deathAnimation, bool quitOnDeath)
:   These will call `Ghost_DeathAnimation()` with the specified animation if the
    enemy dies. See `Ghost_DeathAnimation()` for `deathAnimation` arguments.

## Modification

void Ghost_SetSize(ffc this, npc ghost, int tileWidth, int tileHeight)
:   Sets the tile size of the enemy. The new width and height are given
    in tiles and must be between 1 and 4. If -1 is given for either dimension,
    it will not be changed. If you called `Ghost_SetHitOffsets()` previously,
    it will be undone; the enemy's size and position will be set to match
    the FFC's.

void Ghost_Transform(ffc this, npc ghost, int combo, int cset, int tileWidth, int tileHeight)
:   Change the FFC to a new combo and CSet and resize the FFC and npc. The FFC's
    and npc's positions will be adjusted so that they're centered on the same
    spotas before. For all four numeric arguments, pass in -1 if you don't want to change
    the existing value. If you called `Ghost_SetHitOffsets()` previously, it will
    be undone. Attached FFCs will not be affected.

void Ghost_SetHitOffsets(npc ghost, float top, float bottom, float left, float right)
:   If you want the enemy's hitbox to be smaller than the FFC, use this function
    to adjust it. Each argument will cause the hitbox to shrink away from the
    corresponding edge. This applies to both collision detection and movement.
    For each argument, if the number is 1 or greater, it will be interpreted as
    the difference in pixels; if the number is between 0 and 1, it will be
    treated as a fraction of the FFC's full size. So, for instance, a `top`
    argument of 0.25 would shrink the hitbox by 1/4 of the FFC's height.

void Ghost_SwapNPC(npc oldGhost, npc newGhost, bool copyHP)
:   Copies size, position, Misc[], and HP (optionally) from the old ghost to the
    new one, then moves the old one out of the way.

void Ghost_ReplaceNPC(npc oldGhost, npc newGhost, bool copyHP)
:   Copies data from the old ghost to the new one, then silently kills the old one.

void Ghost_StoreDefenses(npc ghost, int storedDefense[])
:   Copies `ghost->Defense[]` into `storedDefense`. The array size must be at least 18.

void Ghost_SetDefenses(npc ghost, int defense[])
:   Copies `defense` into `ghost->Defense[]`. The array size must be at least 18.

void Ghost_SetAllDefenses(npc ghost, int defType)
:   Sets all of the enemy's defenses to `defType`, which should be an NPCDT constant.

## Movement

void Ghost_Move(int dir, float step, int imprecision)
void Ghost_MoveXY(float xStep, float yStep, int imprecision)
void Ghost_MoveAtAngle(float angle, float step, int imprecision)
void Ghost_MoveTowardLink(float step, int imprecision)
:   Makes the enemy move. `Ghost_CanMove()` will be checked automatically.
    If the `GHF_SETDIRECTION` flag is set, the npc's direction will be changed
    accordingly unless `Ghost_ForceDir()` was called beforehand. 
    
    `step` is given in pixels.
    
    `Ghost_MoveAtAngle()`'s `angle` argument is in degrees.
    
    `imprecision` makes the function ignore a couple of pixels at the
    edges of the enemy's hitbox so that it doesn't get stuck on corners.

bool Ghost_CanMove(int dir, float step, int imprecision)
bool Ghost_CanMove(int dir, float step, int imprecision, bool inAir)
:   Determines whether the enemy can move in the given direction and distance.
    step is given in pixels.
    
    On sideview screens, a direction of `DIR_UP` will always return false unless
    the `GHF_NO_FALL` flag is set.
    
    `inAir` determines whether non-flying enemies check for **No Ground Enemies** or
    **No Jumping Enemies**. If it is not specified, `inAir` is true if `Ghost_Z>0`.

bool Ghost_CanMovePixel(int x, int y)
bool Ghost_CanMovePixel(int x, int y, bool inAir)
:   Used internally by `Ghost_CanMove()`. Returns true if the enemy can move onto
    the given pixel.

float Ghost_HaltingWalk4(int counter, int step, int rate, int homing, int hunger, int haltRate, int haltTime)
:   This function mimics the built-in movement function used by walking enemies.
    
    `step` corresponds to `npc->Step`. It represents speed in hundredths of a pixel
    per frame.
    
    `rate` corresponds to `npc->Rate` and should be between 0 and 16.
    
    `homing` corresponds to `npc->Homing` and should be between 0 and 256.
    
    `hunger` corresponds to `npc->Hunger` and should be between 0 and 4.
    
    `haltRate` corresponds to `npc->Haltrate` and should be between 0 and 16.
    
    `haltTime` determines how long the enemy will stand still when it
    halts. Zols use `Rand(8)<<4`, Gels use `(Rand(8)<<3)+2`, Goriyas use 1, and all
    others use 48.
    
    `counter` should initially be -1, then set to the function's
    return value each frame. While the enemy is halted, the return value will be
    the remaining halt time, which you can use to determine when to fire. So:
    
        :::cpp
        int counter = -1;
        while(true)
        {
            counter=Ghost_HaltingWalk4(counter, step, rate, homing, hunger, haltRate, 48);
            if(counter==16) // Fire after halted for 32 frames
                FireEWeapon(...);
            Ghost_Waitframe(...);
        }

    This function depends on `Ghost_Dir` and will set it even if `GHF_SET_DIRECTION`
    is not used. It will also set `Ghost_Dir` to -1 if the enemy is trapped.

int Ghost_ConstantWalk4(int counter, int step, int rate, int homing, int hunger)
:   This is the movement function used by Dodongos and Lanmolas. The arguments
    are the same as above.
    
    This function depends on `Ghost_Dir` and will set it
    even if `GHF_SET_DIRECTION` is not used. It will also set `Ghost_Dir` to -1 if
    the enemy is trapped.
 
int Ghost_ConstantWalk8(int counter, int step, int rate, int homing, int hunger)
:   This is the movement function used by Moldorms. The arguments are the same as above.
    
    This function depends on `Ghost_Dir` and will set it even if `GHF_SET_DIRECTION`
    is not used. It will also set `Ghost_Dir` to -1 if the enemy is trapped.

int Ghost_VariableWalk8(int counter, int step, int rate, int homing, int hunger, int turnCheckTime)
:   This is the movement function used by Digdoggers, Manhandlas, Patras, and
    fairies.
    
    `turnCheckTime` deterines how many frames the enemy moves before checking
    whether it should change direction.
    
    The other arguments are the same as above.
    
    This function depends on `Ghost_Dir` and will set it even if `GHF_SET_DIRECTION`
    is not used. It will also set `Ghost_Dir` to -1 if the enemy is trapped.

float Ghost_FloaterWalk(float counter, int normalStep, int accelTime, float accelAmount, int accelSteps, int rate, int homing, int hunger, int turnCheckTime, int restTime)
:   This is the movement function used by Peahats, Keese, and Ghinis. It works
    like `Ghost_VariableWalk8()`, plus the enemy will periodically stop moving.
    When the enemy is moving at full speed, there is a 1 in 768 chance each frame
    that it will stop.
    
    When the enemy starts or stops moving, its speed will change in discrete
    increments of `accelAmount`. `accelSteps` is the number of times this will
    happen, and `accelTime` is the time in frames between each step.
    
    `restTime` is how long the enemy stays stopped. Peahats use 80, while
    Ghinis and Keese use 120.

## Other

float Ghost_GetAttribute(npc ghost, int index, float defaultVal)
float Ghost_GetAttribute(npc ghost, int index, float defaultVal, float min, float max)
:   Retrieves the value of `ghost->Attribute[index]` bound to the range `min` - `max`.
    If the value is 0, `defaultVal` will be returned. `defaultVal` does not need to be
    between `min` and `max`.

void Ghost_SpawnAnimationPuff(ffc this, npc ghost)
:   The initialization functions interrupt the normal enemy spawning animation;
    this function fakes it. It draws a sprite (lweapon) using sprite number
    `GH_SPAWN_SPRITE` at the enemy's position and returns after the animation completes.

void Ghost_SpawnAnimationFlicker(ffc this, npc ghost)
:   Draws the other spawn animation, in which the enemy flickers for 64 frames.
    With a visible enemy and invisible FFC, the timing won't be quite right
    unless the enemy's spawn type is **Instant**.

void Ghost_DeathAnimation(ffc this, npc ghost, int anim)
:   Displays a death animation based on `anim`, then clears the enemy. This does
    not call `Quit()`.
    
    `anim` should be one of these:
    
    GHD_EXPLODE
    :   A series of explosions appear randomly around the enemy.
    
    GHD_EXPLODE_FLASH
    :   The same, plus the enemy flashes.
    
    GHD_SHRINK
    :   The enemy grows slightly and then shrinks away to nothing. This does not
        work with enemies that use additional combos.
    
    Any other value will simply cause the enemy to disappear.

void Ghost_AddCombo(int combo, float x, float y)
void Ghost_AddCombo(int combo, float x, float y, int width, int height)
:   Add another combo to the enemy. It will move, flash, and flicker along with
    the enemy. `x` and `y` are offsets from `Ghost_X` and `Ghost_Y`. Up to four combos
    may be added in this way.

void Ghost_ClearCombos
:   Clears all combos added by `Ghost_AddCombo()`.

bool Ghost_GotHit()
:   Returns true if the enemy was hit in the last frame.

bool Ghost_WasFrozen()
:   Returns true if the enemy was stunned or frozen by a clock in the last frame.
    This only works if `GHF_STUN` or `GHF_CLOCK` is used.

void Ghost_ForceDir(int dir)
:   Sets `Ghost_Dir` and stops it from being changed automatically until the next frame.

void Ghost_ForceCSet(int cset)
:   Sets `Ghost_CSet` and stops flashing until the next frame.
    
    This won't work with scripts that use a visible enemy and invisible FFC.

void Ghost_StartFlashing()
void Ghost_StartFlashing(int time)
:   Makes the enemy start flashing or flickering as though it had been hit.
    
    If `time` is specified, the enemy will flash for that many frames
    instead of the standard 32.
    
    This won't work with scripts that use a visible enemy and invisible FFC.

void Ghost_StopFlashing()
:   Makes the enemy stop flashing or flickering.
    
    This won't work with scripts that use a visible enemy and invisible FFC.

void Ghost_StopKnockback()
:   Stops the enemy from being knocked back.

void Ghost_CheckHit(ffc this, npc ghost)
:   This will cause the enemy to flash and be knocked back when it is damaged.

    This is used internally by the `Ghost_Waitframe` functions. If you use
    one of those, you don't need to use this as well.

bool Ghost_CheckFreeze(ffc this, npc ghost)
:   Checks whether the npc has been stunned or frozen by a clock. If so, the
    function does not return until the npc either recovers or dies. `Ghost_CheckHit()`
    will be called each frame during that time. The return value is true if the
    npc is still alive and false if it's dead.
    
    This is used internally by the `Ghost_Waitframe` functions. If you use
    one of those, you don't need to use this as well.

void Ghost_SetPosition(ffc this, npc ghost)
:   Positions the NPC and FFC according to the position variables and
    `ghost->Draw*Offset`. This is done automatically in the `Ghost_Waitframe`
    functions, so you generally will never need to use this.
