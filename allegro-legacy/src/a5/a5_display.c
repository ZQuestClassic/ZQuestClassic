/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Stuff for BeOS.
 *
 *      By Jason Wilkins.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/ainta5.h"
#include "allegro/platform/ala5.h"

void all_render_screen(void);

#define ALLEGRO_LEGACY_PIXEL_FORMAT_8888  0
#define ALLEGRO_LEGACY_PIXEL_FORMAT_OTHER 1

static ALLEGRO_THREAD * _a5_screen_thread = NULL;
static ALLEGRO_BITMAP * _a5_screen = NULL;
static ALLEGRO_COLOR _a5_screen_palette[256];
static uint32_t _a5_screen_palette_a5[256];
static int _a5_screen_format = ALLEGRO_LEGACY_PIXEL_FORMAT_OTHER;

/* display thread data */
static bool _a5_disable_threaded_display = false;
static int _a5_display_width = 0;
static int _a5_display_height = 0;
static volatile int _a5_display_creation_done = 0;
static ALLEGRO_EVENT_QUEUE * _a5_display_thread_event_queue = NULL;
static ALLEGRO_TIMER * _a5_display_thread_timer = NULL;
static ALLEGRO_EVENT_SOURCE _a5_display_thread_event_source;
static ALLEGRO_EVENT_QUEUE * _a5_display_vsync_event_queue = NULL;

static bool _a5_setup_screen(int w, int h)
{
  ALLEGRO_STATE old_state;
  int pixel_format;
  _a5_display = al_create_display(w, h);
  if(!_a5_display)
  {
    goto fail;
  }
  al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
  al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE);

  _a5_screen = al_create_bitmap(w, h);
  al_restore_state(&old_state);
  if(!_a5_screen)
  {
    goto fail;
  }

  _a5_display_vsync_event_queue = al_create_event_queue();
  if(!_a5_display_vsync_event_queue)
  {
    goto fail;
  }
  al_init_user_event_source(&_a5_display_thread_event_source);
  al_register_event_source(_a5_display_vsync_event_queue, &_a5_display_thread_event_source);

  /* see if we need to hide the mouse cursor */
  if(al_is_mouse_installed() && !(gfx_capabilities & GFX_SYSTEM_CURSOR))
  {
      al_hide_mouse_cursor(_a5_display);
  }

  pixel_format = al_get_bitmap_format(_a5_screen);
  if(pixel_format == ALLEGRO_PIXEL_FORMAT_ARGB_8888 || pixel_format == ALLEGRO_PIXEL_FORMAT_ABGR_8888 || pixel_format == ALLEGRO_PIXEL_FORMAT_RGBA_8888 || pixel_format == ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE)
  {
    _a5_screen_format = ALLEGRO_LEGACY_PIXEL_FORMAT_8888;
  }
  return true;

  fail:
  {
    if(_a5_display_vsync_event_queue)
    {
      al_destroy_event_queue(_a5_display_vsync_event_queue);
      goto fail;
    }
    if(_a5_screen)
    {
      al_destroy_bitmap(_a5_screen);
      _a5_screen = NULL;
    }
    if(_a5_display)
    {
      al_destroy_display(_a5_display);
      _a5_display = NULL;
    }
    return false;
  }
}

static void _a5_destroy_screen(void)
{
  al_destroy_event_queue(_a5_display_vsync_event_queue);
  _a5_display_vsync_event_queue = NULL;
  al_destroy_bitmap(_a5_screen);
  _a5_screen = NULL;
  al_destroy_display(_a5_display);
  _a5_display = NULL;
}

