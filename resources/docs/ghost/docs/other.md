# Miscellaneous functions

void SetEnemyProperty(npc enemy, int property, float newValue)
float GetEnemyProperty(npc enemy, int property)
:   These functions allow some of the properties of ghost.zh scripts to be read
    and set from other scripts. property must be one of the following:
    
    * `ENPROP_X`
    * `ENPROP_Y`
    * `ENPROP_Z`
    * `ENPROP_JUMP`
    * `ENPROP_DIR`
    * `ENPROP_HP`
    * `ENPROP_CSET`
    
    These work for all enemies, not just scripted ones, so there's no need
    to check that beforehand.

int FindSpawnPoint(bool landOK, bool wallsOK, bool waterOK, bool pitsOK)
:   Finds a random combo position meeting the given criteria some distance away
    from Link and enemies. This can be useful if you want the enemy to spawn in
    a random position that ZC would not select itself. The result is undefined
    if no suitable location exists, but that will not happen unless there is
    almost no space abailable on the screen.
    
    `landOK`: Walkable, non-water, non-pit combos are okay
    
    `wallsOK`: Unwalkable, non-water, non-pit combos are okay
    
    `waterOK`: All deep water combos are okay
    
    `pitsOK`: Direct warps are okay

int FindSpawnPoint(int type, int flag)
:   Like the above, but finds a combo with the given combo type or flag,
    regardless of solidity. Only layer 0 is checked. Use -1 for either argument
    to ignore it.

npc SpawnNPC(int id)
:   Spawns an npc with the given ID in a random location and returns a pointer to it.

int FindUnusedFFC()
int FindUnusedFFC(int startingFrom)
:   Returns the ID number of an FFC that is not currently in use in the range
    `AUTOGHOST_MIN_FFC` to `AUTOGHOST_MAX_FFC`. Returns 0 if no FFC is available in
    that range. If `startingFrom` is given, the ID returned will be at least
    `startingFrom+1`.

void Ghost_MarkAsInUse(npc ghost)
:   Mark an enemy as being in use by a script. This allows you to create
    additional enemies without `AutoGhost()` or `Ghost_InitWait2()` trying to use them.

bool Ghost_IsInUse(npc ghost)
:   Returns true if the given enemy is in use by a ghost.zh script.

bool ClockIsActive()
:   Returns true if a clock is currently in effect.

void SuspendGhostZHScripts()
:   Suspends the execution of all ghost.zh scripts. Until scripts are resumed,
    `Ghost_Waitframe` functions will not return and `UpdateEWeapon()` will not
    do anything. This also stops `AutoGhost()` from running, so any new enemies
    will not have scripts launched until `ResumeGhostZHScripts()` is called.

void ResumeGhostZHScripts()
:   Resumes execution of ghost.zh scripts.
