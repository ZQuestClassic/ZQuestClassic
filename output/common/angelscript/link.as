const int SWORDCHARGEFRAME = 20;
const int SWORDTAPFRAME = SWORDCHARGEFRAME-8;
const int HAMMERCHARGEFRAME = 11;
const int WANDCHARGEFRAME = 12;

class LinkClass: sprite
{
    __RealLink@ realLink;
    
    LinkClass()
    {
        @realLink=__getRealLink();
        @Link=this;
    }
    
    // Properties ==============================================================
    
    // Used in checking slashable/poundable/etc. combos to remember
    // if a combo or FFC was already hit.
    array<bool> screengrid(176);
    array<bool> ffcgrid(32);
    
    bool tapping
    {
        get const { return realLink.tapping; }
        set { realLink.tapping=value; }
    }
    
    bool stomping
    {
        get const { return realLink.stomping; }
        set { realLink.stomping=value; }
    }
    
    int ladderx
    {
        get const { return realLink.ladderx; }
        set { realLink.ladderx=value; }
    }
    
    int laddery
    {
        get const { return realLink.laddery; }
        set { realLink.laddery=value; }
    }
    
    int charging
    {
        get const { return realLink.charging; }
        set { realLink.charging=value; }
    }
    
    int spins
    {
        get const { return realLink.spins; }
        set { realLink.spins=value; }
    }
    
    int drunkclk
    {
        get const { return realLink.drunkclk; }
        set { realLink.drunkclk=value; }
    }
    
    int hoverclk
    {
        get const { return realLink.hoverclk; }
        set { realLink.hoverclk=value; }
    }
    
    int hclk
    {
        get const { return realLink.hclk; }
        set { realLink.hclk=value; }
    }
    
    int attackclk
    {
        get const { return realLink.attackclk; }
        set { realLink.attackclk=value; }
    }
    
    int attack
    {
        get const { return realLink.attack; }
        set { realLink.attack=value; }
    }
    
    int attackid
    {
        get const { return realLink.attackid; }
        set { realLink.attackid=value; }
    }
    
    int drownclk
    {
        get const { return realLink.drownclk; }
        set { realLink.drownclk=value; }
    }
    
    int slashxofs
    {
        get const { return realLink.slashxofs; }
        set { realLink.slashxofs=value; }
    }
    
    int slashyofs
    {
        get const { return realLink.slashyofs; }
        set { realLink.slashyofs=value; }
    }
    
    uint8 hopclk
    {
        get const { return realLink.hopclk; }
        set { realLink.hopclk=value; }
    }
    
    uint8 diveclk
    {
        get const { return realLink.diveclk; }
        set { realLink.diveclk=value; }
    }
    
    uint8 inlikelike
    {
        get const { return realLink.inlikelike; }
        set { realLink.inlikelike=value; }
    }
    
    int shiftdir
    {
        get const { return realLink.shiftdir; }
        set { realLink.shiftdir=value; }
    }
    
    int sdir
    {
        get const { return realLink.sdir; }
        set { realLink.sdir=value; }
    }
    
    int hopdir
    {
        get const { return realLink.hopdir; }
        set { realLink.hopdir=value; }
    }
    
    int holddir
    {
        get const { return realLink.holddir; }
        set { realLink.holddir=value; }
    }
    
    int landswim
    {
        get const { return realLink.landswim; }
        set { realLink.landswim=value; }
    }
    
    bool ilswim
    {
        get const { return realLink.ilswim; }
        set { realLink.ilswim=value; }
    }
    
    bool walkable
    {
        get const { return realLink.walkable; }
        set { realLink.walkable=value; }
    }
    
    int action
    {
        get const { return realLink.action; }
        set { realLink.action=value; }
    }
    
    fix climb_cover_x
    {
        get const { return realLink.climb_cover_x; }
        set { realLink.climb_cover_x=value; }
    }
    
    fix climb_cover_y
    {
        get const { return realLink.climb_cover_y; }
        set { realLink.climb_cover_y=value; }
    }
    
    fix falling_oldy
    {
        get const { return realLink.falling_oldy; }
        set { realLink.falling_oldy=value; }
    }
    
    bool diagonalMovement
    {
        get const { return realLink.diagonalMovement; }
        set { realLink.diagonalMovement=value; }
    }
    
    // Methods =================================================================
    
    // Temporary return values:
    // 0: Keep going
    // 1: Return true
    // 2: Return false
    int Update()
    {
        // Here we go...
        
        if(action!=climbcovertop&&action!=climbcoverbottom)
        {
            climb_cover_x=-1000;
            climb_cover_y=-1000;
        }
        
        if(stomping)
            stomping = false;
        
        return 0;
    }
    
    void hitlink(int hit2)
    {
        realLink.hitlink(hit2);
    }
    
    // Find the attack power of the current melee weapon.
    // The Whimsical Ring is applied on a target-by-target basis.
    int weaponattackpower()
    {
        int power = directWpn>-1 ?
          getItemProp(directWpn, ITEMPROP_POWER) :
          (current_item_power(attack==wWand ?
            itype_wand :
            attack==wHammer ?
              itype_hammer :
              itype_sword));
        
        // Multiply it by the power of the spin attack/quake hammer, if applicable.
        power *= (spins>0 ? getItemProp(current_item_id(attack==wHammer ? itype_quakescroll : (spins>5 || current_item_id(itype_spinscroll) < 0) ? itype_spinscroll2 : itype_spinscroll), ITEMPROP_POWER) : 1);
        return power;
    }
    
