class Ghini: enemy
{
    int clk4;
    
    Ghini()
    {
        scriptFlags= asf_ALWAYSDIE;
        
        fading=fade_flicker;
        count_enemy=false;
        dir=12;
        movestatus=1;
        step=0;
        clk=0;
        clk4=0;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(dmisc1!=0)
        {
            if(misc!=0)
            {
                if(clk4>160)
                    misc=2;
                    
                floater_walk((misc==1)?0:rate,hrate,dstep/100.0,dstep/1000.0,10,120,10);
                removearmos(x,y);
            }
            else if(clk4>=60)
            {
                misc=1;
                clk3=32;
                fading=0;
                setGuyGridAt(x, y, 0);
                removearmos(x,y);
            }
        }
        
        clk4++;
        
        return enemy::animate(index);
    }

    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }
}

