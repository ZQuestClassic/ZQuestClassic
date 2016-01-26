class TrapLOS: enemy
{
    int ox2;
    int oy2;
    
    TrapLOS()
    {
        ox2=x;                                                     //original x
        oy2=y;                                                     //original y
        
        if(questRuleEnabled(qr_TRAPPOSFIX))
        {
            yofs = playing_field_offset;
        }
        
        mainguy=false;
        count_enemy=false;
        dummy_int1=0;
    }

    bool Update(int index)
    {
        if(clk<0)
            return enemy::animate(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(misc==0)                                               // waiting
        {
            ox2 = x;
            oy2 = y;
            double ddir=atan2(y-Link.y, Link.x-x);
            
            if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
            {
                dir=down;
            }
            else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
            {
                dir=right;
            }
            else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/4)))
            {
                dir=up;
            }
            else
            {
                dir=left;
            }
            
            int d2=lined_up(15,true);
            
            if(((d2<left || d2 > right) && (dmisc1==1)) ||
                    ((d2>down) && (dmisc1==2)) ||
                    ((d2>right) && (dmisc1==0)) ||
                    ((d2<l_up) && (dmisc1==4)) ||
                    ((d2!=r_up) && (d2!=l_down) && (dmisc1==6)) ||
                    ((d2!=l_up) && (d2!=r_down) && (dmisc1==8)))
            {
                d2=-1;
            }
            
            if(d2!=-1 && trapmove(d2))
            {
                dir=d2;
                misc=1;
                clk2=(dir==down)?3:0;
            }
        }
        
        if(misc==1)                                               // charging
        {
            clk2=(clk2+1)&3;
            step=(clk2==3)?1:2;
            
            if(!trapmove(dir) || clip())
            {
                misc=2;
                
                if(dir<l_up)
                {
                    dir=dir^1;
                }
                else
                {
                    dir=dir^3;
                }
            }
            else
            {
                sprite::move(step);
            }
        }
        
        if(misc==2)                                               // retreating
        {
            clk2+=1;
            step=((clk2&1)!=0)?1:0;
            bool trap_rest=false;
            
            switch(dir)
            {
            case up:
                if(int(y)<=oy2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case left:
                if(int(x)<=ox2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case down:
                if(int(y)>=oy2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case right:
                if(int(x)>=ox2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case l_up:
                if(int(x)<=ox2 && int(y)<=oy2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case r_up:
                if(int(x)>=ox2 && int(y)<=oy2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case l_down:
                if(int(x)<=ox2 && int(y)>=oy2) trap_rest=true;
                else sprite::move(step);
                
                break;
                
            case r_down:
                if(int(x)>=ox2 && int(y)>=oy2) trap_rest=true;
                else sprite::move(step);
                
                break;
            }
            
            if(trap_rest)
            {
                x=ox2;
                y=oy2;
                misc=0;
            }
        }
        
        return enemy::animate(index);
    }

    bool trapmove(int ndir)
    {
        if(questRuleEnabled(qr_MEANTRAPS))
        {
            if(screenFlagSet(fFLOATTRAPS))
                return canmove(ndir, 1, spw_floater, 0, 0, 15, 15);
                
            return canmove(ndir, 1, spw_water, 0, 0, 15, 15);
        }
        
        if(oy2==80 && !(ndir==left || ndir == right))
            return false;
            
        if(ox2==128 && !(ndir==up || ndir==down))
            return false;
            
        if(oy2<80 && ndir==up)
            return false;
            
        if(oy2>80 && ndir==down)
            return false;
            
        if(ox2<128 && ndir==left)
            return false;
            
        if(ox2>128 && ndir==right)
            return false;
            
        if(ox2<128 && oy2<80 && ndir==l_up)
            return false;
            
        if(ox2<128 && oy2>80 && ndir==l_down)
            return false;
            
        if(ox2>128 && oy2<80 && ndir==r_up)
            return false;
            
        if(ox2>128 && oy2>80 && ndir==r_down)
            return false;
            
        return true;
    }

    bool clip()
    {
        if(questRuleEnabled(qr_MEANPLACEDTRAPS))
        {
            switch(dir)
            {
            case up:
                if(y<=0)           return true;
                
                break;
                
            case down:
                if(y>=160)         return true;
                
                break;
                
            case left:
                if(x<=0)           return true;
                
                break;
                
            case right:
                if(x>=240)         return true;
                
                break;
                
            case l_up:
                if(y<=0||x<=0)     return true;
                
                break;
                
            case l_down:
                if(y>=160||x<=0)   return true;
                
                break;
                
            case r_up:
                if(y<=0||x>=240)   return true;
                
                break;
                
            case r_down:
                if(y>=160||x>=240) return true;
                
                break;
            }
            
            return false;
        }
        else
        {
            switch(dir)
            {
            case up:
                if(oy2>80 && y<=86) return true;
                
                break;
                
            case down:
                if(oy2<80 && y>=80) return true;
                
                break;
                
            case left:
                if(ox2>128 && x<=124) return true;
                
                break;
                
            case right:
                if(ox2<120 && x>=116) return true;
                
                break;
                
            case l_up:
                if(oy2>80 && y<=86 && ox2>128 && x<=124) return true;
                
                break;
                
            case l_down:
                if(oy2<80 && y>=80 && ox2>128 && x<=124) return true;
                
                break;
                
            case r_up:
                if(oy2>80 && y<=86 && ox2<120 && x>=116) return true;
                
                break;
                
            case r_down:
                if(oy2<80 && y>=80 && ox2<120 && x>=116) return true;
                
                break;
            }
            
            return false;
        }
    }

    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }

    int takehit(weapon@ w)
    {
        return 0;
    }
}
