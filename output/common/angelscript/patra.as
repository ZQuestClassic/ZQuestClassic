class Patra: enemy
{
    int flycnt;
    int flycnt2;
    int loopcnt;
    int lookat;
    double circle_x;
    double circle_y;
    double temp_x;
    double temp_y;
    bool adjusted;
    
    Patra()
    {
        x=128;
        y=48;
        adjusted=false;
        dir=(rand()&7)+8;
        flycnt=dmisc1;
        flycnt2=dmisc2;
        loopcnt=0;
        
        if(dmisc6<1)
            dmisc6=1; // ratio cannot be 0!
        
        if(flycnt!=0 || flycnt2!=0)
            scriptFlags=asf_PATRADEFENSE;
    }

    bool Update(int index)
    {
        if(dying)
        {
            for(int i=index+1; i<index+flycnt+flycnt2+1; i++)
            {
                getGuy(i).hp = -1000;
            }
            
            return Dead(index);
        }
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        variable_walk_8(rate, homing, hrate, spw_floater);
        
        clk2+=1;
        if(clk2>84)
        {
            clk2=0;
            
            if(loopcnt!=0)
                --loopcnt;
            else
            {
                if((misc%dmisc6)==0)
                    loopcnt=dmisc7;
            }
            
            misc+=1;
        }
        
        double size=1;
        
        for(int i=index+1; i<index+flycnt+1; i++)
        {
            enemy@ orbiter=getGuy(i);
            
            //outside ring
            if(!adjusted)
            {
                if(questRuleEnabled(qr_NEWENEMYTILES))
                {
                    orbiter.o_tile=o_tile+dmisc8;
                }
                else
                {
                    orbiter.o_tile=o_tile+1;
                }
                
                orbiter.cs=dmisc9;
                orbiter.hp=dmisc3;
            }
            
            if(orbiter.hp <= 0)
            {
                for(int j=i; j<index+flycnt+flycnt2; j++)
                {
                    swapGuys(j, j+1);
                }
                
                --flycnt;
                checkOrbiters();
            }
            else
            {
                int pos2 = orbiter.misc;
                double a2 = (clk2-pos2*84.0/(dmisc1 == 0 ? 1 : dmisc1))*PI/42;
                
                if(dmisc4==0)
                {
                    //maybe playing_field_offset here?
                    if(loopcnt>0)
                    {
                        orbiter.x =  cos(a2+PI/2)*56*size - sin(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1))*28*size;
                        orbiter.y = -sin(a2+PI/2)*56*size + cos(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1))*28*size;
                    }
                    else
                    {
                        orbiter.x =  cos(a2+PI/2)*28*size;
                        orbiter.y = -sin(a2+PI/2)*28*size;
                    }
                    
                    temp_x=orbiter.x;
                    temp_y=orbiter.y;
                }
                else
                {
                    circle_x =  cos(a2+PI/2)*42;
                    circle_y = -sin(a2+PI/2)*42;
                    
                    if(loopcnt>0)
                    {
                        orbiter.x =  cos(a2+PI/2)*42;
                        orbiter.y = (-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1)))*21;
                    }
                    else
                    {
                        orbiter.x = circle_x;
                        orbiter.y = circle_y;
                    }
                    
                    temp_x=circle_x;
                    temp_y=circle_y;
                }
                
                double ddir=atan2(temp_y, temp_x);
                
                if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
                {
                    orbiter.dir=l_down;
                }
                else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
                {
                    orbiter.dir=left;
                }
                else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
                {
                    orbiter.dir=l_up;
                }
                else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
                {
                    orbiter.dir=up;
                }
                else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
                {
                    orbiter.dir=r_up;
                }
                else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
                {
                    orbiter.dir=right;
                }
                else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
                {
                    orbiter.dir=r_down;
                }
                else
                {
                    orbiter.dir=down;
                }
                
                orbiter.x += x;
                orbiter.y += y;
            }
        }
        
        if(dmisc5==1)
        {
            if((rand()&127)==0)
                fireWeapon();
        }
        
        size=.5;
        
        if(flycnt2!=0)
        {
            for(int i=index+flycnt+1; i<index+flycnt+flycnt2+1; i++)//inner ring
            {
                enemy@ orbiter=getGuy(i);
                
                if(!adjusted)
                {
                    orbiter.hp=12*DAMAGE_MULTIPLIER;
                    
                    if(questRuleEnabled(qr_NEWENEMYTILES))
                    {
                        switch(dmisc5)
                        {
                            // Center eye shoots projectiles; make room for its firing tiles
                        case 1:
                            orbiter.o_tile=o_tile+120;
                            break;
                            
                            // Center eyes does not shoot; use tiles two rows below for inner eyes.
                        case 2:
                        default:
                            orbiter.o_tile=o_tile+40;
                            break;
                        }
                    }
                    else
                    {
                        orbiter.o_tile=o_tile+1;
                    }
                    
                    orbiter.cs=dmisc9;
                }
                
                if(flycnt>0)
                {
                    orbiter.superman=1;
                }
                else
                {
                    orbiter.superman=0;
                }
                
                if(orbiter.hp <= 0)
                {
                    for(int j=i; j<index+flycnt+flycnt2; j++)
                    {
                        swapGuys(j,j+1);
                    }
                    
                    --flycnt2;
                    checkOrbiters();
                }
                else
                {
                    if(dmisc5==2)
                    {
                        if((rand()&127)==0)
                        {
                            // This is stupid...
                            setAttackOwner(i);
                            fireWeapon();
                            // No reason to set the owner back
                        }
                    }
                    
                    int pos2 = orbiter.misc;
                    double a2 = ((clk2-pos2*84/(dmisc2==0 ? 1 : dmisc2))*PI/(42));
                    
                    if(dmisc4==0)
                    {
                        if(loopcnt>0)
                        {
                            orbiter.x =  cos(a2+PI/2)*56*size - sin(pos2*PI*2/(dmisc2==0? 1 : dmisc2))*28*size;
                            orbiter.y = -sin(a2+PI/2)*56*size + cos(pos2*PI*2/(dmisc2==0?1:dmisc2))*28*size;
                        }
                        else
                        {
                            orbiter.x =  cos(a2+PI/2)*28*size;
                            orbiter.y = -sin(a2+PI/2)*28*size;
                        }
                        
                        temp_x=orbiter.x;
                        temp_y=orbiter.y;
                    }
                    else
                    {
                        circle_x =  cos(a2+PI/2)*42*size;
                        circle_y = -sin(a2+PI/2)*42*size;
                        
                        if(loopcnt>0)
                        {
                            orbiter.x =  cos(a2+PI/2)*42*size;
                            orbiter.y = (-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc2 == 0 ? 1 : dmisc2)))*21*size;
                        }
                        else
                        {
                            orbiter.x = circle_x;
                            orbiter.y = circle_y;
                        }
                        
                        temp_x=circle_x;
                        temp_y=circle_y;
                    }
                    
                    double ddir=atan2(temp_y, temp_x);
                    
                    if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
                    {
                        orbiter.dir=l_down;
                    }
                    else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
                    {
                        orbiter.dir=left;
                    }
                    else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
                    {
                        orbiter.dir=l_up;
                    }
                    else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
                    {
                        orbiter.dir=up;
                    }
                    else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
                    {
                        orbiter.dir=r_up;
                    }
                    else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
                    {
                        orbiter.dir=right;
                    }
                    else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
                    {
                        orbiter.dir=r_down;
                    }
                    else
                    {
                        orbiter.dir=down;
                    }
                    
                    orbiter.x += x;
                    orbiter.y = y-orbiter.y;
                    
                }
            }
        }
        
        adjusted=true;
        return enemy::animate(index);
    }

    void Draw()
    {
        tile=o_tile;
        update_enemy_frame();
        enemy::draw();
    }

    void checkOrbiters()
    {
        if(flycnt==0 && flycnt2==0)
            // All orbiters are dead - no longer invincible
            scriptFlags=0;
    }
}

