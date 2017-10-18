class NPC: enemy
{
    NPC()
    {
        o_tile+=getSpriteTile(iwNPCs);
        count_enemy=false;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        switch(dmisc2)
        {
        case 0:
        {
            double ddir=atan2(y-Link.y, Link.x-x);
            
            if((ddir<=(((-1)*PI)/4))&&(ddir>(((-3)*PI)/4)))
            {
                dir=down;
            }
            
            else if((ddir<=(((1)*PI)/4))&&(ddir>(((-1)*PI)/4)))
            {
                dir=right;
            }
            else if((ddir<=(((3)*PI)/4))&&(ddir>(((1)*PI)/8)))
            {
                dir=up;
            }
            else
            {
                dir=left;
            }
        }
        break;
        
        case 1:
            halting_walk(rate, homing, 0, hrate, 48);
            
            if(clk2==1 && (misc < dmisc1) && (rand()&15)==0)
            {
                newdir(rate, homing, 0);
                clk2=48;
                misc+=1;
            }
            
            if(clk2==0)
                misc=0;
                
            break;
        }
        
        return enemy::animate(index);
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
