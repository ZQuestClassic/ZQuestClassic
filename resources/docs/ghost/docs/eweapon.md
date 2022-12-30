# Eweapon functions

eweapon FireEWeapon(int weaponID, int x, int y, float angle, int step, int damage, int sprite, int sound, int flags)
eweapon FireAimedEWeapon(int weaponID, int x, int y, float angle, int step, int damage, int sprite, int sound, int flags)
eweapon FireNonAngularEWeapon(int weaponID, int x, int y, int direction, int step, int damage, int sprite, int sound, int flags)
eweapon FireBigEWeapon(int weaponID, int x, int y, float angle, int step, int damage, int sprite, int sound, int flags, int width, int height)
eweapon FireBigAimedEWeapon(int weaponID, int x, int y, float angle, int step, int damage, int sprite, int sound, int flags, int width, int height)
eweapon FireBigNonAngularEWeapon(int weaponID, int x, int y, int direction, int step, int damage, int sprite, int sound, int flags, int width, int height)
:   Create an eweapon with the given properties.
    
    `angle` is given in radians. If the weapon is aimed, this is an offset from
    the base angle. So, for instance, an `angle` of 0.2 will always aim slightly
    away from Link.
    
    `sprite` should be the ID of a sprite from **Quest > Graphics > Sprites > Weapons/Misc.** 
    If this is -1, the standard sprite for the weapon type will be used.
    
    `sound` specifies a sound to play. If this is 0, no sound will be played.
    If it's -1, a sound will be chosen automatically based on the type of weapon.
    
    `flags` argument should be one or more `EWF_` constants ORed together, or 0 for none.
    
    EWF_UNBLOCKABLE
    :   The weapon is unblockable.

    EWF_ROTATE
    :   The weapon's sprite will be rotated and flipped according to the weapon's direction.

    EWF_ROTATE_360
    :   The weapon will be drawn using Screen->DrawTile(), allowing the sprite
        to rotate in whichever direction the weapon is moving.
        
        !!! tip
            The base sprite set in **Quest > Graphics > Sprites > Weapons/Misc.**
            should be pointing to the right.

    EWF_SHADOW
    :   The weapon will cast a shadow if its Z position is greater than 0.

    EWF_FLICKER
    :   The weapon will be invisible every other frame.

    EWF_NO_COLLISION
    :   The weapon's collision detection will be disabled.

    EWF_FAKE_Z
    :   The weapon's hitbox will always be on the ground.
        
        !!! note
            This does not actually keep the weapon on the ground; it only
            simulates this by setting the weapon's hit and draw offsets.

eweapon CreateDummyEWeapon(int weaponID, int step, int damage, int sprite, int sound, int flags)
eweapon CreateBigDummyEWeapon(int weaponID, int step, int damage, int sprite, int sound, int flags, int width, int height)
:   These will create a dummy eweapon to be used as a prototype for
    `SetEWeaponDeathEffect()`. The dummy weapon will be invisible and will have
    collision detection disabled. It can have a movement, lifespan, and death
    effect set, but the dummy weapon itself will not make use of them - only
    its copies will.
    
    A dummy weapon will never be removed automatically. If reaching the weapon
    limit is a concern, you should destroy the weapon yourself when it's no
    longer needed.

void SetEWeaponMovement(eweapon wpn, int type, float arg, float arg2)
:   This sets an eweapon's movement pattern. The type argument should be one of
    the `EWM_` constants below. The effect of `arg` and `arg2` vary depending on
    the type of movement.
    
    EWM_SINE_WAVE
    :   Move in a sine wave.
        
        `arg`: Amplitude
        
        `arg2`: Angular frequency in degrees per frame
    
    EWM_HOMING
    :   Turn toward Link each frame.
        
        `arg`: Maximum rotation per frame in radians
        
        `arg2`: Homing time in frames - after this many frames, the weapon
        will die. Use -1 to home indefinitely.
    
    EWM_HOMING_REAIM
    :   Move in straight lines, stopping and re-aiming at Link.
        
        `arg`: Number of re-aims
        
        `arg2`: Time between re-aims
    
    EWM_RANDOM
    :   Turn randomly each frame.
        
        `arg`: Maximum rotation per frame in radians
        
        `arg2`: Rotation time in frames - after this many frames, the weapon
        will die. Use -1 to keep moving indefinitely.
    
    EWM_RANDOM_REAIM
    :   Stop frequently and aim in a random direction
        
        `arg`: Number of re-aims
        
        `arg2`: Time between re-aims
    
    EWM_VEER
    :   Accelerate in a given direction.
        
        `arg`: Direction of acceleration
        
        `arg2`: Rate of acceleration
    
    EWM_DRIFT
    :   Drift in the given direction at a constant speed.
        
        `arg`: Drift direction
        
        `arg2`: Drift speed
    
    EWM_DRIFT_WAIT
    :   Drift in the given direction at a constant speed and wait 16 frames before
        beginning normal movement. This mimics the behavior of boss fireballs.
        
        `arg`: Drift direction
        
        `arg2`: Drift speed
    
    EWM_THROW
    :   Arc through the air as if thrown. The weapon stops moving when it hits
        the ground.
        
        `arg`: Initial upward velocity; use -1 to make the weapon travel the
        distance to Link
        
        `arg2`: OR together these flags, or use 0 for none:
    
        EWMF_BOUNCE
        :   The weapon will bounce on impact.
        
        EWMF_DIE
        :   The weapon will die on impact rather than simply stopping.
    
    EWM_FALL
    :   Fall straight down. The weapon stops moving when it hits the ground.
        
        `arg`: Initial height
        
        `arg2`: OR together these flags, or use 0 for none:
        
        EWMF_BOUNCE
        :   The weapon will bounce on impact.
        
        EWMF_DIE
        :    The weapon will die on impact rather than simply stopping.

