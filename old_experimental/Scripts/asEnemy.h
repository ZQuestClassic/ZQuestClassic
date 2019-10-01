
// TODO:
// This is not the final Enemy Interface.


class IEnemy
{
    zcEnemy@ realEnemy;
    
    enemy()
    {
        @realEnemy=__getRealEnemy();
        
        clk3=0;
        movestatus=0;
        posframe=0;
        dummy_int2=0;
        dummy_int3=0;
        dummy_bool0=false;
        dummy_bool1=false;
        dummy_bool2=false;
        ox=0;
        oy=0;
    }
    
    // Properties ==============================================================
    
    int o_tile
    {
        get const { return realEnemy.o_tile; }
        set { realEnemy.o_tile=value; }
    }
    
    int frate
    {
        get const { return realEnemy.frate; }
        set { realEnemy.frate=value; }
    }
    
    int hp
    {
        get const { return realEnemy.hp; }
        set { realEnemy.hp=value; }
    }
    
    int hclk
    {
        get const { return realEnemy.hclk; }
        set { realEnemy.hclk=value; }
    }
    
    int clk3;
    
    int stunclk
    {
        get const { return realEnemy.stunclk; }
        set { realEnemy.stunclk=value; }
    }
    
    int fading
    {
        get const { return realEnemy.fading; }
        set { realEnemy.fading=value; }
    }
    
    int superman
    {
        get const { return realEnemy.superman; }
        set { realEnemy.superman=value; }
    }
    
    bool mainguy
    {
        get const { return realEnemy.mainguy; }
        set { realEnemy.mainguy=value; }
    }
    
    bool did_armos
    {
        get const { return realEnemy.did_armos; }
        set { realEnemy.did_armos=value; }
    }
    
    uint8 movestatus;
    
    uint8 item_set
    {
        get const { return realEnemy.item_set; }
        set { realEnemy.item_set=value; }
    }
    
    uint8 grumble
    {
        get const { return realEnemy.grumble; }
        set { realEnemy.grumble=value; }
    }
    
    uint8 posframe;
    
    bool itemguy
    {
        get const { return realEnemy.itemguy; }
        set { realEnemy.itemguy=value; }
    }
    
    bool count_enemy
    {
        get const { return realEnemy.count_enemy; }
        set { realEnemy.count_enemy=value; }
    }
    
    fix step
    {
        get const { return realEnemy.step; }
        set { realEnemy.step=value; }
    }
    
    fix floor_y
    {
        get const { return realEnemy.floor_y; }
        set { realEnemy.floor_y=value; }
    }
    
    bool dying
    {
        get const { return realEnemy.dying; }
        set { realEnemy.dying=value; }
    }
    
    bool ceiling
    {
        get const { return realEnemy.ceiling; }
        set { realEnemy.ceiling=value; }
    }
    
    bool leader
    {
        get const { return realEnemy.leader; }
        set { realEnemy.leader=value; }
    }
    
    bool scored
    {
        get const { return realEnemy.scored; }
        set { realEnemy.scored=value; }
    }
    
    bool script_spawned
    {
        get const { return realEnemy.script_spawned; }
        set { realEnemy.script_spawned=value; }
    }
    
    uint flags
    {
        get const { return realEnemy.flags; }
        set { realEnemy.flags=value; }
    }
    
    uint flags2
    {
        get const { return realEnemy.flags2; }
        set { realEnemy.flags2=value; }
    }
    
    int16 family
    {
        get const { return realEnemy.family; }
        set { realEnemy.family=value; }
    }
    
    int16 dcset
    {
        get const { return realEnemy.dcset; }
        set { realEnemy.dcset=value; }
    }
    
    int16 anim
    {
        get const { return realEnemy.anim; }
        set { realEnemy.anim=value; }
    }
    
    int16 dp
    {
        get const { return realEnemy.dp; }
        set { realEnemy.dp=value; }
    }
    
    int16 wpn
    {
        get const { return realEnemy.wpn; }
        set { realEnemy.wpn=value; }
    }
    
    int16 rate
    {
        get const { return realEnemy.rate; }
        set { realEnemy.rate=value; }
    }
    
    int16 hrate
    {
        get const { return realEnemy.hrate; }
        set { realEnemy.hrate=value; }
    }
    
    int16 homing
    {
        get const { return realEnemy.homing; }
        set { realEnemy.homing=value; }
    }
    
    fix dstep
    {
        get const { return realEnemy.dstep; }
        set { realEnemy.dstep=value; }
    }
    
    int dmisc1
    {
        get const { return realEnemy.dmisc1; }
        set { realEnemy.dmisc1=value; }
    }
    
    int dmisc2
    {
        get const { return realEnemy.dmisc2; }
        set { realEnemy.dmisc2=value; }
    }
    
    int dmisc3
    {
        get const { return realEnemy.dmisc3; }
        set { realEnemy.dmisc3=value; }
    }
    
    int dmisc4
    {
        get const { return realEnemy.dmisc4; }
        set { realEnemy.dmisc4=value; }
    }
    
    int dmisc5
    {
        get const { return realEnemy.dmisc5; }
        set { realEnemy.dmisc5=value; }
    }
    
    int dmisc6
    {
        get const { return realEnemy.dmisc6; }
        set { realEnemy.dmisc6=value; }
    }
    
    int dmisc7
    {
        get const { return realEnemy.dmisc7; }
        set { realEnemy.dmisc7=value; }
    }
    
    int dmisc8
    {
        get const { return realEnemy.dmisc8; }
        set { realEnemy.dmisc8=value; }
    }
    
    int dmisc9
    {
        get const { return realEnemy.dmisc9; }
        set { realEnemy.dmisc9=value; }
    }
    
    int dmisc10
    {
        get const { return realEnemy.dmisc10; }
        set { realEnemy.dmisc10=value; }
    }
    
    int bgsfx
    {
        get const { return realEnemy.bgsfx; }
        set { realEnemy.bgsfx=value; }
    }
    
    int dummy_int1 // This one's still used for traps in C++...
    {
        get const { return realEnemy.dummy_int1; }
        set { realEnemy.dummy_int1=value; }
    }
    
    int dummy_int2, dummy_int3;
    bool dummy_bool0, dummy_bool1, dummy_bool2;
    
    int clk2
    {
        get const { return realEnemy.clk2; }
        set { realEnemy.clk2=value; }
    }
    
    int sclk
    {
        get const { return realEnemy.sclk; }
        set { realEnemy.sclk=value; }
    }
    
    int ox;
    int oy;
    
    uint16 s_tile
    {
        get const { return realEnemy.s_tile; }
        set { realEnemy.s_tile=value; }
    }
    
