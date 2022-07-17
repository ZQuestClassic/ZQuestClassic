// ROUTINES TO SCALE WINDOWED ZQUEST BY AN INTEGER FACTOR
// SHOULD WORK FOR ALL WINDOWED DRIVERS ON ANY SYSTEM
//
//

#include "precompiled.h" //always first

#include "allegro.h"
#include "a5alleg.h"
#include "zqscale.h"

extern int32_t zq_screen_w, zq_screen_h;
extern void Z_message(char *format,...);
extern int32_t mouse_in_rect(int32_t x,int32_t y,int32_t w,int32_t h);

BITMAP *orig_screen = NULL;
BITMAP *proxy_screen = NULL;
GFX_VTABLE *orig_vtable = NULL;
GFX_VTABLE *proxy_vtable = NULL;
int32_t zqwin_scale = 1;


void clip_and_stretch_blit(BITMAP *src, BITMAP *dest, int32_t sx, int32_t sy, int32_t sw, int32_t sh, int32_t dx, int32_t dy, int32_t dw, int32_t dh, bool masked=false)
{
    //clip source and destination aarrgh
    
    //compute the scale factors. These must be preserved.
    double scalew = ((double)sw)/((double)dw);
    double scaleh = ((double)sh)/((double)dw);
    
    //clip the source
    if(sx < 0)
    {
        sw+=sx;
        dw+=(int32_t)(sx*scalew);
        sx=0;
    }
    
    if(sy<0)
    {
        sh+=sy;
        dh+=(int32_t)(sy*scaleh);
        sy=0;
    }
    
    if(sx + sw > src->w)
    {
        int32_t neww = src->w-sx;
        int32_t difw = neww-sw;
        sw = neww;
        dw += (int32_t)(difw*scalew);
    }
    
    if(sy+sh > src->h)
    {
        int32_t newh = src->h-sy;
        int32_t difh = newh-sh;
        sh = newh;
        dh += (int32_t)(difh*scaleh);
    }
    
    //clip the destination
    if(dx < 0)
    {
        dw+=dx;
        sw+=(int32_t)(dx/scalew);
        dx=0;
    }
    
    if(dy < 0)
    {
        dh+=dy;
        sh+=(int32_t)(dy/scaleh);
        dy=0;
    }
    
    if(dx+dw > dest->w)
    {
        int32_t neww = dest->w-dx;
        int32_t difw = neww-dw;
        dw = neww;
        sw += (int32_t)(difw/scalew);
    }
    
    if(dy+dh > dest->h)
    {
        int32_t newh = dest->h-dy;
        int32_t difh = newh-dh;
        dh = newh;
        sh += (int32_t)(difh/scaleh);
    }
    
    if(sw<0 || sh < 0 || dw < 0 || dh < 0)
        return;
        
    if(masked)
        masked_stretch_blit(src, dest, sx, sy, sw, sh, dx, dy, dw, dh);
    else
        stretch_blit(src, dest, sx, sy, sw, sh, dx, dy, dw, dh);
}

void clip_and_stretch_sprite(BITMAP *dest, BITMAP *src, int32_t x, int32_t y, int32_t w, int32_t h)
{
    clip_and_stretch_blit(src, dest, 0, 0, src->w, src->h, x, y, w, h,true);
}

void clip_and_rectfill(BITMAP *dest, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color)
{
    if(x1 >= dest->w || x2 < 0 || y1 >= dest->h || y2 < 0)
        return;
        
    if(x1<0)
    {
        x1=0;
    }
    
    if(y1<0)
    {
        y1=0;
    }
    
    if(x2>dest->w)
    {
        x2 = dest->w;
    }
    
    if(y2>dest->h)
    {
        y2 = dest->h;
    }
    
    rectfill(dest,x1,y1,x2,y2,color);
}



void zq_set_mouse_range(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t tempscale=zqwin_scale;
    
    if(!is_windowed_mode())
    {
        tempscale=1;
    }
    
    set_mouse_range(x1*tempscale, y1*tempscale, (x2+1)*tempscale-1, (y2+1)*tempscale-1);
}


INLINE int32_t offset_x(BITMAP *bmp)
{
    int32_t ret = 0;
    
    if(is_sub_bitmap(bmp))
    {
        ret = bmp->x_ofs;
    }
    
    return ret;
}

INLINE int32_t offset_y(BITMAP *bmp)
{
    int32_t ret = 0;
    
    if(is_sub_bitmap(bmp))
    {
        ret = bmp->y_ofs;
    }
    
    return ret;
}

void zqwin_acquire(struct BITMAP *bmp)
{
    if(orig_vtable->acquire)
    {
        orig_vtable->acquire(bmp);
    }
    
    acquire_bitmap(orig_screen);
    return;
}

