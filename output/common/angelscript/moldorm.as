class MoldormController: enemy
{
    int segcnt;
    int segid;
    
    MoldormController()
    {
        x=128;
        y=48;
        dir=(rand()&7)+8;
        superman=1;
        fading=fade_invisible;
        hxofs=1000;
        segcnt=clk;
        segid=id|0x1000;
        clk=0;
        id=numGuys();
        yofs=playing_field_offset;
        tile=o_tile;
    }

    bool Update(int index)
    {
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(clk2!=0)
        {
            clk2-=1;
            if(clk2 == 0)
            {
                if((flags&guy_neverret)!=0)
                    never_return(index);
                    
                if(dmisc2==0)
                    leave_item();
                    
                stopBGSFX();
                return true;
            }
        }
        else
        {
            constant_walk_8(rate, homing, spw_floater);
            misc=dir;
            
            // If any higher-numbered segments were killed, segcnt can be too high,
            // leading to a crash
            if(index+segcnt>=numGuys())
                segcnt=numGuys()-index-1;
            
            for(int i=index+1; i<index+segcnt+1; i++)
            {
                enemy@ segment=getGuy(i);
                
                // More validation - if segcnt was wrong, this may not
                // actually be a Moldorm segment
                if(segment.id!=segid)
                {
                    segcnt=i-index-1;
                    break;
                }
                
                if(i==index+1)
                {
                    x=segment.x;
                    y=segment.y;
                }
                
                segment.o_tile=tile;
                
                if((i==index+segcnt)&&(i!=index+1))                   //tail
                {
                    segment.dummy_int1=2;
                }
                else
                {
                    segment.dummy_int1=1;
                }
                
                if(i==index+1)                                        //head
                {
                    segment.dummy_int1=0;
                }
                
                if(segment.hp <= 0)
                {
                    int offset=1;
                    
                    for(int j=i; j<index+segcnt; j++)
                    {
                        // Triple-check
                        enemy@ segmentJPlus1=getGuy(j);
                        if(segmentJPlus1.id!=segid)
                        {
                            segcnt=j-index+1; // Add 1 because of --segcnt below
                            break;
                        }
                        
                        enemy@ segmentJ=getGuy(j+1);
                        
                        int temp=segmentJ.hp;
                        segmentJ.hp=segmentJPlus1.hp;
                        segmentJPlus1.hp=temp;
                        
                        temp=segmentJ.hclk;
                        segmentJ.hclk=segmentJPlus1.hclk;
                        segmentJPlus1.hclk=temp;
                    }
                    
                    segment.hclk=33;
                    --segcnt;
                    --i; // Recheck the same index in case multiple segments died at once
                }
            }
            
            if(segcnt==0)
            {
                clk2=19;
                enemy@ segment=getGuy(index);
                x=segment.x;
                y=segment.y;
            }
        }
        
        return false;
    }
}

class MoldormSegment: enemy
{
    int parentclk;
    
    MoldormSegment()
    {
        x=128;
        y=48;
        yofs=playing_field_offset;
        hyofs=4;
        hxsz=8;
        hysz=8;
        hxofs=1000;
        mainguy=false;
        count_enemy=false;
        parentclk=0;
        bgsfx=-1;
        flags&=~guy_neverret;
    }

    bool Update(int index)
    {
        // Shouldn't be possible, but better to be sure
        if(index==0)
            dying=true;
        
        if(dying)
        {
            if(dmisc2==0)
                item_set=0;
                
            return Dead(index);
        }
        
        if(clk>=0)
        {
            enemy@ en=getGuy(index-1);
            hxofs=4;
            step=en.step;
            
            if(parentclk == 0)
            {
                misc=dir;
                dir=en.misc;
                //do alignment, as in parent's animation :-/ -DD
                x.vAND(0xFFFF0000);
                y.vAND(0xFFFF0000);
            }
            
            parentclk=(parentclk+1)%(int(8.0/step));
            
            if(!watch)
            {
                sprite::move(step);
            }
        }
        
        return enemy::animate(index);
    }

    int takehit(weapon@ w)
    {
        if(enemy::takehit(w)!=0)
            return (w.id==wSBomb) ? 1 : 2;                         // force it to wait a frame before checking sword attacks again
            
        return 0;
    }

    void Draw()
    {
        tile=o_tile;
        int fdiv = frate/4;
        int efrate = fdiv == 0 ? 0 : clk/fdiv;
        
        int f2=questRuleEnabled(qr_NEWENEMYTILES)?
               efrate:((clk>=(frate>>1))?1:0);
               
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            tile+=dummy_int1*40;
            
            if(dir<8)
            {
                flip=0;
                tile+=4*dir;
                
                if(dir>3) // Skip to the next row for diagonals
                    tile+=4;
            }
            else
            {
                switch(dir-8)                                           //directions get screwed up after 8.  *shrug*
                {
                case up:                                              //u
                    flip=0;
                    break;
                    
                case l_up:                                            //d
                    flip=0;
                    tile+=4;
                    break;
                    
                case l_down:                                          //l
                    flip=0;
                    tile+=8;
                    break;
                    
                case left:                                            //r
                    flip=0;
                    tile+=12;
                    break;
                    
                case r_down:                                          //ul
                    flip=0;
                    tile+=20;
                    break;
                    
                case down:                                            //ur
                    flip=0;
                    tile+=24;
                    break;
                    
                case r_up:                                            //dl
                    flip=0;
                    tile+=28;
                    break;
                    
                case right:                                           //dr
                    flip=0;
                    tile+=32;
                    break;
                }
            }
            
            tile+=f2;
        }
        
        if(clk>=0)
            enemy::draw();
    }
}
