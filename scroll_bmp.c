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

#include "scroll_bmp.h"
#include "zc_alleg.h"

static void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg);

inline int is_in_rect(int x,int y,int rx1,int ry1,int rx2,int ry2)
{
  return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

/* _draw_scrollable_frame:
 *  Helper function to draw a frame for all objects with vertical scrollbars.
 */
void _draw_scrollable_frame_vh(DIALOG *d,
int doc_height, int y_offset, int frame_height,
int doc_width, int x_offset, int frame_width,
int vscroll_bar_style, int hscroll_bar_style,
int fg_color, int bg)
{
  int vs_thumb_len, hs_thumb_wid, len, wid;
  BITMAP *pattern;
  int hh, ww;
  int vs_thumb_x, vs_thumb_y;
  int hs_thumb_x, hs_thumb_y;
  int draw_vscroll=(((doc_height > frame_height)||
    (vscroll_bar_style==scrollbar_on))&&
    (vscroll_bar_style!=scrollbar_off));
  int draw_hscroll=(((doc_width > frame_width)||
    (hscroll_bar_style==scrollbar_on))&&
    (hscroll_bar_style!=scrollbar_off));
  int vscroll_w=0, hscroll_h=0;

  // draw frame
  rect(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, fg_color);

  // possibly draw scrollbar
  if (draw_hscroll||draw_vscroll)
  {
    if (draw_vscroll)
    {
      vline(screen, d->x+d->w-13, d->y+1, d->y+d->h-2, fg_color);
      vscroll_w=12;
    }
    if (draw_hscroll)
    {
      hline(screen, d->x+1, d->y+d->h-13, d->x+d->w-2, fg_color);
      hscroll_h=12;
    }
    if (draw_hscroll&&draw_vscroll)
    {
      // scrollbar with focus
      if (d->flags & D_GOTFOCUS)
      {
        //text area
        dotted_rect(d->x+1, d->y+1, d->x+d->w-14, d->y+d->h-14, fg_color, bg);
        //horizontal scroll bar
        dotted_rect(d->x+1, d->y+d->h-12, d->x+d->w-14, d->y+d->h-2, fg_color, bg);
        //vertical scroll bar
        dotted_rect(d->x+d->w-12, d->y+1, d->x+d->w-2, d->y+d->h-14, fg_color, bg);
        //corner piece
        dotted_rect(d->x+d->w-12, d->y+d->h-12, d->x+d->w-2, d->y+d->h-2, fg_color, bg);
        rectfill(screen, d->x+d->w-11, d->y+d->h-11, d->x+d->w-3, d->y+d->h-3, bg);
      }
      else
      {
        //text area
        rect(screen, d->x+1, d->y+1, d->x+d->w-14, d->y+d->h-14, bg);
        //horizontal scroll bar
        rect(screen, d->x+1, d->y+d->h-12, d->x+d->w-14, d->y+d->h-2, bg);
        //vertical scroll bar
        rect(screen, d->x+d->w-12, d->y+1, d->x+d->w-2, d->y+d->h-14, bg);
        //corner piece
        rectfill(screen, d->x+d->w-12, d->y+d->h-12, d->x+d->w-2, d->y+d->h-2, bg);
      }
    }
    else if (draw_hscroll)
    {
      // scrollbar with focus
      if (d->flags & D_GOTFOCUS)
      {
        //text area
        dotted_rect(d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-14, fg_color, bg);
        //horizontal scroll bar
        dotted_rect(d->x+1, d->y+d->h-12, d->x+d->w-2, d->y+d->h-2, fg_color, bg);
      }
      else
      {
        //text area
        rect(screen, d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-14, bg);
        //horizontal scroll bar
        rect(screen, d->x+1, d->y+d->h-12, d->x+d->w-2, d->y+d->h-2, bg);
      }
    }                                                       //draw_vscroll
    else
    {
      // scrollbar with focus
      if (d->flags & D_GOTFOCUS)
      {
        //text area
        dotted_rect(d->x+1, d->y+1, d->x+d->w-14, d->y+d->h-2, fg_color, bg);
        //vertical scroll bar
        dotted_rect(d->x+d->w-12, d->y+1, d->x+d->w-2, d->y+d->h-2, fg_color, bg);
      }
      else
      {
        //text area
        rect(screen, d->x+1, d->y+1, d->x+d->w-14, d->y+d->h-2, bg);
        //vertical scroll bar
        rect(screen, d->x+d->w-12, d->y+1, d->x+d->w-2, d->y+d->h-2, bg);
      }
    }

    // create and draw the scrollbar
    pattern = create_bitmap(2, 2);
    putpixel(pattern, 0, 1, bg);
    putpixel(pattern, 1, 0, bg);
    putpixel(pattern, 0, 0, fg_color);
    putpixel(pattern, 1, 1, fg_color);

    hh = d->h - 5 - hscroll_h;
    ww = d->w - 5 - vscroll_w;

    if (draw_vscroll)
    {
      al_trace("draw vscroll\n");
      //x pos of thumb
      vs_thumb_x = d->x+d->w-11;
      //y pos of thumb
      vs_thumb_y = d->y+2;
      //length of thumb
      vs_thumb_len = (hh * frame_height + doc_height/2) / doc_height;
      //if thumb is not at top, draw a blank spot there
      if (y_offset > 0)
      {
        len = ((hh * y_offset) + doc_height/2) / doc_height;
        rectfill(screen, vs_thumb_x, vs_thumb_y, vs_thumb_x+8, vs_thumb_y+len, bg);
        vs_thumb_y += len;
      }
      //if thumb doesn't reach the bottom, draw a blank spot there, too
      if (vs_thumb_y+vs_thumb_len < d->y+d->h-3-hscroll_h)
      {
        drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
        rectfill(screen, vs_thumb_x, vs_thumb_y, vs_thumb_x+8, vs_thumb_y+vs_thumb_len, fg_color);
        solid_mode();
        vs_thumb_y += vs_thumb_len+1;
        rectfill(screen, vs_thumb_x, vs_thumb_y, vs_thumb_x+8, d->y+d->h-3-hscroll_h, bg);
      }
      else
      {
        drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
        rectfill(screen, vs_thumb_x, vs_thumb_y, vs_thumb_x+8, d->y+d->h-3-hscroll_h, fg_color);
        solid_mode();
      }
    }

    if (draw_hscroll)
    {
      //x pos of thumb
      hs_thumb_x = d->x+2;
      //y pos of thumb
      hs_thumb_y = d->y+d->h-11;
      //width of thumb
      hs_thumb_wid = (ww * frame_width + doc_width/2) / doc_width;
      //if thumb is not at left, draw a blank spot there
      if (x_offset > 0)
      {
        wid = ((ww * x_offset) + doc_width/2) / doc_width;
        rectfill(screen, hs_thumb_x, hs_thumb_y, hs_thumb_x+wid, hs_thumb_y+8, bg);
        hs_thumb_x += wid;
      }
      //if thumb doesn't reach the right, draw a blank spot there, too
      if (hs_thumb_x+hs_thumb_wid < d->x+d->w-3-vscroll_w)
      {
        drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
        rectfill(screen, hs_thumb_x, hs_thumb_y, hs_thumb_x+hs_thumb_wid, hs_thumb_y+8, fg_color);
        solid_mode();
        hs_thumb_x += hs_thumb_wid+1;
        rectfill(screen, hs_thumb_x, hs_thumb_y, d->x+d->w-3-vscroll_w, hs_thumb_y+8, bg);
      }
      else
      {
        drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
        rectfill(screen, hs_thumb_x, hs_thumb_y, d->x+d->w-3-vscroll_w, hs_thumb_y+8, fg_color);
        solid_mode();
      }
    }
    destroy_bitmap(pattern);
  }
  else
  {
    // no scrollbar necessary
    if (d->flags & D_GOTFOCUS)
    {
      dotted_rect(d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, fg_color, bg);
    }
    else
    {
      rect(screen, d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, bg);
    }
  }
}

/* _handle_scrollable_click:
 *  Helper to process a click on a scrollable object.
 */
void _handle_scrollable_scroll_click_vh(DIALOG *d,
int doc_height, int *y_offset, int frame_height,
int doc_width, int *x_offset, int frame_width,
int vscroll_bar_style, int hscroll_bar_style,
int handle_what)
{
  int vs_thumb_y, vs_thumb_mouse_y;
  int hs_thumb_x, hs_thumb_mouse_x;
  int hh, ww;
  int vs_thumb_len, hs_thumb_wid;
  int len, wid;

  int draw_vscroll=(((doc_height > frame_height)||
    (vscroll_bar_style==scrollbar_on))&&
    (vscroll_bar_style!=scrollbar_off));
  int draw_hscroll=(((doc_width > frame_width)||
    (hscroll_bar_style==scrollbar_on))&&
    (hscroll_bar_style!=scrollbar_off));
  int vscroll_w=0, hscroll_h=0;

  // possibly draw scrollbar
  if (draw_vscroll)
  {
    vscroll_w=12;
  }
  if (draw_hscroll)
  {
    hscroll_h=12;
  }

  hh = d->h - 5 - hscroll_h;
  ww = d->w - 5 - vscroll_w;

  while (gui_mouse_b())
  {
    switch (handle_what)
    {
      case handle_vscroll:
        //length of thumb
        vs_thumb_len = (hh * frame_height + doc_height/2) / doc_height;
        len = (((hh * (*y_offset)) + doc_height/2) / doc_height)+2;
        //        len = (hh * (*y_offset) + doc_height/2) / doc_height + 2;

        if ((gui_mouse_y() >= d->y+len) && (gui_mouse_y() <= d->y+len+vs_thumb_len))
        {
          vs_thumb_mouse_y = gui_mouse_y() - len + 2;
          while (gui_mouse_b())
          {
            vs_thumb_y = (doc_height * (gui_mouse_y() - vs_thumb_mouse_y) + hh/2) / hh;
            if (vs_thumb_y > doc_height-frame_height)
            {
              vs_thumb_y = doc_height-frame_height;
            }

            if (vs_thumb_y < 0)
            {
              vs_thumb_y = 0;
            }

            if (vs_thumb_y != *y_offset)
            {
              *y_offset = vs_thumb_y;
              object_message(d, MSG_DRAW, 0);
            }

            /* let other objects continue to animate */
            broadcast_dialog_message(MSG_IDLE, 0);
          }
        }
        else
        {
          if (gui_mouse_y() <= d->y+len)
          {
            vs_thumb_y = *y_offset - frame_height;
          }
          else
          {
            vs_thumb_y = *y_offset + frame_height;
          }

          if (vs_thumb_y > doc_height-frame_height)
          {
            vs_thumb_y = doc_height-frame_height;
          }

          if (vs_thumb_y < 0)
          {
            vs_thumb_y = 0;
          }

          if (vs_thumb_y != *y_offset)
          {
            *y_offset = vs_thumb_y;
            object_message(d, MSG_DRAW, 0);
          }
        }
        break;
      case handle_hscroll:
        //length of thumb
        hs_thumb_wid = (ww * frame_width + doc_width/2) / doc_width;
        wid = (((ww * (*x_offset)) + doc_width/2) / doc_width)+2;
        //        len = (hh * (*y_offset) + doc_height/2) / doc_height + 2;

        if ((gui_mouse_x() >= d->x+wid) && (gui_mouse_x() <= d->x+wid+hs_thumb_wid))
        {
          hs_thumb_mouse_x = gui_mouse_x() - wid + 2;
          while (gui_mouse_b())
          {
            hs_thumb_x = (doc_width * (gui_mouse_x() - hs_thumb_mouse_x) + ww/2) / ww;
            if (hs_thumb_x > doc_width-frame_width)
            {
              hs_thumb_x = doc_width-frame_width;
            }

            if (hs_thumb_x < 0)
            {
              hs_thumb_x = 0;
            }

            if (hs_thumb_x != *x_offset)
            {
              *x_offset = hs_thumb_x;
              object_message(d, MSG_DRAW, 0);
            }

            /* let other objects continue to animate */
            broadcast_dialog_message(MSG_IDLE, 0);
          }
        }
        else
        {
          if (gui_mouse_x() <= d->x+wid)
          {
            hs_thumb_x = *x_offset - frame_width;
          }
          else
          {
            hs_thumb_x = *x_offset + frame_width;
          }

          if (hs_thumb_x > doc_width-frame_width)
          {
            hs_thumb_x = doc_width-frame_width;
          }

          if (hs_thumb_x < 0)
          {
            hs_thumb_x = 0;
          }

          if (hs_thumb_x != *x_offset)
          {
            *x_offset = hs_thumb_x;
            object_message(d, MSG_DRAW, 0);
          }
        }
        break;
    }

    /* let other objects continue to animate */
    broadcast_dialog_message(MSG_IDLE, 0);
  }
}

/* dotted_rect:
 *  Draws a dotted rectangle, for showing an object has the input focus.
 */
static void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
  int x = ((x1+y1) & 1) ? 1 : 0;
  int c;

  /* two loops to avoid bank switches */
  for (c=x1; c<=x2; c++)
  {
    putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
  }
  for (c=x1; c<=x2; c++)
  {
    putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
  }

  for (c=y1+1; c<y2; c++)
  {
    putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
    putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
  }
}