void SetEWeaponLifespan(eweapon wpn, int type, int arg)
:   This controls the conditions under which a weapon dies. "Dying" does not mean
    the weapon is removed, but that its scripted movement is no longer handled,
    and, optionally, a death effect is activated. Use one of the EWL_ constants
    for the type argument. The effect of arg depends on the type.

    EWL_TIMER
    :   Die after a certain amount of time.
        
        `arg`: Time in frames

    EWL_NEAR_LINK
    :   Die when within a certain distance of Link.
        
        `arg`: Distance in pixels

    EWL_SLOW_TO_HALT
    :   Slow down until stopped, then die. This can behave oddly if movement is set.
        
        `arg`: Step per frame

void SetEWeaponDeathEffect(eweapon wpn, int type, int arg)
:   This determines what happens when the weapon dies. The type argument should
    be one of the `EWD_` constants below. The effect of arg depends on the type.
    
    EWD_VANISH
    :   The weapon is removed.
        
        `arg`: No effect

    EWD_AIM_AT_LINK
    :   The weapon pauses for a moment, then aims at Link.
        
        `arg`: Delay

    EWD_EXPLODE
    :   The weapon explodes.
        
        `arg`: Explosion damage

    EWD_SBOMB_EXPLODE
    :   The weapon explodes like a super bomb.
        
        `arg`: Explosion damage

    EWD_4_FIREBALLS_HV
    :   Shoots fireballs horizontally and vertically.
        
        `arg`: Fireball sprite

    EWD_4_FIREBALLS_DIAG
    :   Shoots fireballs at 45-degree angles.
        
        `arg`: Fireball sprite

    EWD_4_FIREBALLS_RANDOM
    :   Randomly shoots fireballs either vertically and horizontally or at
        45-degree angles.
        
        `arg`: Fireball sprite

    EWD_8_FIREBALLS
    :   Shoots fireballs horizontally, vertically, and at 45-degree angles
        
        `arg`: Fireball sprite

    EWD_FIRE
    :   Leaves a single, immobile fire
        
        `arg`: Fire sprite

    EWD_4_FIRES_HV
    :   Shoots fires horizontally and vertically.
        
        `arg`: Fire sprite

    EWD_4_FIRES_DIAG
    :   Shoots fires at 45-degree angles.
        
        `arg`: Fire sprite

    EWD_4_FIRES_RANDOM
    :   Randomly shoots fires either vertically and horizontally or at
        45-degree angles.
        
        `arg`: Fire sprite

    EWD_8_FIRES
    :   Shoots fires horizontally, vertically, and at 45-degree angles
        
        `arg`: Fire sprite

    EWD_SPAWN_NPC
    :   Creates an npc at the weapon's location. This is done without regard for
        the suitability of the location.
        
        `arg`: npc to spawn

    EWD_RUN_SCRIPT
    :   Loads an unused FFC, positions it at the weapons position, and runs a
        script. The weapon will not be removed; it should be loaded by the script
        with `GetAssociatedEWeapon()`. The number to pass into it will be given to
        the script as argument D0. If no FFC is free, the weapon will simply be removed.
        
        `arg`: Script to run. If this is not a valid script, `EWD_VANISH` will be
        set instead

void SetEWeaponDeathEffect(eweapon wpn, eweapon prototype, int numShots, int spreadType, float angle)
:   This will make a weapon spawn copies of a prototype weapon when it dies.
    The prototype must be a dummy eweapon.
    
    `spreadType` is one of the following:
    
    EWD_EVEN
    :   Weapons are evenly spaced
        
        `angle`: First weapon's offset from 0 in radians

    EWD_AIMED
    :   Weapons are aimed at Link
        
        `angle`: Angle range of spawned weapons in radians

    EWD_RANDOM
    :   Weapons are fired at random angles
        
        `angle`: No effect

void SetEWeaponDir(eweapon wpn)
:   Set the direction of an angled eweapon so that it interacts correctly
    with shields. This is normally handled automatically, but you may need
    to use it if you script a weapon's movement yourself.

void SetEWeaponRotation(eweapon wpn)
void SetEWeaponRotation(eweapon wpn, int direction)
:   Rotate the weapon's sprite. If `direction` is not given,
    this is done according to the weapon's direction.

void KillEWeapon(eweapon wpn)
:   Kill the eweapon, stopping scripted movement and activating any
    death effects.

void DrawEWeaponShadow(eweapon wpn)
:   Draws a shadow under the eweapon according to the `GH_SHADOW_` constants. This
    is used internally when `EWF_SHADOW` is set.

int GetDefaultEWeaponSprite(int weaponID)
:   Returns the sprite normally used by the given weapon type.

int GetDefaultEWeaponSound(int weaponID)
:   Returns the sound normally used by the given weapon type.

eweapon GetAssociatedEWeapon(int weaponID)
:   Finds an eweapon using an internal ID number. This should be used by scripts
    launched by `EWD_RUN_SCRIPT`, and the weaponID argument should be the number
    passed to the script as D0. If the requested weapon is not found, an
    uninitialized pointer will be returned.

bool IsGhostZHEWeapon(eweapon wpn)
:   Returns true if the weapon is one created or handled by ghost.zh.

bool IsDummyEWeapon(eweapon wpn)
:   Returns true if the eweapon is a dummy.

