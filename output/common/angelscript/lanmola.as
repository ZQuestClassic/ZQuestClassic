class LanmolaBase: enemy
{
    array<fix> prevStateX;
    array<fix> prevStateY;
    array<int> prevStateDir;
    
    void pushFrontX(fix x)
    {
        prevStateX.insertAt(0, x);
    }
    
    void pushBackX(fix x)
    {
        prevStateX.insertLast(x);
    }
    
    fix getFrontX()
    {
        return prevStateX[0];
    }
    
    fix popFrontX()
    {
        fix ret=prevStateX[0];
        prevStateX.removeAt(0);
        return ret;
    }
    
    void pushFrontY(fix y)
    {
        prevStateY.insertAt(0, y);
    }
    
    void pushBackY(fix y)
    {
        prevStateY.insertLast(y);
    }
    
    fix getFrontY()
    {
        return prevStateY[0];
    }
    
    fix popFrontY()
    {
        fix ret=prevStateY[0];
        prevStateY.removeAt(0);
        return ret;
    }
    
    void pushFrontDir(int dir)
    {
        prevStateDir.insertAt(0, dir);
    }
    
    void pushBackDir(int dir)
    {
        prevStateDir.insertLast(dir);
    }
    
    int getFrontDir()
    {
        return prevStateDir[0];
    }
    
    int popFrontDir()
    {
        int ret=prevStateDir[0];
        prevStateDir.removeAt(0);
        return ret;
    }
}

class LanmolaController: LanmolaBase
{
    int segcnt;
    
    LanmolaController()
    {
        x=64;
        y=80;
        dir=up;
        superman=1;
        fading=fade_invisible;
        hxofs=1000;
        segcnt=clk;
        clk=0;
        
        //set up move history
        for(int i=0; i <= (1<<dmisc2); i++)
        {
            pushBackX(x);
            pushBackY(y);
            pushBackDir(dir);
        }
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
                if(dmisc3==0)
                    leave_item();
                
                stopBGSFX();
                return true;
            }
            
            return false;
        }
        
        
        //this animation style plays ALL KINDS of havoc on the Lanmola segments, since it causes
        //the direction AND x,y position of the lanmola to vary in uncertain ways.
        //I've added a complete movement history to this enemy to compensate -DD
        constant_walk(rate, homing, spw_none);
        popFrontX();
        popFrontY();
        popFrontDir();
        pushFrontX(x);
        pushFrontY(y);
        pushFrontDir(dir);
        //prevState.pop_front();
        //prevState.push_front(std::pair<std::pair<fix, fix>, int>(std::pair<fix, fix>(x,y), dir));
        
        // This could cause a crash with Moldorms. I didn't see the same problem
        // with Lanmolas, but it looks like it ought to be possible, so here's
        // the same solution. - Saf
        if(index+segcnt>=numGuys())
            segcnt=numGuys()-index-1;
        
        for(int i=index+1; i<index+segcnt+1; i++)
        {
            enemy@ segment=getGuy(i);
            
            // More validation in case segcnt is wrong
            if((segment.id&0xFFF)!=(id&0xFFF))
            {
                segcnt=i-index-1;
                break;
            }
            
            segment.o_tile=o_tile;
            
            if((i==index+segcnt)&&(i!=index+1))
            {
                segment.dummy_int1=1;                //tail
            }
            else
            {
                segment.dummy_int1=0;
            }
            
            if(segment.hp <= 0)
            {
                for(int j=i; j<index+segcnt; j++)
                {
                    // Triple-check
                    enemy@ segmentJPlus1=getGuy(j+1);
                    if((segmentJPlus1.id&0xFFF)!=(id&0xFFF))
                    {
                        segcnt=j-index+1; // Add 1 because of --segcnt below
                        break;
                    
                    }
                    enemy@ segmentJ=getGuy(j);
                    
                    int temp=segmentJ.hp;
                    segmentJ.hp=segmentJPlus1.hp;
                    segmentJPlus1.hp=temp;
                    
                    temp=segmentJ.hclk;
                    segmentJ.hclk=segmentJPlus1.hclk;
                    segmentJPlus1.hclk=temp;
                }
                
                enemy@ segmentI=getGuy(i);
                segmentI.hclk=33;
                --segcnt;
                --i; // Recheck the same index in case multiple segments died at once
            }
        }
        
        if(segcnt==0)
        {
            clk2=19;
            enemy@ segment=getGuy(index+1);
            x=segment.x;
            y=segment.y;
            setmapflag(mTMPNORET);
        }
        
        //this enemy is invincible.. BUT scripts don't know that, and can "kill" it by setting the hp negative.
        //which is... disastrous.
        hp = 1;
        return enemy::animate(index);
    }
}

class LanmolaSegment: LanmolaBase
{
    LanmolaSegment()
    {
        x=64;
        y=80;
        hxofs=1000;
        hxsz=8;
        mainguy=false;
        count_enemy=(id<0x2000)?true:false;
        
        //set up move history
        for(int i=0; i <= (1<<dmisc2); i++)
        {
            pushBackX(x);
            pushBackY(y);
            pushBackDir(dir);
        }
            
        bgsfx=-1;
        flags&=~guy_neverret;
    }

    bool Update(int index)
    {
        // Shouldn't be possible, but who knows
        if(index==0)
            dying=true;
        
        if(dying)
        {
            xofs=0;
            
            if(dmisc3==0)
                item_set=0;
                
            return Dead(index);
        }
        
        if(clk>=0)
        {
            hxofs=4;
            
            if(!watch)
            {
                // TODO: Maybe try not to crash if the previous enemy isn't actually a Lanmola part for some reason
                LanmolaBase@ prev=cast<LanmolaBase@>(getGuy(index-1));
                x=prev.getFrontX();
                y=prev.getFrontY();
                dir=prev.getFrontDir();
                popFrontX();
                popFrontY();
                popFrontDir();
                pushBackX(x);
                pushBackY(y);
                pushBackDir(dir);
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
            if(id>=0x2000)
            {
                tile+=20;
                
                if(dummy_int1==1)
                {
                    tile+=20;
                }
            }
            
            switch(dir)
            {
            case up:
                flip=0;
                break;
                
            case down:
                flip=0;
                tile+=4;
                break;
                
            case left:
                flip=0;
                tile+=8;
                break;
                
            case right:
                flip=0;
                tile+=12;
                break;
            }
            
            tile+=f2;
        }
        else
        {
            if(id>=0x2000)
            {
                tile+=1;
            }
        }
        
        if(clk>=0)
            enemy::draw();
    }
}