/* d_bitmap_proc:
 *  Simple dialog procedure: draws the bitmap which is pointed to by dp.
 */
int d_scroll_bmp_proc(int msg, DIALOG *d, int c)
{
  BITMAP *b;
  int hsbar_h, vsbar_w;
  int height = d->h-8;
  int width = d->w-8;
  int fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
  int hsbarstyle=scrollbar_on;
  int vsbarstyle=scrollbar_on;
  int bmp_scroll_y;
  int bmp_scroll_x;
  int bmp_scroll_mouse_y;
  int bmp_scroll_mouse_x;
  ASSERT(d);

  b = (BITMAP *)d->dp;
  if (!b) return D_O_K;

  //just in case the width or height changed, recalc scroll bar sizes...
  if (((b->h>height)&&
    (vsbarstyle!=scrollbar_off))||
    (vsbarstyle==scrollbar_on))
  {
    vsbar_w = 12;
  }
  else
  {
    vsbar_w = 0;
  }

  if (((b->w>width)&&
    (hsbarstyle!=scrollbar_off))||
    (hsbarstyle==scrollbar_on))
  {
    hsbar_h = 12;
  }
  else
  {
    hsbar_h = 0;
  }

  switch (msg)
  {
    case MSG_DRAW:
      blit(b, screen, d->d1, d->d2, d->x+4, d->y+4, width-vsbar_w, height-hsbar_h);
      /* draw the frame around */
      _draw_scrollable_frame_vh(d, b->h, d->d2, height-hsbar_h,
        b->w, d->d1, width-vsbar_w,
        vsbarstyle,hsbarstyle,
        fg_color, d->bg);
      break;
    case MSG_CLICK:
      /* figure out if it's on the text or the scrollbar */

      if (is_in_rect(gui_mouse_x()-d->x, gui_mouse_y()-d->y,
        0, 0, d->w-vsbar_w-1, d->h-hsbar_h-1))
      {
        bmp_scroll_mouse_y = gui_mouse_y()+d->d2;
        bmp_scroll_mouse_x = gui_mouse_x()+d->d1;
        while (gui_mouse_b())
        {
          bmp_scroll_y = bmp_scroll_mouse_y-gui_mouse_y();
          bmp_scroll_x = bmp_scroll_mouse_x-gui_mouse_x();
          if (bmp_scroll_y > b->h-height+hsbar_h)
          {
            bmp_scroll_y = b->h-height+hsbar_h;
          }
          if (bmp_scroll_x > b->w-width+vsbar_w)
          {
            bmp_scroll_x = b->w-width+vsbar_w;
          }

          if (bmp_scroll_y < 0)
          {
            bmp_scroll_y = 0;
          }

          if (bmp_scroll_x < 0)
          {
            bmp_scroll_x = 0;
          }

          if ((bmp_scroll_y != d->d2)||(bmp_scroll_x != d->d1))
          {
            d->d2 = bmp_scroll_y;
            d->d1 = bmp_scroll_x;
            object_message(d, MSG_DRAW, 0);
          }

          /* let other objects continue to animate */
          broadcast_dialog_message(MSG_IDLE, 0);
        }
      }
      else
      {
        /* clicked on the scroll area */
        // is it on the vertical scroll bar?
        if (is_in_rect(gui_mouse_x()-d->x, gui_mouse_y()-d->y,
          d->w-vsbar_w, 0, d->w-1, d->h-hsbar_h-1))
        {
          _handle_scrollable_scroll_click_vh(d, b->h, &d->d2, height-hsbar_h,
            b->w, &d->d1, width-vsbar_w,
            vsbarstyle,hsbarstyle,
            handle_vscroll);
        } else if (is_in_rect(gui_mouse_x()-d->x, gui_mouse_y()-d->y,
          0, d->h-hsbar_h, d->w-vsbar_w-1, d->h-1))
        {
          _handle_scrollable_scroll_click_vh(d, b->h, &d->d2, height-hsbar_h,
            b->w, &d->d1, width-vsbar_w,
            vsbarstyle,hsbarstyle,
            handle_hscroll);
        }
      }
      break;
    case MSG_WANTFOCUS:
      return D_WANTFOCUS;
      break;
  }

  return D_O_K;
}

/***  The End  ***/