    int scriptFlags
    {
        get const { return realEnemy.scriptFlags; }
        set { realEnemy.scriptFlags=value; }
    }
    
    // Methods =================================================================
    
    // Supplemental animation code that all derived classes should call
    // as a return value for animate().
    // Handles the death animation and returns true when enemy is finished.
    bool Dead(int index)
    {
        if(dying)
        {
            clk2-=1;
            
            if(clk2==12 && hp>-1000) // not killed by ringleader
                playDeathSFX();
                
            if(clk2==0)
            {
                if((flags&guy_neverret)!=0)
                    never_return(index);
                    
                if(leader)
                    kill_em_all();
                    
                leave_item();
            }
            
            stopBGSFX();
            return (clk2==0);
        }
        
        return false;
    }
    
    // Basic animation code that all derived classes should call.
    // The one with an index is the one that is called by
    // the guys sprite list; index is the enemy's index in the list.
    bool animate(int index)
    {
        int nx = real_x(x);
        int ny = real_y(y);
        
        if(ox!=nx || oy!=ny)
        {
            posframe=(posframe+1)%(questRuleEnabled(qr_NEWENEMYTILES)?4:2);
        }
        
        ox = nx;
        oy = ny;
        
        // Maybe they fell off the bottom in sideview, or were moved by a script.
        if(OUTOFBOUNDS())
            hp=-1000;
            
        //fall down
        if(canfall(id) && fading != fade_flicker && clk>=0)
        {
            if(screenFlagSet(fSIDEVIEW))
            {
                if(!ON_SIDEPLATFORM())
                {
                    y+=fall/100;
                    
                    if(fall <= getTerminalVelocity())
                        fall += getGravity();
                }
                else
                {
                    if(fall!=0)   // Only fix pos once
                        y-=int(y)%8; // Fix position
                        
                    fall = 0;
                }
            }
            else
            {
                if(fall!=0)
                    z-=(fall/100);
                    
                if(z<0)
                {
                    z = 0;
                    fall = 0;
                }
                else if(fall <= getTerminalVelocity())
                    fall += getGravity();
            }
        }
        
        // clk is incremented here
        clk+=1;
        if(clk >= frate)
            clk=0;
            
        // hit and death handling
        if(hclk>0)
            hclk-=1;
            
        if(stunclk>0)
            stunclk-=1;
            
        if(ceiling && z<=0)
            ceiling = false;
            
        if(!dying && hp<=0)
        {
            if(itemguy && (hasitem&2)!=0)
            {
                for(int i=0; i<numItems(); i++)
                {
                    item@ it=loadItem(i);
                    if((it.pickup&ipENEMY)!=0)
                    {
                        it.x = x;
                        it.y = y - 2;
                    }
                }
            }
            
            dying=true;
            
            if(fading==fade_flash_die)
                clk2=19+18*4;
            else
            {
                clk2 = BSZ ? 15 : 19;
                
                if(fading!=fade_blue_poof)
                    fading=0;
            }
            
            if(itemguy)
            {
                hasitem&=~2;
                item_set=0;
            }
            
            if(currscr<128 && count_enemy && !script_spawned)
                decGuyCount((currmap<<7)+currscr);
        }
        
        scored=false;
        
        c_clk+=1;
        
        // returns true when enemy is defeated
        return Dead(index);
    }
    
    // auomatically kill off enemy (for rooms with ringleaders)
    void kickbucket()
    {
        if(superman==0)
            hp=-1000; // don't call death_sfx()
    }
    
    bool hit(weapon@ w)
    {
        if((w.scriptcoldet&1)==0)
            return false;
        return (dying || hclk>0) ? false : sprite::hit(w);
    }
    
    int takehit(weapon@ w)
    {
        return realEnemy.takehit(w.realWeapon);
    }
    
    void move(fix dx,fix dy)
    {
        if(!watch && (!screenFlagSet(fSIDEVIEW) || ON_SIDEPLATFORM() || !canfall(id)))
        {
            x+=dx;
            y+=dy;
        }
    }

    void move(fix s)
    {
        if(!watch && (!screenFlagSet(fSIDEVIEW) || ON_SIDEPLATFORM() || !canfall(id)))
            sprite::move(s);
    }
    
    void removearmos(int ax,int ay)
    {
        if(did_armos)
        {
            return;
        }
        
        did_armos=true;
        ax&=0xF0;
        ay&=0xF0;
        int cd = (ax>>4)+ay;
        int f = MAPFLAG(ax,ay);
        int f2 = MAPCOMBOFLAG(ax,ay);
        mapscr@ tmpscr=getTmpscr(0);
        
        if(getComboProp(tmpscr.data[cd], COMBOPROP_TYPE)!=cARMOS)
        {
            return;
        }
        
        tmpscr.data[cd] = tmpscr.undercombo;
        tmpscr.cset[cd] = tmpscr.undercset;
        tmpscr.sflag[cd] = 0;
        
        if(f == mfARMOS_SECRET || f2 == mfARMOS_SECRET)
        {
            tmpscr.data[cd] = tmpscr.secretcombo[sSTAIRS];
            tmpscr.cset[cd] = tmpscr.secretcset[sSTAIRS];
            tmpscr.sflag[cd]=tmpscr.secretflag[sSTAIRS];
            sfx(tmpscr.secretsfx);
        }
        
        if(f == mfARMOS_ITEM || f2 == mfARMOS_ITEM)
        {
            if(!getmapflag())
            {
                additem(ax,ay,tmpscr.catchall, (ipONETIME2 + ipBIGRANGE) | (screenFlagSet(fHOLDITEM) ? ipHOLDUP : 0));
                sfx(tmpscr.secretsfx);
            }
        }
        
        // WEIRDNESS
        // This doesn't appear to do anything useful.
        //putcombo(scrollbuf,ax,ay,tmpscr->data[cd],tmpscr->cset[cd]);
    }
    
    void leave_item()
    {
        realEnemy.leave_item();
    }
    
    // returns true if Link is within 'range' pixels of the enemy
    bool LinkInRange(int range)
    {
        int lx = Link.x;
        int ly = Link.y;
        return abs(lx-int(x))<=range && abs(ly-int(y))<=range;
    }
    
