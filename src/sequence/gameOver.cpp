#include <precompiled.h>
#include "gameOver.h"
#include <link.h>
#include <sfx.h>
#include <sound.h>
#include <zelda.h>

extern mapscr tmpscr[2];
extern mapscr tmpscr2[6];

static void red_shift()
{
    int tnum=176;
    
    // set up the new palette
    for(int i=CSET(2); i < CSET(4); i++)
    {
        int r = (i-CSET(2)) << 1;
        RAMpal[i+tnum].r = r;
        RAMpal[i+tnum].g = r >> 3;
        RAMpal[i+tnum].b = r >> 4;
    }
    
    // color scale the game screen
    for(int y=0; y<168; y++)
    {
        for(int x=0; x<256; x++)
        {
            int c = framebuf->line[y+playing_field_offset][x];
            int r = zc_min(int(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
            framebuf->line[y+playing_field_offset][x] = (c ? (r+tnum+CSET(2)) : 0);
        }
    }
    
    refreshpal = true;
}

static void slide_in_color(int color)
{
    for(int i=1; i<16; i+=3)
    {
        RAMpal[CSET(2)+i+2] = RAMpal[CSET(2)+i+1];
        RAMpal[CSET(2)+i+1] = RAMpal[CSET(2)+i];
        RAMpal[CSET(2)+i]   = NESpal(color);
    }
    
    refreshpal=true;
}

GameOver::GameOver(LinkClass& l):
    counter(0),
    link(l),
    subscrbmp(create_bitmap_ex(8, framebuf->w, framebuf->h))
{
    
}

void GameOver::activate()
{
    link.setAction(none);
    Playing=false;
    
    if(!debug_enabled)
        Paused=false;
    
    game->set_deaths(zc_min(game->get_deaths()+1,999));
    link.dir=down;
    music_stop();
    kill_sfx();
    link.cancelAttack();
    link.setHitTimer(0);
    link.setInvincible(false);
    link.scriptcoldet = 1;
    
    for(int i=0; i<16; i++)
        link.miscellaneous[i] = 0;
    
    //get rid off all sprites but Link
    guys.clear();
    items.clear();
    Ewpns.clear();
    Lwpns.clear();
    Sitems.clear();
    chainlinks.clear();
    decorations.clear();
    
    playing_field_offset=56; // otherwise, red_shift() may go past the bottom of the screen
    quakeclk=wavy=0;
    
    //in original Z1, Link marker vanishes at death.
    //code in subscr.cpp, put_passive_subscr checks the following value.
    //color 255 is a GUI color, so quest makers shouldn't be using this value.
    //Also, subscreen is static after death in Z1.
    int tmp_link_dot = QMisc.colors.link_dot;
    QMisc.colors.link_dot = 255;
    //doesn't work
    //scrollbuf is tampered with by draw_screen()
    //put_passive_subscr(scrollbuf, &QMisc, 256, passive_subscreen_offset, false, false);//save this and reuse it.
    clear_bitmap(subscrbmp);
    put_passive_subscr(subscrbmp, &QMisc, 0, passive_subscreen_offset, false, sspUP);
    QMisc.colors.link_dot = tmp_link_dot;
}

void GameOver::update()
{
    if(counter<254)
    {
        if(counter<=32)
            link.setHitTimer(32-counter);
        
        if(counter>=62 && counter<138)
        {
            switch((counter-62)%20)
            {
            case 0:
                link.dir=right;
                break;
                
            case 5:
                link.dir=up;
                break;
                
            case 10:
                link.dir=left;
                break;
                
            case 15:
                link.dir=down;
                break;
            }
            
            link.linkstep();
        }
        
        if(counter>=194 && counter<208)
        {
            if(counter==194)
                link.setAction(dying);
                
            link.extend = 0;
            link.cs = wpnsbuf[iwDeath].csets&15;
            link.tile = wpnsbuf[iwDeath].tile;
            
            if(BSZ)
                link.tile+=(counter-194)/3;
            else if(counter>=204)
                link.tile++;
        }
        
        if(counter==208)
            link.setDontDraw(true);
            
        if(get_bit(quest_rules,qr_FADE))
        {
            if(counter < 170)
            {
                if(counter<60)
                {
                    draw_screen(tmpscr);
                    //reuse our static subscreen
                    set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
                    blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
                }
                
                if(counter==60)
                {
                    red_shift();
                    create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
                    create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
                    memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
                    
                    for(int q=0; q<PAL_SIZE; q++)
                    {
                        trans_table2.data[0][q] = q;
                        trans_table2.data[q][q] = q;
                    }
                }
                
                if(counter>=60 && counter<=169)
                {
                    draw_screen(tmpscr);
                    //reuse our static subscreen
                    blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
                    red_shift();
                    
                }
                
                if(counter>=139 && counter<=169)//fade from red to black
                {
                    fade_interpolate(RAMpal,black_palette,RAMpal, (counter-138)<<1, 224, 255);
                    create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
                    create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
                    memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
                    
                    for(int q=0; q<PAL_SIZE; q++)
                    {
                        trans_table2.data[0][q] = q;
                        trans_table2.data[q][q] = q;
                    }
                    
                    refreshpal=true;
                }
            }
            else //counter>=170
            {
                if(counter==170)//make Link grayish
                {
                    fade_interpolate(RAMpal,black_palette,RAMpal,64, 224, 255);
                    
                    for(int i=CSET(6); i < CSET(7); i++)
                    {
                        int g = (RAMpal[i].r + RAMpal[i].g + RAMpal[i].b)/3;
                        RAMpal[i] = _RGB(g,g,g);
                    }
                    
                    refreshpal = true;
                }
                
                //draw only link. otherwise black layers might cover him.
                rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
                link.draw(framebuf);
                blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
            }
        }
        else //!qr_FADE
        {
            if(counter==58)
            {
                for(int i = 0; i < 96; i++)
                    tmpscr->cset[i] = 3;
                    
                for(int j=0; j<6; j++)
                    if(tmpscr->layermap[j]>0)
                        for(int i=0; i<96; i++)
                            tmpscr2[j].cset[i] = 3;
            }
            
            if(counter==59)
            {
                for(int i = 96; i < 176; i++)
                    tmpscr->cset[i] = 3;
                    
                for(int j=0; j<6; j++)
                    if(tmpscr->layermap[j]>0)
                        for(int i=96; i<176; i++)
                            tmpscr2[j].cset[i] = 3;
            }
            
            if(counter==60)
            {
                for(int i=0; i<176; i++)
                {
                    tmpscr->cset[i] = 2;
                }
                
                for(int j=0; j<6; j++)
                    if(tmpscr->layermap[j]>0)
                        for(int i=0; i<176; i++)
                            tmpscr2[j].cset[i] = 2;
                            
                for(int i=1; i<16; i+=3)
                {
                    RAMpal[CSET(2)+i]   = NESpal(0x17);
                    RAMpal[CSET(2)+i+1] = NESpal(0x16);
                    RAMpal[CSET(2)+i+2] = NESpal(0x26);
                }
                
                refreshpal=true;
            }
            
            if(counter==139)
                slide_in_color(0x06);
                
            if(counter==149)
                slide_in_color(0x07);
                
            if(counter==159)
                slide_in_color(0x0F);
                
            if(counter==169)
            {
                slide_in_color(0x0F);
                slide_in_color(0x0F);
            }
            
            if(counter==170)
            {
                for(int i=1; i<16; i+=3)
                {
                    RAMpal[CSET(6)+i]   = NESpal(0x10);
                    RAMpal[CSET(6)+i+1] = NESpal(0x30);
                    RAMpal[CSET(6)+i+2] = NESpal(0x00);
                    refreshpal = true;
                }
            }
            
            if(counter < 169)
            {
                draw_screen(tmpscr);
                //reuse our static subscreen
                blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
            }
            else
            {
                //draw only link. otherwise black layers might cover him.
                rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
                link.draw(framebuf);
                blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
            }
        }
    }
    else if(counter<350)//draw 'GAME OVER' text
    {
        clear_bitmap(framebuf);
        blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
        textout_ex(framebuf,zfont,"GAME OVER",96,playing_field_offset+80,1,-1);
    }
    else
    {
        clear_bitmap(framebuf);
    }
    
    //SFX... put them all here
    switch(counter)
    {
    case   0:
        sfx(WAV_OUCH,pan(int(link.x)));
        break;
        
    case  60:
        sfx(WAV_SPIRAL);
        break;
        
    case 194:
        sfx(WAV_MSG);
        break;
    }
    
    //advanceframe(true);
    counter++;
    if(counter<353)
        return;
    
    finish();
    destroy_bitmap(subscrbmp);
    link.setAction(none);
    link.setDontDraw(false);
}
