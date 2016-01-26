class Zora: enemy
{
    private int maxHP;
    
    Zora()
    {
        mainguy=false;
        count_enemy=false;
        maxHP=hp;
    }
    
    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(watch)
        {
            // WEIRDNESS: This doesn't match the C++ implementation,
            // but it seems to work. Better make sure, though...
            clk=0;
            return false;
        }
        
        if(questRuleEnabled(qr_NEWENEMYTILES))
            facelink();
        
        switch(clk)
        {
        case 0: // reposition him
        {
            int t=0;
            int pos2=rand()%160 + 16;
            bool placed=false;
            
            while(!placed && t<160)
            {
                if(iswater(pos2) && (pos2&15)>0 && (pos2&15)<15)
                {
                    x=(pos2&15)<<4;
                    y=pos2&0xF0;
                    hp=maxHP; // refill life each time
                    hxofs=1000; // avoid hit detection
                    stunclk=0;
                    placed=true;
                }
                
                pos2+=19;
                
                if(pos2>=176)
                    pos2-=160;
                    
                ++t;
            }
            
            if(!placed || whistleclk>=88) // can't place him, he's gone
                return true;
                
        }
        break;
        
        case 35:
            if(!questRuleEnabled(qr_NEWENEMYTILES))
                dir=(Link.y+8<y)?up:down;
            
            hxofs=0;
            break;
            
        case 35+19:
            fireWeapon();
            sfx(wpnsfx(wpn), x);
            break;
            
        case 35+66:
            hxofs=1000;
            break;
            
        case 198:
            clk=-1;
            break;
        }
        
        return enemy::animate(index);
    }
    
    void facelink()
    {
        if(Link.x-x==0)
            dir=(Link.y+8<y)?up:down;
        else
        {
            double ddir=atan2(y-Link.y, Link.x-x);
            
            if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
            {
                dir=l_down;
            }
            else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
            {
                dir=down;
            }
            else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
            {
                dir=r_down;
            }
            else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
            {
                dir=right;
            }
            else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
            {
                dir=r_up;
            }
            else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
            {
                dir=up;
            }
            else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
            {
                dir=l_up;
            }
            else
            {
                dir=left;
            }
        }
    }
    
    void Draw()
    {
        if(clk<3)
            return;
            
        update_enemy_frame();
        enemy::draw();
    }
}