    // Must only be called once per frame!
    void positionSword(weapon@ w, int itemid)
    {
        itemid=vbound(itemid, 0, MAXITEMS-1);
        // Place a sword weapon at the right spot.
        int wy=1;
        int wx=1;
        int f=0,t,cs2;
        
        t = w.o_tile;
        cs2 = w.o_cset;
        slashxofs=0;
        slashyofs=0;
        
        switch(dir)
        {
        case up:
            wx=-1;
            wy=-12;
            
            if(get_canslash() && w.id==wSword && (getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_FLAG4)!=0 && charging==0)
            {
                if(attackclk>10) //extended stab
                {
                    slashyofs-=3;
                    wy-=2;
                }
                
                if(attackclk>=14) //retracting stab
                {
                    slashyofs+=3;
                    wy+=2;
                }
            }
            else
            {
                if(attackclk==SWORDCHARGEFRAME)
                {
                    wy+=4;
                }
                else if(attackclk==13)
                {
                    wy+=4;
                }
                else if(attackclk==14)
                {
                    wy+=8;
                }
            }
            
            break;
            
        case down:
            f=questRuleEnabled(qr_SWORDWANDFLIPFIX)?3:2;
            wy=11;
            
            if(get_canslash() && w.id==wSword && (getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_FLAG4)!=0 && charging==0)
            {
                if(attackclk>10) //extended stab
                {
                    slashyofs+=3;
                    wy+=2;
                }
                
                if(attackclk>=14) //retracting stab
                {
                    slashyofs-=3;
                    wy-=2;
                }
            }
            else
            {
                if(attackclk==SWORDCHARGEFRAME)
                {
                    wy-=2;
                }
                else if(attackclk==13)
                {
                    wy-=4;
                }
                else if(attackclk==14)
                {
                    wy-=8;
                }
            }
            
            break;
            
        case left:
            f=1;
            wx=-11;
            ++t;
            
            if(get_canslash() && w.id==wSword && (getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_FLAG4)!=0 && charging==0)
            {
                if(attackclk>10)  //extended stab
                {
                    slashxofs-=4;
                    wx-=7;
                }
                
                if(attackclk>=14) //retracting stab
                {
                    slashxofs+=3;
                    wx+=7;
                }
            }
            else
            {
                if(attackclk==SWORDCHARGEFRAME)
                {
                    wx+=2;
                }
                else if(attackclk==13)
                {
                    wx+=4;
                }
                else if(attackclk==14)
                {
                    wx+=8;
                }
            }
            
            break;
            
        case right:
            wx=11;
            ++t;
            
            if(get_canslash() && w.id==wSword && (getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_FLAG4)!=0 && charging==0)
            {
                if(attackclk>10) //extended stab
                {
                    slashxofs+=4;
                    wx+=7;
                }
                
                if(attackclk>=14) //retracting stab
                {
                    slashxofs-=3;
                    wx-=7;
                }
            }
            else
            {
                if(attackclk==SWORDCHARGEFRAME)
                {
                    wx-=2;
                }
                else if(attackclk==13)
                {
                    wx-=4;
                }
                else if(attackclk==14)
                {
                    wx-=8;
                }
            }
            
            break;
        }
        
        if(get_canslash() && (getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_FLAG4)!=0 && attackclk<11)
        {
            int wpn2=getItemProp(itemid, ITEMPROP_WPN2);
            wpn2=vbound(wpn2, 0, MAXWPNS);
            
            //slashing tiles
            switch(dir)
            {
            case up:
                wx=15;
                wy=-3;
                ++t;
                f=0;                                     //starts pointing right
                
                if(attackclk>=7)
                {
                    wy-=9;
                    wx-=3;
                    t = getWeaponProp(wpn2, WPNPROP_TILE);
                    cs2 = getWeaponProp(wpn2, WPNPROP_CSETS)&15;
                    f=0;
                }
                
                break;
                
            case down:
                wx=-13;
                wy=-1;
                ++t;
                f=1;                                     //starts pointing left
                
                if(attackclk>=7)
                {
                    wy+=15;
                    wx+=2;
                    t = getWeaponProp(wpn2, WPNPROP_TILE);
                    cs2 = getWeaponProp(wpn2, WPNPROP_CSETS)&15;
                    ++t;
                    f=0;
                }
                
                break;
                
            case left:
                wx=3;
                wy=-15;
                --t;
                f=0;                                     //starts pointing up
                
                if(attackclk>=7)
                {
                    wx-=15;
                    wy+=3;
                    slashxofs-=1;
                    t = getWeaponProp(wpn2, WPNPROP_TILE);
                    cs2 = getWeaponProp(wpn2, WPNPROP_CSETS)&15;
                    t+=2;
                    f=0;
                }
                
                break;
                
            case right:
                --t;
                
                if(spins>0 || questRuleEnabled(qr_SLASHFLIPFIX))
                {
                    wx=1;
                    wy=13;
                    f=2;
                }
                else
                {
                    wx=3;
                    wy=-15;
                    f=0;
                }
                
                if(attackclk>=7)
                {
                    wx+=15;
                    slashxofs+=1;
                    t = getWeaponProp(wpn2, WPNPROP_TILE);
                    cs2 = getWeaponProp(wpn2, WPNPROP_CSETS)&15;
                    
                    if(spins>0 || questRuleEnabled(qr_SLASHFLIPFIX))
                    {
                        wx-=1;
                        wy-=2;
                    }
                    else
                    {
                        t+=3;
                        f=0;
                        wy+=3;
                    }
                }
                
                break;
            }
        }
        
        int itemid2 = current_item_id(itype_chargering);
        
        if(charging>(itemid2>=0 ? getItemProp(itemid2, ITEMPROP_MISC1) : 64))
        {
            cs2=(BSZ ? (frame&3)+6 : ((frame>>2)&1)+7);
        }
        
        w.x = x+wx;
        w.y = y+wy-(54-(yofs+slashyofs));
        w.z = (z+zofs);
        w.tile = t;
        w.flip = f;
        w.power = weaponattackpower();
        w.dir = dir;
    }
    
