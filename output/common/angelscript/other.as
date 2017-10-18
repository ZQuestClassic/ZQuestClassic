class Other: enemy
{
    int clk4;
    bool shield;
    
    Other()
    {
        clk4=0;
        shield=(flags&(inv_left | inv_right | inv_back |inv_front))!=0;
        
        // Spawn type
        if((flags & guy_fadeflicker)!=0)
        {
            clk=0;
            superman = 1;
            fading=fade_flicker;
            count_enemy=false;
            dir=down;
            
            if(!canmove(down, 8, spw_none))
                clk3=int(13.0/step);
        }
        else if((flags & guy_fadeinstant)!=0)
        {
            clk=0;
        }
    }

    bool Update(int index)
    {
        if(fading!=0)
        {
            if(++clk4 > 60)
            {
                clk4=0;
                superman=0;
                fading=0;
                
                if((flags2&cmbflag_armos)!=0 && z==0)
                    removearmos(x,y);
                    
                clk2=0;
                
                if(!canmove(down, 8, spw_none))
                {
                    dir=0;
                    y.vAND(0xF00000);
                }
                
                return Dead(index);
            }
            else if((flags2&cmbflag_armos)!=0 && z==0 && clk==0)
                removearmos(x, y);
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }

    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        int wpnDir = w.dir;
        
        if(wpnId==wHammer && shield && (flags & guy_bkshield)!=0
                && (((flags&inv_front)!=0 && wpnDir==(dir^down)) || ((flags&inv_back)!=0 && wpnDir==(dir^up))
                    || ((flags&inv_left)!=0 && wpnDir==(dir^left)) || ((flags&inv_right)!=0 && wpnDir==(dir^right))))
        {
            shield = false;
            flags &= ~(inv_left|inv_right|inv_back|inv_front);
            
            if(questRuleEnabled(qr_BRKNSHLDTILES))
                o_tile=s_tile;
        }
        
        int ret = enemy::takehit(w);
        return ret;
    }

    void break_shield()
    {
        if(!shield)
            return;
            
        flags&=~(inv_front | inv_back | inv_left | inv_right);
        shield=false;
        
        if(questRuleEnabled(qr_BRKNSHLDTILES))
            o_tile=s_tile;
    }
}