    // place the enemy in line with Link (red wizzrobes)
    void place_on_axis(bool floater, bool solid_ok)
    {
        int lx=zc_min(zc_max(int(Link.x)&0xF0,32),208);
        int ly=zc_min(zc_max(int(Link.x)&0xF0,32),128);
        int pos2=rand()%23;
        int tried=0;
        bool last_resort,placed=false;
        
        do
        {
            if(pos2<14)
            {
                x=(pos2<<4)+16;
                y=ly;
            }
            else
            {
                x=lx;
                y=((pos2-14)<<4)+16;
            }
            
            // Don't commit to a last resort if position is out of bounds.
            last_resort= !(x<32 || y<32 || x>=224 || y>=144);
            
            if(abs(lx-int(x))>16 || abs(ly-int(y))>16)
            {
                // Red Wizzrobes should be able to appear on water, but not other
                // solid combos; however, they could appear on solid combos in 2.10,
                // and some quests depend on that.
                if((solid_ok || !m_walkflag(x,y,floater ? spw_water : spw_door))
                        && !flyerblocked(x,y,floater ? spw_floater : spw_door))
                    placed=true;
            }
            
            if(!placed && tried>=22 && last_resort)
            {
                placed=true;
            }
            
            ++tried;
            pos2=(pos2+3)%23;
        }
        while(!placed);
        
        if(y==ly)
            dir=(x<lx)?right:left;
        else
            dir=(y<ly)?down:up;
            
        clk2=tried;
    }
    
    void n_frame_n_dir(int frames, int ndir, int f4)
    {
        int t = o_tile;
        
        // Darknuts, but also Wizzrobes and Wallmasters
        switch(family)
        {
        case eeWALK:
            if(dmisc9==e9tPOLSVOICE && clk2>=0)
            {
                tile=s_tile;
                t=s_tile;
            }
            
            break;
            
        case eeTRAP:
            if(dummy_int1!=0 && (getGuyProp(id, GUYPROP_FLAGS2) & eneflag_trp2)!=0)  // Just to make sure
            {
                tile=s_tile;
                t=s_tile;
            }
            
            break;
            
        case eeSPINTILE:
            if(misc>=96)
            {
                tile=o_tile+frames*ndir;
                t=tile;
            }
            
            break;
        }
        
        if(ndir!=0) switch(frames)
            {
            case 2:
                tiledir_small(dir,ndir==4);
                break;
                
            case 3:
                tiledir_three(dir);
                break;
                
            case 4:
                tiledir(dir,ndir==4);
                break;
            }
            
        if(family==eeWALK)
            tile=zc_min(tile+f4, t+frames*(zc_max(dir, 0)+1)-1);
        else
            tile+=f4;
    }

    void tiledir_three(int ndir)
    {
        flip=0;
        
        switch(ndir)
        {
        case right:
            tile+=3; // fallthrough
            
        case left:
            tile+=3;  // fallthrough
            
        case down:
            tile+=3;  // fallthrough
            
        case up:
            break;
        }
    }

    void tiledir_small(int ndir, bool fourdir)
    {
        flip=0;
        
        switch(ndir)
        {
        case 8:
        case up:
            break;
            
        case 12:
        case down:
            tile+=2;
            break;
            
        case 14:
        case left:
            tile+=4;
            break;
            
        case 10:
        case right:
            tile+=6;
            break;
            
        case 9:
        case r_up:
            if(fourdir)
                break;
                
            tile+=10;
            break;
            
        case 11:
        case r_down:
            if(fourdir)
                tile+=2;
            else
                tile+=14;
                
            break;
            
        case 13:
        case l_down:
            if(fourdir)
                tile+=2;
            else
                tile+=12;
                
            break;
            
        case 15:
        case l_up:
            if(fourdir)
                break;
                
            tile+=8;
            break;
            
        default:
            break;
        }
    }

    void tiledir(int ndir, bool fourdir)
    {
        flip=0;
        
        switch(ndir)
        {
        case 8:
        case up:
            break;
            
        case 12:
        case down:
            tile+=4;
            break;
            
        case 14:
        case left:
            tile+=8;
            break;
            
        case 10:
        case right:
            tile+=12;
            break;
            
        case 9:
        case r_up:
            if(fourdir)
                break;
            else
                tile+=24;
                
            break;
            
        case 11:
        case r_down:
            if(fourdir)
                tile+=4;
            else
                tile+=32;
                
            break;
            
        case 13:
        case l_down:
            if(fourdir)
                tile+=4;
            else
                tile+=28;
                
            break;
            
        case 15:
        case l_up:
            if(fourdir)
                break;
            else
                tile+=20;
                
            break;
            
        default:
            break;
        }
    }

    void tiledir_big(int ndir, bool fourdir)
    {
        flip=0;
        
        switch(ndir)
        {
        case 8:
        case up:
            break;
            
        case 12:
        case down:
            tile+=8;
            break;
            
        case 14:
        case left:
            tile+=40;
            break;
            
        case 10:
        case right:
            tile+=48;
            break;
            
        case 9:
        case r_up:
            if(fourdir)
                break;
                
            tile+=88;
            break;
            
        case 11:
        case r_down:
            if(fourdir)
                tile+=8;
            else
                tile+=128;
                
            break;
            
        case 13:
        case l_down:
            if(fourdir)
                tile+=8;
            else
                tile+=120;
                
            break;
            
        case 15:
        case l_up:
            if(fourdir)
                break;
                
            tile+=80;
            break;
            
        default:
            break;
        }
    }
    