void zqwin_release(struct BITMAP *bmp)
{
    if(orig_vtable->release)
    {
        orig_vtable->release(bmp);
    }
    
    release_bitmap(orig_screen);
    return;
}

void zqwin_putpixel(struct BITMAP *bmp, int32_t x, int32_t y, int32_t color)
{
    orig_vtable->putpixel(bmp, x, y, color);
    x += offset_x(bmp);
    y += offset_y(bmp);
    /*
      for(int32_t i = 0; i < zqwin_scale; i++)
      {
      for(int32_t j = 0; j < zqwin_scale; j++)
      {
      putpixel(orig_screen, x*zqwin_scale+i, y*zqwin_scale+j, color);
      }
      }
      */
    clip_and_rectfill(orig_screen, x*zqwin_scale, y*zqwin_scale, (((x+1)*zqwin_scale)-1), (((y+1)*zqwin_scale)-1), color);
    return;
}

void zqwin_vline(struct BITMAP *bmp, int32_t x, int32_t y1, int32_t y2, int32_t color)
{
    orig_vtable->vline(bmp, x, y1, y2, color);
    x += offset_x(bmp);
    int32_t dy = offset_y(bmp);
    y1 += dy;
    y2 += dy;
    /*
      for(int32_t i = 0; i < zqwin_scale; i++)
      {
      _allegro_vline(orig_screen, x*zqwin_scale+i, y1*zqwin_scale, (y2+1)*zqwin_scale-1, color);
      }
      */
    clip_and_rectfill(orig_screen, x*zqwin_scale, y1*zqwin_scale, (((x+1)*zqwin_scale)-1), (((y2+1)*zqwin_scale)-1), color);
    return;
}

void zqwin_hline(struct BITMAP *bmp, int32_t x1, int32_t y, int32_t x2, int32_t color)
{
    orig_vtable->hline(bmp, x1, y, x2, color);
    int32_t dx = offset_x(bmp);
    x1 += dx;
    x2 += dx;
    y += offset_y(bmp);
    /*
      for(int32_t i = 0; i < zqwin_scale; i++)
      {
      _allegro_hline(orig_screen, x1*zqwin_scale, y*zqwin_scale+i, (x2+1)*zqwin_scale-1, color);
      }
      */
    clip_and_rectfill(orig_screen, x1*zqwin_scale, y*zqwin_scale, (((x2+1)*zqwin_scale)-1), (((y+1)*zqwin_scale)-1), color);
    return;
}

void zqwin_hfill(struct BITMAP *bmp, int32_t x1, int32_t y, int32_t x2, int32_t color)
{
    orig_vtable->hfill(bmp, x1, y, x2, color);
    int32_t dx = offset_x(bmp);
    x1 += dx;
    x2 += dx;
    y += offset_y(bmp);
    
    for(int32_t i = 0; i < zqwin_scale; i++)
    {
        orig_screen->vtable->hfill(orig_screen, x1*zqwin_scale, y*zqwin_scale+i, (x2+1)*zqwin_scale-1, color);
    }
    
    return;
}

void line_helper(BITMAP *bmp, int32_t x, int32_t y, int32_t d)
{
    clip_and_rectfill(bmp, x*zqwin_scale, y*zqwin_scale, (((x+1)*zqwin_scale)-1), (((y+1)*zqwin_scale)-1), d);
}

void zqwin_line(struct BITMAP *bmp, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color)
{
    orig_vtable->line(bmp, x1, y1, x2, y2, color);
    //not sure what this stuff does here...
    /*
      int32_t dx = offset_x(bmp);
      x1 += dx;
      x2 += dx;
      int32_t dy = offset_y(bmp);
      y1 += dy;
      y2 += dy;
      do_line(orig_screen, x1*zqwin_scale, y1*zqwin_scale, (((x2+1)*zqwin_scale)-1), (((y2+1)*zqwin_scale)-1), color, line_helper);
    */
    
    return;
}

void zqwin_rectfill(struct BITMAP *bmp, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color)
{
    orig_vtable->rectfill(bmp, x1, y1, x2, y2, color);
    int32_t dx = offset_x(bmp);
    x1 += dx;
    x2 += dx;
    int32_t dy = offset_y(bmp);
    y1 += dy;
    y2 += dy;
    clip_and_rectfill(orig_screen, x1*zqwin_scale, y1*zqwin_scale, (x2+1)*zqwin_scale-1, (y2+1)*zqwin_scale-1, color);
    return;
}

void zqwin_draw_sprite(struct BITMAP *bmp, struct BITMAP *sprite, int32_t x, int32_t y)
{
    orig_vtable->draw_sprite(bmp, sprite, x, y);
    x += offset_x(bmp);
    y += offset_y(bmp);
    //this method crashes on OS X if we try to blit outside the destination sprite.
    //do some manual clipping
    clip_and_stretch_sprite(orig_screen, sprite, x*zqwin_scale, y*zqwin_scale, sprite->w*zqwin_scale, sprite->h*zqwin_scale);
    return;
}

