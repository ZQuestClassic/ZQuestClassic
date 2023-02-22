/*                 __                  __
  *                /_/\  __  __  __    /_/\  ______
  *               _\_\/ / /\/ /\/ /\  _\_\/ / ____ \
  *              / /\  / / / / / / / / /\  / /\_ / /\
  *         __  / / / / /_/ /_/ / / / / / / / / / / /
  *        / /_/ / / /_________/ / /_/ / /_/ / /_/ /
  *        \____/ /  \_________\/  \_\/  \_\/  \_\/
  *         \___\/
  *
  *
  *
  *     jwin.c
  *
  *     Windows(R) style GUI for Allegro.
  *     by Jeremy Craner
  *
  *     Most routines are adaptations of Allegro code.
  *     Allegro is by Shawn Hargreaves, et al.
  *
  *     Version: 3/22/00
  *     Allegro version: 3.1x  (don't know if it works with WIP)
  *
  */

/* This code is not fully tested */

#include "precompiled.h" //always first

#define ALLEGRO_NO_COMPATIBILITY
#include "base/allegro_wrapper.h"
#include "tab_ctl.h"
#include "jwin.h"

extern volatile int32_t myvsync;
void update_hw_screen(bool force);

extern bool is_zquest();

int32_t vc2(int32_t x)
{
    switch(x)
    {
    case 0:                                                 //black
        return makecol(0, 0, 0);
        break;
        
    case 1:                                                 //blue
        return makecol(0, 0, 170);
        break;
        
    case 2:                                                 //green
        return makecol(0, 170, 0);
        break;
        
    case 3:                                                 //cyan
        return makecol(0, 170, 170);
        break;
        
    case 4:                                                 //red
        return makecol(170, 0, 0);
        break;
        
    case 5:                                                 //magenta
        return makecol(170, 0, 170);
        break;
        
    case 6:                                                 //brown
        return makecol(170, 85, 170);
        break;
        
    case 7:                                                 //light grey
        return makecol(170, 170, 170);
        break;
        
    case 8:                                                 //dark grey
        return makecol(85, 85, 85);
        break;
        
    case 9:                                                 //light blue
        return makecol(85, 85, 255);
        break;
        
    case 10:                                                //light green
        return makecol(85, 255, 85);
        break;
        
    case 11:                                                //light cyan
        return makecol(85, 255, 255);
        break;
        
    case 12:                                                //light red (pink)
        return makecol(255, 85, 85);
        break;
        
    case 13:                                                //light magenta (lavender)
        return makecol(255, 85, 255);
        break;
        
    case 14:                                                //light brown (yellow)
        return makecol(255, 255, 85);
        break;
        
    case 15:                                                //white
        return makecol(255, 255, 255);
        break;
    }
    
    return -1;
}

INLINE int32_t is_in_rect(int32_t x,int32_t y,int32_t rx1,int32_t ry1,int32_t rx2,int32_t ry2)
{
    return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

void draw_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,int32_t flags)
{
    int32_t temp;
    
    if(flags&D_SELECTED)
    {
        temp=fg;
        fg=bg;
        bg=temp;
    }
    
    rect(dest,x+1,y+1,x+w-1,y+h-1,fg);
    rectfill(dest,x+1,y+1,x+w-3,y+h-3,bg);
    rect(dest,x,y,x+w-2,y+h-2,fg);
    textout_centre_ex(dest,font,text,(x+x+w)>>1,((y+y+h)>>1)-4,fg,-1);
}

bool do_text_button(int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        vsync();
        
        if(is_in_rect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
        {
            if(!over)
            {
                scare_mouse();
                draw_button(screen,x,y,w,h,text,bg,fg,D_SELECTED);
                unscare_mouse();
                over=true;
            }
        }
        else
        {
            if(over)
            {
                scare_mouse();
                draw_button(screen,x,y,w,h,text,bg,fg,0);
                unscare_mouse();
                over=false;
            }
        }
		
		update_hw_screen(false);
    }
    
    return over;
}