    void update_enemy_frame()
    {
        int newfrate = zc_max(frate,4);
        int f4=clk/(newfrate/4); // casts clk to [0,1,2,3]
        int f2=clk/(newfrate/2); // casts clk to [0,1]
        int fx = questRuleEnabled(qr_NEWENEMYTILES) ? f4 : f2;
        tile = o_tile;
        int tilerows = 1; // How many rows of tiles? The Extend code needs to know.
        
        switch(anim)
        {
        case aDONGO:
        {
            int fr = stunclk>0 ? 16 : 8;
            
            if(!dying && clk2>0 && clk2<=64)
            {
                // bloated
                switch(dir)
                {
                case up:
                    tile+=9;
                    flip=0;
                    xofs=0;
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case down:
                    tile+=7;
                    flip=0;
                    xofs=0;
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case left:
                    flip=1;
                    tile+=4;
                    xofs=16;
                    dummy_int1=1; //second tile is next tile
                    break;
                    
                case right:
                    flip=0;
                    tile+=5;
                    xofs=16;
                    dummy_int1=-1; //second tile is previous tile
                    break;
                }
            }
            else if(!dying || clk2>19)
            {
                // normal
                switch(dir)
                {
                case up:
                    tile+=8;
                    flip=(clk&fr)!=0?1:0;
                    xofs=0;
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case down:
                    tile+=6;
                    flip=(clk&fr)!=0?1:0;
                    xofs=0;
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case left:
                    flip=1;
                    tile+=(clk&fr)!=0?2:0;
                    xofs=16;
                    dummy_int1=1; //second tile is next tile
                    break;
                    
                case right:
                    flip=0;
                    tile+=(clk&fr)!=0?3:1;
                    xofs=16;
                    dummy_int1=-1; //second tile is next tile
                    break;
                }
            }
        }
        break;
        
        case aNEWDONGO:
        {
            int fr4=0;
            
            if(!dying && clk2>0 && clk2<=64)
            {
                // bloated
                if(clk2>=0)
                {
                    fr4=3;
                }
                
                if(clk2>=16)
                {
                    fr4=2;
                }
                
                if(clk2>=32)
                {
                    fr4=1;
                }
                
                if(clk2>=48)
                {
                    fr4=0;
                }
                
                switch(dir)
                {
                case up:
                    xofs=0;
                    tile+=8+fr4;
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case down:
                    xofs=0;
                    tile+=12+fr4;
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case left:
                    tile+=29+(2*fr4);
                    xofs=16;
                    dummy_int1=-1; //second tile is previous tile
                    break;
                    
                case right:
                    tile+=49+(2*fr4);
                    xofs=16;
                    dummy_int1=-1; //second tile is previous tile
                    break;
                }
            }
            else if(!dying || clk2>19)
            {
                // normal
                switch(dir)
                {
                case up:
                    xofs=0;
                    tile+=((clk&12)>>2);
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case down:
                    xofs=0;
                    tile+=4+((clk&12)>>2);
                    dummy_int1=0; //no additional tiles
                    break;
                    
                case left:
                    tile+=21+((clk&12)>>1);
                    xofs=16;
                    dummy_int1=-1; //second tile is previous tile
                    break;
                    
                case right:
                    flip=0;
                    tile+=41+((clk&12)>>1);
                    xofs=16;
                    dummy_int1=-1; //second tile is previous tile
                    break;
                }
            }
        }
        break;
        
        case aDONGOBS:
        {
            int fr4=0;
            
            if(!dying && clk2>0 && clk2<=64)
            {
                // bloated
                if(clk2>=0)
                {
                    fr4=3;
                }
                
                if(clk2>=16)
                {
                    fr4=2;
                }
                
                if(clk2>=32)
                {
                    fr4=1;
                }
                
                if(clk2>=48)
                {
                    fr4=0;
                }
                
                switch(dir)
                {
                case up:
                    tile+=28+fr4;
                    yofs+=8;
                    dummy_int1=-20; //second tile change
                    dummy_int2=0;   //new xofs change
                    dummy_int3=-16; //new xofs change
                    break;
                    
                case down:
                    tile+=12+fr4;
                    yofs-=8;
                    dummy_int1=20; //second tile change
                    dummy_int2=0;  //new xofs change
                    dummy_int3=16; //new xofs change
                    break;
                    
                case left:
                    tile+=49+(2*fr4);
                    xofs+=8;
                    dummy_int1=-1; //second tile change
                    dummy_int2=-16; //new xofs change
                    dummy_int3=0;  //new xofs change
                    break;
                    
                case right:
                    tile+=69+(2*fr4);
                    xofs+=8;
                    dummy_int1=-1; //second tile change
                    dummy_int2=-16; //new xofs change
                    dummy_int3=0;  //new xofs change
                    break;
                }
            }
            else if(!dying || clk2>19)
            {
                // normal
                switch(dir)
                {
                case up:
                    tile+=20+((clk&24)>>3);
                    yofs+=8;
                    dummy_int1=-20; //second tile change
                    dummy_int2=0;   //new xofs change
                    dummy_int3=-16; //new xofs change
                    break;
                    
                case down:
                    tile+=4+((clk&24)>>3);
                    yofs-=8;
                    dummy_int1=20; //second tile change
                    dummy_int2=0;  //new xofs change
                    dummy_int3=16; //new xofs change
                    break;
                    
                case left:
                    xofs=-8;
                    tile+=40+((clk&24)>>2);
                    dummy_int1=1; //second tile change
                    dummy_int2=16; //new xofs change
                    dummy_int3=0; //new xofs change
                    break;
                    
                case right:
                    tile+=60+((clk&24)>>2);
                    xofs=-8;
                    dummy_int1=1; //second tile change
                    dummy_int2=16; //new xofs change
                    dummy_int3=0; //new xofs change
                    break;
                }
            }
        }
        break;
        
        case aWIZZ:
        {
            if(dmisc1!=0)
            {
                if((clk&8)!=0)
                {
                    tile+=1;
                }
            }
            else
            {
                if((frame&4)!=0)
                {
                    tile+=1;
                }
            }
            
            switch(dir)
            {
            case 9:
            case 15:
            case up:
                tile+=2;
                break;
                
            case down:
                break;
                
            case 13:
            case left:
                flip=1;
                break;
                
            default:
                flip=0;
                break;
            }
        }
        break;
        
        case aNEWWIZZ:
        {
            tiledir(dir,true);
            
            if(dmisc1!=0)                                            //walking wizzrobe
            {
                if((clk&8)!=0)
                {
                    tile+=2;
                }
                
                if((clk&4)!=0)
                {
                    tile+=1;
                }
                
                if(!(dummy_bool1||dummy_bool2))                               //should never be charging or firing for these wizzrobes
                {
                    if(dummy_int1>0)
                    {
                        tile+=40;
                    }
                }
            }
            else
            {
                if(dummy_bool1||dummy_bool2)
                {
                    tile+=20;
                    
                    if(dummy_bool2)
                    {
                        tile+=20;
                    }
                }
                
                tile+=((frame>>1)&3);
            }
        }
        break;
        
        case a3FRM:
        {
            n_frame_n_dir(3, 0, (f4==3) ? 1 : f4);
        }
        break;
        
        case a3FRM4DIR:
        {
            n_frame_n_dir(3, 4, (f4==3) ? 1 : f4);
        }
        break;
        
        case aVIRE:
        {
            if(dir==up)
            {
                tile+=2;
            }
            
            tile+=fx;
        }
        break;
        
        case aROPE:
        {
            tile+=(1-fx);
            flip = dir==left ? 1:0;
        }
        break;
        
        case aZORA:
        {
            int dl;
            
            if(clk<36)
            {
                dl=clk+5;
                tile=((dl/11)&1)+s_tile;
                break;
            }
            
            if(clk<36+66)
                tile=(dir==up)?o_tile+1:o_tile;
            else
            {
                dl=clk-36-66;
                tile=((dl/11)&1)+s_tile;
            }
        }
        break;
        
        case aNEWZORA:
        {
            f4=(clk/16)%4;
            
            tiledir(dir,true);
            int dl;
            
            if((clk>35)&&(clk<36+67))                               //surfaced
            {
                if((clk>=(35+10))&&(clk<(38+56)))                     //mouth open
                {
                    tile+=80;
                }                                                     //mouth closed
                else
                {
                    tile+=40;
                }
                
                tile+=f4;
            }
            else
            {
                if(clk<36)
                {
                    dl=clk+5;
                }
                else
                {
                    dl=clk-36-66;
                }
                
                tile+=((dl/5)&3);
            }
        }
        break;
        
        case a4FRM4EYE:
        case a2FRM4EYE:
        case a4FRM8EYE:
        {
            tilerows = 2;
            double ddir=atan2(y-Link.y, Link.x-x);
            int lookat=rand()&15;
            
            if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
            {
                lookat=l_down;
            }
            else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
            {
                lookat=down;
            }
            else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
            {
                lookat=r_down;
            }
            else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
            {
                lookat=right;
            }
            else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
            {
                lookat=r_up;
            }
            else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
            {
                lookat=up;
            }
            else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
            {
                lookat=l_up;
            }
            else
            {
                lookat=left;
            }
            
            int dir2 = dir;
            dir = lookat;
            n_frame_n_dir(anim==a2FRM4EYE ? 2:4, anim==a4FRM8EYE ? 8 : 4, anim==a2FRM4EYE ? (f2&1):f4);
            dir = dir2;
        }
        break;
        
        case aFLIP:
        {
            flip = f2&1;
        }
        break;
        
        case a2FRM:
        {
            tile += (1-f2);
        }
        break;
        
        case a2FRMB:
        {
            tile+= 2*(1-f2);
        }
        break;
        
        case a2FRM4DIR:
        {
            n_frame_n_dir(2, 4, f2&1);
        }
        break;
        
        case a4FRM4DIRF:
        {
            n_frame_n_dir(4,4,f4);
            
            if(clk2>0)                                              //stopped to fire
            {
                tile+=20;
                
                if(clk2<17)                                           //firing
                {
                    tile+=20;
                }
            }
        }
        break;
        
        case a4FRM4DIR:
        {
            n_frame_n_dir(4,4,f4);
        }
        break;
        
        case a4FRM8DIRF:
        {
            tilerows = 2;
            n_frame_n_dir(4,8,f4);
            
            if(clk2>0)                                              //stopped to fire
            {
                tile+=40;
                
                if(clk2<17)                                           //firing
                {
                    tile+=40;
                }
            }
        }
        break;
        
        case a4FRM8DIRB:
        {
            tilerows = 2;
            tiledir_big(dir,false);
            tile+=2*f4;
        }
        break;
        
        case aOCTO:
        {
            switch(dir)
            {
            case up:
                flip = 2;
                break;
                
            case down:
                flip = 0;
                break;
                
            case left:
                flip = 0;
                tile += 2;
                break;
                
            case right:
                flip = 1;
                tile += 2;
                break;
            }
            
            tile+=f2;
        }
        break;
        
        case aWALK:
        {
            switch(dir)
            {
            case up:
                tile+=3;
                flip = f2;
                break;
                
            case down:
                tile+=2;
                flip = f2;
                break;
                
            case left:
                flip=1;
                tile += f2;
                break;
                
            case right:
                flip=0;
                tile += f2;
                break;
            }
        }
        break;
        
        case aDWALK:
        {
            if(questRuleEnabled(qr_BRKNSHLDTILES) && dummy_bool1)
            {
                tile=s_tile;
            }
            
            switch(dir)
            {
            case up:
                tile+=2;
                flip=f2;
                break;
                
            case down:
                flip=0;
                tile+=(1-f2);
                break;
                
            case left:
                flip=1;
                tile+=(3+f2);
                break;
                
            case right:
                flip=0;
                tile+=(3+f2);
                break;
            }
        }
        break;
        
        case aTEK:
        {
            if(misc==0)
            {
                tile += f2;
            }
            else if(misc!=1)
            {
                tile+=1;
            }
        }
        break;
        
        case aNEWTEK:
        {
            if(step<0)                                              //up
            {
                switch(clk3)
                {
                case left:
                    flip=0;
                    tile+=20;
                    break;
                    
                case right:
                    flip=0;
                    tile+=24;
                    break;
                }
            }
            else if(step==0)
            {
                switch(clk3)
                {
                case left:
                    flip=0;
                    tile+=8;
                    break;
                    
                case right:
                    flip=0;
                    tile+=12;
                    break;
                }
            }                                                       //down
            else
            {
                switch(clk3)
                {
                case left:
                    flip=0;
                    tile+=28;
                    break;
                    
                case right:
                    flip=0;
                    tile+=32;
                    break;
                }
            }
            
            if(misc==0)
            {
                tile+=f4;
            }
            else if(misc!=1)
            {
                tile+=2;
            }
        }
        break;
        
        case aARMOS:
        {
            if(fading==0)
            {
                tile += fx;
                
                if(dir==up)
                    tile += 2;
            }
        }
        break;
        
        case aARMOS4:
        {
            switch(dir)
            {
            case up:
                flip=0;
                break;
                
            case down:
                flip=0;
                tile+=4;
                break;
                
            case left:
                flip=0;
                tile+=8;
                break;
                
            case right:
                flip=0;
                tile+=12;
                break;
            }
            
            if(fading==0)
            {
                tile+=f4;
            }
        }
        break;
        
        case aGHINI:
        {
            switch(dir)
            {
            case 8:
            case 9:
            case up:
                tile+=1;
                flip=0;
                break;
                
            case 15:
                tile+=1;
                flip=1;
                break;
                
            case 10:
            case 11:
            case right:
                flip=1;
                break;
                
            default:
                flip=0;
                break;
            }
        }
        break;
        
        case a2FRMPOS:
        {
            tile+=posframe;
        }
        break;
        
        case a4FRMPOS4DIR:
        {
            n_frame_n_dir(4,4,0);
            tile+=posframe;
        }
        break;
        
        case a4FRMPOS4DIRF:
        {
            n_frame_n_dir(4,4,0);
            
            if(clk2>0)                                              //stopped to fire
            {
                tile+=20;
                
                if(clk2<17)                                           //firing
                {
                    tile+=20;
                }
            }
            
            tile+=posframe;
        }
        break;
        
        case a4FRMPOS8DIR:
        {
            tilerows = 2;
            n_frame_n_dir(4,8,0);
            tile+=posframe;
        }
        break;
        
        case a4FRMPOS8DIRF:
        {
            tilerows = 2;
            n_frame_n_dir(4,8,f4);
            
            if(clk2>0)                                              //stopped to fire
            {
                tile+=40;
                
                if(clk2<17)                                           //firing
                {
                    tile+=40;
                }
            }
            
            tile+=posframe;
        }
        break;
        
        case aNEWLEV:
        {
            tiledir(dir,true);
            
            switch(misc)
            {
            case -1:
            case 0:
                return;
                
            case 1:
            
            case 5:
                cs = dmisc2;
                break;
                
            case 2:
            case 4:
                tile += 20;
                break;
                
            case 3:
                tile += 40;
                break;
            }
            
            tile+=f4;
        }
        break;
        
        case aLEV:
        {
            f4 = ((clk/5)&1);
            
            switch(misc)
            {
            case -1:
            case 0:
                return;
                
            case 1:
            
            case 5:
                tile += (f2!=0) ? 1 : 0;
                cs = dmisc2;
                break;
                
            case 2:
            case 4:
                tile += 2;
                break;
                
            case 3:
                tile += (f4!=0) ? 4 : 3;
                break;
            }
        }
        break;
        
        case aWALLM:
        {
            if(!dummy_bool1)
            {
                tile += f2;
            }
        }
        break;
        
        case aNEWWALLM:
        {
            int tempdir=0;
            
            switch(misc)
            {
            case 1:
            case 2:
                tempdir=clk3;
                break;
                
            case 3:
            case 4:
            case 5:
                tempdir=dir;
                break;
                
            case 6:
            case 7:
                tempdir=clk3^1;
                break;
            }
            
            tiledir(tempdir,true);
            
            if(!dummy_bool1)
            {
                tile+=f4;
            }
        }
        break;
        
        case a4FRMNODIR:
        {
            tile+=f4;
        }
        break;
        
        }                                                         // switch(d->anim)
        
        // flashing
        if((flags2 & guy_flashing)!=0)
        {
            cs = (frame&3) + 6;
        }
        
        if((flags2&guy_transparent)!=0)
        {
            drawstyle=1;
        }
        
        int change = tile-o_tile;
        
        if(extend > 2)
        {
            if(o_tile/TILES_PER_ROW==(o_tile+((txsz*change)/tilerows))/TILES_PER_ROW)
            {
                tile=o_tile+txsz*change;
            }
            else
            {
                tile=o_tile+(txsz*change)+((tysz-1)*TILES_PER_ROW)*((o_tile+txsz*change)/TILES_PER_ROW)-(o_tile/TILES_PER_ROW);
            }
        }
        else
        {
            tile=o_tile+change;
        }
    }

