class Aquamentus: enemy
{
    private int moveTimer;
    
    Aquamentus()
    {
        y=64;
        
        if(dmisc1==0) // Facing left
            x=176;
        else // Facing right
        {
            x=64;
            if(!questRuleEnabled(qr_NEWENEMYTILES))
                flip=1;
        }
        
        yofs=playing_field_offset+1;
        clk3=32;
        clk2=0;
        moveTimer=clk;
        dir=left;
    }
    
    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        clk3-=1;
        if(clk3==0)
            fireWeapon();
        
        if(clk3<-80 && (rand()&63)==0)
            clk3=32;
        
        if(((moveTimer+1)&63)==0)
        {
            int d2=(rand()%3)+1;
            
            if(d2>=left)
                dir=d2;
            
            if(dmisc1!=0)
            {
                if(x<=40)
                    dir=right;
                
                if(x>=104)
                    dir=left;
            }
            else
            {
                if(x<=136)
                    dir=right;
                
                if(x>=200)
                    dir=left;
            }
        }
        
        if(moveTimer>=-1 && ((moveTimer+1)&7)==0)
        {
            if(dir==left)
                x-=1;
            else
                x+=1;
        }
        
        moveTimer=(moveTimer+1)%256;
        
        return enemy::animate(index);
    }
    
    void Draw()
    {
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            xofs=((dmisc1!=0)?-16:0);
            tile=o_tile+((clk&24)>>2)+(clk3>-32?(clk3>0?40:80):0);
            
            if(dying)
            {
                xofs=0;
                enemy::draw();
            }
            else
                drawblock();
        }
        else
        {
            int xblockofs=((dmisc1!=0)?-16:16);
            xofs=0;
            
            if(clk<0 || dying)
            {
                enemy::draw();
                return;
            }
            
            // face (0=firing, 2=resting)
            tile=o_tile+((clk3>0)?0:2);
            enemy::draw();
            // tail (
            tile=o_tile+(((clk&16)!=0)?1:3);
            xofs=xblockofs;
            enemy::draw();
            // body
            yofs+=16;
            xofs=0;
            tile=o_tile+(((clk&16)!=0)?20:22);
            enemy::draw();
            xofs=xblockofs;
            tile=o_tile+(((clk&16)!=0)?21:23);
            enemy::draw();
            yofs-=16;
        }
    }
    
    bool Hit(weapon@ w)
    {
        if((w.scriptcoldet&1)==0)
            return false;
        
        switch(w.id)
        {
        case wBeam:
        case wRefBeam:
        case wMagic:
            hysz=32;
        }
        
        bool ret = (dying || hclk>0) ? false : sprite::hit(w);
        hysz=16;
        return ret;
        
    }
}

