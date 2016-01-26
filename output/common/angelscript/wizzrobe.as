class Wizzrobe: enemy
{
    bool charging;
    bool firing;
    int fclk;
    
    Wizzrobe()
    {
        switch(dmisc1)
        {
        case 0:
            hxofs=1000;
            fading=fade_invisible;
            // Set clk to just before the 'reappear' threshold
            clk=zc_min(clk+(146+zc_max(0,dmisc5))+14,(146+zc_max(0,dmisc5))-1);
            break;
            
        default:
            dir=(loadside==right)?right:left;
            misc=-3;
            break;
        }
        
        charging=false;
        firing=false;
        fclk=0;
        
        if(dmisc1==0) frate=1200+146; //1200 = 20 seconds
    }

    bool Update(int index)
    {
        if(dying)
        {
            return Dead(index);
        }
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(dmisc1!=0) // Floating
        {
            wizzrobe_attack();
        }
        else // Teleporting
        {
            if(watch)
            {
                fading=0;
                hxofs=0;
            }
            else switch(clk)
                {
                case 0:
                    if(dmisc2!=0)
                    {
                        place_on_axis(true, dmisc4!=0);
                    }
                    else
                    {
                        int t=0;
                        bool placed=false;
                        
                        while(!placed && t<160)
                        {
                            if(isdungeon())
                            {
                                x=((rand()%12)+2)*16;
                                y=((rand()%7)+2)*16;
                            }
                            else
                            {
                                x=((rand()%14)+1)*16;
                                y=((rand()%9)+1)*16;
                            }
                            
                            if(!m_walkflag(x,y,spw_door)&&((abs(x-Link.x)>=32)||(abs(y-Link.y)>=32)))
                            {
                                placed=true;
                            }
                            
                            ++t;
                        }
                        
                        if(abs(x-Link.x)<abs(y-Link.y))
                        {
                            if(y<Link.y)
                            {
                                dir=down;
                            }
                            else
                            {
                                dir=up;
                            }
                        }
                        else
                        {
                            if(x<Link.x)
                            {
                                dir=right;
                            }
                            else
                            {
                                dir=left;
                            }
                        }
                        
                        if(!placed)                                       // can't place him, he's gone
                            return true;
                    }
                    
                    fading=fade_flicker;
                    hxofs=0;
                    break;
                    
                case 64:
                    fading=0;
                    charging=true;
                    break;
                    
                case 73:
                    charging=false;
                    firing=true; // 40; // Why was this 40...?
                    break;
                    
                case 83:
                    fireWeapon();
                    break;
                    
                case 119:
                    firing=false;
                    charging=true;
                    break;
                    
                case 128:
                    fading=fade_flicker;
                    charging=false;
                    break;
                    
                case 146:
                    fading=fade_invisible;
                    hxofs=1000;
                    
                    //Fall through
                default:
                    if(clk>=(146+zc_max(0,dmisc5)))
                        clk=-1;
                        
                    break;
                }
        }
        
        return enemy::animate(index);
    }

    void wizzrobe_attack()
    {
        if(clk<0 || dying || stunclk!=0 || watch || ceiling)
            return;
            
        if(clk3<=0 || ((clk3&31)==0 && !canmove(dir, 1, spw_door) && misc==0))
        {
            fix_coords();
            
            switch(misc)
            {
            case 1:                                               //walking
                if(!m_walkflag(x,y,spw_door))
                    misc=0;
                else
                {
                    clk3=16;
                    
                    if(!canmove(dir, 1, spw_wizzrobe))
                    {
                        wizzrobe_newdir(0);
                    }
                }
                
                break;
                
            case 2:                                               //phasing
            {
                int jx=x;
                int jy=y;
                int jdir=-1;
                
                switch(rand()&7)
                {
                case 0:
                    jx-=32;
                    jy-=32;
                    jdir=15;
                    break;
                    
                case 1:
                    jx+=32;
                    jy-=32;
                    jdir=9;
                    break;
                    
                case 2:
                    jx+=32;
                    jy+=32;
                    jdir=11;
                    break;
                    
                case 3:
                    jx-=32;
                    jy+=32;
                    jdir=13;
                    break;
                }
                
                if(jdir>0 && jx>=32 && jx<=208 && jy>=32 && jy<=128)
                {
                    misc=3;
                    clk3=32;
                    dir=jdir;
                    break;
                }
            }
            
            case 3:
                dir&=3;
                misc=0;
                
            case 0:
                wizzrobe_newdir(64);
                
            default:
                if(!canmove(dir, 1, spw_door))
                {
                    if(canmove(dir, 15, spw_wizzrobe))
                    {
                        misc=1;
                        clk3=16;
                    }
                    else
                    {
                        wizzrobe_newdir(64);
                        misc=0;
                        clk3=32;
                    }
                }
                else
                {
                    clk3=32;
                }
                
                break;
            }
            
            if(misc<0)
                misc==1;
        }
        
        clk3-=1;
        
        switch(misc)
        {
        case 1:
        case 3:
            step=1.0;
            break;
            
        case 2:
            step=0;
            break;
            
        default:
            step=0.5;
            break;
            
        }
        
        move(step);
        
        if(dmisc1!=0 && misc<=0 && clk3==28)
        {
            if(dmisc2 != 1)
            {
                if(lined_up(8,false) == dir)
                {
                    fireWeapon();
                    fclk=30;
                }
            }
            else
            {
                if((rand()%500)>=400)
                {
                    fireWeapon();
                    fclk=30;
                }
            }
        }
        
        if(misc==0 && (rand()&127)==0)
            misc=2;
            
        if(misc==2 && clk3==4)
            fix_coords();
            
        if(!(charging||firing))                               //should never be charging or firing for these wizzrobes
        {
            if(fclk>0)
            {
                --fclk;
            }
        }
        
    }

    void wizzrobe_newdir(int homing)
    {
        // Wizzrobes shouldn't move to the edge of the screen;
        // if they're already there, they should move toward the center
        if(x<32)
            dir=right;
        else if(x>=224)
            dir=left;
        else if(y<32)
            dir=down;
        else if(y>=144)
            dir=up;
        else
            newdir(4, homing, spw_wizzrobe);
    }

    void Draw()
    {
        if(dmisc1!=0 && (misc==1 || misc==3) && (clk3&1)!=0 && hp>0 && !watch && stunclk==0)                          // phasing
            return;
            
        int tempint=dummy_int1;
        bool tempbool1=dummy_bool1;
        bool tempbool2=dummy_bool2;
        dummy_int1=fclk;
        dummy_bool1=charging;
        dummy_bool2=firing;
        update_enemy_frame();
        dummy_int1=tempint;
        dummy_bool1=tempbool1;
        dummy_bool2=tempbool2;
        enemy::draw();
    }
}