    void fireWeapon()
    {
        realEnemy.fireWeapon();
    }
    
    bool isFiring() const
    {
        return realEnemy.isFiring();
    }
    
    void updateFiring()
    {
        realEnemy.updateFiring();
    }
    
    void setBreathTimer(int time)
    {
        realEnemy.setBreathTimer(time);
    }
    
    void setAttackOwner(int index)
    {
        realEnemy.setAttackOwner(index);
    }
    
    void activateDeathAttack()
    {
        realEnemy.activateDeathAttack();
    }
    
    int defenditemclass(int wpnId, int power)
    {
        return realEnemy.defenditemclass(wpnId, power);
    }
    
    int getDefendedItemPower()
    {
        return realEnemy.getDefendedItemPower();
    }
    
    bool dont_draw()
    {
        if(fading==fade_invisible || (((flags2&guy_blinking)!=0||(fading==fade_flicker)) && (clk&1)!=0))
            return true;
            
        if((flags&guy_invisible)!=0)
            return true;
            
        if((flags&lens_only)!=0 && lensclk==0)
            return true;
            
        return false;
    }
    void draw()
    {
        realEnemy.draw();
    }
    
    void drawshadow()
    {
        realEnemy.drawshadow();
    }
    
    void drawblock()
    {
        realEnemy.drawblock();
    }
    