bool do_text_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg)
{
    bool over=false;
    
    while(gui_mouse_b())
    {
        vsync();
        
        if(is_in_rect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
        {
            if(!over)
            {
                scare_mouse();
                draw_button(screen,x,y,w,h,text,bg,fg,D_SELECTED);
                unscare_mouse();
                over=true;
            }
        }
        else
        {
            if(over)
            {
                scare_mouse();
                draw_button(screen,x,y,w,h,text,bg,fg,0);
                unscare_mouse();
                over=false;
            }
        }
        
		update_hw_screen(false);
    }
    
    if(over)
    {
        vsync();
        scare_mouse();
        draw_button(screen,x,y,w,h,text,bg,fg,0);
        unscare_mouse();
        
		update_hw_screen(false);
    }
    
    return over;
}

int32_t tab_count(TABPANEL *panel)
{
    int32_t i=0;
    
    for(i=0; panel[i].text; ++i)
    {
        /* do nothing */
    }
    
    return i;
}

int32_t tabs_width(TABPANEL *panel)
{
    int32_t i=0;
    int32_t w=0;
    
    for(i=0; panel[i].text; ++i)
    {
        w+=text_length(font, (char *)panel[i].text)+15;
    }
    
    return w+1;
}

bool uses_tab_arrows(TABPANEL *panel, int32_t maxwidth)
{
//  return (((d->d1&0xFF00)>>8)!=0||last_visible_tab(panel,((d->d1&0xFF00)>>8),d->w)+1<tab_count(panel));
    return (tabs_width(panel)>maxwidth);
}

int32_t last_visible_tab(TABPANEL *panel, int32_t first_tab, int32_t maxwidth)
{
	int32_t i=0;
	int32_t w=0;
	
	if(uses_tab_arrows(panel, maxwidth))
	{
		maxwidth-=28;
	}
	
	for(i=first_tab; panel[i].text; ++i)
	{
		w+=text_length(font, (char *)panel[i].text)+15;
		if(w>maxwidth)
		{
			return i-1;
		}
	}
	return i-1;
}

int32_t displayed_tabs_width(TABPANEL *panel, int32_t first_tab, int32_t maxwidth)
{
    int32_t i=0;
    int32_t w=0;
    
    for(i=first_tab; panel[i].text&&i<=last_visible_tab(panel, first_tab, maxwidth); ++i)
    {
        w+=text_length(font, (char *)panel[i].text)+15;
    }
    
    return w+1;
}

int32_t discern_tab(TABPANEL *panel, int32_t first_tab, int32_t x)
{
    int32_t i=0;
    int32_t w=0;
    
    for(i=first_tab; panel[i].text; i++)
    {
        w+=text_length(font, (char *)panel[i].text)+15;
        
        if(w>x)
        {
            return i;
        }
    }
    
    return -1;
}

int32_t d_tab_proc(int32_t msg, DIALOG *d, int32_t c)
{
    int32_t fg;
    int32_t i;
    int32_t tx;
    int32_t sd=2; //selected delta
    TABPANEL *panel=(TABPANEL *)d->dp;
    DIALOG   *panel_dialog=NULL, *current_object=NULL;
    int32_t selected=0;
    int32_t counter=0;
    ASSERT(d);
    (void) c;
    int32_t temp_d, temp_d2;
    
    if(d->dp==NULL)
    {
        return 0;
    }
    
    //d->d1&(0xFF00)>>8 is the first visible tab
    //d->d1&0x00FF is the currently? selected tab
    switch(msg)
    {
    case MSG_START:
    {
        d->d1&=0xFF00;
        d->d1|=0x00FF;
        
        //for each tab...
        for(i=0; panel[i].text; i++)
        {
            panel[i].objects=0;
            
            //see how many controls (i) are handled by this tab
            while(panel[i].dialog[(panel[i].objects)++]!=-1)
            {
                /* do nothing */
            }
            
            //because the -1 is counted, drop back one
            (panel[i].objects)--;
            //allocate space to store the x and y coordinates for them
            panel[i].xy=(int32_t*)malloc(panel[i].objects*2*sizeof(int32_t));
            //what dialog is this tab control in (programmer must set manually)
            panel_dialog=(DIALOG *)d->dp3;
            
            //for each object handled by this tab...
            for(counter=0; counter<panel[i].objects; counter++)
            {
                //assign current_object to one of the controls handled by the tab
                current_object=panel_dialog+(panel[i].dialog[counter]);
                //remember the x and y positions of the control
                panel[i].xy[counter*2]=current_object->x;
                panel[i].xy[counter*2+1]=current_object->y;
                //move the control offscreen
                current_object->x=screen->w*3;
                current_object->y=screen->h*3;
                current_object->flags|=D_HIDDEN;
            }
        }
        
        d->d2=1;
    }
    break;
    
    case MSG_END:
    {
        for(i=0; panel[i].text; i++)
        {
            //what dialog is this tab control in (programmer must set manually)
            panel_dialog=(DIALOG *)d->dp3;
            
            //for each object handled by this tab...
            for(counter=0; counter<panel[i].objects; counter++)
            {
                //assign current_object to one of the controls handled by the tab
                current_object=panel_dialog+(panel[i].dialog[counter]);
                //put the controls back where they belong
                current_object->x=panel[i].xy[counter*2];
                current_object->y=panel[i].xy[counter*2+1];
            }
        }
    }
    break;
    
    case MSG_IDLE:
    {
        //if we are off-screen
        if(d->x>screen->w||d->y>screen->h)
        {
            if(d->d2==1)
            {
                //for each tab
                for(i=0; panel[i].text; i++)
                {
                    //what dialog is this tab control in (programmer must set manually)
                    panel_dialog=(DIALOG *)d->dp3;
                    
                    //for each object handled by this tab...
                    for(counter=0; counter<panel[i].objects; counter++)
                    {
                        //assign current_object to one of the controls handled by the tab
                        current_object=panel_dialog+(panel[i].dialog[counter]);
                        //move the control offscreen
                        current_object->x=screen->w*3;
                        current_object->y=screen->h*3;
                    }
                }
                
                d->d2=0;
            }
        }
        else
        {
            d->d2=1;
        }
    }
    break;
    
    case MSG_DCLICK:
    case MSG_LPRESS:
    case MSG_LRELEASE:
    case MSG_MPRESS:
    case MSG_MRELEASE:
    case MSG_RPRESS:
    case MSG_RRELEASE:
    case MSG_GOTMOUSE:
    case MSG_LOSTMOUSE:
        break;
        
        
    case MSG_DRAW:
    {
        //backup the default font
        FONT *oldfont = font;
        
        if(d->x<screen->w&&d->y<screen->h)
        {
            //do the tabs have a custom font?
            if(d->dp2)
            {
                font = (FONT *)d->dp2;
            }
            
            panel_dialog=(DIALOG *)d->dp3;
            rectfill(screen, d->x, d->y, d->x+d->w-1, d->y+8+text_height(font), d->bg); //tab area
            fg = (d->flags & D_DISABLED) ? jwin_pal[jcDISABLED_FG] : d->fg;
            rectfill(screen, d->x+1, d->y+sd+text_height(font)+7, d->x+d->w-2, d->y+sd+d->h-2, d->bg); //panel
            //left, right, and bottom borders of the tab control
            _allegro_vline(screen, d->x, d->y+sd+7+text_height(font), d->y+sd+d->h-1, fg);
            _allegro_vline(screen, d->x+d->w-1, d->y+sd+7+text_height(font), d->y+sd+d->h-1, fg);
            _allegro_hline(screen, d->x+1, d->y+sd+d->h-1, d->x+d->w-2, fg);
            //tx=tab control x position
            tx=d->x;
            
            //if the programmer gave us a tab panel set to use...
            if(d->dp)
            {
                //if the first tab is not selected
                if(!(panel[((d->d1&0xFF00)>>8)].flags&D_SELECTED))
                {
                    //draw the initial tab panel top border
                    _allegro_hline(screen, tx, d->y+sd+6+text_height(font), tx+1, fg); //initial bottom
                }
                
                tx+=2;
                
                //for each tab (that actually has text)
                for(i=0; panel[i].text; ++i)
                {
                    if(panel[i].flags&D_SELECTED)
                    {
                        selected=i;
                    }
                }
                
                for(i=((d->d1&0xFF00)>>8); panel[i].text&&i<=last_visible_tab(panel,((d->d1&0xFF00)>>8),d->w); ++i)
                {
                    //sd is 2 if the panel is not selected, 0 if it is
                    sd=(panel[i].flags&D_SELECTED)?0:2;
                    
                    //if this is the first tab or if the previous tab is not selected (doesn't cover the left side of this tab)
                    if((i==((d->d1&0xFF00)>>8)) || (!(panel[i-1].flags&D_SELECTED)))
                    {
                        //draw the left side of the tab
                        _allegro_vline(screen, tx-(2-sd), d->y+sd+2, d->y+8+text_height(font), fg); //left side
                        putpixel(screen, tx+1-(2-sd), d->y+sd+1, fg);                               //left angle
                    }
                    
                    _allegro_hline(screen, tx+2-(2-sd), d->y+sd, tx+12+(2-sd)+text_length(font, (char *)panel[i].text), fg); //top
                    
                    //if the tab is not selected...
                    if(!(panel[i].flags&D_SELECTED))
                    {
                        //draw the top border of the tab panel under the tab itself
                        _allegro_hline(screen, tx+1, d->y+sd+6+text_height(font), tx+13+text_length(font, (char *)panel[i].text), fg); //bottom
                    }
                    
                    tx+=4;
                    //draw the text of the panel
                    gui_textout_ex(screen, (char *)panel[i].text, tx+4, d->y+sd+4, fg, d->bg, FALSE);
                    tx+=text_length(font, (char *)panel[i].text)+10;
                    
                    //if this is the last tab or the next tab is not selected...
                    if(!(panel[i+1].text) || (!(panel[i+1].flags&D_SELECTED)))
                    {
                        //draw the right side of this tab
                        putpixel(screen, tx-1+(2-sd), d->y+sd+1, fg); //right angle
                        _allegro_vline(screen, tx+(2-sd), d->y+sd+2, d->y+8+text_height(font), fg); //right side
                    }
                    
                    tx++;
                }
                
                if(uses_tab_arrows(panel, d->w))
                {
                    draw_button(screen,d->x+d->w-28,d->y+2, 14, 14, "\x8A", vc2(15), vc2(0), 0);
                    draw_button(screen,d->x+d->w-14,d->y+2, 14, 14, "\x8B", vc2(15), vc2(0), 0);
                }
            }
            
            //draw the remaining top border of the tab panel control
            _allegro_hline(screen, tx+(2-sd), d->y+8+text_height(font), d->x+d->w-1, fg); //ending bottom
            //restore the default font
            font = oldfont;
            
            //what dialog is this tab control in (programmer must set manually)
            panel_dialog=(DIALOG *)d->dp3;
            
            //for each object handled by the currently selected tab...
            for(counter=0; counter<panel[selected].objects; counter++)
            {
                //assign current_object to one of the controls handled by the tab
                current_object=panel_dialog+(panel[selected].dialog[counter]);
                //put the controls back where they belong
                current_object->x=panel[selected].xy[counter*2];
                current_object->y=panel[selected].xy[counter*2+1];
                object_message(current_object, MSG_DRAW, 0);
            }
        }
        
        //if there was a previously selected tab...
        if((d->d1&0x00FF)!=0x00FF)
        {
            //for each object handled by the tab
            for(counter=0; counter<panel[d->d1&0x00FF].objects; counter++)
            {
                //assign current_object to one of the controls handled by the tab
                current_object=panel_dialog+(panel[d->d1&0x00FF].dialog[counter]);
                //move them off screen
                current_object->x=screen->w*3;
                current_object->y=screen->h*3;
            }
        }
        
        for(i=0; panel[i].text; i++)
        {
            //if the tab is the selected one...
            if(panel[i].flags&D_SELECTED)
            {
                //remember it
                d->d1&=0xFF00;
                d->d1|=(i&0x00FF);
            }
        }
    }
    break;
    
    case MSG_CLICK:
    {
        FONT *oldfont = font;
        
        //do the tabs have a custom font?
        if(d->dp2)
        {
            font = (FONT *)d->dp2;
        }
        
        d->d1&=0xFF00;
        d->d1|=0x00FF;
        
        // is the mouse on one of the tab arrows (if visible) or in the tab area?
        if(uses_tab_arrows(panel, d->w)&&(is_in_rect(gui_mouse_x(),gui_mouse_y(), d->x+d->w-28, d->y+2, d->x+d->w-1, d->y+15)))
        {
            if(is_in_rect(gui_mouse_x(),gui_mouse_y(), d->x+d->w-28, d->y+2, d->x+d->w-15, d->y+15))
            {
                if(do_text_button_reset(d->x+d->w-28, d->y+2, 14, 14, "\x8A",d->bg,d->fg))
                {
                    temp_d=((d->d1&0xFF00)>>8);
                    temp_d2=(d->d1&0x00FF);
                    
                    if(temp_d>0)
                    {
                        --temp_d;
                    }
                    
                    d->d1=(temp_d<<8)|temp_d2;
                    d->flags|=D_DIRTY;
                }
            }
            else if(is_in_rect(gui_mouse_x(),gui_mouse_y(), d->x+d->w-14, d->y+2, d->x+d->w-1, d->y+15))
            {
                if(do_text_button_reset(d->x+d->w-14, d->y+2, 14, 14, "\x8B",d->bg,d->fg))
                {
                    temp_d=((d->d1&0xFF00)>>8);
                    temp_d2=(d->d1&0x00FF);
                    
                    if(last_visible_tab(panel, temp_d, d->w)<(tab_count(panel)-1))
                    {
                        ++temp_d;
                    }
                    
                    d->d1=(temp_d<<8)|temp_d2;
                    d->flags|=D_DIRTY;
                }
            }
        }
        else if(is_in_rect(gui_mouse_x(),gui_mouse_y(), d->x+2, d->y+2, d->x+displayed_tabs_width(panel,((d->d1&0xFF00)>>8),d->w), d->y+text_height(font)+9))
        {
            // for each tab...
            for(i=0; panel[i].text; i++)
            {
                // see if it's the selected tab and make note of it (in d->d1&0x00FF)
                if(panel[i].flags&D_SELECTED)
                {
                    d->d1&=0xFF00;
                    d->d1|=(i&0x00FF);
                }
            }
            
            // find out what the new tab (tb) will be (where the mouse is)
            selected=discern_tab(panel, ((d->d1&0xFF00)>>8), gui_mouse_x()-d->x-2);
            
            if(selected!=-1&&selected!=(d->d1&0x00FF))
            {
                for(i=0; panel[i].text; i++)
                {
                    panel[i].flags &= ~D_SELECTED;
                }
                
                panel[selected].flags |= D_SELECTED;
                object_message(d, MSG_DRAW, 0);
            }
        }
        
        font = oldfont;
    }
    break;
    }
    
    return D_O_K;
}

/***  The End  ***/

