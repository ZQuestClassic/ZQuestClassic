class WallMaster: enemy
{
    //bool haslink;
    bool haslink
    {
        get const { return realEnemy.haslink; }
        set { realEnemy.haslink=value; }
    }
    
    WallMaster()
    {
        haslink=false;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        hxofs=1000;
        
        if(misc==0) //inside wall, ready to spawn?
        {
            if(frame-wallm_load_clk>80 && clk>=0)
            {
                int wall=link_on_wall();
                int wallm_cnt=0;
                
                for(int i=0; i<numGuys(); i++)
                {
                    enemy@ en=getGuy(i);
                    if(en.family==eeWALLM)
                    {
                        int m=en.misc;
                        
                        if(m!=0 && en.clk3==(wall^1))
                        {
                            ++wallm_cnt;
                        }
                    }
                }
                    
                if(wall>0)
                {
                    --wall;
                    misc=1; //emerging from the wall?
                    clk2=0;
                    clk3=wall^1;
                    wallm_load_clk=frame;
                    
                    if(wall<=down)
                    {
                        if(LinkDir()==left)
                            dir=right;
                        else
                            dir=left;
                    }
                    else
                    {
                        if(LinkDir()==up)
                            dir=down;
                        else
                            dir=up;
                    }
                    
                    switch(wall)
                    {
                    case up:
                        y=0;
                        break;
                        
                    case down:
                        y=160;
                        break;
                        
                    case left:
                        x=0;
                        break;
                        
                    case right:
                        x=240;
                        break;
                    }
                    
                    switch(dir)
                    {
                    case up:
                        y=LinkY()+48-(wallm_cnt&1)*12;
                        flip=wall&1;
                        break;
                        
                    case down:
                        y=LinkY()-48+(wallm_cnt&1)*12;
                        flip=((wall&1)^1)+2;
                        break;
                        
                    case left:
                        x=LinkX()+48-(wallm_cnt&1)*12;
                        flip=(wall==up?2:0)+1;
                        break;
                        
                    case right:
                        x=LinkX()-48+(wallm_cnt&1)*12;
                        flip=(wall==up?2:0);
                        break;
                    }
                }
            }
        }
        else
            wallm_crawl();
            
        return enemy::animate(index);
    }

    void wallm_crawl()
    {
        bool w=watch;
        hxofs=0;
        
        if(slide())
        {
            return;
        }
        
        //  if(dying || watch || (!haslink && stunclk))
        if(dying || (!haslink && stunclk!=0))
        {
            return;
        }
        
        watch=false;
        clk2+=1;
        // Misc1: slightly different movement
        misc=(clk2/(dmisc1==1?40:int((40.0/dstep)*40)))+1;
        
        if(w&&misc>=3&&misc<=5)
        {
            clk2-=1;
        }
        
        switch(misc)
        {
        case 1:
        case 2:
        {
            int temp=dir;
            dir=clk3;
            clk3=temp;
            move(step);
            temp=dir;
            dir=clk3;
            clk3=temp;
            break;
        }
        case 3:
        case 4:
        case 5:
            if(w)
            {
                watch=w;
                return;
            }
            
            move(step);
            break;
            
        case 6:
        case 7:
        {
            int temp=dir;
            dir=clk3;
            clk3=temp;
            dir^=1;
            move(step);
            dir^=1;
            temp=dir;
            dir=clk3;
            clk3=temp;
            break;
        }
            
        default:
            misc=0;
            break;
        }
        
        // The transition to AngelScript seems to have affected the movement
        // slightly, suggesting this was very fragile in the first place...
        if(clk3==right && x<0)
            x=0;
        else if(clk3==left && x>240)
            x=240;
        else if(clk3==down && y<0)
            y=0;
        else if(clk3==up && y>160)
            y=160;
        
        watch=w;
    }
    
    // TODO: Currently unused - fix that.
    // This was calle by GrabLink(), which was called by LinkClass::hitlink().
    void grablink()
    {
        haslink=true;
        superman=1;
    }

    void Draw()
    {
        dummy_bool1=haslink;
        update_enemy_frame();
        
        if(misc>0)
        {
            masked_draw(16,playing_field_offset+16,224,144);
        }
    }
    
    int link_on_wall()
    {
        int lx = Link.x;
        int ly = Link.y;
        
        if(lx>=48 && lx<=192)
        {
            if(ly==32)  return up+1;
            
            if(ly==128) return down+1;
        }
        
        if(ly>=48 && ly<=112)
        {
            if(lx==32)  return left+1;
            
            if(lx==208) return right+1;
        }
        
        return 0;
    }
}

