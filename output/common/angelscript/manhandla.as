class Manhandla: enemy
{
    array<int> headside;
    int headcnt;
    bool adjusted;
    
    Manhandla()
    {
        clk=0;
        
        superman=1;
        dir=(rand()&7)+8;
        headcnt=(dmisc2!=0)?8:4;
        
        for(int i=0; i<headcnt; i++)
            headside.insertLast(i);
            
        fading=fade_blue_poof;
        adjusted=false;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        
        // check arm status, move dead ones to end of group
        for(int i=0; i<headcnt; i++)
        {
            enemy@ head=getGuy(index+i+1);
            if(!adjusted)
            {
                if(dmisc2==0)
                {
                    head.o_tile=o_tile+40;
                }
                else
                {
                    head.o_tile=o_tile+160;
                }
            }
            
            if(head.dying)
            {
                for(int j=i; j<headcnt-1; j++)
                {
                    int temp=headside[j];
                    headside[j]=headside[j+1];
                    headside[j+1]=temp;
                    swapGuys(index+j+1,index+j+2);
                }
                // Reload head?
                --headcnt;
            }
        }
        
        adjusted=true;
        
        // move or die
        if(headcnt==0)
            hp=0;
        else
        {
            // Speed starts at 0.5, and increases by 0.5 for each head lost. Max speed is 4.5.
            step=zc_min(4.5, fix((((dmisc2==0)?4:8)-headcnt)*0.5+(dstep/100.0)));
            int dx1=0, dy1=-8, dx2=15, dy2=15;
            
            if(dmisc2==0)
            {
                for(int i=0; i<headcnt; i++)
                {
                    switch(headside[i])
                    {
                    case 0:
                        dy1=-24;
                        break;
                        
                    case 1:
                        dy2=31;
                        break;
                        
                    case 2:
                        dx1=-16;
                        break;
                        
                    case 3:
                        dx2=31;
                        break;
                    }
                }
            }
            else
            {
                dx1=-8; dy1=-16; dx2=23; dy2=23;
                
                for(int i=0; i<headcnt; i++)
                {
                    switch(headside[i]&3)
                    {
                    case 0:
                        dy1=-32;
                        break;
                        
                    case 1:
                        dy2=39;
                        break;
                        
                    case 2:
                        dx1=-24;
                        break;
                        
                    case 3:
                        dx2=39;
                        break;
                    }
                }
            }
            
            variable_walk_8(rate,homing,hrate,spw_floater,dx1,dy1,dx2,dy2);
            
            for(int i=0; i<headcnt; i++)
            {
                fix dx=0,dy=0;
                
                if(dmisc2==0)
                {
                    switch(headside[i])
                    {
                    case 0:
                        dy=-16;
                        break;
                        
                    case 1:
                        dy=16;
                        break;
                        
                    case 2:
                        dx=-16;
                        break;
                        
                    case 3:
                        dx=16;
                        break;
                    }
                }
                else
                {
                    switch(headside[i])
                    {
                    case 0:
                        dy=-24;
                        dx=-8;
                        break;
                        
                    case 1:
                        dy=24;
                        dx=8;
                        break;
                        
                    case 2:
                        dx=-24;
                        dy=8;
                        break;
                        
                    case 3:
                        dx=24;
                        dy=-8;
                        break;
                        
                    case 4:
                        dy=-24;
                        dx=8;
                        break;
                        
                    case 5:
                        dy=24;
                        dx=-8;
                        break;
                        
                    case 6:
                        dx=-24;
                        dy=-8;
                        break;
                        
                    case 7:
                        dx=24;
                        dy=8;
                        break;
                    }
                }
                
                enemy@ head=getGuy(index+i+1);
                head.x = x+dx;
                head.y = y+dy;
            }
        }
        
        return enemy::animate(index);
    }


    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        
        if(dying)
            return 0;
            
        switch(wpnId)
        {
        case wLitBomb:
        case wLitSBomb:
        case wBait:
        case wWhistle:
        case wFire:
        case wWind:
        case wSSparkle:
        case wFSparkle:
        case wPhantom:
            return 0;
            
        case wHookshot:
        case wBrang:
            sfx(WAV_CHINK, x);
            break;
            
        default:
            sfx(WAV_EHIT, x);
        }
        
        return 1;
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
            if(dmisc2==0)
            {
                switch(dir-8)                                         //directions get screwed up after 8.  *shrug*
                {
                case up:                                            //u
                    flip=0;
                    break;
                    
                case l_up:                                          //d
                    flip=0;
                    tile+=4;
                    break;
                    
                case l_down:                                        //l
                    flip=0;
                    tile+=8;
                    break;
                    
                case left:                                          //r
                    flip=0;
                    tile+=12;
                    break;
                    
                case r_down:                                        //ul
                    flip=0;
                    tile+=20;
                    break;
                    
                case down:                                          //ur
                    flip=0;
                    tile+=24;
                    break;
                    
                case r_up:                                          //dl
                    flip=0;
                    tile+=28;
                    break;
                    
                case right:                                         //dr
                    flip=0;
                    tile+=32;
                    break;
                }
                
                tile+=f2;
                enemy::draw();
            }                                                       //manhandla 2, big body
            else
            {
            
                switch(dir-8)                                         //directions get screwed up after 8.  *shrug*
                {
                case up:                                            //u
                    flip=0;
                    break;
                    
                case l_up:                                          //d
                    flip=0;
                    tile+=8;
                    break;
                    
                case l_down:                                        //l
                    flip=0;
                    tile+=40;
                    break;
                    
                case left:                                          //r
                    flip=0;
                    tile+=48;
                    break;
                    
                case r_down:                                        //ul
                    flip=0;
                    tile+=80;
                    break;
                    
                case down:                                          //ur
                    flip=0;
                    tile+=88;
                    break;
                    
                case r_up:                                          //dl
                    flip=0;
                    tile+=120;
                    break;
                    
                case right:                                         //dr
                    flip=0;
                    tile+=128;
                    break;
                }
                
                tile+=(f2*2);
                xofs-=8;
                yofs-=8;
                drawblock();
                xofs+=8;
                yofs+=8;
            }
        }
        else
        {
            if(dmisc2==0)
            {
                enemy::draw();
            }
            else
            {
                xofs-=8;
                yofs-=8;
                enemy::draw();
                xofs+=16;
                enemy::draw();
                yofs+=16;
                enemy::draw();
                xofs-=16;
                enemy::draw();
                xofs+=8;
                yofs-=8;
            }
        }
    }
}

class ManhandlaHead: enemy // Head? Arm? Claw? Whatever...
{
    ManhandlaHead()
    {
        id=clk;
        misc=clk;
        
        dir = clk & 3;
        clk=0;
        mainguy=false;
        count_enemy=false;
        dummy_bool0=false;
        item_set=0;
        bgsfx=-1;
        flags &= ~guy_neverret;
    }

    bool Update(int index)

    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        clk2-=1;
        if(clk2<=0)
        {
            clk2=rand()%5+5;
            clk3^=1;
        }
        
        if((rand()&127)==0)
            fireWeapon();
        
        return enemy::animate(index);
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
            switch(misc&3)
            {
            case up:
                break;
                
            case down:
                tile+=4;
                break;
                
            case left:
                tile+=8;
                break;
                
            case right:
                tile+=12;
                break;
            }
            
            tile+=f2;
        }
        else
        {
            switch(misc&3)
            {
            case down:
                flip=2;
                
            case up:
                tile=(clk3!=0)?188:189;
                break;
                
            case right:
                flip=1;
                
            case left:
                tile=(clk3!=0)?186:187;
                break;
            }
        }
        
        enemy::draw();
    }
}
