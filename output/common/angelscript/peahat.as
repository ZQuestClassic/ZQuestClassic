class Peahat: enemy
{
    Peahat()
    {
        floater_walk((misc!=0) ? rate : 0, hrate, dstep/100.0, dstep/1000.0, 10,  80, 16);
        dir=8;
        movestatus=1;
        clk=0;
        step=0;
        scriptFlags=asf_ALWAYSDIE;
    }
    
    bool Update(int index)
    {
        if(slide())
        {
            return false;
        }
        
        if(dying)
            return Dead(index);
           
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(stunclk==0 && clk>96)
            misc=1;
        
        if(!watch)
            floater_walk((misc!=0)?rate:0, hrate, dstep/100.0, dstep/1000.0, 10,  80, 16);
        
        if(questRuleEnabled(qr_ENEMIESZAXIS) && !screenFlagSet(fSIDEVIEW))
        {
            z=int(step*1.1/fix((dstep/1000.0)*1.1));
        }
        
        if(watch && questRuleEnabled(qr_PEAHATCLOCKVULN))
            superman=0;
        else
            superman=(movestatus!=0 && !questRuleEnabled(qr_ENEMIESZAXIS)) ? 1 : 0;
        stunclk=0;
        
        if(x<16) dir=right; //this is ugly, but so is moving or creating these guys with scripts.
        
        return enemy::animate(index);
    }
    
    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        int enemyHitWeapon = w.parentitem;
        
        if(dying || clk<0 || hclk>0)
            return 0;
            
        if(superman!=0 && wpnId!=wSBomb            // vulnerable to super bombs
                // fire boomerang, for nailing peahats
                && !(wpnId==wBrang && (enemyHitWeapon>-1 ? getItemProp(enemyHitWeapon, ITEMPROP_POWER) : current_item_power(itype_brang))>0))
            return 0;
            
        // Time for a kludge...
        int s = superman;
        superman = 0;
        int ret = enemy::takehit(w);
        superman = s;
        
        // Anyway...
        if(stunclk == 160)
        {
            clk2=0;
            movestatus=0;
            misc=0;
            clk=0;
            step=0;
        }
        
        return ret;
    }
    
    void Draw()
    {
        update_enemy_frame();
        enemy::draw();
    }
    
    void DrawShadow()
    {
        int tempy=yofs;
        flip = 0;
        shadowtile = getSpriteTile(iwShadow)+posframe;
        
        if(!questRuleEnabled(qr_ENEMIESZAXIS))
        {
            yofs+=8;
            yofs+=step/(dstep/1000.0);
        }
        
        enemy::drawshadow();
        yofs=tempy;
    }
}

