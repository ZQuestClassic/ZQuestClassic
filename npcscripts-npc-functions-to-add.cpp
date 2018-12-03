//Aff all of these to npc-> in ZScript for easier control over npcs via npc scripts. 

// Supplemental animation code that all derived classes should call
    // as a return value for animate().
    // Handles the death animation and returns true when enemy is finished.
    virtual bool Dead(int index);
    // Basic animation code that all derived classes should call.
    // The one with an index is the one that is called by
    // the guys sprite list; index is the enemy's index in the list.
    virtual bool animate(int index);
    
    // auomatically kill off enemy (for rooms with ringleaders)
    virtual void kickbucket();
    // Stop BG SFX only if no other enemy is playing it
    void stop_bgsfx(int index);
    // Take damage or ignore it
    virtual int takehit(weapon *w);
    // override hit detection to check for invicibility, stunned, etc
    virtual bool hit(sprite *s);
    virtual bool hit(int tx,int ty,int tz,int txsz,int tysz,int tzsz);
    virtual bool hit(weapon *w);
    virtual void break_shield() {}; // Overridden by types that can have shields
    
     // returns true if next step is ok, false if there is something there
    bool canmove(int ndir,fix s,int special,int dx1,int dy1,int dx2,int dy2);
    bool canmove(int ndir,fix s,int special);
    bool canmove(int ndir,int special);
    bool canmove(int ndir);
    // 8-directional
    void newdir_8(int rate,int homing, int special,int dx1,int dy1,int dx2,int dy2);
    void newdir_8(int rate,int homing, int special);
    // makes the enemy slide backwards when hit
    // sclk: first byte is clk, second byte is dir
    bool slide();
    bool can_slide();
    bool fslide();
    // changes enemy's direction, checking restrictions
    // rate:   0 = no random changes, 16 = always random change
    // homing: 0 = none, 256 = always
    // grumble 0 = none, 4 = strongest appetite
    void newdir(int rate,int homing,int special);
    void newdir();
    fix distance_left();
    // keeps walking around
    void constant_walk(int rate,int homing,int special);
    void constant_walk();
    // falls through the Z axis;
    void falldown();
    int pos(int x,int y);
    // for variable step rates
    void variable_walk(int rate,int homing,int special);
    // pauses for a while after it makes a complete move (to a new square)
    void halting_walk(int rate,int homing,int special,int hrate, int haltcnt);
    // 8-directional movement, aligns to 8 pixels
    void constant_walk_8(int rate,int homing,int special);
    // 8-directional movement, halting
    void halting_walk_8(int newrate,int newhoming, int newclk,int special,int newhrate, int haltcnt);
    // 8-directional movement, no alignment
    void variable_walk_8(int rate,int homing,int newclk,int special);
    // same as above but with variable enemy size
    void variable_walk_8(int rate,int homing,int newclk,int special,int dx1,int dy1,int dx2,int dy2);
    // the variable speed floater movement
    // ms is max speed
    // ss is step speed
    // s is step count
    // p is pause count
    // g is graduality :)
    void floater_walk(int rate,int newclk,fix ms,fix ss,int s,int p, int g);
    void floater_walk(int rate,int newclk,fix s);
    // Checks if enemy is lined up with Link. If so, returns direction Link is
    // at as compared to enemy. Returns -1 if not lined up. Range is inclusive.
    int lined_up(int range, bool dir8);
    // returns true if Link is within 'range' pixels of the enemy
    bool LinkInRange(int range);
    // Breathe fire
    void FireBreath(bool seeklink);
    // Shoot weapons
    void FireWeapon();
    // place the enemy in line with Link (red wizzrobes)
    void place_on_axis(bool floater, bool solid_ok);
    void update_enemy_frame();
    void n_frame_n_dir(int frames, int dir, int f4);
    void tiledir_small(int ndir, bool fourdir);
    void tiledir_three(int ndir);
    void tiledir(int ndir, bool fourdir);
    void tiledir_big(int ndir, bool fourdir);
    // Enemies that cannot ever be penetrated by weapons
    bool cannotpenetrate();