class Digdogger: enemy
{
    Digdogger()
    {
        superman=1;
        hxofs=-8;
        hyofs=-8;
        hxsz=32;
        hysz=32;
        hzsz=16; // hard to jump.
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        switch(misc)
        {
        case 0:
            variable_walk_8(rate,homing,hrate,spw_floater,-8,-16,23,23);
            break;
            
        case 1:
            misc+=1;
            break;
            
        case 2:
            for(int i=0; i<dmisc5; i++)
            {
                addenemy(x, y, dmisc1+0x1000, -15);
            }
            
            for(int i=0; i<dmisc6; i++)
            {
                addenemy(x, y, dmisc2+0x1000, -15);
            }
            
            for(int i=0; i<dmisc7; i++)
            {
                addenemy(x, y, dmisc3+0x1000, -15);
            }
            
            for(int i=0; i<dmisc8; i++)
            {
                addenemy(x, y, dmisc4+0x1000, -15);
            }
            
            if(itemguy) // Hand down the carried item
            {
                // Um, what if it didn't create any children?
                guycarryingitem = numGuys()-1;
                enemy@ en=getGuy(guycarryingitem);
                en.itemguy = true;
                itemguy = false;
            }
            
            stopBGSFX();
            playDeathSFX();
            return true;
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        if(anim!=aDIG)
        {
            update_enemy_frame();
            xofs-=8;
            yofs-=8;
            drawblock();
            xofs+=8;
            yofs+=8;
            return;
        }
        
        tile = o_tile;
        int fdiv = frate/4;
        int efrate = fdiv == 0 ? 0 : clk/fdiv;
        
        int f2=questRuleEnabled(qr_NEWENEMYTILES)?
               efrate:((clk>=(frate>>1))?1:0);
               
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            switch(dir-8)                                           //directions get screwed up after 8.  *shrug*
            {
            case up:                                              //u
                flip=0;
                break;
                
            case l_up:                                            //d
                flip=0;
                tile+=8;
                break;
                
            case l_down:                                          //l
                flip=0;
                tile+=40;
                break;
                
            case left:                                            //r
                flip=0;
                tile+=48;
                break;
                
            case r_down:                                          //ul
                flip=0;
                tile+=80;
                break;
                
            case down:                                            //ur
                flip=0;
                tile+=88;
                
                break;
                
            case r_up:                                            //dl
                flip=0;
                tile+=120;
                break;
                
            case right:                                           //dr
                flip=0;
                tile+=128;
                break;
            }
            
            tile+=(f2*2);
        }
        else
        {
            tile+=(f2!=0)?0:2;
            flip=((clk&1)!=0)?1:0;
        }
        
        xofs-=8;
        yofs-=8;
        drawblock();
        xofs+=8;
        yofs+=8;
    }

    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        
        if(wpnId==wWhistle && misc==0)
            misc=1;
            
        return 0;
    }
}

class DigdoggerKid: enemy
{
    DigdoggerKid()
    {
        count_enemy=(id==(id&0xFFF));
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(misc<=128)
        {
            misc+=1;
            if((misc&31)==0)
                step+=0.25;
        }
        
        variable_walk_8(rate, homing, hrate, spw_floater);
        return enemy::animate(index);
    }

    void Draw()
    {
        tile = o_tile;
        int fdiv = frate/4;
        int efrate = fdiv == 0 ? 0 : clk/fdiv;
        int f2=questRuleEnabled(qr_NEWENEMYTILES)?
               efrate:((clk>=(frate>>1))?1:0);
               
               
        if(questRuleEnabled(qr_NEWENEMYTILES))
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
            
            tile+=f2;
        }
        else
        {
            tile+=(clk>=6)?1:0;
        }
        
        enemy::draw();
    }
}