void zqwin_draw_256_sprite(struct BITMAP *bmp, struct BITMAP *sprite, int32_t x, int32_t y)
{
    orig_vtable->draw_256_sprite(bmp, sprite, x, y);
    x += offset_x(bmp);
    y += offset_y(bmp);
    clip_and_stretch_sprite(orig_screen, sprite, x*zqwin_scale, y*zqwin_scale, sprite->w*zqwin_scale, sprite->h*zqwin_scale);
    return;
}

void zqwin_draw_character(struct BITMAP *bmp, struct BITMAP *sprite, int32_t x, int32_t y, int32_t color, int32_t bg)
{
    orig_vtable->draw_character(bmp,sprite,x,y,color,bg);
    clip_and_stretch_blit(bmp, orig_screen, x, y, sprite->w, sprite->h, (x+offset_x(bmp))*zqwin_scale, (y+offset_y(bmp))*zqwin_scale, sprite->w*zqwin_scale, sprite->h*zqwin_scale);
    return;
}

void zqwin_draw_glyph(struct BITMAP *bmp, AL_CONST struct FONT_GLYPH *glyph, int32_t x, int32_t y, int32_t color, int32_t bg)
{
    orig_vtable->draw_glyph(bmp,glyph,x,y,color,bg);
    clip_and_stretch_blit(bmp, orig_screen, x, y, glyph->w, glyph->h, (x+offset_x(bmp))*zqwin_scale, (y+offset_y(bmp))*zqwin_scale, glyph->w*zqwin_scale, glyph->h*zqwin_scale);
    return;
}

