class Gleeok: enemy
{
    int necktile;
    
    Gleeok()
    {
        x=120;
        y=48;
        
        hzsz = 32; // can't be jumped.
        misc=clk;                                                 // total head count
        clk3=clk;                                                 // live head count
        clk=0;
        clk2=60;                                                  // fire ball clock
        hp=getGuyProp(id, GUYPROP_MISC2)*(misc-1)*DAMAGE_MULTIPLIER+getGuyProp(id, GUYPROP_HP);
        dir = down;
        hxofs=4;
        hxsz=8;
        fading=fade_blue_poof;
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            necktile=o_tile+dmisc6;
        }
        else
        {
            necktile=s_tile;
        }
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        // Check if a head was killed somehow...
        if(index+1+clk3>=numGuys())
            clk3=numGuys()-index-1;
        if(index+1+misc>=numGuys())
            misc=numGuys()-index-1;
        
        //fix for the "kill all enemies" item
        if(hp==-1000)
        {
            for(int i=0; i<clk3; ++i)
            {
                enemy@ head=getGuy(index+i+1);
                // I haven't seen this fail, but it seems like it ought to be
                // possible, so I'm checking for it. - Saf
                if((head.id&0xFFF)!=(id&0xFFF))
                    break;
                head.hp=1;                   // re-animate each head,
                head.misc = -1;              // disconnect it,
                head.animate(index+i+1);     // let it animate one frame,
                head.hp=-1000;               // and kill it for good
            }
            
            clk3=0;
            
            for(int i=0; i<misc; i++)
            {
                enemy@ head=getGuy(index+i+1);
                if((head.id&0xFFF)!=(id&0xFFF))
                    break;
                head.misc = -2;             // give the signal to disappear
            }
        }
        
        for(int i=0; i<clk3; i++)
        {
            enemy@ head = getGuy(index+i+1);
            head.dummy_int1=necktile;
            
            if(questRuleEnabled(qr_NEWENEMYTILES))
            {
                head.dummy_int2=o_tile+dmisc8; //connected head tile
                head.dummy_int3=o_tile+dmisc9; //flying head tile
            }
            else
            {
                head.dummy_int2=necktile+1; //connected head tile
                head.dummy_int3=necktile+2; //flying head tile
            }
            
            head.dmisc5=dmisc5; //neck segments
            
            if(head.hclk!=0)
            {
                if(hclk==0)
                {
                    hp -= 1000 - head.hp;
                    hclk = 33;
                    
                    playHitSFX(head.x);
                    sfx(WAV_EHIT, head.x);
                }
                
                head.hclk = 0;
            }
            
            // Must be set in case of naughty ZScripts
            head.hp = 1000;
        }
        
        if(hp<=getGuyProp(id, GUYPROP_MISC2)*(clk3-1)*DAMAGE_MULTIPLIER)
        {
            getGuy(index+clk3).misc = -1;              // give signal to fly off
            clk3-=1;
            hp=getGuyProp(id, GUYPROP_MISC2)*(clk3)*DAMAGE_MULTIPLIER;
        }
        
        if(dmisc3==0)
        {
            clk2+=1;
            if(clk2>72 && (rand()&3)==0)
            {
                int i=rand()%misc;
                setAttackOwner(index+i+1);
                fireWeapon();
                clk2=0;
            }
        }
        else
        {
            clk2+=1;
            if(clk2>100 && (rand()&3)==0)
            {
                enemy@ head = getGuy(rand()%misc+index+1);
                head.fireWeapon();
                clk2=0;
            }
        }
        
        if(hp<=0)
        {
            for(int i=0; i<misc; i++)
                getGuy(index+i+1).misc = -2;             // give the signal to disappear
                
            if((flags&guy_neverret)!=0)
                never_return(index);
        }
        
        return enemy::animate(index);
    }

    int takehit(weapon@ w)
    {
        return 0;
    }

    void Draw()
    {
        tile=o_tile;
        
        if(dying)
        {
            enemy::draw();
            return;
        }
        
        int f=clk/17;
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            // body
            xofs=-8;
            yofs=32;
            
            switch(f)
            {
            case 0:
                tile+=0;
                break;
                
            case 1:
                tile+=2;
                break;
                
            case 2:
                tile+=4;
                break;
                
            default:
                tile+=6;
                break;
            }
        }
        else
        {
            // body
            xofs=-8;
            yofs=32;
            
            switch(f)
            {
            case 0:
                tile+=0;
                break;
                
            case 2:
                tile+=4;
                break;
                
            default:
                tile+=2;
                break;
            }
        }
        
        enemy::drawblock();
    }

    void Draw2()
    {
        // the neck stub
        tile=necktile;
        xofs=0;
        yofs=playing_field_offset;
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            tile+=((clk&24)>>3);
        }
        
        if(hp > 0 && !dont_draw())
        {
            if(screenFlagSet(fINVISROOM) && current_item(itype_amulet)==0)
                sprite::drawcloaked();
            else
                sprite::draw();
        }
    }
}

class GleeokHead: enemy
{
    array<int> nx, ny;                                        //neck x and y?
    int headtile;
    int flyingheadtile;
    int necktile;
    Gleeok@ parent; //parent Gleeok to whose neck this segment is attached
    fix xoffset, yoffset; //coords relative to the parent Gleeok
    array<int> nxoffset, nyoffset; //random "jiggling" of the next tiles
    
