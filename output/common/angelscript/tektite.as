class Tektite: enemy
{
    private fix old_y; // a kludge used to check for ZScript modification of Y
    private int c;
    private int clk2start;
    private int cstart;
    
    Tektite()
    {
        old_y=y;
        dir=down;
        misc=1;
        clk=-15;
        
        if(!BSZ)
            clk*=rand()%3+1;
            
        // avoid divide by 0 errors
        if(dmisc1 == 0)
            dmisc1 = 24;
            
        if(dmisc2 == 0)
            dmisc2 = 3;
    }
    
    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(questRuleEnabled(qr_ENEMIESZAXIS))
        {
            y=floor_y;
        }
        
        if(clk>=0 && stunclk==0 && (!watch || misc==0))
        {
            int nb;
            switch(misc)
            {
            case 0:                                               // normal
                if((rand()%dmisc1)==0)
                {
                    misc=1;
                    clk2=32;
                }
                
                break;
                
            case 1:                                               // waiting to pounce
                clk2-=1;
                if(clk2<=0)
                {
                    int r=rand();
                    misc=2;
                    step=0-((dstep)/100.0);                           // initial speed
                    clk3=(r&1)+2;                                    // left or right
                    clk2=(r&31)+10;                                 // flight time
                    clk2start=clk2;
                    
                    if(y<32)  clk2+=2;                                  // make them come down from top of screen
                    
                    if(y>112) clk2-=2;                                  // make them go back up
                    
                    cstart=c = 9-((r&31)>>3);                           // time before gravity kicks in
                }
                
                break;
                
            case 2:                                                 // in flight
                move(step);
                
                if(step>0)                                            //going down
                {
                    if(COMBOTYPE(x+8,y+16)==cNOJUMPZONE)
                    {
                        step=0-step;
                    }
                    else if(COMBOTYPE(x+8,y+16)==cNOENEMY)
                    {
                        step=0-step;
                    }
                    else if(MAPFLAG(x+8,y+16)==mfNOENEMY)
                    {
                        step=0-step;
                    }
                    else if(MAPCOMBOFLAG(x+8,y+16)==mfNOENEMY)
                    {
                        step=0-step;
                    }
                }
                else if(step<0)
                {
                    if(COMBOTYPE(x+8,y)==cNOJUMPZONE)
                    {
                        step=0-step;
                    }
                    else if(COMBOTYPE(x+8,y)==cNOENEMY)
                    {
                        step=0-step;
                    }
                    else if(MAPFLAG(x+8,y)==mfNOENEMY)
                    {
                        step=0-step;
                    }
                    else if(MAPCOMBOFLAG(x+8,y)==mfNOENEMY)
                    {
                        step=0-step;
                    }
                }
                
                if(clk3==left)
                {
                    if(COMBOTYPE(x,y+8)==cNOJUMPZONE)
                    {
                        clk3^=1;
                    }
                    else if(COMBOTYPE(x,y+8)==cNOENEMY)
                    {
                        clk3^=1;
                    }
                    else if(MAPFLAG(x,y+8)==mfNOENEMY)
                    {
                        clk3^=1;
                    }
                    else if(MAPCOMBOFLAG(x,y+8)==mfNOENEMY)
                    {
                        clk3^=1;
                    }
                }
                else
                {
                    if(COMBOTYPE(x+16,y+8)==cNOJUMPZONE)
                    {
                        clk3^=1;
                    }
                    else if(COMBOTYPE(x+16,y+8)==cNOENEMY)
                    {
                        clk3^=1;
                    }
                    else if(MAPFLAG(x+16,y+8)==mfNOENEMY)
                    {
                        clk3^=1;
                    }
                    else if(MAPCOMBOFLAG(x+16,y+8)==mfNOENEMY)
                    {
                        clk3^=1;
                    }
                }
                
                --c;
                
                if(c<0 && step<(dstep/100.0))
                {
                    step+=(dmisc3/100.0);
                }
                
                nb=questRuleEnabled(qr_NOBORDER) ? 16 : 0;
                
                if(x<=16-nb)  clk3=right;
                
                if(x>=224+nb) clk3=left;
                
                x += (clk3==left) ? -1 : 1;
                
                clk2-=1;
                if((clk2<=0 && y>=16-nb) || y>=144+nb)
                {
                    if(y>=144+nb && questRuleEnabled(qr_ENEMIESZAXIS))
                    {
                        step=0-step;
                        y--;
                    }
                    else if((rand()%dmisc2)!=0)                                 //land and wait
                    {
                        clk=0;
                        misc=0;
                    }                                                   //land and jump again
                    else
                    {
                        misc=1;
                        clk2=0;
                    }
                }
                
                break;
            }                                                         // switch
        }
        
        if(questRuleEnabled(qr_ENEMIESZAXIS) && misc==2)
        {
            int tempy = floor_y;
            z=zc_max(0,zc_min(clk2start-clk2,clk2));
            floor_y = y;
            y=tempy-z;
            old_y = y;
        }
        
        if(stunclk!=0 && (clk&31)==1)
            clk=0;
        
        return enemy::animate(index);
    }
    
    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }
    
    void DrawShadow()
    {
        if(z<1 && questRuleEnabled(qr_ENEMIESZAXIS))
            return;
            
        int tempy=yofs;
        int fdiv = frate/4;
        int efrate = fdiv == 0 ? 0 : clk/fdiv;
        int f2=questRuleEnabled(qr_NEWENEMYTILES)?
               efrate:((clk>=(frate>>1))?1:0);
        flip = 0;
        shadowtile = getSpriteTile(iwShadow);
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            if(misc==0)
            {
                shadowtile+=f2;
            }
            else if(misc!=1)
                shadowtile+=2;
        }
        else
        {
            if(misc==0)
            {
                shadowtile += (f2!=0) ? 1 : 0;
            }
            else if(misc!=1)
            {
                shadowtile+=1;
            }
        }
        
        yofs+=8;
        
        if(!questRuleEnabled(qr_ENEMIESZAXIS) && misc==2)
        {
            yofs+=zc_max(0,zc_min(clk2start-clk2,clk2));
        }
        
        enemy::drawshadow();
        yofs=tempy;
    }
}

