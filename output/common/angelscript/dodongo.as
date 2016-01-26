class Dodongo: enemy
{
    Dodongo()
    {
        fading=fade_flash_die;
        
        if(dir==down&&y>=128)
        {
            dir=up;
        }
        
        if(dir==right&&x>=208)
        {
            dir=left;
        }
    }

    bool Update(int index)
    {
        if(dying)
        {
            return Dead(index);
        }
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        if(clk2!=0)                                                  // ate a bomb
        {
            clk2-=1;
            if(clk2==0)
                hp-=misc;                                             // store bomb's power in misc
        }
        else if(dmisc10!=1) // NES Dodongo
            constant_walk(rate, homing, spw_clipright);
        else // BS Dodongo
            constant_walk(rate,homing,spw_clipbottomright);
        
        if(dmisc10!=1)
            hxsz = (dir<=down) ? 16 : 32;
        else
        {
            hxsz=(dir<=down) ? 16 : 32;
            hysz=(dir>=left) ? 16 : 32;
            hxofs=(dir>=left) ? -8 : 0;
            hyofs=(dir<left) ? -8 : 0;
        }
        
        return enemy::animate(index);
    }

    void Draw()
    {
        tile=o_tile;
        
        if(clk<0)
        {
            enemy::draw();
            return;
        }
        
        if(dmisc10!=1)
        {
            update_enemy_frame();
            enemy::draw();
            
            if(dummy_int1!=0)  //additional tiles
            {
                tile+=dummy_int1; //second tile is previous tile
                xofs-=16;           //new xofs change
                enemy::draw();
                xofs+=16;
            }
        }
        else
        {
            int tempx=xofs;
            int tempy=yofs;
            update_enemy_frame();
            enemy::draw();
            tile+=dummy_int1; //second tile change
            xofs+=dummy_int2; //new xofs change
            yofs+=dummy_int3; //new yofs change
            enemy::draw();
            xofs=tempx;
            yofs=tempy;
        }
    }

    int takehit(weapon@ w)
    {
        int wpnId = w.id;
        int power = w.power;
        int wpnx = w.x;
        int wpny = w.y;
        
        if(dmisc10!=1)
        {
            if(dying || clk<0 || clk2>0 || (superman!=0 && !(superman>1 && wpnId==wSBomb)))
                return 0;
        }
        else
        {
            if(dying || clk<0 || clk2>0 || superman!=0)
                return 0;
        }
            
        switch(wpnId)
        {
        case wPhantom:
            return 0;
            
        case wFire:
        case wBait:
        case wWhistle:
        case wWind:
        case wSSparkle:
        case wFSparkle:
            return 0;
            
        case wLitBomb:
        case wLitSBomb:
            if(abs(wpnx-getHitX()) > 7 || abs(wpny-getHitY()) > 7)
                return 0;
                
            clk2=96;
            misc=power;
            
            if(wpnId==wLitSBomb)
                item_set=isSBOMB100;
                
            return 1;
        case wBomb:
        case wSBomb:
            if(abs(wpnx-getHitX()) > 8 || abs(wpny-getHitY()) > 8)
                return 0;
                
            stunclk=160;
            misc=wpnId;                                           // store wpnId
            return 1;
        case wSword:
            if(stunclk!=0)
            {
                sfx(WAV_EHIT, x);
                hp=0;
                item_set = (misc==wSBomb) ? isSBOMB100 : isBOMB100;
                fading=0;                                           // don't flash
                return 1;
            }
            
        default:
            sfx(WAV_CHINK, x);
        }
        
        return 1;
    }
    
    fix getHitX()
    {
        if(dmisc10!=1)
        {
            if(dir==right)
                return x+16;
            else
                return x;
        }
        else
        {
            if(dir==left)
                return x-8;
            else if(dir==right)
                return x+8;
            else
                return x;
        }
    }
    
    fix getHitY()
    {
        if(dmisc10!=1)
            return y;
        else
        {
            if(dir==up)
                return y-8;
            else if(dir==down)
                return y+8;
            else
                return y;
        }
    }
}

