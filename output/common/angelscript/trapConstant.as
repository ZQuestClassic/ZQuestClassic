class TrapConstant: enemy
{
    TrapConstant()
    {
        lasthit=-1;
        lasthitclk=0;
        mainguy=false;
        count_enemy=false;
        step=2;
        
        if(dmisc1==1 || (dmisc1==0 && (rand()&2)!=0))
        {
            dir=(x<=112)?right:left;
        }
        else
        {
            dir=(y<=72)?down:up;
        }
        
        if(questRuleEnabled(qr_TRAPPOSFIX))
        {
            yofs = playing_field_offset;
        }
        
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
        
        if(!questRuleEnabled(qr_PHANTOMPLACEDTRAPS))
        {
            if(lasthitclk>0)
            {
                lasthitclk-=1;
            }
            else
            {
                lasthit=-1;
            }
            
            bool hitenemy=false;
            
            for(int j=0; j<numGuys(); j++)
            {
                if((j!=index) && (lasthit!=j))
                {
                    enemy@ en=getGuy(j);
                    if(hit(en))
                    {
                        lasthit=j;
                        lasthitclk=10;
                        hitenemy=true;
                        en.lasthit=index;
                        en.lasthitclk=10;
                    }
                }
            }
            
            if(!trapmove(dir) || clip() || hitenemy)
            {
                if(!trapmove(dir) || clip())
                {
                    lasthit=-1;
                    lasthitclk=0;
                }
                
                if(questRuleEnabled(qr_MORESOUNDS))
                    sfx(WAV_ZN1TAP, x);
                    
                dir=dir^1;
            }
            
            sprite::move(step);
        }
        else
        {
            if(!trapmove(dir) || clip())
            {
                if(questRuleEnabled(qr_MORESOUNDS))
                    sfx(WAV_ZN1TAP, x);
                    
                dir=dir^1;
            }
            
            sprite::move(step);
        }
        
        return enemy::animate(index);
    }

    bool trapmove(int ndir)
    {
        if(screenFlagSet(fFLOATTRAPS))
            return canmove(ndir, 1, spw_floater, 0, 0, 15, 15);
            
        return canmove(ndir, 1, spw_water, 0, 0, 15, 15);
    }

    bool clip()
    {
        switch(dir)
        {
        case up:
            if(y<=0)
                return true;
            break;
            
        case down:
            if(y>=160)
                return true;
            break;
            
        case left:
            if(x<=0)
                return true;
            break;
            
        case right:
            if(x>=240)
                return true;
            break;
        }
        
        return false;
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