    // separate case for sword/wand/hammer/slashed weapons only
    // the main weapon checking is in the global function check_collisions()
    void checkstab()
    {
        if(action!=attacking || (attack!=wSword && attack!=wWand && attack!=wHammer && attack!=wCByrna && attack!=wFire)
                || (attackclk<=4))
            return;
            
        weapon@ w;
        
        int wx=0,wy=0,wz=0,wxsz=0,wysz=0;
        bool found = false;
        
        for(int i=0; i<numLwpns(); i++)
        {
            @w = getLwpn(i);
            
            if(w.id == ((attack==wCByrna || attack==wFire) ? int(wWand) : attack))  // Kludge: Byrna and Candle sticks are wWand-type.
            {
                found = true;
                
                // Position the sword as Link slashes with it.
                if(w.id!=wHammer)
                    positionSword(w, w.parentitem);
                    
                wx=w.x;
                wy=w.y;
                wz=w.z;
                wxsz = w.hxsz;
                wysz = w.hysz;
                break;
            }
        }
        
        if(attack==wSword && attackclk>=14 && charging==0)
            return;
            
        if(!found)
            return;
            
        if(attack==wCByrna||attack==wFire)
            return;
            
        if(attack==wHammer)
        {
            if(attackclk<15)
            {
                switch(w.dir)
                {
                case up:
                    wx=x-1;
                    wy=y-4;
                    break;
                    
                case down:
                    wx=x+8;
                    wy=y+28;
                    break; // This is consistent with 2.10
                    
                case left:
                    wx=x-13;
                    wy=y+14;
                    break;
                    
                case right:
                    wx=x+21;
                    wy=y+14;
                    break;
                }
                
                if(attackclk==12 && z==0 && sideviewhammerpound())
                {
                    addDecoration(dHAMMERSMACK, wx, wy);
                }
                
                return;
            }
            else if(attackclk==15 && w.dir==up)
            {
                // Hammer's reach needs adjusted slightly for backward compatibility
                w.hyofs-=1;
            }
        }
        
        // The return of Spaghetti Code Constants!
        int itype = (attack==wWand ? itype_wand : itype_sword);
        int itemid = (directWpn>-1 && getItemProp(directWpn, ITEMPROP_FAMILY)==itype) ? directWpn : current_item_id(itype);
        itemid = vbound(itemid, 0, MAXITEMS-1);
        
        // The sword offsets aren't based on anything other than what felt about right
        // compared to the NES game and what mostly kept it from hitting things that
        // should clearly be out of range. They could probably still use more tweaking.
        // Don't use 2.10 for reference; it's pretty far off.
        // - Saf
        
        if(get_canslash() && (attack==wSword || attack==wWand) && (getItemProp(directWpn, ITEMPROP_FLAGS) & ITEM_FLAG4)!=0)
        {
            switch(w.dir)
            {
            case up:
                if(attackclk<8)
                {
                    wy-=4;
                }
                
                break;
                
            case down:
                //if(attackclk<8)
            {
                wy-=2;
            }
            break;
            
            case left:
            
                //if(attackclk<8)
            {
                wx+=2;
            }
            
            break;
            
            case right:
            
                //if(attackclk<8)
            {
                wx-=3;
                //wy+=((spins>0 || get_bit(quest_rules, qr_SLASHFLIPFIX)) ? -4 : 4);
            }
            
            break;
            }
        }
        
        switch(w.dir)
        {
        case up:
            wx+=2;
            break;
            
        case down:
            break;
            
        case left:
            wy-=3;
            break;
            
        case right:
            wy-=3;
            break;
        }
        
        wx+=w.hxofs;
        wy+=w.hyofs;
        
        for(int i=0; i<numGuys(); i++)
        {
            // So that Link can actually hit peahats while jumping, his weapons' hzsz becomes 16 in midair.
            if((getGuy(i).hit(wx,wy,wz,wxsz,wysz,wz>0?16:8) && (attack!=wWand || !questRuleEnabled(qr_NOWANDMELEE)))
                    || (attack==wWand && getGuy(i).hit(wx,wy-8,z,16,24,z>0?16:8) && !questRuleEnabled(qr_NOWANDMELEE))
                    || (attack==wHammer && getGuy(i).hit(wx,wy-8,z,16,24,z>0?16:8)))
            {
                // Checking the whimsical ring for every collision check causes
                // an odd bug. It's much more likely to activate on a 0-damage
                // weapon, since a 0-damage hit won't make the enemy invulnerable
                // to damaging hits in the following frames.
                
                int whimsyid = current_item_id(itype_whimsicalring);
                int whimsypower = 0;
                
                if(whimsyid>-1)
                {
                    whimsypower = rand()%zc_max(getItemProp(current_item_id(itype_whimsicalring), ITEMPROP_MISC1), 1)!=0 ?
                                  0 : current_item_power(itype_whimsicalring);
                }
                
                int h = hit_enemy(i,attack,(weaponattackpower() + whimsypower)*DAMAGE_MULTIPLIER,wx,wy,dir,directWpn);
                
                if(h<0 && whimsypower!=0)
                {
                    sfx(getItemProp(whimsyid, ITEMPROP_SOUND));
                }
                
                if(h!=0 && charging>0)
                {
                    attackclk = SWORDTAPFRAME;
                    spins=0;
                }
                
                if(h!=0 && hclk==0 && inlikelike != 1)
                {
                    if(GuyHit(i,x+7,y+7,z,2,2,hzsz)!=-1)
                    {
                        hitlink(i);
                    }
                }
                
                if(h==2)
                    break;
            }
        }
        
        if(!questRuleEnabled(qr_NOITEMMELEE))
        {
            for(int j=0; j<numItems(); j++)
            {
                item@ it=loadItem(j);
                if((it.pickup & ipTIMER)!=0)
                {
                    if(it.clk2 >= 32)
                    {
                        if(it.hit(wx,wy,z,wxsz,wysz,1) || (attack==wWand && it.hit(x,y-8,z,wxsz,wysz,1))
                                || (attack==wHammer && it.hit(x,y-8,z,wxsz,wysz,1)))
                        {
                            int pickup = it.pickup;
                            
                            if((pickup&ipONETIME)!=0) // set mITEM for one-time-only items
                                setmapflag(mITEM);
                            else if((pickup&ipONETIME2)!=0) // set mBELOW flag for other one-time-only items
                                setmapflag();
                                
                            if(getItemProp(it.id, ITEMPROP_PICKUP_SCRIPT)!=0)
                            {
                                runZScript(SCRIPT_ITEM, getItemProp(it.id, ITEMPROP_PICKUP_SCRIPT), it.id & 0xFFF);
                            }
                            
                            getitem(it.id);
                            deleteItem(j);//items.del(j);
                            
                            for(int i=0; i<numLwpns(); i++)
                            {
                                weapon@ w2 = getLwpn(i);
                                w2.adjustDraggedItem(j);
                            }
                            
                            --j;
                        }
                    }
                }
            }
        }
        
        if(attack==wSword)
        {
            if(attackclk == 6)
            {
                for(int q=0; q<176; q++)
                {
                    screengrid[q]=false;
                }
                
                for(int q=0; q<32; q++)
                    ffcgrid[q]=false;
            }
            
            if(dir==up && ((int(x)&15)==0))
            {
                check_slash_block(wx,wy);
                check_slash_block(wx,wy+8);
            }
            else if(dir==up && ((int(x)&15)==8||diagonalMovement))
            {
                check_slash_block(wx,wy);
                check_slash_block(wx,wy+8);
                check_slash_block(wx+8,wy);
                check_slash_block(wx+8,wy+8);
            }
            
            if(dir==down && ((int(x)&15)==0))
            {
                check_slash_block(wx,wy+wysz-8);
                check_slash_block(wx,wy+wysz);
            }
            else if(dir==down && ((int(x)&15)==8||diagonalMovement))
            {
                check_slash_block(wx,wy+wysz-8);
                check_slash_block(wx,wy+wysz);
                check_slash_block(wx+8,wy+wysz-8);
                check_slash_block(wx+8,wy+wysz);
            }
            
            if(dir==left)
            {
                check_slash_block(wx,wy+8);
                check_slash_block(wx+8,wy+8);
            }
            
            if(dir==right)
            {
                check_slash_block(wx+wxsz,wy+8);
                check_slash_block(wx+wxsz-8,wy+8);
            }
        }
        else if(attack==wWand)
        {
            if(attackclk == 5)
            {
                for(int q=0; q<176; q++)
                {
                    screengrid[q]=false;
                }
                
                for(int q=0; q<32; q++)
                    ffcgrid[q]=false;
            }
            
            // cutable blocks
            if(dir==up && (int(x)&15)==0)
            {
                check_wand_block(wx,wy);
                check_wand_block(wx,wy+8);
            }
            else if(dir==up && ((int(x)&15)==8||diagonalMovement))
            {
                check_wand_block(wx,wy);
                check_wand_block(wx,wy+8);
                check_wand_block(wx+8,wy);
                check_wand_block(wx+8,wy+8);
            }
            
            if(dir==down && (int(x)&15)==0)
            {
                check_wand_block(wx,wy+wysz-8);
                check_wand_block(wx,wy+wysz);
            }
            else if(dir==down && ((int(x)&15)==8||diagonalMovement))
            {
                check_wand_block(wx,wy+wysz-8);
                check_wand_block(wx,wy+wysz);
                check_wand_block(wx+8,wy+wysz-8);
                check_wand_block(wx+8,wy+wysz);
            }
            
            if(dir==left)
            {
                check_wand_block(wx,y+8);
                check_wand_block(wx+8,y+8);
            }
            
            if(dir==right)
            {
                check_wand_block(wx+wxsz,y+8);
                check_wand_block(wx+wxsz-8,y+8);
            }
        }
        else if((attack==wHammer) && (attackclk==15))
        {
            // poundable blocks
            for(int q=0; q<176; q++)
            {
                screengrid[q]=false;
            }
            
            for(int q=0; q<32; q++)
                ffcgrid[q]=false;
                
            if(dir==up && (int(x)&15)==0)
            {
                check_pound_block(wx,wy);
                check_pound_block(wx,wy+8);
            }
            else if(dir==up && ((int(x)&15)==8||diagonalMovement))
            {
                check_pound_block(wx,wy);
                check_pound_block(wx,wy+8);
                check_pound_block(wx+8,wy);
                check_pound_block(wx+8,wy+8);
            }
            
            if(dir==down && (int(x)&15)==0)
            {
                check_pound_block(wx,wy+wysz-8);
                check_pound_block(wx,wy+wysz);
            }
            else if(dir==down && ((int(x)&15)==8||diagonalMovement))
            {
                check_pound_block(wx,wy+wysz-8);
                check_pound_block(wx,wy+wysz);
                check_pound_block(wx+8,wy+wysz-8);
                check_pound_block(wx+8,wy+wysz);
            }
            
            if(dir==left)
            {
                check_pound_block(wx,y+8);
                check_pound_block(wx+8,y+8);
            }
            
            if(dir==right)
            {
                check_pound_block(wx+wxsz,y+8);
                check_pound_block(wx+wxsz-8,y+8);
            }
        }
    }
    
