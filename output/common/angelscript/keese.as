class Keese: enemy
{
    int clk4;
    
    Keese()
    {
        dir=(rand()&7)+8;
        step=0;
        movestatus=1;
        hxofs=2;
        hxsz=12;
        hyofs=4;
        hysz=8;
        clk4=0;
        dummy_int1=0;
    }

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
            
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(dmisc1!=0)
        {
            floater_walk(rate, hrate, dstep/100, 0, 10, 0, 0);
        }
        else
        {
            floater_walk(rate, hrate, dstep/100, dstep/1000, 10, 120, 16);
        }
        
        if(dmisc2 == e2tKEESETRIB)
        {
            if(++clk4==256)
            {
                if(!m_walkflag(x, y, 0))
                {
                    int kids = numGuys();
                    bool success = false;
                    int id2=dmisc3;
                    success = 0 != addenemy(x, y, id2, -24);
                    
                    if(success)
                    {
                        if(itemguy) // Hand down the carried item
                        {
                            guycarryingitem = numGuys()-1;
                            enemy@ en=getGuy(guycarryingitem);
                            en.itemguy = true;
                            itemguy = false;
                        }
                        
                        enemy@ en=getGuy(kids);
                        en.count_enemy = count_enemy;
                    }
                    
                    return true;
                }
                else
                {
                    clk4=0;
                }
            }
        }
        // Keese Tribbles stay on the ground, so there's no problem when they transform.
        else if(questRuleEnabled(qr_ENEMIESZAXIS) && !screenFlagSet(fSIDEVIEW))
        {
            z=int(step/(dstep/100.0));
            // Some variance in keese flight heights when away from Link
            z+=int(step*zc_max(0,(distance(x,y,LinkX(),LinkY())-128)/10));
        }
        
        return enemy::animate(index);
    }

    void DrawShadow()
    {
        int tempy=yofs;
        flip = 0;
        shadowtile = getSpriteTile(iwShadow)+posframe;
        yofs+=8;
        
        if(!questRuleEnabled(qr_ENEMIESZAXIS))
            yofs+=int(step/(dstep/1000.0));
            
        enemy::drawshadow();
        yofs=tempy;
    }

    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }
}