static void * _a5_display_thread(ALLEGRO_THREAD * thread, void * data)
{
  ALLEGRO_EVENT event;
  float refresh_rate = 60.0;

  if(!_a5_setup_screen(_a5_display_width, _a5_display_height))
  {
    return NULL;
  }
  if(_refresh_rate_request > 0)
  {
    refresh_rate = _refresh_rate_request;
  }
  _a5_display_thread_timer = al_create_timer(1.0 / refresh_rate);
  if(!_a5_display_thread_timer)
  {
    goto fail;
  }
  _a5_display_thread_event_queue = al_create_event_queue();
  if(!_a5_display_thread_event_queue)
  {
    goto fail;
  }
  al_register_event_source(_a5_display_thread_event_queue, al_get_timer_event_source(_a5_display_thread_timer));
  al_start_timer(_a5_display_thread_timer);
  _a5_display_creation_done = 1;
  while(!al_get_thread_should_stop(_a5_screen_thread))
  {
    al_wait_for_event(_a5_display_thread_event_queue, &event);
    if(al_event_queue_is_empty(_a5_display_thread_event_queue))
    {
      all_render_screen();
      event.user.type = ALLEGRO_GET_EVENT_TYPE('V','S','N','C');
      al_emit_user_event(&_a5_display_thread_event_source, &event, NULL);
    }
  }
  if(_a5_display_thread_timer)
  {
    al_destroy_timer(_a5_display_thread_timer);
    _a5_display_thread_timer = NULL;
  }
  if(_a5_display_thread_event_queue)
  {
    al_destroy_event_queue(_a5_display_thread_event_queue);
    _a5_display_thread_event_queue = NULL;
  }
  _a5_destroy_screen();
  return NULL;

  fail:
  {
    if(_a5_display_thread_timer)
    {
      al_destroy_timer(_a5_display_thread_timer);
    }
    if(_a5_display_thread_event_queue)
    {
      al_destroy_event_queue(_a5_display_thread_event_queue);
    }
    _a5_destroy_screen();
    return NULL;
  }
}

static BITMAP * a5_display_init(int w, int h, int vw, int vh, int color_depth)
{
    BITMAP * bp;
    ALLEGRO_STATE old_state;
    int pixel_format;

    bp = create_bitmap(w, h);
    if(bp)
    {
      if(!_a5_disable_threaded_display)
      {
        _a5_display_creation_done = 0;
        _a5_display_width = w;
        _a5_display_height = h;
        _a5_screen_thread = al_create_thread(_a5_display_thread, NULL);
        al_start_thread(_a5_screen_thread);
        while(!_a5_display_creation_done);
      }
      else
      {
        if(!_a5_setup_screen(w, h))
        {
          return NULL;
        }
      }
      gfx_driver->w = bp->w;
      gfx_driver->h = bp->h;
      return bp;
    }
    return NULL;
}

static void a5_display_exit(BITMAP * bp)
{
  if(_a5_screen_thread)
  {
    al_destroy_thread(_a5_screen_thread);
    _a5_screen_thread = NULL;
  }
  else
  {
    _a5_destroy_screen();
  }
}

static void a5_display_vsync(void)
{
  ALLEGRO_EVENT event;

  if(!_a5_disable_threaded_display)
  {
    /* chew up already queued up events so we get only the next vsync */
    while(!al_event_queue_is_empty(_a5_display_vsync_event_queue))
    {
      al_wait_for_event(_a5_display_vsync_event_queue, &event);
    }
    al_wait_for_event(_a5_display_vsync_event_queue, &event);
  }
  else
  {
    al_wait_for_vsync();
  }
}

static void a5_palette_from_a4_palette(const PALETTE a4_palette, ALLEGRO_COLOR * a5_palette, int from, int to)
{
    int i;
    unsigned char r, g, b;

    if(a4_palette)
    {
        for(i = from; i <= to; i++)
        {
            a5_palette[i] = al_map_rgba_f((float)a4_palette[i].r / 63.0, (float)a4_palette[i].g / 63.0, (float)a4_palette[i].b / 63.0, 1.0);

            /* create palette of pre-packed pixels for various pixel formats */
            al_unmap_rgb(a5_palette[i], &r, &g, &b);
            if(al_get_bitmap_format(_a5_screen) == ALLEGRO_PIXEL_FORMAT_ABGR_8888)
            {
                _a5_screen_palette_a5[i] = r | (g << 8) | (b << 16) | (255 << 24);
            }
            else if(al_get_bitmap_format(_a5_screen) == ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE)
            {
                _a5_screen_palette_a5[i] = r | (g << 8) | (b << 16) | (255 << 24);
            }
            else if(al_get_bitmap_format(_a5_screen) == ALLEGRO_PIXEL_FORMAT_ARGB_8888)
            {
                _a5_screen_palette_a5[i] = b | (g << 8) | (r << 16) | (255 << 24);
            }
            else if(al_get_bitmap_format(_a5_screen) == ALLEGRO_PIXEL_FORMAT_RGBA_8888)
            {
                _a5_screen_palette_a5[i] = 255 | (b << 8) | (g << 16) | (r << 24);
            }
        }
    }
}

static void a5_display_set_palette(const struct RGB * palette, int from, int to, int vsync)
{
    if(vsync)
    {
      a5_display_vsync();
    }
    a5_palette_from_a4_palette(palette, _a5_screen_palette, from, to);
}