void zqwin_blit_from_memory(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_from_memory(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_blit_to_memory(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_to_memory(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_blit_from_system(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_from_system(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_blit_to_system(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_to_system(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_blit_to_self(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_to_self(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        int32_t mx = gui_mouse_x() - mouse_x_focus;
        int32_t my = gui_mouse_y() - mouse_y_focus;
        int32_t x1 = dest_x + offset_x(dest);
        int32_t y1 = dest_y + offset_y(dest);
        
        if(mouse_sprite)
        {
            int32_t mw = mouse_sprite->w;
            int32_t mh = mouse_sprite->h;
            
            if((mx < x1 + width) && (my < y1 + height) && (mx + mw >= x1) && (my + mh >= y1))
            {
                static BITMAP* tmp = NULL;
                mx = (mx > 0) ? mx : 0;
                my = (my > 0) ? my : 0;
                mw = MIN((proxy_screen->w-mx),mw);
                mh = MIN((proxy_screen->h-my),mh);
                int32_t l_x = (mx < x1) ? mx : x1;
                int32_t l_y = (my < y1) ? my : y1;
                int32_t __mx2 = mx + mw;
                int32_t __my2 = my + mh;
                int32_t x2 = x1 + width;
                int32_t y2 = y1 + height;
                x2 = (x2 > __mx2) ? x2 : __mx2;
                y2 = (y2 > __my2) ? y2 : __my2;
                int32_t l_w = x2 - l_x;
                int32_t l_h = y2 - l_y;
                
                if(!tmp)
                {
                    tmp = create_bitmap_ex(8, mw, mh);
                }
                else if(mw > tmp->w  ||  mh > tmp->h)
                {
                    destroy_bitmap(tmp);
                    tmp = create_bitmap_ex(8, mw, mh);
                }
                
                orig_vtable->blit_to_self(proxy_screen, tmp, mx, my, 0, 0, mw, mh);
                //orig_vtable->draw_256_sprite(proxy_screen, mouse_sprite, mx, my);
                clip_and_stretch_blit(proxy_screen, orig_screen, l_x, l_y, l_w, l_h, l_x*zqwin_scale, l_y*zqwin_scale, l_w*zqwin_scale, l_h*zqwin_scale);
                orig_vtable->blit_to_self(tmp, proxy_screen, 0, 0, mx, my, mw, mh);
            }
            else
            {
                clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, x1*zqwin_scale, y1*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
            }
        }
        else
        {
            clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, x1*zqwin_scale, y1*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
        }
    }
    
    return;
}

void zqwin_blit_to_self_forward(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_to_self_forward(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_blit_to_self_backward(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_to_self_backward(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_blit_between_formats(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->blit_between_formats(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_masked_blit(struct BITMAP *source, struct BITMAP *dest, int32_t source_x, int32_t source_y, int32_t dest_x, int32_t dest_y, int32_t width, int32_t height)
{
    orig_vtable->masked_blit(source,dest,source_x,source_y,dest_x,dest_y,width,height);
    
    if(is_same_bitmap(dest, proxy_screen))
    {
        clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
    }
    
    return;
}

void zqwin_clear_to_color(struct BITMAP *bitmap, int32_t color)
{
//  orig_vtable->clear_to_color(bitmap, color);
    orig_vtable->rectfill(bitmap,0,0,bitmap->w-1,bitmap->h-1,color);
    
    if(is_same_bitmap(bitmap, proxy_screen))
    {
// clear_to_color(orig_screen, color);
        rectfill(orig_screen,0,0,orig_screen->w-1,orig_screen->h-1,color);
    }
    
    return;
}

#include <stdio.h>
void zqwin_set_clip(struct BITMAP *bitmap)
{
    if(is_same_bitmap(bitmap, proxy_screen))
    {
        int32_t dx = offset_x(bitmap);
        int32_t dy = offset_y(bitmap);
        orig_screen->cl = (bitmap->cl+dx)*zqwin_scale;
        orig_screen->cr = (bitmap->cr+dx+1)*zqwin_scale-1;
        orig_screen->ct = (bitmap->ct+dy)*zqwin_scale;
        orig_screen->cb = (bitmap->cb+dy+1)*zqwin_scale-1;
    }
    
    return;
}


bool is_windowed_mode(int32_t mode)
{
    switch(mode)
    {
#ifdef ALLEGRO_WINDOWS
    
    case GFX_AUTODETECT_WINDOWED:
    case GFX_DIRECTX_WIN:
    case GFX_DIRECTX_OVL:
    case GFX_GDI:
        return true;
#elif defined ALLEGRO_LINUX
        
    case GFX_AUTODETECT_WINDOWED:
    case GFX_XWINDOWS:
    case GFX_XDGA:
    case GFX_XDGA2:
    case GFX_XDGA2_SOFT:
        return true;
#elif defined ALLEGRO_MACOSX
        
    case GFX_AUTODETECT_WINDOWED:
    case GFX_QUARTZ_WINDOW:
        return true;
#endif
        
    default:
        ;
    }
    
    return false;
}


int32_t zqwin_set_gfx_mode(int32_t card, int32_t w, int32_t h, int32_t v_w, int32_t v_h)
{
    //these are here to bypass compiler warnings about unused arguments
    v_w=v_w;
    v_h=v_h;
    
    int32_t failed = 0;
    
    bool wasProxied = (screen == proxy_screen) && (proxy_screen != NULL);
    
    if(wasProxied)
    {
        screen = orig_screen;
    }
    
    bool becomesWindowed = true;
    
    if(is_windowed_mode(card))
    {
        failed = set_gfx_mode(card,w*zqwin_scale,h*zqwin_scale,0,0);
    }
    else
    {
        //    zqwin_set_scale(2, true);
        failed = set_gfx_mode(card,w*zqwin_scale,h*zqwin_scale,0,0);
    }
    
    if(failed)
    {
        if(wasProxied)
        {
            screen = proxy_screen;
        }
    }
    else
    {
        orig_screen = screen;
        
        if(becomesWindowed && (zqwin_scale > 1))
        {
            clear_bitmap(proxy_screen);
            screen = proxy_screen;
            //set_mouse_speed(zqwin_scale<<1, zqwin_scale<<1);
            zq_set_mouse_range(0,0,w-1,h-1);
        }
        
        /*
          else
          {
          set_mouse_speed(2,2);
          }
          */
    }
    
    set_mouse_speed(2,2);
// if(is_windowed_mode())
    //{
    //if(zcgui_screen) destroy_bitmap(zcgui_screen);
    //zcgui_screen = create_bitmap_ex(get_color_depth(), screen->w, screen->h);
    //zcgui_scrref = zcgui_screen;
    //}
    //else
    //{
    //zcgui_scrref = screen;
    //}
    
    
    return failed;
}

int32_t zqscale_mouse_x()
{
    double my_x=(double)mouse_x/(double)(gfx_driver->windowed?zqwin_scale:1);
    return (int32_t)my_x;
}

int32_t zqscale_mouse_y()
{
    double my_y=(double)mouse_y/(double)(gfx_driver->windowed?zqwin_scale:1);
    return (int32_t)my_y;
}


bool zqwin_set_scale(int32_t scale, bool defer)
{
    // These are here to make compiler happy
    defer=defer;
    
    int32_t old_scale;
    old_scale = zqwin_scale;

    all_set_scale(scale);

    zqwin_scale = (scale > 1) ? scale : 1;
#ifndef COMPILE_FOR_LINUX
    gui_mouse_x=zqscale_mouse_x;
    gui_mouse_y=zqscale_mouse_y;
#endif
    return true;
}
