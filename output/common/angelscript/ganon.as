class Ganon: enemy
{
    int ganonstunclk;
    
    Ganon()
    {
        hxofs=8;
        hyofs=8;
        hzsz=16; //can't be jumped.
        clk2=70;
        misc=-1;
        ganonstunclk=0;
        mainguy=!getmapflag();
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        switch(misc)
        {
        case -1:
            misc=0;
            
        case 0:
            clk2+=1;
            if(clk2>72 && (rand()&3)==0)
            {
                fireWeapon();
                clk2=0;
            }
            
            ganonstunclk=0;
            constant_walk(rate,homing,spw_none);
            break;
            
        case 1:
        case 2:
            if(--ganonstunclk<=0)
            {
                int r=rand();
                
                if((r&1)!=0)
                {
                    y=96;
                    
                    if((r&2)!=0)
                        x=160;
                    else
                        x=48;
                        
                    if(tooclose(x,y,48))
                        x=208-x;
                }
                
                loadpalset(csBOSS, pSprite(getGuyProp(id, GUYPROP_BOSSPAL)));
                misc=0;
            }
            
            break;
            
        case 3:
            if(hclk>0)
                break;
                
            misc=4;
            clk=0;
            hxofs=1000;
            loadpalset(9, pSprite(spPILE));
            music_stop();
            stop_sfx(WAV_ROAR);
            
            playDeathSFX();
            
            sfx(WAV_GANON);
            placeitem(x+8, y+8, iPile, ipDUMMY);
            break;
            
        case 4:
            if(clk>=80)
            {
                misc=5;
                
                if(getmapflag())
                {
                    setLevelItem(dlevel, liBOSS);
                    playLevelMusic();
                    return true;
                }
                
                sfx(WAV_CLEARED);
                placeitem(x+8, y+8, iBigTri, ipBIGTRI);
                setmapflag();
            }
            
            break;
        }
        
        return enemy::animate(index);
    }


    int takehit(weapon@ w)
    {
        //these are here to bypass compiler warnings about unused arguments
        int wpnId = w.id;
        int power = w.power;
        int enemyHitWeapon = w.parentitem;
        
        switch(misc)
        {
        case 0:
            if(wpnId!=wSword)
                return 0;
                
            hp-=power;
            
            if(hp>0)
            {
                misc=1;
                ganonstunclk=64;
            }
            else
            {
                loadpalset(csBOSS, pSprite(spBROWN));
                misc=2;
                ganonstunclk=284;
                hp=getGuyProp(id, GUYPROP_HP);
            }
            
            sfx(WAV_EHIT, x);
            playHitSFX(x);
            
            return 1;
            
        case 2:
            if(wpnId!=wArrow || (enemyHitWeapon>-1 ? getItemProp(enemyHitWeapon, ITEMPROP_POWER) : current_item_power(itype_arrow))<4)
                return 0;
                
            misc=3;
            hclk=81;
            loadpalset(9, pSprite(spBROWN));
            return 1;
        }
        
        return 0;
    }

    void Draw()
    {
        switch(misc)
        {
        case 0:
            if((clk&3)==3)
                tile=(rand()%5)*2+o_tile;
                
            if(db!=999)
                break;
                
        case 2:
            if(ganonstunclk<64 && (ganonstunclk&1)!=0)
                break;
                
        case -1:
            tile=o_tile;
            
            //fall through
        case 1:
        case 3:
            drawblock();
            break;
            
        case 4:
            draw_guts();
            draw_flash();
            break;
        }
    }

    void draw_guts()
    {
        int c = zc_min(clk>>3,8);
        tile = clk<24 ? 74 : 75;
        overtile16(tile,x+8,y+c+playing_field_offset,9,0);
        overtile16(tile,x+8,y+16-c+playing_field_offset,9,0);
        overtile16(tile,x+c,y+8+playing_field_offset,9,0);
        overtile16(tile,x+16-c,y+8+playing_field_offset,9,0);
        overtile16(tile,x+c,y+c+playing_field_offset,9,0);
        overtile16(tile,x+16-c,y+c+playing_field_offset,9,0);
        overtile16(tile,x+c,y+16-c+playing_field_offset,9,0);
        overtile16(tile,x+16-c,y+16-c+playing_field_offset,9,0);
    }

    void draw_flash()
    {
        int c = clk-(clk>>2);
        cs = (frame&3)+6;
        overtile16(194,x+8,y+8-clk+playing_field_offset,cs,0);
        overtile16(194,x+8,y+8+clk+playing_field_offset,cs,2);
        overtile16(195,x+8-clk,y+8+playing_field_offset,cs,0);
        overtile16(195,x+8+clk,y+8+playing_field_offset,cs,1);
        overtile16(196,x+8-c,y+8-c+playing_field_offset,cs,0);
        overtile16(196,x+8+c,y+8-c+playing_field_offset,cs,1);
        overtile16(196,x+8-c,y+8+c+playing_field_offset,cs,2);
        overtile16(196,x+8+c,y+8+c+playing_field_offset,cs,3);
    }
}

