class SpinningTile: enemy
{
    SpinningTile()
    {
        if(clk>0)  // clk>0 when created by a Spinning Tile combo
        {
            o_tile=clk;
            cs=id>>12;
        }
        
        id=id&0xFFF;
        clk=0;
        step=0;
        mainguy=false;
    }

    void facelink()
    {
        if(Link.x-x==0)
        {
            dir=(Link.y+8<y)?up:down;
        }
        else
        {
            double ddir=atan2(double(y-(Link.y)),double(Link.x-x));
            
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

    bool Update(int index)
    {
        if(dying)
            return Dead(index);
        
        if(clk==0)
        {
            removearmos(x,y);
        }
        
        misc+=1;
        
        if(misc==96)
        {
            facelink();
            double ddir=atan2(double((Link.y)-y),double(Link.x-x));
            angular=true;
            angle=ddir;
            step=(dstep/100.0);
        }
        
        if(y>186 || y<=-16 || x>272 || x<=-16)
            kickbucket();
            
        sprite::move(step);
        
        return enemy::animate(index);
    }

    void Draw()
    {
        update_enemy_frame();
        y-=(misc>>4);
        yofs+=2;
        enemy::draw();
        yofs-=2;
        y+=(misc>>4);
    }

    void DrawShadow()
    {
        flip = 0;
        shadowtile = getSpriteTile(iwShadow)+(clk%4);
        yofs+=4;
        enemy::drawshadow();
        yofs-=4;
    }
}
