// ROUTINES TO SCALE WINDOWED ZQUEST BY AN INTEGER FACTOR
// SHOULD WORK FOR ALL WINDOWED DRIVERS ON ANY SYSTEM
//
//
#include "allegro.h"

extern int zq_screen_w, zq_screen_h;
extern void Z_message(char *format,...);
extern int mouse_in_rect(int x,int y,int w,int h);

BITMAP *orig_screen = NULL;
BITMAP *proxy_screen = NULL;
GFX_VTABLE *orig_vtable = NULL;
GFX_VTABLE *proxy_vtable = NULL;
int zqwin_scale = 1;

bool zqwin_set_scale(int scale, bool defer = false);

void clip_and_stretch_blit(BITMAP *src, BITMAP *dest, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, bool masked=false)
{
	//clip source and destination aarrgh

	//compute the scale factors. These must be preserved.
	double scalew = ((double)sw)/((double)dw);
	double scaleh = ((double)sh)/((double)dw);
	//clip the source
	if(sx < 0)
	{
		sw+=sx;
		dw+=(int)(sx*scalew);
		sx=0;
	}
	if(sy<0)
	{
		sh+=sy;
		dh+=(int)(sy*scaleh);
		sy=0;
	}
	if(sx + sw > src->w)
	{
		int neww = src->w-sx;
		int difw = neww-sw;
		sw = neww;
		dw += (int)(difw*scalew);
	}
	if(sy+sh > src->h)
	{
		int newh = src->h-sy;
		int difh = newh-sh;
		sh = newh;
		dh += (int)(difh*scaleh);
	}
	//clip the destination
	if(dx < 0)
	{
		dw+=dx;
		sw+=(int)(dx/scalew);
		dx=0;
	}
	if(dy < 0)
	{
		dh+=dy;
		sh+=(int)(dy/scaleh);
		dy=0;
	}
	if(dx+dw > dest->w)
	{
		int neww = dest->w-dx;
		int difw = neww-dw;
		dw = neww;
		sw += (int)(difw/scalew);
	}
	if(dy+dh > dest->h)
	{
		int newh = dest->h-dy;
		int difh = newh-dh;
		dh = newh;
		sh += (int)(difh/scaleh);
	}
	if(sw<0 || sh < 0 || dw < 0 || dh < 0)
		return;
	if(masked)
		masked_stretch_blit(src, dest, sx, sy, sw, sh, dx, dy, dw, dh);
	else
		stretch_blit(src, dest, sx, sy, sw, sh, dx, dy, dw, dh);
}

void clip_and_stretch_sprite(BITMAP *dest, BITMAP *src, int x, int y, int w, int h)
{
	clip_and_stretch_blit(src, dest, 0, 0, src->w, src->h, x, y, w, h,true);
}

void clip_and_rectfill(BITMAP *dest, int x1, int y1, int x2, int y2, int color)
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



void zq_set_mouse_range(int x1, int y1, int x2, int y2)
{
  int tempscale=zqwin_scale;
  if (!is_windowed_mode())
  {
    tempscale=1;
  }
  set_mouse_range(x1*tempscale, y1*tempscale, (x2+1)*tempscale-1, (y2+1)*tempscale-1);
}


INLINE int offset_x(BITMAP *bmp)
{
  int ret = 0;
  if (is_sub_bitmap(bmp))
  {
    ret = bmp->x_ofs;
  }
  return ret;
}