    GleeokHead()
    {
        // Find parent first
        for(int i=numGuys()-1; i>=0; i--)
        {
            @parent=cast<Gleeok>(getGuy(i));
            if(parent !is null)
                break;
        }
        
        // Any chance parent is null here? That would be bad...
        
        xoffset=0;
        yoffset=dmisc5*4+2;
        //  fixing
        hp=1000;
        step=1;
        item_set=0;
        x = xoffset+parent.x;
        y = yoffset+parent.y;
        hxofs=4;
        hxsz=8;
        yofs=playing_field_offset;
        clk2=clk;                                                 // how long to wait before moving first time
        clk=0;
        mainguy=false;
        count_enemy=false;
        dir=rand();
        clk3=((dir&2)>>1)+2;                                      // left or right
        dir&=1;                                                   // up or down
        dmisc5=vbound(dmisc5,1,255);
        
        for(int i=0; i<dmisc5; i++)
        {
            nxoffset.insertLast(0);
            nyoffset.insertLast(0);
            nx.insertLast((((i*int(x)) + (dmisc5-i)*(int(parent.x)))) /dmisc5);
            ny.insertLast((((i*int(y)) + (dmisc5-i)*(int(parent.y)))) /dmisc5);
        }
        
        necktile=0;
        bgsfx=-1;
    }
    
    bool Update(int index)
    {
        // don't call removearmos() - it's a segment.
        
        if(misc == 0)
        {
            x = (xoffset+parent.x);
            y = (yoffset+parent.y);
            
            for(int i=0; i<dmisc5; i++)
            {
                nx[i] = ((((i*int(x)) + (dmisc5-i)*(int(parent.x)))) /dmisc5) + 3 + nxoffset[i];
                ny[i] = ((((i*int(y)) + (dmisc5-i)*(int(parent.y)))) /dmisc5) + nyoffset[i];
            }
        }
        
        //  set up the head tiles
        headtile=dummy_int2; //5580, actually.  must adjust for direction later on
        flyingheadtile=dummy_int3;
        
        //  set up the neck tiles
        necktile=dummy_int1;
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            necktile+=((clk&24)>>3);
        }
        
        switch(misc)
        {
        case 0:                                                 // live head
            //  set up the attached head tiles
            tile=headtile;
            
            if(questRuleEnabled(qr_NEWENEMYTILES))
            {
                tile+=((clk&24)>>3);
            }
            
            clk2+=1;
            if(clk2>=0 && (clk2&3)==0)
            {
                if(y<= int(parent.y) + 8) dir=down;
                
                if(y>= int(parent.y) + dmisc5*8) dir = up;
                
                if(y<= int(parent.y) + 10 && (rand()&31)==0)
                {
                    dir^=1;
                }
                
                fix tempx = x;
                fix tempy = y;
                
                sprite::move(step);
                xoffset += (x-tempx);
                yoffset += (y-tempy);
                
                if(clk2>=4)
                {
                    clk3^=1;
                    clk2=-4;
                }
                else
                {
                    if(x <= int(parent.x)-(dmisc5*6))
                    {
                        clk3=right;
                    }
                    
                    if(x >= int(parent.x)+(dmisc5*6))
                    {
                        clk3=left;
                    }
                    
                    if(y <= int(parent.y)+(dmisc5*6) && (rand()&15)==0)
                    {
                        clk3^=1;                                        // x jig
                    }
                    else
                    {
                        if(y<=int(parent.y)+(dmisc5*4) && (rand()&31)==0)
                        {
                            clk3^=1;                                      // x switch back
                        }
                        
                        clk2=-4;
                    }
                }
                
                int temp=dir;
                dir=clk3;
                clk3=temp;
                tempx = x;
                tempy = y;
                sprite::move(step);
                xoffset += (x-tempx);
                yoffset += (y-tempy);
                temp=dir;
                dir=clk3;
                clk3=temp;
                
                for(int i=1; i<dmisc5; i++)
                {
                    nxoffset[i] = (rand()%3);
                    nyoffset[i] = (rand()%3);
                }
            }
            
            break;
            
        case 1:                                                 // flying head
            if(clk>=0)
            
            {
                variable_walk_8(rate,homing,hrate,spw_floater);
            }
            
            break;
            
            // the following are messages sent from the main guy...
        case -1:                                                // got chopped off
        {
            misc=1;
            superman=1;
            hxofs=0;
            xofs=0;
            hxsz=16;
            cs=8;
            clk=-24;
            clk2=40;
            dir=(rand()&7)+8;
            step=8.0/9.0;
        }
        break;
        
        case -2:                                                // the big guy is dead
            return true;
        }
        
        updateFiring();
        
        return enemy::animate(index);
    }

    int takehit(weapon@ w)
    {
        int ret = enemy::takehit(w);
        
        if(ret==-1)
            return 2; // force it to wait a frame before checking sword attacks again
            
        return ret;
    }

    void Draw()
    {
        switch(misc)
        {
        case 0:                                                 //neck
            if(!dont_draw())
            {
                for(int i=1; i<dmisc5; i++)                              //draw the neck
                {
                    if(questRuleEnabled(qr_NEWENEMYTILES))
                    {
                        if(screenFlagSet(fINVISROOM) && current_item(itype_amulet)==0)
                            overtilecloaked16(necktile+(i*dmisc7),nx[i]-4,ny[i]+playing_field_offset,0);
                        else
                            overtile16(necktile+(i*dmisc7),nx[i]-4,ny[i]+playing_field_offset,cs,0);
                    }
                    else
                    {
                        if(screenFlagSet(fINVISROOM) && current_item(itype_amulet)==0)
                            overtilecloaked16(necktile,nx[i]-4,ny[i]+playing_field_offset,0);
                        else
                            overtile16(necktile,nx[i]-4,ny[i]+playing_field_offset,cs,0);
                    }
                }
            }
            
            break;
            
        case 1:                                                 //flying head
            tile=flyingheadtile;
            
            if(questRuleEnabled(qr_NEWENEMYTILES))
            {
                tile+=((clk&24)>>3);
                break;
            }
        }
    }

    void Draw2()
    {
        enemy::draw();
    }
}