    void check_slash_block(int bx, int by)
    {
        if(z>8 || attackclk==SWORDCHARGEFRAME  // is not charging>0, as tapping a wall reduces attackclk but retains charging
                || (attackclk>SWORDTAPFRAME && tapping))
            return;
        
        //keep things inside the screen boundaries
        bx=vbound(bx, 0, 255);
        by=vbound(by, 0, 176);
        int fx=vbound(bx, 0, 255);
        int fy=vbound(by, 0, 176);
            
        //find out which combo row/column the coordinates are in
        bx &= 0xF0;
        by &= 0xF0;
        
        int type = COMBOTYPE(bx,by);
        int type2 = FFCOMBOTYPE(fx,fy);
        int flag = MAPFLAG(bx,by);
        int flag2 = MAPCOMBOFLAG(bx,by);
        int flag3 = MAPFFCOMBOFLAG(fx,fy);
        int i = (bx>>4) + by;
        
        if(i > 175)
            return;
            
        bool ignorescreen=false;
        bool ignoreffc=false;
        
        if(screengrid[i])
        {
            ignorescreen = true;
        }
        
        int current_ffcombo = getFFCAt(fx,fy);
        
        if(current_ffcombo == -1 || ffcgrid[current_ffcombo])
        {
            ignoreffc = true;
        }
        
        if(!isCuttableType(type) &&
                (flag<mfSWORD || flag>mfXSWORD) &&  flag!=mfSTRIKE && (flag2<mfSWORD || flag2>mfXSWORD) && flag2!=mfSTRIKE)
        {
            ignorescreen = true;
        }
        
        if(!isCuttableType(type2) &&
                (flag3<mfSWORD || flag3>mfXSWORD) && flag3!=mfSTRIKE)
        {
            ignoreffc = true;
        }
        
        mapscr@ s = getTmpscr((currscr>=128) ? 1 : 0);
        
        int sworditem = (directWpn>-1 && getItemProp(directWpn, ITEMPROP_FAMILY)==itype_sword) ? getItemProp(directWpn, ITEMPROP_LEVEL) : current_item(itype_sword);
        
        if(!ignorescreen)
        {
            if((flag >= 16)&&(flag <= 31))
            {
                s.data[i] = s.secretcombo[(s.sflag[i])-16+4];
                s.cset[i] = s.secretcset[(s.sflag[i])-16+4];
                s.sflag[i] = s.secretflag[(s.sflag[i])-16+4];
            }
            else if(flag == mfARMOS_SECRET)
            {
                s.data[i] = s.secretcombo[sSTAIRS];
                s.cset[i] = s.secretcset[sSTAIRS];
                s.sflag[i] = s.secretflag[sSTAIRS];
                sfx(getTmpscr(0).secretsfx);
            }
            else if(((flag>=mfSWORD&&flag<=mfXSWORD)||(flag==mfSTRIKE)))
            {
                for(int i2=0; i2<=zc_min(sworditem-1,3); i2++)
                {
                    findentrance(bx,by,mfSWORD+i2,true);
                }
                
                findentrance(bx,by,mfSTRIKE,true);
            }
            else if(((flag2 >= 16)&&(flag2 <= 31)))
            {
                s.data[i] = s.secretcombo[(s.sflag[i])-16+4];
                s.cset[i] = s.secretcset[(s.sflag[i])-16+4];
                s.sflag[i] = s.secretflag[(s.sflag[i])-16+4];
            }
            else if(flag2 == mfARMOS_SECRET)
            {
                s.data[i] = s.secretcombo[sSTAIRS];
                s.cset[i] = s.secretcset[sSTAIRS];
                s.sflag[i] = s.secretflag[sSTAIRS];
                sfx(getTmpscr(0).secretsfx);
            }
            else if(((flag2>=mfSWORD&&flag2<=mfXSWORD)||(flag2==mfSTRIKE)))
            {
                for(int i2=0; i2<=zc_min(sworditem-1,3); i2++)
                {
                    findentrance(bx,by,mfSWORD+i2,true);
                }
                
                findentrance(bx,by,mfSTRIKE,true);
            }
            else
            {
                if(isCuttableNextType(type))
                {
                    s.data[i]=s.data[i]+1;
                }
                else
                {
                    s.data[i] = s.undercombo;
                    s.cset[i] = s.undercset;
                    s.sflag[i] = 0;
                }
                
                //pausenow=true;
            }
        }
        
        if(((flag3>=mfSWORD&&flag3<=mfXSWORD)||(flag3==mfSTRIKE)) && !ignoreffc)
        {
            for(int i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                findentrance(bx,by,mfSWORD+i2,true);
            }
            
            findentrance(fx,fy,mfSTRIKE,true);
        }
        else if(!ignoreffc)
        {
            if(isCuttableNextType(type2))
                s.ffcs[current_ffcombo].modCombo(1);
            else
            {
                s.ffcs[current_ffcombo].setCombo(s.undercombo);
                s.ffcs[current_ffcombo].setCSet(s.undercset);
            }
        }
        
        if(!ignorescreen)
        {
            if(!isTouchyType(type)) screengrid[i]=true;
            
            if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && !getmapflag())
            {
                placeitem(bx, by, getTmpscr(0).catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP);
                sfx(getTmpscr(0).secretsfx);
            }
            else if(isCuttableItemType(type))
            {
                int it = select_dropitem(12, bx, by);
                
                if(it!=-1)
                {
                    placeitem(bx, by, it, ipBIGRANGE + ipTIMER);
                }
            }
            
            // WEIRDNESS
            // This doesn't appear to do anything useful.
            //putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
            
            if(isBushType(type) || isFlowersType(type) || isGrassType(type))
            {
                if(questRuleEnabled(qr_MORESOUNDS))
                {
                    sfx(WAV_ZN1GRASSCUT, bx);
                }
                
                if(isBushType(type))
                {
                    addDecoration(dBUSHLEAVES, fx, fy);
                }
                else if(isFlowersType(type))
                {
                    addDecoration(dFLOWERCLIPPINGS, fx, fy);
                }
                else if(isGrassType(type))
                {
                    addDecoration(dGRASSCLIPPINGS, fx, fy);
                }
            }
        }
        
