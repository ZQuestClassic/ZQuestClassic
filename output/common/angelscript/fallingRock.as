class FallingRock: enemy
{
    private bool small;
    private int bounceDir;
    private int drawDir;
    
    FallingRock()
    {
        clk=0;
        mainguy=false;
        clk2=-14;
        
        if(dmisc10!=1)
        {
            small=true;
            hxofs=-2;
            hyofs=-2;
            hxsz=20;
            hysz=20;
        }
        else
        {
            small=false;
            hxofs=-10;
            hyofs=-10;
            hxsz=36;
            hysz=36;
            hzsz=16; //can't be jumped
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
        
        clk2+=1;
        if(clk2==0)                                             // start it
        {
            x=rand()&0xF0;
            y=small ? 0 : -32;
            clk3=0;
            clk2=rand()&15;
        }
        
        if(clk2>16)                                               // move it
        {
            if(clk3<=0)                                             // start bounce
            {
                if(x<32)
                    dir=1;
                else if(x>208)
                    dir=0;
                else
                    dir=rand()&1;
            }
            
            if(clk3<29)
            {
                x+=(dir==1) ? 1 : -1;                                    //right, left
                bounceDir=dir;
                
                if(clk3<2)
                {
                    y-=2;    //up
                    drawDir=(bounceDir==1)?r_up:l_up;
                }
                else if(clk3<5)
                {
                    y--;    //up
                    drawDir=(bounceDir==1)?r_up:l_up;
                }
                else if(clk3<8)
                {
                    drawDir=(bounceDir==1)?right:left;
                }
                else if(clk3<11)
                {
                    y++;   //down
                    drawDir=(bounceDir==1)?r_down:l_down;
                }
                else
                {
                    y+=2; //down
                    drawDir=(bounceDir==1)?r_down:l_down;
                }
                
                clk3+=1;
            }
            else if(y<176)
                clk3=0;                                               // next bounce
            else
                clk2 = -(rand()&63);                                  // back to top
        }
        
        return enemy::animate(index);
    }
    
    void Draw()
    {
        if(clk2<0)
            return;
        
        int tempdir=dir;
        dir=drawDir;
        update_enemy_frame();
        if(small)
            enemy::draw();
        else
        {
            xofs-=8;
            yofs-=8;
            drawblock();
            xofs+=8;
            yofs+=8;
        }
        dir=tempdir;
    }
    
    void DrawShadow()
    {
        if(clk2<0)
            return;
        
        if(small)
        {
            int tempy=yofs;
            flip = 0;
            int fdiv = frate/4;
            int efrate = fdiv == 0 ? 0 : clk/fdiv;
            int f2=questRuleEnabled(qr_NEWENEMYTILES) ?
                   efrate : ((clk>=(frate>>1))?1:0);
            shadowtile=getSpriteTile(iwShadow)+f2;
            
            yofs+=8;
            yofs+=vbound(clk3, 0, 29-clk3);
            enemy::drawshadow();
            yofs=tempy;
        }
        else
        {
            int tempy=yofs;
            flip = 0;
            int f2=((clk<<2)/frate)<<1;
            shadowtile=getSpriteTile(iwLargeShadow)+f2;
            yofs+=vbound(clk3, 0, 29-clk3);
            
            yofs+=8;
            xofs-=8;
            enemy::drawshadow();
            xofs+=16;
            shadowtile+=1;
            enemy::drawshadow();
            yofs+=16;
            shadowtile+=20;
            enemy::drawshadow();
            xofs-=16;
            shadowtile-=1;
            enemy::drawshadow();
            xofs+=8;
            yofs=tempy;
        }
    }
    
    int takehit(weapon@ w)
    {
        return 0;
    }
}