INLINE int offset_y(BITMAP *bmp)
{
  int ret = 0;
  if (is_sub_bitmap(bmp))
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

void zqwin_putpixel(struct BITMAP *bmp, int x, int y, int color)
{
  orig_vtable->putpixel(bmp, x, y, color);
  x += offset_x(bmp);
  y += offset_y(bmp);
  /*
    for(int i = 0; i < zqwin_scale; i++)
    {
    for(int j = 0; j < zqwin_scale; j++)
    {
    putpixel(orig_screen, x*zqwin_scale+i, y*zqwin_scale+j, color);
    }
    }
    */
  clip_and_rectfill(orig_screen, x*zqwin_scale, y*zqwin_scale, (((x+1)*zqwin_scale)-1), (((y+1)*zqwin_scale)-1), color);
  return;
}

void zqwin_vline(struct BITMAP *bmp, int x, int y1, int y2, int color)
{
  orig_vtable->vline(bmp, x, y1, y2, color);
  x += offset_x(bmp);
  int dy = offset_y(bmp);
  y1 += dy;
  y2 += dy;
  /*
    for(int i = 0; i < zqwin_scale; i++)
    {
    _allegro_vline(orig_screen, x*zqwin_scale+i, y1*zqwin_scale, (y2+1)*zqwin_scale-1, color);
    }
    */
  clip_and_rectfill(orig_screen, x*zqwin_scale, y1*zqwin_scale, (((x+1)*zqwin_scale)-1), (((y2+1)*zqwin_scale)-1), color);
  return;
}

void zqwin_hline(struct BITMAP *bmp, int x1, int y, int x2, int color)
{
  orig_vtable->hline(bmp, x1, y, x2, color);
  int dx = offset_x(bmp);
  x1 += dx;
  x2 += dx;
  y += offset_y(bmp);
  /*
    for(int i = 0; i < zqwin_scale; i++)
    {
    _allegro_hline(orig_screen, x1*zqwin_scale, y*zqwin_scale+i, (x2+1)*zqwin_scale-1, color);
    }
    */
  clip_and_rectfill(orig_screen, x1*zqwin_scale, y*zqwin_scale, (((x2+1)*zqwin_scale)-1), (((y+1)*zqwin_scale)-1), color);
  return;
}

void zqwin_hfill(struct BITMAP *bmp, int x1, int y, int x2, int color)
{
  orig_vtable->hfill(bmp, x1, y, x2, color);
  int dx = offset_x(bmp);
  x1 += dx;
  x2 += dx;
  y += offset_y(bmp);
  for(int i = 0; i < zqwin_scale; i++)
  {
    orig_screen->vtable->hfill(orig_screen, x1*zqwin_scale, y*zqwin_scale+i, (x2+1)*zqwin_scale-1, color);
  }
  return;
}

void line_helper(BITMAP *bmp, int x, int y, int d)
{
  clip_and_rectfill(bmp, x*zqwin_scale, y*zqwin_scale, (((x+1)*zqwin_scale)-1), (((y+1)*zqwin_scale)-1), d);
}

void zqwin_line(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
  orig_vtable->line(bmp, x1, y1, x2, y2, color);
  //not sure what this stuff does here...
/*
  int dx = offset_x(bmp);
  x1 += dx;
  x2 += dx;
  int dy = offset_y(bmp);
  y1 += dy;
  y2 += dy;
  do_line(orig_screen, x1*zqwin_scale, y1*zqwin_scale, (((x2+1)*zqwin_scale)-1), (((y2+1)*zqwin_scale)-1), color, line_helper);
*/

  return;
}

void zqwin_rectfill(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
  orig_vtable->rectfill(bmp, x1, y1, x2, y2, color);
  int dx = offset_x(bmp);
  x1 += dx;
  x2 += dx;
  int dy = offset_y(bmp);
  y1 += dy;
  y2 += dy;
  clip_and_rectfill(orig_screen, x1*zqwin_scale, y1*zqwin_scale, (x2+1)*zqwin_scale-1, (y2+1)*zqwin_scale-1, color);
  return;
}

void zqwin_draw_sprite(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y)
{
  orig_vtable->draw_sprite(bmp, sprite, x, y);
  x += offset_x(bmp);
  y += offset_y(bmp);
  //this method crashes on OS X if we try to blit outside the destination sprite.
  //do some manual clipping
  clip_and_stretch_sprite(orig_screen, sprite, x*zqwin_scale, y*zqwin_scale, sprite->w*zqwin_scale, sprite->h*zqwin_scale);
  return;
}

void zqwin_draw_256_sprite(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y)
{
  orig_vtable->draw_256_sprite(bmp, sprite, x, y);
  x += offset_x(bmp);
  y += offset_y(bmp);
  clip_and_stretch_sprite(orig_screen, sprite, x*zqwin_scale, y*zqwin_scale, sprite->w*zqwin_scale, sprite->h*zqwin_scale);
  return;
}

void zqwin_draw_character(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color, int bg)
{
  orig_vtable->draw_character(bmp,sprite,x,y,color,bg);
  clip_and_stretch_blit(bmp, orig_screen, x, y, sprite->w, sprite->h, (x+offset_x(bmp))*zqwin_scale, (y+offset_y(bmp))*zqwin_scale, sprite->w*zqwin_scale, sprite->h*zqwin_scale);
  return;
}

void zqwin_draw_glyph(struct BITMAP *bmp, AL_CONST struct FONT_GLYPH *glyph, int x, int y, int color, int bg)
{
  orig_vtable->draw_glyph(bmp,glyph,x,y,color,bg);
  clip_and_stretch_blit(bmp, orig_screen, x, y, glyph->w, glyph->h, (x+offset_x(bmp))*zqwin_scale, (y+offset_y(bmp))*zqwin_scale, glyph->w*zqwin_scale, glyph->h*zqwin_scale);
  return;
}

void zqwin_blit_from_memory(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_from_memory(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_blit_to_memory(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_to_memory(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_blit_from_system(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_from_system(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_blit_to_system(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_to_system(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_blit_to_self(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_to_self(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    int mx = gui_mouse_x() - mouse_x_focus;
    int my = gui_mouse_y() - mouse_y_focus;
    int x1 = dest_x + offset_x(dest);
    int y1 = dest_y + offset_y(dest);
    if(mouse_sprite)
    {
      int mw = mouse_sprite->w;
      int mh = mouse_sprite->h;
      if( (mx < x1 + width) && (my < y1 + height) && (mx + mw >= x1) && (my + mh >= y1) )
      {
        static BITMAP* tmp = NULL;
        mx = (mx > 0) ? mx : 0;
        my = (my > 0) ? my : 0;
        mw = MIN((proxy_screen->w-mx),mw);
        mh = MIN((proxy_screen->h-my),mh);
        int l_x = (mx < x1) ? mx : x1;
        int l_y = (my < y1) ? my : y1;
        int __mx2 = mx + mw;
        int __my2 = my + mh;
        int x2 = x1 + width;
        int y2 = y1 + height;
        x2 = (x2 > __mx2) ? x2 : __mx2;
        y2 = (y2 > __my2) ? y2 : __my2;
        int l_w = x2 - l_x;
        int l_h = y2 - l_y;
        if (!tmp)
        {
          tmp = create_bitmap(mw, mh);
        }
        else if (mw > tmp->w  ||  mh > tmp->h)
          {
            destroy_bitmap(tmp);
            tmp = create_bitmap(mw, mh);
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

void zqwin_blit_to_self_forward(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_to_self_forward(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_blit_to_self_backward(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_to_self_backward(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_blit_between_formats(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->blit_between_formats(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_masked_blit(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
  orig_vtable->masked_blit(source,dest,source_x,source_y,dest_x,dest_y,width,height);
  if(is_same_bitmap(dest, proxy_screen))
  {
    clip_and_stretch_blit(dest, orig_screen, dest_x, dest_y, width, height, (dest_x+offset_x(dest))*zqwin_scale, (dest_y+offset_y(dest))*zqwin_scale, width*zqwin_scale, height*zqwin_scale);
  }
  return;
}

void zqwin_clear_to_color(struct BITMAP *bitmap, int color)
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
		int dx = offset_x(bitmap);
		int dy = offset_y(bitmap);
		orig_screen->cl = (bitmap->cl+dx)*zqwin_scale;
		orig_screen->cr = (bitmap->cr+dx+1)*zqwin_scale-1;
		orig_screen->ct = (bitmap->ct+dy)*zqwin_scale;
		orig_screen->cb = (bitmap->cb+dy+1)*zqwin_scale-1;
	}
	return;
}


bool is_windowed_mode(int mode)
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


int zqwin_set_gfx_mode(int card, int w, int h, int v_w, int v_h)
{
  //these are here to bypass compiler warnings about unused arguments
  v_w=v_w;
  v_h=v_h;

  int failed = 0;

  bool wasProxied = (screen == proxy_screen) && (proxy_screen != NULL);
  if (wasProxied)
  {
    screen = orig_screen;
  }
  bool becomesWindowed = true;
  if (is_windowed_mode(card))
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

int zqscale_mouse_x()
{
  double my_x=(double)mouse_x/(double)(gfx_driver->windowed?zqwin_scale:1);
  return (int)my_x;
}

int zqscale_mouse_y()
{
  double my_y=(double)mouse_y/(double)(gfx_driver->windowed?zqwin_scale:1);
  return (int)my_y;
}


bool zqwin_set_scale(int scale, bool defer)
{
  // These are here to make compiler happy
  defer=defer;

  int old_scale;
  old_scale = zqwin_scale;

#ifndef ROMVIEW_SCALE
  zqwin_scale = (scale > 1) ? scale : 1;
  #ifndef COMPILE_FOR_LINUX
	gui_mouse_x=zqscale_mouse_x;
	gui_mouse_y=zqscale_mouse_y;
  #endif
  return true;
#else

  if ((!proxy_screen) && (scale > 1))
  {
    proxy_screen = create_bitmap(zq_screen_w, zq_screen_h);
    proxy_vtable = (GFX_VTABLE*)malloc(sizeof(GFX_VTABLE));
    if (!proxy_screen || !proxy_vtable)
    {
      zqwin_scale = 1;
      return false;
    }
    orig_vtable = proxy_screen->vtable;
    *proxy_vtable = *orig_vtable;
    //FILL IT IN
    proxy_vtable->acquire = zqwin_acquire;
    proxy_vtable->release = zqwin_release;
	proxy_vtable->set_clip = zqwin_set_clip;
    if(proxy_vtable->putpixel) proxy_vtable->putpixel = zqwin_putpixel;
    if(proxy_vtable->vline) proxy_vtable->vline = zqwin_vline;
    if(proxy_vtable->hline) proxy_vtable->hline = zqwin_hline;
    if(proxy_vtable->hfill) proxy_vtable->hfill = zqwin_hfill;
    if(proxy_vtable->line) proxy_vtable->line = zqwin_line;
    if(proxy_vtable->rectfill) proxy_vtable->rectfill = zqwin_rectfill;
    if(proxy_vtable->draw_sprite) proxy_vtable->draw_sprite = zqwin_draw_sprite;
    if(proxy_vtable->draw_256_sprite) proxy_vtable->draw_256_sprite = zqwin_draw_256_sprite;
    if(proxy_vtable->draw_character) proxy_vtable->draw_character = zqwin_draw_character;
    if(proxy_vtable->draw_glyph) proxy_vtable->draw_glyph = zqwin_draw_glyph;
    if(proxy_vtable->blit_from_memory) proxy_vtable->blit_from_memory = zqwin_blit_from_memory;
    if(proxy_vtable->blit_to_memory) proxy_vtable->blit_to_memory = zqwin_blit_to_memory;
    if(proxy_vtable->blit_from_system) proxy_vtable->blit_from_system = zqwin_blit_from_system;
    if(proxy_vtable->blit_to_system) proxy_vtable->blit_to_system = zqwin_blit_to_system;
    if(proxy_vtable->blit_to_self) proxy_vtable->blit_to_self = zqwin_blit_to_self;
    if(proxy_vtable->blit_to_self_forward) proxy_vtable->blit_to_self_forward = zqwin_blit_to_self_forward;
    if(proxy_vtable->blit_to_self_backward) proxy_vtable->blit_to_self_backward = zqwin_blit_to_self_backward;
    if(proxy_vtable->blit_between_formats) proxy_vtable->blit_between_formats = zqwin_blit_between_formats;
    if(proxy_vtable->masked_blit) proxy_vtable->masked_blit = zqwin_masked_blit;
    if(proxy_vtable->clear_to_color) proxy_vtable->clear_to_color = zqwin_clear_to_color;
    //THEN INTERJECT IT
    proxy_screen->vtable = proxy_vtable;
  }

  zqwin_scale = (scale > 1) ? scale : 1;
  if ((!defer) && (zqwin_scale != old_scale))
    //if (zqwin_scale != old_scale)
  {
    if (gfx_driver && gfx_driver->windowed)
    {
	  if (zqwin_set_gfx_mode(gfx_driver->id,zq_screen_w,zq_screen_h,0,0))
      {
        zqwin_scale = old_scale;
        return false;
      }
    }
  }


	// Mouse Scale Issue Fix Just commented these out and presto it works :-D
#ifndef COMPILE_FOR_LINUX
	gui_mouse_x=zqscale_mouse_x;
	gui_mouse_y=zqscale_mouse_y;
#endif

  return true;
#endif
}

 
