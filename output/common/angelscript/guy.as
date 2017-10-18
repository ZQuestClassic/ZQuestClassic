class Guy: enemy
{
    // good guys, fires, fairy, and other non-enemies
    // based on enemy class b/c guys in dungeons act sort of like enemies
    // also easier to manage all the guys this way
    Guy()
    {
        mainguy=(id&0x8000)!=0;
        id&=0xFFF;
        canfreeze=false;
        dir=down;
        yofs=playing_field_offset;
        hxofs=2;
        hzsz=8;
        hxsz=12;
        hysz=17;
        
        if(superman==0 && (!isdungeon() || id==gFAIRY || id==gFIRE || id==gZELDA))
        {
            superman = 1;
            hxofs=1000;
        }
    }

    bool Update(int index)
    {
        if(mainguy && clk==0 && misc==0)
        {
            setupscreen();
            misc = 1;
        }
        
        if(mainguy && fadeclk==0)
            return true;
            
        hp=256;                                                   // good guys never die...
        
        if(hclk!=0 && clk2==0)
        {
            // but if they get hit...
            clk2+=1;                                                 // only do this once
            
            if(!questRuleEnabled(qr_NOGUYFIRES))
            {
                addenemy(BSZ?64:72,68,eSHOOTFBALL,0);
                addenemy(BSZ?176:168,68,eSHOOTFBALL,0);
            }
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        update_enemy_frame();
        
        if(!mainguy || fadeclk<0 || (fadeclk&1)!=0)
            enemy::draw();
    }
}

