
 //sprite::draw() before adding scripttile and scriptflip
void sprite::draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),0,16,((scriptflip > -1) ? scriptflip : flip));
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-(flip?-1:1),0,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+(flip?-1:1),32,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-(flip?-1:1),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)+(flip?-1:1),32,16,cs,((scriptflip > -1) ? scriptflip : flip));
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-(flip?-1:1),0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+(flip?-1:1),32,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-(flip?-1:1),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)+(flip?-1:1),32,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-(flip?-1:1),0,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+(flip?-1:1),32,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),16,16,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-(flip?-1:1),0,16,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)+(flip?-1:1),32,16,((scriptflip > -1) ? scriptflip : flip));
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            int flip_type = ((scriptflip > -1) ? scriptflip : flip);
            switch(flip_type)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,((scriptflip > -1) ? scriptflip : flip));
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,((scriptflip > -1) ? scriptflip : flip));
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip));
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,((scriptflip > -1) ? scriptflip : flip));
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip));
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,((scriptflip > -1) ? scriptflip : flip));
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,((scriptflip > -1) ? scriptflip : flip));
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,((scriptflip > -1) ? scriptflip : flip));
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,((scriptflip > -1) ? scriptflip : flip));
                else if(drawstyle==1)
                    overtiletranslucent16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,((scriptflip > -1) ? scriptflip : flip),128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,((scriptflip > -1) ? scriptflip : flip));
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].newtile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((fix)sx,(fix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            
	    if ( w.scripttile <= -1 ) 
	    {
		    if(BSZ)
		    {
			if(clk>=-10)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
			
			if(clk>=-5)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
		    }
		    else
		    {
			if(clk>=-12)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
			
			if(clk>=-6)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
		    }
	    }
            
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}

void sprite::old_draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),0,16,((scriptflip > -1) ? scriptflip : flip));
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-(flip?-1:1),0,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+(flip?-1:1),32,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-(flip?-1:1),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)+(flip?-1:1),32,16,cs,((scriptflip > -1) ? scriptflip : flip));
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-(flip?-1:1),0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+(flip?-1:1),32,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-(flip?-1:1),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)+(flip?-1:1),32,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-(flip?-1:1),0,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+(flip?-1:1),32,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),16,16,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-(flip?-1:1),0,16,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)+(flip?-1:1),32,16,((scriptflip > -1) ? scriptflip : flip));
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            int flip_type = ((scriptflip > -1) ? scriptflip : flip);
            switch(flip_type)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,flip_type);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,flip_type);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,flip_type);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,flip_type);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,flip_type);
                else if(drawstyle==1)
                    overtiletranslucent16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,flip_type,128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,flip_type);
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].newtile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((fix)sx,(fix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            if ( w.scripttile <= -1 ) 
	    {
		    if(BSZ)
		    {
			if(clk>=-10)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
			
			if(clk>=-5)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
		    }
		    else
		    {
			if(clk>=-12)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
			
			if(clk>=-6)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
		    }
	    }   
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}

void sprite::draw8(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        switch(drawstyle)
        {
        case 0:                                               //normal
            overtile8(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,((scriptflip > -1) ? scriptflip : flip));
            break;
            
        case 1:                                               //phantom
            overtiletranslucent8(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,((scriptflip > -1) ? scriptflip : flip),128);
            break;
        }
    }
}

void sprite::drawcloaked(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        overtilecloaked16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,((scriptflip > -1) ? scriptflip : flip));
    }
    else
    {
        int t  = wpnsbuf[iwSpawn].newtile;
        int cs2 = wpnsbuf[iwSpawn].csets&15;
        
        if(BSZ)
        {
            if(clk>=-10) ++t;
            
            if(clk>=-5) ++t;
        }
        else
        {
            if(clk>=-12) ++t;
            
            if(clk>=-6) ++t;
        }
        
        overtile16(dest,t,x,sy,cs2,0);
    }
    
    if(get_debug() && key[KEY_O])
        rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(id));
}

