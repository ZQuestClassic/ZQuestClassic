class ProjectileShooter: enemy
{
    int minRange;
    
    ProjectileShooter()
    {
        mainguy=false;
        count_enemy=false;
        minRange=clk;                                                 // the "no fire" range
        clk=0;
        clk3=96;
        
        if(o_tile==0)
        {
            superman=1;
            hxofs=1000;
        }
    }

    bool Update(int index)
    {
        if(clk==0)
        {
            removearmos(x,y);
        }
        
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
        
        clk3+=1;
        if(clk3>80)
        {
            if(dmisc1==9) // Breath type
            {
                if(isFiring())
                {
                    updateFiring();
                    if(!isFiring())
                        clk3=0;
                }
                else if((rand()&63)==0)
                    fireWeapon();
            }
            
            else // Not breath type
            {
                int r=rand();
                
                if((r&63)==0 && !LinkInRange(minRange))
                {
                    fireWeapon();
                
                    if(wpn==ewFireball || wpn==ewFireball2) // Shouldn't it check that it's single-shot, too?
                    {
                        if((r&(15<<7))==0) // Is this right? Seems too frequent.
                        {
                            x-=4;
                            fireWeapon();
                            x+=4;
                        }
                    }
                    clk3=0;
                }
            }
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }
}