class PatraOrbiter: enemy
{
    PatraOrbiter()
    {
        item_set=0;
        misc=clk;
        clk = -((misc*21)>>1)-1;
        yofs=playing_field_offset;
        hyofs=2;
        hxsz=12;
        hysz=12;
        hxofs=2;
        mainguy=false;
        count_enemy=false;
        bgsfx=-1;
        flags &= (~guy_neverret);
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        return enemy::animate(index);
    }

    void Draw()
    {
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            tile = o_tile+(clk&3);
            
            switch(dir)                                             //directions get screwed up after 8.  *shrug*
            {
            case up:                                              //u
                flip=0;
                break;
                
            case down:                                            //d
                flip=0;
                tile+=4;
                break;
                
            case left:                                            //l
                flip=0;
                tile+=8;
                break;
                
            case right:                                           //r
                flip=0;
                tile+=12;
                break;
                
            case l_up:                                            //ul
                flip=0;
                tile+=20;
                break;
                
            case r_up:                                            //ur
                flip=0;
                tile+=24;
                break;
                
            case l_down:                                          //dl
                flip=0;
                tile+=28;
                break;
                
            case r_down:                                          //dr
                flip=0;
                tile+=32;
                break;
            }
        }
        else
        {
            tile = o_tile+((clk&2)>>1);
        }
        
        if(clk>=0)
            enemy::draw();
    }
}