    void masked_draw(int mx, int my, int mw, int mh)
    {
        realEnemy.masked_draw(mx, my, mw, mh);
    }
    
    void overtilecloaked16(int tile, int x, int y, int flip)
    {
        realEnemy.overtilecloaked16(tile, x, y, flip);
    }
    
    void overtile16(int tile, int x, int y, int cset, int flip)
    {
        realEnemy.overtile16(tile, x, y, cset, flip);
    }
    
    void stopBGSFX()
    {
        realEnemy.stopBGSFX();
    }
    
    void playDeathSFX()
    {
        realEnemy.playDeathSFX();
    }
    
    void playHitSFX(int x)
    {
        realEnemy.playHitSFX(x);
    }
    
    bool canmove(int ndir, fix s, int special, int dx1, int dy1, int dx2, int dy2)
    {
        return realEnemy.canmove(ndir, s, special, dx1, dy1, dx2, dy2);
    }
    
    bool canmove(int ndir,fix s,int special)
    {
        return realEnemy.canmove(ndir, s, special, 0, -8, 15, 15);
    }
    
    bool canmove(int ndir,int special)
    {
        bool dodongo_move=true; //yes, it's an ugly hack, but we're going to rewrite everything later anyway - DN
        
        if(special==spw_clipright && ndir==right)
            dodongo_move=realEnemy.canmove(ndir, 1, special, 0, -8, 31, 15);
        
        return dodongo_move && realEnemy.canmove(ndir, 1, special, 0, -8, 15, 15);
    }
    
    bool canmove(int ndir)
    {
        return realEnemy.canmove(ndir, 1, spw_none, 0, -8, 15, 15);
    }
    
    // makes the enemy slide backwards when hit
    // sclk: first byte is clk, second byte is dir
    bool slide()
    {
        if(sclk==0 || hp<=0)
            return false;
        
        if((sclk&255)==16 && !canmove(sclk>>8, 12, 0))
        {
            sclk=0;
            return false;
        }
        
        sclk-=1;
        
        switch(sclk>>8)
        {
        case up:
            if(y<=16)
            {
                sclk=0;
                return false;
            }
            
            break;
            
        case down:
            if(y>=160)
            {
                sclk=0;
                return false;
            }
            
            break;
            
        case left:
            if(x<=16)
            {
                sclk=0;
                return false;
            }
            
            break;
            
        case right:
            if(x>=240)
            {
                sclk=0;
                return false;
            }
            
            break;
        }
        
        switch(sclk>>8)
        {
        case up:
            y-=4;
            break;
            
        case down:
            y+=4;
            break;
            
        case left:
            x-=4;
            break;
            
        case right:
            x+=4;
            break;
        }
        
        if(!canmove(sclk>>8, 0, 0))
        {
            switch(sclk>>8)
            {
            case up:
            case down:
                if((int(y)&15) > 7)
                    y=(int(y)&0xF0)+16;
                else
                    y=(int(y)&0xF0);
                    
                break;
                
            case left:
            case right:
                if((int(x)&15) > 7)
                    x=(int(x)&0xF0)+16;
                else
                    x=(int(x)&0xF0);
                    
                break;
            }
            
            sclk=0;
        }
        
        if((sclk&255)==0)
            sclk=0;
            
        return true;
    }
    
