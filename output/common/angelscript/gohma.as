class Gohma: enemy
{
    int clk4;
    
    Gohma()
    {
        x=128;
        y=48;
        clk=0;
        
        hxofs=-16;
        hxsz=48;
        clk4=0;
        yofs=playing_field_offset+1;
        dir=rand()%3+1;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        // Movement clk must be separate from animation clk because of the Clock item
        if(!watch)
            clk4++;
            
        if((clk4&63)==0)
        {
            if((clk4&64)!=0)
                dir^=1;
            else
                dir=rand()%3+1;
        }
        
        if(dmisc1!=2)
        {
            if((clk&63)==3)
                fireWeapon();
        }
        else if(clk3>=16 && clk3<116) // Breath
                updateFiring();
        
        if((clk4&1)!=0)
            move(1);
        
        clk3+=1;
        if(clk3>=400)
        {
            clk3=0;
            
            if(dmisc1==2)
                // Reset for the next cycle
                setBreathTimer(104);
        }
            
        return enemy::animate(index);
    }

    void Draw()
    {
        tile=o_tile;
        
        if(clk<0 || dying)
        {
            enemy::draw();
            return;
        }
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
        {
            // left side
            xofs=-16;
            flip=0;
            tile+=(3*((clk&48)>>4));
            enemy::draw();
            
            // right side
            xofs=16;
            tile=o_tile;
            tile+=(3*((clk&48)>>4))+2;
            enemy::draw();
            
            // body
            xofs=0;
            tile=o_tile;
            
            if(clk3<16)
                tile+=7;
            else if(clk3<116)
                tile+=10;
            else if(clk3<132)
                tile+=7;
            else
                tile+=(((clk3-132)&24)!=0)?4:1;
                
            enemy::draw();
            
        }
        else
        {
            // left side
            xofs=-16;
            flip=0;
            
            if((clk&16)==0)
            {
                tile+=2;
                flip=1;
            }
            
            enemy::draw();
            
            // right side
            tile=o_tile;
            xofs=16;
            
            if((clk&16)!=0) tile+=2;
            
            enemy::draw();
            
            // body
            tile=o_tile;
            xofs=0;
            
            if(clk3<16)
                tile+=4;
            else if(clk3<116)
                tile+=5;
            else if(clk3<132)
                tile+=4;
            else tile+=(((clk3-132)&8)!=0)?3:1;
            
            enemy::draw();
            
        }
    }

    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        int wpnx = w.x;
        int wpnDir = w.dir;
        
        // Really got to come up with a better way to handle this...
        int def = defenditemclass(wpnId, w.power);
        int power = getDefendedItemPower();
        
        if(def < 0)
        {
            if(!((wpnDir==up || wpnDir==l_up || wpnDir==r_up) && abs(int(x)-wpnx)<=8 && clk3>=16 && clk3<116))
            {
                sfx(WAV_CHINK, x);
                return 1;
            }
        }
        
        return enemy::takehit(w);
    }
}