        if(!ignoreffc)
        {
            if(!isTouchyType(type)) ffcgrid[current_ffcombo]=true;
            
            if(isCuttableItemType(type2))
            {
                int it=-1;
                int r=rand()%100;
                
                if(r<15)
                {
                    it=iHeart;                                // 15%
                }
                else if(r<35)
                {
                    it=iRupy;                                 // 20%
                }
                
                if(it!=-1 && getItemProp(it, ITEMPROP_FAMILY) != itype_misc) // Don't drop non-gameplay items
                {
                    placeitem(fx, fy, it, ipBIGRANGE + ipTIMER);
                }
            }
            
            if(isBushType(type2) || isFlowersType(type2) || isGrassType(type2))
            {
                if(questRuleEnabled(qr_MORESOUNDS))
                {
                    sfx(WAV_ZN1GRASSCUT, bx);
                }
                
                if(isBushType(type2))
                {
                    addDecoration(dBUSHLEAVES, fx, fy);
                }
                else if(isFlowersType(type2))
                {
                    addDecoration(dFLOWERCLIPPINGS, fx, fy);
                }
                else if(isGrassType(type2))
                {
                    addDecoration(dGRASSCLIPPINGS, fx, fy);
                }
            }
        }
    }
    
    void check_wand_block(int bx, int by)
    {
        //first things first
        if(z>8) return;
        
        //keep things inside the screen boundaries
        bx=vbound(bx, 0, 255);
        by=vbound(by, 0, 176);
        int fx=vbound(bx, 0, 255);
        int fy=vbound(by, 0, 176);
        
        //find out which combo row/column the coordinates are in
        bx &= 0xF0;
        by &= 0xF0;
        
        int flag = MAPFLAG(bx,by);
        int flag2 = MAPCOMBOFLAG(bx,by);
        int flag3=0;
        int flag31 = MAPFFCOMBOFLAG(fx,fy);
        int flag32 = MAPFFCOMBOFLAG(fx,fy);
        int flag33 = MAPFFCOMBOFLAG(fx,fy);
        int flag34 = MAPFFCOMBOFLAG(fx,fy);
        
        if(flag31==mfWAND||flag32==mfWAND||flag33==mfWAND||flag34==mfWAND)
            flag3=mfWAND;
            
        if(flag31==mfSTRIKE||flag32==mfSTRIKE||flag33==mfSTRIKE||flag34==mfSTRIKE)
            flag3=mfSTRIKE;
            
        int i = (bx>>4) + by;
        
        if(flag!=mfWAND&&flag2!=mfWAND&&flag3!=mfWAND&&flag!=mfSTRIKE&&flag2!=mfSTRIKE&&flag3!=mfSTRIKE)
            return;
            
        if(i > 175)
            return;
        
        if(!(findentrance(bx,by,mfWAND,true))&&(!findentrance(bx,by,mfSTRIKE,true)))
        {
            if(flag3==mfWAND||flag3==mfSTRIKE)
            {
                findentrance(fx,fy,mfWAND,true);
                findentrance(fx,fy,mfSTRIKE,true);
            }
        }
    }

    void check_pound_block(int bx, int by)
    {
        //first things first
        if(z>8) return;
        
        //keep things inside the screen boundaries
        bx=vbound(bx, 0, 255);
        by=vbound(by, 0, 176);
        int fx=vbound(bx, 0, 255);
        int fy=vbound(by, 0, 176);
        
        //find out which combo row/column the coordinates are in
        bx &= 0xF0;
        by &= 0xF0;
        
        int type = COMBOTYPE(bx,by);
        int type2 = FFCOMBOTYPE(fx,fy);
        int flag = MAPFLAG(bx,by);
        int flag2 = MAPCOMBOFLAG(bx,by);
        int flag3 = MAPFFCOMBOFLAG(fx,fy);
        int i = (bx>>4) + by;
        
        if(i > 175)
            return;
            
        bool ignorescreen=false;
        bool ignoreffc=false;
        bool pound=false;
        
        if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE && flag2!=mfHAMMER && flag2!=mfSTRIKE)
            ignorescreen = true; // Affect only FFCs
            
        if(screengrid[i])
            ignorescreen = true;
            
        int current_ffcombo = getFFCAt(fx,fy);
        
        if(current_ffcombo == -1 || ffcgrid[current_ffcombo])
            ignoreffc = true;
            
        if(type2!=cPOUND && flag3!=mfSTRIKE && flag3!=mfHAMMER)
            ignoreffc = true;
        
        if(ignorescreen && ignoreffc)  // Nothing to do.
            return;
        
        mapscr@ s= getTmpscr((currscr>=128) ? 1 : 0);
        
        if(!ignorescreen)
        {
            if(flag==mfHAMMER||flag==mfSTRIKE)  // Takes precedence over Secret Tile and Armos->Secret
            {
                findentrance(bx,by,mfHAMMER,true);
                findentrance(bx,by,mfSTRIKE,true);
            }
            else if(flag2==mfHAMMER||flag2==mfSTRIKE)
            {
                findentrance(bx,by,mfHAMMER,true);
                findentrance(bx,by,mfSTRIKE,true);
            }
            else if((flag >= 16)&&(flag <= 31))
            {
                s.data[i] = s.secretcombo[(s.sflag[i])-16+4];
                s.cset[i] = s.secretcset[(s.sflag[i])-16+4];
                s.sflag[i] = s.secretflag[(s.sflag[i])-16+4];
            }
            else if(flag == mfARMOS_SECRET)
            {
                s.data[i] = s.secretcombo[sSTAIRS];
                s.cset[i] = s.secretcset[sSTAIRS];
                s.sflag[i] = s.secretflag[sSTAIRS];
                sfx(getTmpscr(0).secretsfx);
            }
            else if((flag2 >= 16)&&(flag2 <= 31))
            {
                s.data[i] = s.secretcombo[(s.sflag[i])-16+4];
                s.cset[i] = s.secretcset[(s.sflag[i])-16+4];
                s.sflag[i] = s.secretflag[(s.sflag[i])-16+4];
            }
            else if(flag2 == mfARMOS_SECRET)
            {
                s.data[i] = s.secretcombo[sSTAIRS];
                s.cset[i] = s.secretcset[sSTAIRS];
                s.sflag[i] = s.secretflag[sSTAIRS];
                sfx(getTmpscr(0).secretsfx);
            }
            else pound = true;
        }
        
        if(!ignoreffc)
        {
            if(flag3==mfHAMMER||flag3==mfSTRIKE)
            {
                findentrance(fx,fy,mfHAMMER,true);
                findentrance(fx,fy,mfSTRIKE,true);
            }
            else
            {
                s.ffcs[current_ffcombo].modCombo(1);
            }
        }
        
        if(!ignorescreen)
        {
            if(pound)
                s.data[i]=s.data[i]+1;
                
            screengrid[i]=true;
            
            if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && !getmapflag())
            {
                placeitem(bx, by, getTmpscr(0).catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP);
                sfx(getTmpscr(0).secretsfx);
            }
            
            if(type==cPOUND && questRuleEnabled(qr_MORESOUNDS))
                sfx(WAV_ZN1HAMMERPOST,int(bx));
            
            // WEIRDNESS
            // This doesn't appear to do anything useful.
            //putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
        }
        
        if(!ignoreffc)
        {
            ffcgrid[current_ffcombo]=true;
            
            if(type2==cPOUND && questRuleEnabled(qr_MORESOUNDS))
                sfx(WAV_ZN1HAMMERPOST, bx);
        }
    }
    
    // Should swinging the hammer make the 'pound' sound?
    // Or is Link just hitting air?
    bool sideviewhammerpound()
    {
        int wx=0,wy=0;
        
        switch(dir)
        {
        case up:
            wx=-1;
            wy=-15;
            
            if(screenFlagSet(fSIDEVIEW))  wy+=8;
            
            break;
            
        case down:
            wx=8;
            wy=28;
            
            if(screenFlagSet(fSIDEVIEW))  wy-=8;
            
            break;
            
        case left:
            wx=-8;
            wy=14;
            
            if(screenFlagSet(fSIDEVIEW)) wy+=8;
            
            break;
            
        case right:
            wx=21;
            wy=14;
            
            if(screenFlagSet(fSIDEVIEW)) wy+=8;
            
            break;
        }
        
        if(!screenFlagSet(fSIDEVIEW))
        {
            return (COMBOTYPE(x+wx,y+wy)!=cSHALLOWWATER && !iswater(MAPCOMBO(x+wx,y+wy)));
        }
        
        if(_walkflag(x+wx,y+wy,0)) return true;
        
        if(dir==left || dir==right)
        {
            wx+=16;
            
            if(_walkflag(x+wx,y+wy,0)) return true;
        }
        
        return false;
    }
    
    void Drown()
    {
        // Link should never drown if the ladder is out
        if(ladderx!=0 || laddery!=0)
            return;
            
        action=drowning;
        attackclk=0;
        attack=wNone;
        attackid=-1;
        reset_swordcharge();
        drownclk=64;
        z=0;
        fall=0;
    }
    
    void execute(WalkflagInfo@ info)
    {
        int flags = info.getFlags();
        
        if((flags & CLEARILSWIM)!=0)
            ilswim =false;
        else if((flags & SETILSWIM)!=0)
            ilswim = true;
            
        if((flags & CLEARCHARGEATTACK)!=0)
        {
            charging = 0;
            attackclk = 0;
        }
        
        if((flags & SETDIR)!=0)
        {
            dir = info.getDir();
        }
        
        if((flags & SETHOPCLK)!=0)
        {
            hopclk = info.getHopClk();
        }
        
        if((flags & SETHOPDIR)!=0)
        {
            hopdir = info.getHopDir();
        }
        
    }
    
    void reset_swordcharge()
    {
        charging=0;
        spins=0;
        tapping=false;
    }
    
    void setEaten(uint8 i)
    {
        inlikelike=i;
    }
    
    int getEaten()
    {
        return inlikelike;
    }
    
    int getAction()
    {
        if(spins > 0)
            return isspinning;
        else if(charging > 0)
            return ischarging;
        else if(diveclk > 0)
            return isdiving;
            
        return action;
    }
    
    bool isSwimming()
    {
        return ((action==swimming)||(action==swimhit)||
                (action==waterhold1)||(action==waterhold2)||
                (hopclk==0xFF));
    }
    
    bool drunk()
    {
        return ((0==(frame%((rand()%100)+1)))&&(rand()%500<drunkclk));
    }

    bool DrunkUp()
    {
        return drunk()?(rand()%2!=0)?false:!Up():Up();
    }
    
    bool DrunkDown()
    {
        return drunk()?(rand()%2!=0)?false:!Down():Down();
    }
    
    bool DrunkLeft()
    {
        return drunk()?(rand()%2!=0)?false:!Left():Left();
    }
    
    bool DrunkRight()
    {
        return drunk()?(rand()%2!=0)?false:!Right():Right();
    }
    
    bool DrunkcAbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cAbtn():cAbtn();
    }
    
    bool DrunkcBbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cBbtn():cBbtn();
    }
    
    bool DrunkcSbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cSbtn():cSbtn();
    }
    
    bool DrunkcMbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cMbtn():cMbtn();
    }
    
    bool DrunkcLbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cLbtn():cLbtn();
    }
    
    bool DrunkcRbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cRbtn():cRbtn();
    }
    
    bool DrunkcPbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cPbtn():cPbtn();
    }

    bool DrunkrUp()
    {
        return drunk()?(rand()%2!=0)?false:!rUp():rUp();
    }
    
    bool DrunkrDown()
    {
        return drunk()?(rand()%2!=0)?false:!rDown():rDown();
    }
    
    bool DrunkrLeft()
    {
        return drunk()?(rand()%2!=0)?false:!rLeft():rLeft();
    }
    
    bool DrunkrRight()
    {
        return drunk()?(rand()%2!=0)?false:!rRight():rRight();
    }
    
    bool DrunkrAbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rAbtn():rAbtn();
    }
    
    bool DrunkrBbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rBbtn():rBbtn();
    }
    
    bool DrunkrSbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rSbtn():rSbtn();
    }
    
    bool DrunkrMbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rMbtn():rMbtn();
    }
    
    bool DrunkrLbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rLbtn():rLbtn();
    }
    
    bool DrunkrRbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rRbtn():rRbtn();
    }
    
    bool DrunkrPbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rPbtn():rPbtn();
    }
}

// The following are only used for Link damage. Damage is in quarter hearts.
int enemy_dp(int index)
{
    return getGuy(index).dp*4;
}

int ewpn_dp(int index)
{
    return getEwpn(index).power*4;
}

int lwpn_dp(int index)
{
    return getLwpn(index).power*4;
}

void paymagiccost(int itemid)
{
    if(itemid < 0)
        return;
        
    if(getItemProp(itemid, ITEMPROP_MAGIC) <= 0)
        return;
        
    if((getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_RUPEE_MAGIC)!=0)
    {
        change_dcounter(-getItemProp(itemid, ITEMPROP_MAGIC), 1);
        return;
    }
    
    if(current_item_power(itype_magicring) > 0)
        return;
    
    change_counter(-(getItemProp(itemid, ITEMPROP_MAGIC)*get_magicdrainrate()), 4);
}

fix LinkX()
{
    return Link.x;
}

fix LinkY()
{
    return Link.y;
}

fix LinkZ()
{
    return Link.z;
}

int LinkDir()
{
    return Link.dir;
}