static void a5_display_move_mouse(int x, int y)
{
  al_set_mouse_xy(_a5_display, x, y);
}

static int a5_display_show_mouse(BITMAP * bp, int x, int y)
{
  if(bp)
  {
    return -1;
  }
  al_show_mouse_cursor(_a5_display);
  a5_display_move_mouse(x, y);
  return 0;
}

static void a5_display_hide_mouse(void)
{
  al_hide_mouse_cursor(_a5_display);
}

static ALLEGRO_COLOR a5_get_color(int depth, int color)
{
    int r, g, b, a;

    switch(depth)
    {
        case 15:
        {
            r = getr15(color);
            g = getg15(color);
            b = getb15(color);
            a = 0xFF;
            break;
        }
        case 16:
        {
            r = getr16(color);
            g = getg16(color);
            b = getb16(color);
            a = 0xFF;
            break;
        }
        case 24:
        {
            r = getr24(color);
            g = getg24(color);
            b = getb24(color);
            a = 0xFF;
            break;
        }
        case 32:
        {
            r = getr32(color);
            g = getg32(color);
            b = getb32(color);
            a = geta32(color);
            a = 0xFF;
            break;
        }
    }

    return al_map_rgba(r, g, b, a);
}

/* render 8-bit BITMAP to 32-bit ALLEGRO_BITMAP (8888 formats) */
static void render_8_8888(BITMAP * bp, ALLEGRO_BITMAP * a5bp)
{
    ALLEGRO_LOCKED_REGION * lr;
    uint8_t * line_8;
    uint32_t * line_32;
    int i, j;

    lr = al_lock_bitmap(a5bp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
    if(lr)
    {
        line_8 = lr->data;
        line_32 = lr->data;
        for(i = 0; i < bp->h; i++)
        {
            for(j = 0; j < bp->w; j++)
            {
                line_32[j] = _a5_screen_palette_a5[bp->line[i][j]];
            }
            line_8 += lr->pitch;
            line_32 = (uint32_t *)line_8;
        }
        al_unlock_bitmap(a5bp);
    }
}

static void render_other_8(BITMAP * bp)
{
    uint8_t * line_8;
    int i, j;

    for(i = 0; i < bp->h; i++)
    {
        for(j = 0; j < bp->w; j++)
        {
            line_8 = (uint8_t *)(bp->line[i]);
            al_put_pixel(j, i, _a5_screen_palette[line_8[j]]);
        }
    }
}

static void render_other_15(BITMAP * bp)
{
    uint16_t * line_16;
    int i, j;

    for(i = 0; i < bp->h; i++)
    {
        for(j = 0; j < bp->w; j++)
        {
            line_16 = (uint16_t *)(bp->line[i]);
            al_put_pixel(j, i, a5_get_color(15, line_16[j]));
        }
    }
}

static void render_other_16(BITMAP * bp)
{
    uint16_t * line_16;
    int i, j;

    for(i = 0; i < bp->h; i++)
    {
        for(j = 0; j < bp->w; j++)
        {
            line_16 = (uint16_t *)(bp->line[i]);
            al_put_pixel(j, i, a5_get_color(16, line_16[j]));
        }
    }
}

static void render_other_24(BITMAP * bp)
{
    int i, j;

    for(i = 0; i < bp->h; i++)
    {
        for(j = 0; j < bp->w; j++)
        {
            al_put_pixel(j, i, a5_get_color(24, _getpixel24(bp, j, i)));
        }
    }
}

static void render_other_32(BITMAP * bp)
{
    uint32_t * line_32;
    int i, j;

    for(i = 0; i < bp->h; i++)
    {
        for(j = 0; j < bp->w; j++)
        {
            line_32 = (uint32_t *)(bp->line[i]);
            al_put_pixel(j, i, a5_get_color(32, line_32[j]));
        }
    }
}

static void render_other(BITMAP * bp, ALLEGRO_BITMAP * a5bp)
{
    ALLEGRO_STATE old_state;
    int depth;

    depth = bitmap_color_depth(bp);
    al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP);
    al_set_target_bitmap(a5bp);
    al_lock_bitmap(a5bp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
    switch(depth)
    {
        case 8:
        {
            render_other_8(bp);
            break;
        }
        case 15:
        {
            render_other_15(bp);
            break;
        }
        case 16:
        {
            render_other_16(bp);
            break;
        }
        case 24:
        {
            render_other_24(bp);
            break;
        }
        case 32:
        {
            render_other_32(bp);
            break;
        }
    }
    al_unlock_bitmap(a5bp);
    al_restore_state(&old_state);
}

ALLEGRO_DISPLAY * all_get_display(void)
{
  return _a5_display;
}

void all_render_a5_bitmap(BITMAP * bp, ALLEGRO_BITMAP * a5bp)
{
    int depth;

    depth = bitmap_color_depth(bp);
    if(depth == 8 && _a5_screen_format == ALLEGRO_LEGACY_PIXEL_FORMAT_8888)
    {
        render_8_8888(bp, a5bp);
    }
    else
    {
        render_other(bp, a5bp);
    }
}

ALLEGRO_BITMAP * all_get_a5_bitmap(BITMAP * bp)
{
    ALLEGRO_BITMAP * bitmap;

    bitmap = al_create_bitmap(bp->w, bp->h);
    if(bitmap)
    {
        all_render_a5_bitmap(bp, bitmap);
        return bitmap;
    }
    return NULL;
}

void all_render_screen(void)
{
    all_render_a5_bitmap(screen, _a5_screen);
    al_draw_bitmap(_a5_screen, 0, 0, 0);
    al_flip_display();
}

void all_disable_threaded_display(void)
{
  _a5_disable_threaded_display = true;
}

GFX_DRIVER display_allegro_5 = {
   GFX_ALLEGRO_5,                     // int id;
   empty_string,                      // char *name;
   empty_string,                      // char *desc;
   "Allegro 5 Display",               // char *ascii_name;
   a5_display_init,   // AL_LEGACY_METHOD(struct BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
   a5_display_exit, //be_gfx_bwindowscreen_exit,         // AL_LEGACY_METHOD(void, exit, (struct BITMAP *b));
   NULL, //be_gfx_bwindowscreen_scroll,       // AL_LEGACY_METHOD(int, scroll, (int x, int y));
   a5_display_vsync, //be_gfx_vsync,                      // AL_LEGACY_METHOD(void, vsync, (void));
   a5_display_set_palette,  // AL_LEGACY_METHOD(void, set_palette, (struct RGB *p, int from, int to, int vsync));
   NULL, //be_gfx_bwindowscreen_request_scroll,// AL_LEGACY_METHOD(int, request_scroll, (int x, int y));
   NULL, //be_gfx_bwindowscreen_poll_scroll,  // AL_LEGACY_METHOD(int, poll_scroll, (void));
   NULL,                              // AL_LEGACY_METHOD(void, enable_triple_buffer, (void));
   NULL,                              // AL_LEGACY_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height));
   NULL,                              // AL_LEGACY_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_LEGACY_METHOD(int, show_video_bitmap, (struct BITMAP *bitmap));
   NULL, //be_gfx_bwindowscreen_request_video_bitmap,// AL_LEGACY_METHOD(int, request_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_LEGACY_METHOD(struct BITMAP *, create_system_bitmap, (int width, int height));
   NULL,                              // AL_LEGACY_METHOD(void, destroy_system_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_LEGACY_METHOD(int, set_mouse_sprite, (struct BITMAP *sprite, int xfocus, int yfocus));
   a5_display_show_mouse,                              // AL_LEGACY_METHOD(int, show_mouse, (struct BITMAP *bmp, int x, int y));
   a5_display_hide_mouse,                              // AL_LEGACY_METHOD(void, hide_mouse, (void));
   a5_display_move_mouse,                              // AL_LEGACY_METHOD(void, move_mouse, (int x, int y));
   NULL,                              // AL_LEGACY_METHOD(void, drawing_mode, (void));
   NULL,                              // AL_LEGACY_METHOD(void, save_state, (void));
   NULL,                              // AL_LEGACY_METHOD(void, restore_state, (void));
   NULL,                              // AL_LEGACY_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a));
   NULL, //be_gfx_bwindowscreen_fetch_mode_list,// AL_LEGACY_METHOD(int, fetch_mode_list, (void));
   0, 0,                              // int w, h;  /* physical (not virtual!) screen size */
   TRUE,                              // int linear;  /* true if video memory is linear */
   0,                                 // long bank_size;  /* bank size, in bytes */
   0,                                 // long bank_gran;  /* bank granularity, in bytes */
   0,                                 // long vid_mem;  /* video memory size, in bytes */
   0,                                 // long vid_phys_base;  /* physical address of video memory */
   TRUE                              // int windowed;  /* true if driver runs windowed */
};
