class Leever: enemy
{
    Leever()
    {
        if(dmisc1==0)
        {
            misc=-1;    //Line of Sight leevers
            clk-=16;
        }
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(clk>=0 && !slide())
        {
            switch(dmisc1)
            {
            case 0:      //line of sight
            case 2:
                switch(misc) //is this leever active
                {
                case -1:  //submerged
                {
                    if((dmisc1==2)&&(rand()&255)!=0)
                    {
                        break;
                    }
                    
                    int active=0;
                    
                    for(int i=0; i<numGuys(); i++)
                    {
                        enemy@ en=getGuy(i);
                        if(en.id==id && en.misc>=0)
                        {
                            ++active;
                        }
                    }
                    
                    if(active<((dmisc1==2)?1:2))
                    {
                        misc=0; //activate this one
                    }
                }
                break;
                
                case 0:
                {
                    int s=0;
                    
                    for(int i=0; i<numGuys(); i++)
                    {
                        enemy@ en=getGuy(i);
                        if(en.id==id && en.misc==1)
                        {
                            ++s;
                        }
                    }
                    
                    if(s>0)
                    {
                        break;
                    }
                    
                    int d2=rand()&1;
                    
                    if(LinkDir()>=left)
                    {
                        d2+=2;
                    }
                    
                    if(canplace(d2) || canplace(d2^1))
                    {
                        misc=1;
                        clk2=0;
                        clk=0;
                    }
                }
                break;
                
                case 1:
                    clk2+=1;
                    if(clk2>16) misc=2;
                    
                    break;
                    
                case 2:
                    clk2+=1;
                    if(clk2>24) misc=3;
                    
                    break;
                    
    //        case 3: if(stunclk) break; if(scored) dir^=1; if(!canmove(dir)) misc=4; else move((fix)(d->step/100.0)); break;
                case 3:
                    if(stunclk!=0) break;
                    
                    if(scored) dir^=1;
                    
                    if(!canmove(dir)) misc=4;
                    else move(dstep/100.0);
                    
                    break;
                    
                case 4:
                    clk2-=1;
                    if(clk2==16)
                    {
                        misc=5;
                        clk=8;
                    }
                    
                    break;
                    
                case 5:
                    clk2-=1;
                    if(clk2==0)  misc=((dmisc1==2)?-1:0);
                    
                    break;
                }                                                       // switch(misc)
                
                break;
                
            default:  //random
                step=dmisc3/100.0;
                clk2+=1;
                
                if(clk2<32)    misc=1;
                else if(clk2<48)    misc=2;
                else if(clk2<300)
                {
                    misc=3;
                    step = dstep/100.0;
                }
                else if(clk2<316)   misc=2;
                else if(clk2<412)   misc=1;
                else if(clk2<540)
                {
                    misc=0;
                    step=0;
                }
                else clk2=0;
                
                if(clk2==48) clk=0;
                
                variable_walk(rate, homing, 0);
            }                                                         // switch(dmisc1)
        }
        
        hxofs=(misc>=2)?0:1000;
        return enemy::animate(index);
    }

    bool canplace(int d2)
    {
        int nx=LinkX();
        int ny=LinkY();
        
        if(d2<left) ny&=0xF0;
        else       nx&=0xF0;
        
        switch(d2)
        {
        case up:
            ny-=((dmisc1==0||dmisc1==2)?32:48);
            break;
            
        case down:
            ny+=((dmisc1==0||dmisc1==2)?32:48);
            
            if(ny-LinkY()<32) ny+=((dmisc1==0||dmisc1==2)?16:0);
            
            break;
            
        case left:
            nx-=((dmisc1==0||dmisc1==2)?32:48);
            break;
            
        case right:
            nx+=((dmisc1==0||dmisc1==2)?32:48);
            
            if(nx-LinkX()<32) nx+=((dmisc1==0||dmisc1==2)?16:0);
            
            break;
        }
        
        if(m_walkflag(nx,ny,spw_halfstep)||m_walkflag(nx,ny-8,spw_halfstep))                         /*none*/
            return false;
            
        if(d2>=left)
            if(m_walkflag(LinkX(),LinkY(),spw_halfstep)||m_walkflag(LinkX(),LinkY()-8,spw_halfstep))                         /*none*/
                return false;
                
        x=nx;
        y=ny;
        dir=d2^1;
        return true;
    }

    void Draw()
    {
        cs=dcset;
        update_enemy_frame();
        
        switch(misc)
        {
        case -1:
        case 0:
            return;
        }
        
        enemy::draw();
    }
}