    // changes enemy's direction, checking restrictions
    // rate:   0 = no random changes, 16 = always random change
    // homing: 0 = none, 256 = always
    // grumble 0 = none, 4 = strongest appetite
    void newdir(int newrate, int newhoming, int special)
    {
        int ndir=-1;
        
        if(grumble!=0 && uint8(rand()&3)<grumble)
        {
            int w = idOfFirstLwpnOfType(wBait);
            
            if(w>=0)
            {
                weapon@ bait=getLwpn(w);
                int bx = bait.x;
                int by = bait.y;
                
                if(abs(int(y)-by)>14)
                {
                    ndir = (by<y) ? up : down;
                    
                    if(canmove(ndir,special))
                    {
                        dir=ndir;
                        return;
                    }
                }
                
                ndir = (bx<x) ? left : right;
                
                if(canmove(ndir,special))
                {
                    dir=ndir;
                    return;
                }
            }
        }
        
        if((rand()&255)<newhoming)
        {
            ndir = lined_up(8,false);
            
            if(ndir>=0 && canmove(ndir,special))
            {
                dir=ndir;
                return;
            }
        }
        
        int i=0;
        
        for(; i<32; i++)
        {
            int r=rand();
            
            if((r&15)<newrate)
                ndir=(r>>4)&3;
            else
                ndir=dir;
                
            if(canmove(ndir,special))
                break;
        }
        
        if(i==32)
        {
            for(ndir=0; ndir<4; ndir++)
            {
                if(canmove(ndir,special))
                {
                    dir=ndir;
                    return;
                }
            }
            
            ndir = screenFlagSet(fSIDEVIEW) ? ((rand()&1)!=0 ? left : right) : -1; // Sideview enemies get trapped if their dir becomes -1
        }
        
        dir=ndir;
    }
    
    void newdir()
    {
        newdir(4, 0, spw_none);
    }
    
    void fix_coords(bool bound=false)
    {
        if(bound)
        {
            x=vbound(x, 0, 240);
            y=vbound(y, 0, 160);
        }
        
        if(!OUTOFBOUNDS())
        {
            x=(int(x)&0xF0)+((int(x)&8)!=0?16:0);
            
            if(screenFlagSet(fSIDEVIEW))
                y=(int(y)&0xF8)+((int(y)&4)!=0?8:0);
            else
                y=(int(y)&0xF0)+((int(y)&8)!=0?16:0);
        }
    }
    
    /*
    // 8-directional
    void newdir_8(int newrate, int newhoming, int special, int dx1, int dy1, int dx2, int dy2)
    {
        realEnemy.newdir_8(newrate, newhoming, special, dx1, dy1, dx2, dy2);
    }
    */
    
    // 8-directional
    void newdir_8(int newrate, int newhoming, int special, int dx1, int dy1, int dx2, int dy2)
    {
        int ndir=0;
        
        // can move straight, check if it wants to turn
        if(canmove(dir,step,special,dx1,dy1,dx2,dy2))
        {
            if(grumble!=0 && uint8(rand()&4)<grumble) //Homing
            {
                int w = idOfFirstLwpnOfType(wBait);
                
                if(w>=0)
                {
                    weapon@ bait=getLwpn(w);
                    int bx = bait.x;
                    int by = bait.y;
                    
                    ndir = (bx<x) ? left : (bx!=x) ? right : 0;
                    
                    if(abs(int(y)-by)>14)
                    {
                        if(ndir>0)  // Already left or right
                        {
                            // Making the diagonal directions
                            ndir += (by<y) ? 2 : 4;
                        }
                        else
                        {
                            ndir = (by<y) ? up : down;
                        }
                    }
                    
                    if(canmove(ndir,special))
                    {
                        dir=ndir;
                        return;
                    }
                }
            }
            
            // Homing added.
            if(newhoming!=0 && (rand()&255)<newhoming)
            {
                ndir = lined_up(8,true);
                
                if(ndir>=0 && canmove(ndir,special))
                {
                    dir=ndir;
                }
                
                return;
            }
            
            int r=rand();
            
            if(newrate>0 && (r%newrate)==0)
            {
                ndir = ((dir+(((r&64)!=0)?-1:1))&7)+8;
                int ndir2=((dir+(((r&64)!=0)?1:-1))&7)+8;
                
                if(canmove(ndir,step,special,dx1,dy1,dx2,dy2))
                    dir=ndir;
                else if(canmove(ndir2,step,special,dx1,dy1,dx2,dy2))
                    dir=ndir2;
                    
                if(dir==ndir && (newrate>=4)) // newrate>=4, otherwise movement is biased toward upper-left
                    // due to numerous lost fractional components. -L
                {
                    x.vAND(0xFFFF0000);
                    y.vAND(0xFFFF0000);
                }
            }
            
            return;
        }
        
        // can't move straight, must turn
        int i=0;
        
        for(; i<32; i++)  // Try random dir
        {
            ndir=(rand()&7)+8;
            
            if(canmove(ndir,step,special,dx1,dy1,dx2,dy2))
                break;
        }
        
        if(i==32)
        {
            for(ndir=8; ndir<16; ndir++)
            {
                if(canmove(ndir,step,special,dx1,dy1,dx2,dy2))
                {
                    dir=ndir;
                    x.vAND(0xFFFF0000);
                    y.vAND(0xFFFF0000);
                    return;
                }
            }
            
            ndir = screenFlagSet(fSIDEVIEW) ? (rand()&1)!=0 ? left : right : -1;  // Sideview enemies get trapped if their dir becomes -1
        }
       
        dir=ndir;
        x.vAND(0xFFFF0000);
        y.vAND(0xFFFF0000);
    }
    
    void newdir_8(int newrate, int newhoming, int special)
    {
        newdir_8(newrate, newhoming, special, 0, -8, 15, 15);
    }
    
