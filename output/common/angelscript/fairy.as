class Fairy: enemy
{
    Fairy()
    {
        step=getGuyProp(id, GUYPROP_STEP)/100;
        superman=1;
        dir=8;
        hxofs=1000;
        mainguy=false;
        count_enemy=false;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        misc=1;
        bool w=watch;
        watch=false;
        variable_walk_8((misc!=0) ? 3 : 0, 0, 8, spw_floater);
        watch=w;
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        // Nothin'.
    }
}
