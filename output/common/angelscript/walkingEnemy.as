class WalkingEnemy: enemy
{
    bool haslink // Like Like
    {
        get const { return realEnemy.haslink; }
        set { realEnemy.haslink=value; }
    }
    
    int clk4, //Tribble clock
        clk5; // Like Like clock
    bool fired; // Darknut5, Bombchu
    bool shield; // Darknut
    bool dashing; // Rope
    int multishot; // Shoot twice/thrice/n times in one halt
    fix fy, shadowdistance; // Pols Voice
    int starting_hp;
    
    WalkingEnemy()
    {
        multishot = 0;
        fired = false;
        dashing = false;
        haslink = false;
        dummy_bool0=false;
        shield=(flags&(inv_left | inv_right | inv_back |inv_front))==0;
        starting_hp=hp;
        
        if(dmisc9==e9tARMOS && (rand()&1)!=0)
        {
            step=dmisc10/100.0;
            
            if(anim==aARMOS4)
                o_tile+=20;
        }
        
        if((flags & guy_fadeflicker)!=0)
        {
            clk=0;
            superman = 1;
            fading=fade_flicker;
            count_enemy=false;
            dir=down;
            
            if(!canmove(down, 8, spw_none))
                clk3=int(13.0/step);
        }
        else if((flags & guy_fadeinstant)!=0)
        {
            clk=0;
        }
        
        shadowdistance = 0;
        clk4 = 0;
        clk5 = 0;
    }

    bool Update(int index)
    {
        if(dying)
        {
            if(haslink)
            {
                Link.setEaten(0);
                haslink=false;
            }
            
            if(dmisc9==e9tROPE && dmisc2==e2tBOMBCHU && !fired && hp<=0 && hp>-1000 && wpn>wEnemyWeapons)
            {
                hp=-1000;
                activateDeathAttack();
            }
            else if(wpn!=0 && wpn!=ewBrang && dmisc2==e2tFIREOCTO)  // Fire Octo
            {
                activateDeathAttack();
            }
            
            KillWeapon();
            return Dead(index);
        }
        else if((hp<=0 && dmisc2==e2tSPLIT) || (dmisc2==e2tSPLITHIT && hp>0 && hp<getGuyProp(id, GUYPROP_HP) && !slide()))  //Split into enemies
        {
            stopBGSFX();
            int kids = numGuys();
            int id2=dmisc3;
            
            for(int i=0; i < dmisc4; i++)
            {
                if(addenemy(x,y,id2+(getGuyProp(id2, GUYPROP_FAMILY)==eeKEESE ? 0 : (i<<12)),-21-(i%4))!=0)
                    getGuy(kids+i).count_enemy = false;
            }
            
            if(itemguy) // Hand down the carried item
            {
                guycarryingitem = numGuys()-1;
                getGuy(guycarryingitem).itemguy = true;
                itemguy = false;
            }
            
            if(haslink)
            {
                Link.setEaten(0);
                haslink=false;
            }
            
            if(dmisc2==e2tSPLIT)
                playDeathSFX();
                
            return true;
        }
        
        if(fading!=0)
        {
            if(++clk4 > 60)
            {
                clk4=0;
                superman=0;
                fading=0;
                
                if((flags2&cmbflag_armos)!=0 && z==0)
                    removearmos(x,y);
                    
                clk2=0;
                
                newdir();
            }
            else return enemy::animate(index);
        }
        else if((flags2&cmbflag_armos)!=0 && z==0 && clk==0)
            removearmos(x,y);
            
        if(haslink)
        {
            Link.x=x;
            Link.y=y;
            clk2+=1;
            
            if(clk2==(dmisc8==0 ? 95 : dmisc8))
            {
                switch(dmisc7)
                {
                case e7tEATITEMS:
                {
                    for(int i=0; i<MAXITEMS; i++)
                    {
                        if((getItemProp(i, ITEMPROP_FLAGS)&ITEM_EDIBLE)!=0)
                            set_item(i, false);
                    }
                    
                    break;
                }
                
                case e7tEATMAGIC:
                    change_dcounter(4, -1*int(get_magicdrainrate()));
                    break;
                    
                case e7tEATRUPEES:
                    change_dcounter(1, -1);
                    break;
                }
                
                clk2=0;
            }
            
            if((clk&0x18)==8)                                       // stop its animation on the middle frame
                clk-=1;
        }
        else if(!(wpn==ewBrang && WeaponOut()))  //WeaponOut uses misc
        {
            // Movement engine
            if(clk>=0) switch(id>>12)
                {
                case 0: // Normal movement
                    if(dmisc9==e9tVIRE || dmisc9==e9tPOLSVOICE) //Vire
                    {
                        vire_hop();
                        break;
                    }
                    else if(dmisc9==e9tROPE) //Rope charge
                    {
                        if(!fired && dashing && stunclk==0 && !watch)
                        {
                            if(dmisc2==e2tBOMBCHU && LinkInRange(16) && wpn+dmisc3 > wEnemyWeapons) //Bombchu
                            {
                                hp=-1000;
                                activateDeathAttack();
                            }
                        }
                        
                        charge_attack();
                        break;
                    }
                    /*
                     * Boomerang-throwers have a halt count of 1
                     * Zols have a halt count of (rand()&7)<<4
                     * Gels have a halt count of ((rand()&7)<<3)+2
                     * Everything else has 48
                     */
                    else
                    {
                        if(wpn==ewBrang) // Goriya
                        {
                            halting_walk(rate,homing,0,hrate, 1);
                        }
                        else if(dmisc9==e9tNORMAL && wpn==0)
                        {
                            if(dmisc2==e2tSPLITHIT) // Zol
                            {
                                halting_walk(rate,homing,0,hrate,(rand()&7)<<4);
                            }
                            else if(frate<=8 && starting_hp==1) // Gel
                            {
                                halting_walk(rate,homing,0,hrate,((rand()&7)<<3)+2);
                            }
                            else // Other
                            {
                                halting_walk(rate,homing,0,hrate, 48);
                            }
                        }
                        else // Other
                        {
                            halting_walk(rate,homing,0,hrate, 48);
                        }
                    }
                    break;
                    
                    // Following cases are for just after creation-by-splitting.
                case 1:
                    if(misc==1)
                    {
                        dir=up;
                        step=8;
                    }
                    
                    if(misc<=2)
                    {
                        move(step);
                        
                        if(!canmove(dir,0,0))
                            dir=down;
                    }
                    
                    if(misc==3)
                    {
                        if(canmove(right,16,0))
                            x+=16;
                    }
                    
                    misc+=1;
                    break;
                    
                case 2:
                    if(misc==1)
                    {
                        dir=down;
                        step=8;
                    }
                    
                    if(misc<=2)
                    {
                        move(step);
                    }
                    
                    if(misc==3)
                    {
                        if(canmove(left,16,0))
                            x-=16;
                    }
                    
                    misc+=1;
                    break;
                    
                default:
                    if(misc==1)
                    {
                        dir=(rand()%4);
                        step=8;
                    }
                    
                    if(misc<=2)
                    {
                        move(step);
                        
                        if(!canmove(dir,0,0))
                            dir=dir^1;
                    }
                    
                    if(misc==3)
                    {
                        if(dir >= left && canmove(dir,16,0))
                            x+=(dir==left ? -16 : 16);
                    }
                    
                    misc+=1;
                    break;
                }
                
            if((id>>12)!=0 && misc>=4) //recently spawned by a split enemy
            {
                id&=0xFFF;
                step = dstep/100.0;
                
                if(x<32) x=32;
                
                if(x>208) x=208;
                
                if(y<32) y=32;
                
                if(y>128) y=128;
                
                misc=3;
            }
        }
        else
        {
            if(clk2>2) clk2-=1;
        }
        
        // Fire Zol
        if(wpn!=0 && dmisc1==e1tEACHTILE && clk2==1 && hclk==0)
        {
            fireWeapon();
            
            int i=numEwpns()-1;
            weapon@ ew = getEwpn(i);
            
            if(wpn==ewFIRETRAIL && getWeaponProp(ewFIRETRAIL, WPNPROP_FRAMES)>1)
            {
                ew.aframe=rand()%getWeaponProp(ewFIRETRAIL, WPNPROP_FRAMES);
                ew.tile+=ew.aframe;
            }
        }
        // Goriya
        else if(wpn==ewBrang && clk2==1 && sclk==0 && stunclk==0 && !watch && wpn!=0 && !WeaponOut())
        {
            misc=index+100;
            fireWeapon();
            
            if(dmisc1==2)
            {
                int ndir=dir;
                
                if(Link.x-x==0)
                {
                    ndir=(Link.y+8<y)?up:down;
                }
                else //turn to face Link
                {
                    double ddir=atan2(y-Link.y, Link.x-x);
                    
                    if((ddir<=(((-2)*PI)/8))&&(ddir>(((-6)*PI)/8)))
                    {
                        ndir=down;
                    }
                    else if((ddir<=(((2)*PI)/8))&&(ddir>(((-2)*PI)/8)))
                    {
                        ndir=right;
                    }
                    else if((ddir<=(((6)*PI)/8))&&(ddir>(((2)*PI)/8)))
                    {
                        ndir=up;
                    }
                    else
                    {
                        ndir=left;
                    }
                }
                
                getEwpn(numEwpns()-1).aimedBrang=true;
                
                if(canmove(ndir))
                {
                    dir=ndir;
                }
            }
        }
        else if((clk2==16 || dmisc1==e1tCONSTANT) &&  dmisc1!=e1tEACHTILE && wpn!=0 && wpn!=ewBrang && sclk==0 && stunclk==0 && !watch)
            switch(dmisc1)
            {
            case e1tCONSTANT: //Deathnut
            {
                // Overloading clk5 (Like Like clock) to avoid making another clock just for this attack...
                if(clk5>64)
                {
                    clk5=0;
                    fired=false;
                }
                
                clk5+=(rand()&3);
                
                if((clk5>24)&&(clk5<52))
                {
                    tile+=20;                                         //firing
                    
                    if(!fired&&(clk5>=38))
                    {
                        fireWeapon();
                        fired=true;
                    }
                }
                
                break;
            }
            
            case e1tFIREOCTO: //Fire Octo
                fireWeapon();
                break;
                
            default:
                fireWeapon();
                break;
            }
            
        /* Fire again if:
         * - clk2 about to run out
         * - not already double-firing (dmisc1 is 1)
         * - not carrying Link
         * - one in 0xF chance
         */
        if(clk2==1 && (multishot < dmisc6) && dmisc1 != e1tEACHTILE && !haslink && (rand()&15)==0)
        {
            newdir(rate, homing, grumble);
            clk2=28;
            ++multishot;
        }
        
        if(clk2==0)
        {
            multishot = 0;
        }
        
        if(isFiring())
        {
            clk2=15; //this keeps the octo in place until he's done firing
            updateFiring();
        }
        
        if(dmisc2==e2tTRIBBLE)
            ++clk4;
            
        if(clk4==((dmisc5!=0) ? dmisc5 : 256) && (dmisc2==e2tTRIBBLE) && dmisc3!=0 && dmisc4!=0)
        {
            int kids = numGuys();
            int id2=dmisc3;
            
            for(int i=0; i<dmisc4; i++)
            {
                if(addenemy(x,y,id2,-24)!=0)
                {
                    if(itemguy) // Hand down the carried item
                    {
                        guycarryingitem = numGuys()-1;
                        getGuy(guycarryingitem).itemguy = true;
                        itemguy = false;
                    }
                    
                    getGuy(kids+i).count_enemy = false;
                }
            }
            
            if(haslink)
            {
                Link.setEaten(0);
                haslink=false;
            }
            
            return true;
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        update_enemy_frame();
        
        if((dmisc2==e2tBOMBCHU)&&dashing)
        {
            tile+=20;
        }
        
        enemy::draw();
    }

    void DrawShadow()
    {
        int tempy=yofs;
        
        if((dmisc9 == e9tPOLSVOICE || dmisc9==e9tVIRE) && !questRuleEnabled(qr_ENEMIESZAXIS))
        {
            flip = 0;
            int fdiv = frate/4;
            int efrate = fdiv == 0 ? 0 : clk/fdiv;
            
            int f2=questRuleEnabled(qr_NEWENEMYTILES)?
                   efrate:((clk>=(frate>>1))?1:0);
            shadowtile = getWeaponProp(iwShadow, WPNPROP_TILE);
            
            if(questRuleEnabled(qr_NEWENEMYTILES))
            {
                shadowtile+=f2;
            }
            else
            {
                shadowtile+=(f2!=0)?1:0;
            }
            
            yofs+=shadowdistance;
            yofs+=8;
        }
        
        enemy::drawshadow();
        yofs=tempy;
    }

    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        int wpnDir = w.dir;
        
        if(wpnId==wHammer && shield && (flags & guy_bkshield)!=0
                && (((flags&inv_front)!=0 && wpnDir==(dir^down)) || ((flags&inv_back)!=0 && wpnDir==(dir^up))
                    || ((flags&inv_left)!=0 && wpnDir==(dir^left)) || ((flags&inv_right)!=0 && wpnDir==(dir^right))))
        {
            shield = false;
            flags &= ~(inv_left|inv_right|inv_back|inv_front);
            
            if(questRuleEnabled(qr_BRKNSHLDTILES))
                o_tile=s_tile;
        }
        
        int ret = enemy::takehit(w);
        
        if(sclk!=0 && dmisc2==e2tSPLITHIT)
            sclk+=128;
            
        return ret;
    }

    void charge_attack()
    {
        if(slide())
            return;
            
        if(clk<0 || dir<0 || stunclk!=0 || watch || ceiling)
            return;
            
        if(clk3<=0)
        {
            fix_coords(true);
            
            if(!dashing)
            {
                int ldir = lined_up(7,false);
                
                if(ldir!=-1 && canmove(ldir))
                {
                    dir=ldir;
                    dashing=true;
                    step=(dstep/100.0)+1;
                }
                else newdir(4,0,0);
            }
            
            if(!canmove(dir))
            {
                step=dstep/100.0;
                newdir();
                dashing=false;
            }
            
            fix div = step;
            
            if(div == 0)
                div = 1;
                
            clk3=int(16.0/div);
            return;
        }
        
        move(step);
        clk3-=1;
    }

    void vire_hop()
    {
        if(dmisc9!=e9tPOLSVOICE)
        {
            if(sclk!=0)
                return;
        }
        else sclk=0;
        
        if(clk<0 || dying || stunclk!=0 || watch || ceiling)
            return;
            
        int jump_width;
        int jump_height;
        
        if(dmisc9==e9tPOLSVOICE)
        {
            jump_width=2;
            jump_height=27;
        }
        else
        {
            jump_width=1;
            jump_height=16;
        }
        
        y=floor_y;
        
        if(clk3<=0)
        {
            fix_coords();
            
            //if we're not in the middle of a jump or if we can't complete the current jump in the current direction
            if(clk2<=0 || !canmove(dir,1,spw_floater) || (screenFlagSet(fSIDEVIEW) && ON_SIDEPLATFORM()))
                newdir(rate,homing,dmisc9==e9tPOLSVOICE ? spw_floater : spw_none);
                
            if(clk2<=0)
            {
                if(!canmove(dir,2,spw_none) || m_walkflag(x,y,spw_none) || (rand()&15)>=hrate)
                    clk2=wpn==ewBrang ? 1 : int(16*jump_width/step);
            }
            
            if(dmisc9!=e9tPOLSVOICE && dir>=left) //if we're moving left or right
            {
                clk2=int(16*jump_width/step);
            }
            
            clk3=int(16.0/step);
        }
        
        clk3-=1;
        
        if(dmisc9==e9tPOLSVOICE || clk2>0)
            move(step);
            
        floor_y=y;
        clk2-=1;
        
        //if we're in the middle of a jump
        if(clk2>0 && (dir>=left || dmisc9==e9tPOLSVOICE))
        {
            // Casting to double and using regular sin() breaks this terribly.
            // I have no idea why.
            int h = int(fix(clk2*128*step/(16*jump_width)).sin()*jump_height);
            
            if(questRuleEnabled(qr_ENEMIESZAXIS) && !screenFlagSet(fSIDEVIEW))
            {
                z=h;
            }
            else
            {
                y=floor_y-h;
                shadowdistance=h;
            }
        }
        else
            shadowdistance = 0;
    }

    void eatlink()
    {
        if(!haslink && Link.getEaten()==0 && Link.getAction() != hopping && Link.getAction() != swimming)
        {
            haslink=true;
            y=floor_y;
            z=0;
            
            if(Link.isSwimming())
            {
                Link.x=x;
                Link.y=y;
            }
            else
            {
                x=Link.x;
                y=Link.y;
            }
            
            clk2=0;
        }
    }

    bool WeaponOut()
    {
        for(int i=0; i<numEwpns(); i++)
        {
            weapon@ wpn=getEwpn(i);
            if(wpn.parentid==uid && wpn.id==ewBrang)
            {
                return true;
            }
        }
        
        return false;
    }

    void KillWeapon()
    {
        for(int i=0; i<numEwpns(); i++)
        {
            weapon@ wpn=getEwpn(i);
            if(wpn.id==ewBrang && wpn.parentid == uid)
            {
                // This used to check for wpn.type==misc, too, but that seems to be useless
                deleteEwpn(i);
            }
        }
        
        if(wpn==ewBrang && numEwpnsWithID(ewBrang)==0)
        {
            stop_sfx(WAV_BRANG);
        }
    }

    void break_shield()
    {
        if(!shield)
            return;
            
        flags&=~(inv_front | inv_back | inv_left | inv_right);
        shield=false;
        
        if(questRuleEnabled(qr_BRKNSHLDTILES))
            o_tile=s_tile;
    }
}