    // keeps walking around
    void constant_walk(int newrate, int newhoming, int special)
    {
        if(slide())
            return;
            
        if(clk<0 || dying || stunclk!=0 || watch || ceiling)
            return;
            
        if(clk3<=0)
        {
            fix_coords(true);
            newdir(newrate, newhoming, special);
            
            if(step==0)
                clk3=0;
            else
                clk3=int(16.0/step);
        }
        else if(scored)
        {
            dir^=1;
            clk3=int(16.0/step)-clk3;
        }
        
        clk3-=1;
        move(step);
    }

    void constant_walk()
    {
        constant_walk(4, 0, spw_none);
    }
    
    int pos(int newx, int newy)
    {
        return (newy<<8)+newx;
    }
    
    // for variable step rates
    void variable_walk(int newrate, int newhoming, int special)
    {
        if(slide())
            return;
            
        if(clk<0 || dying || stunclk!=0 || watch || step==0 || ceiling)
            return;
            
        fix dx = 0;
        fix dy = 0;
        
        switch(dir)
        {
        case 8:
        case up:
            dy-=step;
            break;
            
        case 12:
        case down:
            dy+=step;
            break;
            
        case 14:
        case left:
            dx-=step;
            break;
            
        case 10:
        case right:
            dx+=step;
            break;
            
        case 15:
        case l_up:
            dx-=step;
            dy-=step;
            break;
            
        case 9:
        case r_up:
            dx+=step;
            dy-=step;
            break;
            
        case 13:
        case l_down:
            dx-=step;
            dy+=step;
            break;
            
        case 11:
        case r_down:
            dx+=step;
            dy+=step;
            break;
        }
        
        if(((int(x)&15)==0 && (int(y)&15)==0 && clk3!=pos(x,y)) ||
                m_walkflag(int(x+dx),int(y+dy), spw_halfstep))
        {
            fix_coords();
            newdir(newrate,newhoming,special);
            clk3=pos(x,y);
        }
        
        move(step);
    }
    
    // pauses for a while after it makes a complete move (to a new square)
    void halting_walk(int newrate, int newhoming, int special, int newhrate, int haltcnt)
    {
        if(sclk!=0 && clk2!=0)
        {
            clk3=0;
        }
        
        if(slide() || clk<0 || dying || stunclk!=0 || watch || ceiling)
        {
            return;
        }
        
        if(clk2>0)
        {
            clk2-=1;
            return;
        }
        
        if(clk3<=0)
        {
            fix_coords(true);
            newdir(newrate,newhoming,special);
            clk3=int(16.0/step);
            
            if(clk2<0)
            {
                clk2=0;
            }
            else if((rand()&15)<newhrate)
            {
                clk2=haltcnt;
                return;
            }
        }
        else if(scored)
        {
            dir^=1;
            
            clk3=int(16.0/step-clk3);
        }
        
        clk3-=1;
        move(step);
    }
    
    // 8-directional movement, aligns to 8 pixels
    void constant_walk_8(int newrate, int newhoming, int special)
    {
        if(clk<0 || dying || stunclk!=0 || watch || ceiling)
            return;
            
        if(clk3<=0)
        {
            newdir_8(newrate, newhoming, special);
            clk3=int(8.0/step);
        }
        
        clk3-=1;
        move(step);
    }
    
    // 8-directional movement, no alignment
    void variable_walk_8(int newrate, int newhoming, int newclk, int special)
    {
        if(clk<0 || dying || stunclk!=0 || watch || ceiling)
            return;
            
        if(!canmove(dir,step,special))
            clk3=0;
            
        if(clk3<=0)
        {
            newdir_8(newrate,newhoming,special);
            clk3=newclk;
        }
        
        clk3-=1;
        move(step);
    }

    // same as above but with variable enemy size
    void variable_walk_8(int newrate, int newhoming, int newclk, int special, int dx1, int dy1, int dx2, int dy2)
    {
        if(clk<0 || dying || stunclk!=0 || watch || ceiling)
            return;
            
        if(!canmove(dir, step, special, dx1, dy1, dx2, dy2))
            clk3=0;
            
        if(clk3<=0)
        {
            newdir_8(newrate, newhoming, special, dx1, dy1, dx2, dy2);
            clk3=newclk;
        }
        
        clk3-=1;
        move(step);
    }
    
    // the variable speed floater movement
    // ms is max speed
    // ss is step speed
    // s is step count
    // p is pause count
    // g is graduality :)
    void floater_walk(int newrate, int newclk, fix ms, fix ss, int s, int p, int g)
    {
        clk2+=1;
        
        switch(movestatus)
        {
        case 0:                                                 // paused
            if(clk2>=p)
            {
                movestatus=1;
                clk2=0;
            }
            
            break;
            
        case 1:                                                 // speeding up
            if(clk2<g*s)
            {
                if(((clk2-1)%g)==0)
                    step+=ss;
            }
            else
            {
                movestatus=2;
                clk2=0;
            }
            
            break;
            
        case 2:                                                 // normal
            step=ms;
            
            if(clk2>48 && (rand()%768)==0)
            {
                step=ss*s;
                movestatus=3;
                clk2=0;
            }
            
            break;
            
        case 3:                                                 // slowing down
            if(clk2<=g*s)
            {
                if((clk2%g)==0)
                    step-=ss;
            }
            else
            {
                movestatus=0;
                step=0;
                clk2=0;
            }
            
            break;
        }
        
        variable_walk_8((movestatus==2)?newrate:0,homing,newclk,spw_floater);
    }

    void floater_walk(int newrate, int newclk, fix s)
    {
        floater_walk(newrate, newclk, s, 0.125, 3, 80, 32);
    }
    
    // Checks if enemy is lined up with Link. If so, returns direction Link is
    // at as compared to enemy. Returns -1 if not lined up. Range is inclusive.
    int lined_up(int range, bool dir8)
    {
        int lx = Link.x;
        int ly = Link.y;
        
        if(abs(lx-int(x))<=range)
        {
            if(ly<y)
            {
                return up;
            }
            
            return down;
        }
        
        if(abs(ly-int(y))<=range)
        {
            if(lx<x)
            {
                return left;
            }
            
            return right;
        }
        
        if(dir8)
        {
            if(abs(lx-x)-abs(ly-y)<=range)
            {
                if(ly<y)
                {
                    if(lx<x)
                    {
                        return l_up;
                    }
                    else
                    {
                        return r_up;
                    }
                }
                else
                {
                    if(lx<x)
                    {
                        return l_down;
                    }
                    else
                    {
                        return r_down;
                    }
                }
            }
        }
        
        return -1;
    }
    
    bool OUTOFBOUNDS()
    {
        return (int(y)>((screenFlagSet(fSIDEVIEW) && canfall(id))?192:352) || y<-176 || x<-256 || x > 512);
    }
} // End enemy class

