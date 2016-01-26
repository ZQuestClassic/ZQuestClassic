class PatraBS: enemy
{
    int flycnt;
    int loopcnt;
    int lookat;
    double temp_x;
    double temp_y;
    bool adjusted;
    
    PatraBS()
    {
        x=128;
        y=48;
        
        adjusted=false;
        dir=(rand()&7)+8;
        step=0.25;
        flycnt=dmisc1;
        loopcnt=0;
        hxsz = 32;
        hxofs=-8;
        
        if(dmisc6<1)
            dmisc6=1; // ratio cannot be 0!
        
        if(flycnt!=0)
            scriptFlags=asf_PATRADEFENSE;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        variable_walk_8(rate, homing, hrate, spw_floater);
        
        clk2+=1;
        if(clk2>90)
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
        
        for(int i=index+1; i<index+flycnt+1; i++)
        {
            enemy@ orbiter=getGuy(i);
            if(!adjusted)
            {
                orbiter.hp=dmisc3;
                
                if(questRuleEnabled(qr_NEWENEMYTILES))
                {
                    orbiter.o_tile=o_tile+dmisc8;
                }
                else
                {
                    orbiter.o_tile=o_tile+1;
                }
                
                orbiter.cs = dmisc9;
            }
            
            if(orbiter.hp <= 0)
            {
                for(int j=i; j<index+flycnt; j++)
                {
                    swapGuys(j,j+1);
                }
                
                --flycnt;
                if(flycnt==0)
                    // No longer invincible
                    scriptFlags=0;
            }
            else
            {
                int pos2 = orbiter.misc;
                double a2 = (clk2-pos2*90/(dmisc1==0?1:dmisc1))*PI/45;
                temp_x =  cos(a2+PI/2)*45;
                temp_y = -sin(a2+PI/2)*45;
                
                if(loopcnt>0)
                {
                    orbiter.x =  cos(a2+PI/2)*45;
                    orbiter.y = (-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc1==0?1:dmisc1)))*22.5;
                }
                else
                {
                    orbiter.x = temp_x;
                    orbiter.y = temp_y;
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
        
        adjusted=true;
        return enemy::animate(index);
    }

    void Draw()
    {
        tile=o_tile;
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            double ddir=atan2(y-Link.y, Link.x-x);
            
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
            
            switch(lookat)                                          //directions get screwed up after 8.  *shrug*
            {
            case up:                                              //u
                flip=0;
                break;
                
            case down:                                            //d
                flip=0;
                tile+=8;
                break;
                
            case left:                                            //l
                flip=0;
                tile+=40;
                break;
                
            case right:                                           //r
                flip=0;
                tile+=48;
                break;
                
            case l_up:                                            //ul
                flip=0;
                tile+=80;
                break;
                
            case r_up:                                            //ur
                flip=0;
                tile+=88;
                break;
                
            case l_down:                                          //dl
                flip=0;
                tile+=120;
                break;
                
            case r_down:                                          //dr
                flip=0;
                tile+=128;
                break;
            }
            
            tile+=(2*(clk&3));
            xofs-=8;
            yofs-=8;
            drawblock();
            xofs+=8;
            yofs+=8;
        }
        else
        {
            flip=(clk&1);
            xofs-=8;
            yofs-=8;
            enemy::draw();
            xofs+=16;
            enemy::draw();
            yofs+=16;
            enemy::draw();
            xofs-=16;
            enemy::draw();
            xofs+=8;
            yofs-=8;
        }
    }
}

class PatraOrbiterBS: enemy
{
    PatraOrbiterBS()
    {
        item_set=0;
        misc=clk;
        clk = -((misc*21)>>1)-1;
        yofs=playing_field_offset;
        hyofs=2;
        hxsz=16;
        hysz=16;
        bgsfx=-1;
        mainguy=false;
        count_enemy=false;
        flags &= ~guy_neverret;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        return enemy::animate(index);
    }

    void Draw()
    {
        tile=o_tile;
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
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
            
            tile += ((clk&6)>>1);
        }
        else
        {
            tile += ((clk&4)!=0)?1:0;
        }
        
        if(clk>=0)
            enemy::draw();
    }
}

