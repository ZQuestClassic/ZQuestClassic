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

#include "eb_intern.h"
#include <string.h>
#include "zc_alleg.h"
#include <allegro/internal/aintern.h>
#include <stdio.h>

#define max(a,b)  ((a)>(b)?(a):(b))
#define min(a,b)  ((a)<(b)?(a):(b))

#define MSG_BLINKCURSOR (MSG_USER + 1)

int always_resize_clipboard=0;

static void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg);
int uchar_width(FONT *cfont, int c);

void _find_char_in_textbox_line(textline *thetext, int wword, int tabsize, int x, int y, int w, int h, int *line, int *firstchar, int *lastchar, int findchar)
{
  int len;
  int ww = w-6;
  char s[16];
  char text[16];
  char space[16];
  char *printed = text;
  char *scanned = text;
  char *oldscan = text;
  char *ignore = NULL;
  char *tmp, *ptmp;
  int width;
  int noignore;
  int lastbreakchar;
  int lines=0, charcount=0, oldcount=0;
  char *tscan=text;

  textline *currentline=thetext;
  
  usetc(s+usetc(s, '.'), 0);
  usetc(text+usetc(text, ' '), 0);
  usetc(space+usetc(space, ' '), 0);

  // find the correct text
  if (currentline != NULL)
  {
    if (currentline->text!=NULL)
    {
      printed = currentline->text;
      scanned = currentline->text;
    }
  }

  // loop over the entire string
  while (1)
  {
    width = 0;

    if (findchar==-1)
    {
      if (*line==lines)
      {
        *firstchar=charcount;
      }
    }
    else
    {
      if (charcount<=findchar)
      {
        *firstchar=charcount;
        *line=lines;
      }
    }
    // find the next break
    while (ugetc(scanned))
    {
      // check for a forced break
      if (is_nr(ugetc(scanned)))
      {
        lastbreakchar=ugetc(scanned);
        tscan=scanned;
        scanned += uwidth(scanned);
        if (is_nr(lastbreakchar))
        {
          if (ugetc(scanned) == othernr(lastbreakchar))
          {
            tscan=scanned;
            scanned += uwidth(scanned);
          }
        }
        // we are done parsing the line end
        break;
      }

      // the next character length
      usetc(s+usetc(s, ugetc(scanned)), 0);
      len = text_length(font, s);

      // modify length if its a tab
      if (ugetc(s) == '\t')
      {
        len=(((width/tabsize)+1)*tabsize)-width;
      }

      // check for the end of a line by excess width of next char
      if (width+((wword!=eb_wrap_none)?len:0) >= ww)
      {
        //we have reached end of line do we go back to find start
        if (wword==eb_wrap_word)
        {
          // remember where we were
          oldscan = scanned;
          noignore = FALSE;

          // go backwards looking for start of word
          while (!uisspace(ugetc(scanned)))
          {
            // don't wrap too far
            if (scanned == printed)
            {
              // the whole line is filled, so stop here
              tmp = ptmp = scanned;
              while (ptmp != oldscan)
              {
                ptmp = tmp;
                tmp += uwidth(tmp);
              }
              scanned = ptmp;
              noignore = TRUE;
              break;
            }
            // look further backwards to wrap
            tmp = ptmp = printed;
            while (tmp < scanned)
            {
              ptmp = tmp;
              tmp += uwidth(tmp);
            }
            tscan=scanned;
            scanned = ptmp;
          }
          // put the space at the end of the line
          if (!noignore)
          {
            ignore = scanned;
            tscan=scanned;
            scanned += uwidth(scanned);
          }
          else
          {
            ignore = NULL;
          }

          // check for endline at the convenient place
          if (is_nr(ugetc(scanned)))
          {
            lastbreakchar=ugetc(scanned);
            tscan=scanned;
            scanned += uwidth(scanned);
            if (is_nr(lastbreakchar))
            {
              if (ugetc(scanned) == othernr(lastbreakchar))
              {
                tscan=scanned;
                scanned += uwidth(scanned);
              }
            }
          }
        }
        else if (wword==eb_wrap_none)
        {
          while (ugetc(scanned)&& !is_nr(ugetc(scanned)))
          {
            tscan=scanned;
            scanned += uwidth(scanned);
          }
          if (scanned)
          {
            lastbreakchar=ugetc(scanned);
            tscan=scanned;
            scanned += uwidth(scanned);
            if (is_nr(lastbreakchar))
            {
              if (ugetc(scanned) == othernr(lastbreakchar))
              {
                tscan=scanned;
                scanned += uwidth(scanned);
              }
            }
          }
        }
        // we are done parsing the line end
        break;
      }

      // the character can be added
      if (scanned)
      {
        tscan=scanned;
        scanned += uwidth(scanned);
        width += len;
      }
    }

    tmp=printed;
    while(tmp<scanned)
    {
      oldcount=charcount;
      charcount+=uwidth(tmp);
      tmp+=uwidth(tmp);
    }

    // print the line end
    printed = scanned;

    // we have done a line
    if (findchar==-1)
    {
      if (*line==lines)
      {
        *lastchar=oldcount;
        return;
      }
    }
    else
    {
      if (charcount>findchar)
      {
        *lastchar=oldcount;
        *line=lines;
        return;
      }
    }

    // we have done a line
    if (ugetc(printed)|| is_nr(ugetc(tscan)))
    {
      (lines)++;
    }

    // check if we are at the end of the string
    if (!ugetc(printed))
    {
      return;
    }
  }
}

int is_color_font(const FONT *fnt)
{
  return fnt->vtable == font_vtable_color;
}

void return_negative_color(AL_CONST PALETTE pal, int x, int y, RGB *rgb)
{
  /* to get the negative color, substract the color values of red, green and
   * blue from the full (63) color value
   */
  rgb->r = 63-pal[y].r;
  rgb->g = 63-pal[y].g;
  rgb->b = 63-pal[y].b;
}

void inv_textout_ex(BITMAP *bmp, AL_CONST FONT *f, AL_CONST char *str, int x, int y, int color, int bg)
{
  static int max_width=0, max_height=0;
  static int resize=0;
  static BITMAP *tmpbmp;
  int curr_width=text_length(f, str), curr_height=text_height(f);

  ASSERT(bmp);
  ASSERT(f);
  ASSERT(str);

  if (curr_height>max_height)
  {
    max_height=curr_height;
    resize=1;
  }

  if (curr_width>max_width)
  {
    max_width=curr_width;
    resize=1;
  }

  if (resize)
  {
    if (tmpbmp)
    {
      destroy_bitmap(tmpbmp);
    }
    tmpbmp=create_bitmap_ex(8, max_width, max_height);
    resize=0;

  }

  if (is_color_font(f))
  {
    clear_to_color(tmpbmp, color_map->data[0][bg]);
    f->vtable->render(f, str, color, -1, tmpbmp, 0, 0);
    drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    rectfill(tmpbmp, 0, 0, curr_width-1, curr_height-1, 0);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    blit(tmpbmp, bmp, 0, 0, x, y, curr_width, curr_height);
  }
  else
  {
    f->vtable->render(f, str, color, bg, bmp, x, y);
  }

}

void hl_textout_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int fg, int bg, int selfg, int selbg, int selstart, int selend, int currchar)
{
  int seltemp;

  ASSERT(bmp);
  ASSERT(f);
  ASSERT(s);

  if (is_color_font(f))
  {
    fg=selfg=-1;
  }
  if (selstart>selend)
  {
    seltemp=selend;
    selend=selstart;
    selstart=seltemp;
  }
  if (currchar>=selstart&&currchar<selend)
  {
    inv_textout_ex(bmp, f, s, x, y, selfg, selbg);
  }
  else
  {
    textout_ex(bmp, f, s, x, y, fg, bg);
  }
}

/* _draw_editbox:
 *  Helper function to draw a textbox object.
 */
void _draw_editbox(textline *thetext, int *listsize, int draw, int x_offset, int y_offset, int wword, int tabsize, int x, int y, int w, int h, int disabled, int fore, int deselect, int disable, int *maxwidth, int selstart, int selend, int selfg, int selbg, int printtab, int printcrlf)
{
  int fg = fore;
  int y1 = y+4-y_offset;
  int x1, tx1;
  int len;
  int ww = w-6;
  char s[16];
  char text[16];
  char space[16];
  char *printed = text;
  char *scanned = text;
  char *oldscan = text;
  char *ignore = NULL;
  char *tmp, *ptmp;
  int width=0;
  int line = 1;
  int noignore;
  int lastbreakchar;
  //  char tabchar=128;
  //  char crlfchar=129;
  //  char crlfchar=0x00B6;
  int tempchar;
  char *tscan=text;
  int charcount=0;
  *maxwidth=0;

  sprintf(scanned, "NULL Text!");
  
  textline *currentline=thetext;
  
  usetc(s+usetc(s, '.'), 0);
  usetc(text+usetc(text, ' '), 0);
  usetc(space+usetc(space, ' '), 0);

  // find the correct text
  if (currentline != NULL)
  {
    if (currentline->text!=NULL)
    {
      printed = currentline->text;
      scanned = currentline->text;
    }
  }

  // do some drawing setup 
  if (draw)
  {
    // initial start blanking at the top 
    rectfill(screen, x+2, y+2, x+w-3, y+3, deselect);
  }

  // choose the text color 
  if (disabled)
  {
    fg = disable;
  }

  // loop over the entire string 
  while (1)
  {
    width = 0;

    // find the next break 
    al_trace("%d: %s", currentline->next==NULL?0:1, scanned);
    while (ugetc(scanned))
    {
      tscan=scanned;
      // check for a forced break 
      if (is_nr(ugetc(scanned)))
      {
        lastbreakchar=ugetc(scanned);
        scanned += uwidth(scanned);
        if (ugetc(scanned) == othernr(lastbreakchar))
        {
          tscan=scanned;
          scanned += uwidth(scanned);
        }
        // we are done parsing the line end
        break;
      }

      // the next character length 
      usetc(s+usetc(s, ugetc(scanned)), 0);
      len = text_length(font, s);

      // modify length if its a tab 
      if (ugetc(s) == '\t')
      {
        len=(((width/tabsize)+1)*tabsize)-width;
      }

      // check for the end of a line by excess width of next char
      if (width-x_offset+((wword!=eb_wrap_none)?len:0) >= ww)
      {
        // we have reached end of line do we go back to find start 
        if (wword==eb_wrap_word)                            //word wrapping
        {
          // remember where we were 
          oldscan = scanned;
          noignore = FALSE;

          // go backwards looking for start of word 
          while (!uisspace(ugetc(scanned)))
          {
            // don't wrap too far 
            if (scanned == printed)
            {
              // the whole line is filled, so stop here 
              tmp = ptmp = scanned;
              while (ptmp != oldscan)
              {
                ptmp = tmp;
                tmp += uwidth(tmp);
              }
              scanned = ptmp;
              noignore = TRUE;
              break;
            }
            // look further backwards to wrap 
            tmp = ptmp = printed;
            while (tmp < scanned)
            {
              ptmp = tmp;
              tmp += uwidth(tmp);
            }
            scanned = ptmp;
          }
          // put the space at the end of the line 
          if (!noignore)
          {
            ignore = scanned;
            scanned += uwidth(scanned);
          }
          else
          {
            ignore = NULL;
          }

          // check for endline at the convenient place 
          if (is_nr(ugetc(scanned)))
          {
            lastbreakchar=ugetc(scanned);
            scanned += uwidth(scanned);
            if (ugetc(scanned) == othernr(lastbreakchar))
            {
              scanned += uwidth(scanned);
            }
          }
        }                                                   //no word wrapping; go to start of next line
        else if (wword==eb_wrap_none)
        {
          while (ugetc(scanned)&&!is_nr(ugetc(scanned)))
          {
            tscan=scanned;
            usetc(s+usetc(s, ugetc(scanned)), 0);
            width+=text_length(font, s);
            scanned += uwidth(scanned);
          }
          lastbreakchar=ugetc(scanned);
          tscan=scanned;
          scanned += uwidth(scanned);
          if (ugetc(scanned) == othernr(lastbreakchar))
          {
            tscan=scanned;
            scanned += uwidth(scanned);
          }
        }
        // we are done parsing the line end 
        break;
      }

      // the character can be added 
      tscan=scanned;
      scanned += uwidth(scanned);
      width += len;
    }
    
    if (width>*maxwidth)
    {
      *maxwidth=width;
    }
    // check if we are to print it 
    if ((draw) && ((line+1)*text_height(font) >= y_offset) && (y1 < (y+h-3)))
    {
      x1 = x+4-x_offset;

      //only allow drawing in the 2-pixel border between text and frame
      set_clip_rect(screen, x+2, y+2, x+w-3, y+h-3);

      // the initial blank bit
      // blanks out the 2-pixel border between the dotted "GOT_FOCUS"
      // rectangle and the start of the text on the line
      rectfill(screen, x+2, y1, x1-1+x_offset, y1+text_height(font), deselect);

      //only allow drawing in the text area
      set_clip_rect(screen, x+4, y+4, x+w-5, y+h-5);

      // print up to the marked character 
      while (printed != scanned)
      {
        if (wword==eb_wrap_none)
        {
          usetc(s+usetc(s, ugetc(printed)), 0);
          if (x1 >= ww+x+4)
          {
            while(printed<scanned)
            {
              printed += uwidth(printed);
              (charcount)++;
            }
            break;
          }
        }
        // do special stuff for each charater 
        switch (ugetc(printed))
        {
          case '\r':
          case '\n':
            tempchar=ugetc(printed);
            if (printcrlf)
            {
              if (tempchar=='\n')
              {
                printed += uwidth(printed);
                if (ugetc(printed)=='\r')
                {
                  x1+=_draw_crlf(x1, y1, eb_crlf_nr, printcrlf, vc(7), deselect, deselect, vc(7), selstart, selend, charcount);
                }
                else
                {
                  x1+=_draw_crlf(x1, y1, eb_crlf_n, printcrlf, vc(7), deselect, deselect, vc(7), selstart, selend, charcount);
                }
                (charcount)++;
                while(printed<scanned)
                {
                  printed += uwidth(printed);
                  (charcount)++;
                }
              }                                             //tempchar=='\r'
              else
              {
                usetc(s+usetc(s, 'r'), 0);
                printed += uwidth(printed);
                if (ugetc(printed)=='\n')
                {
                  x1+=_draw_crlf(x1, y1, eb_crlf_rn, printcrlf, vc(7), deselect, deselect, vc(7), selstart, selend, charcount);
                }
                else
                {
                  x1+=_draw_crlf(x1, y1, eb_crlf_r, printcrlf, vc(7), deselect, deselect, vc(7), selstart, selend, charcount);
                }
                (charcount)++;
                while(printed<scanned)
                {
                  printed += uwidth(printed);
                  (charcount)++;
                }
              }
            }
            break;

            // possibly expand the tabs
          case '\t':
            tx1=x1;                                         //xpos of tab char
                                                            //xpos of next char
            tx1=((((tx1-x-4+x_offset)/tabsize)+1)*tabsize)+x+4-x_offset;
            rectfill(screen, x1, y1, tx1, y1+text_height(font)-1, deselect);
            _draw_tab(x1, y1, tx1, printtab, vc(7), deselect, deselect, vc(7), selstart, selend, charcount);
            x1=tx1;
            break;
            // print a normal character
          default:
            if (printed != ignore)
            {
              usetc(s+usetc(s, ugetc(printed)), 0);
              if (x1+text_length(font, s)>=0)
              {
                hl_textout_ex(screen, font, s, x1, y1, fg, deselect, selfg, selbg, selstart, selend, charcount);
              }
              x1 += text_length(font, s);
            }
        }

        // goto the next character 
        if (printed!=scanned)
        {
          //          tscan=printed;
          printed += uwidth(printed);
          (charcount)++;
        }
      }
      // the last blank bit
      // blanks out from the end of the last character on the line
      // to the edge of the text box
      if (x1 <= x+w-3)
      {
        rectfill(screen, x1, y1, x+w-5, y1+text_height(font)-1, deselect);
        //only allow drawing in the 2-pixel border between text and frame
        set_clip_rect(screen, x+2, y+2, x+w-3, y+h-3);
        rectfill(screen, x+w-4, y1, x+w-3, y1+text_height(font)-1, deselect);
        //only allow drawing in the text area
        set_clip_rect(screen, x+4, y+4, x+w-5, y+h-5);
      }

      // print the line end 
      //      y1 += text_height(font);
    }
    
    y1 += text_height(font);
    while(printed<scanned)
    {
      printed += uwidth(printed);
      (charcount)++;
    }

    // we have done a line 
    if (ugetc(printed)||is_nr(ugetc(tscan)))
    {
      (line)++;
    }

    // check if we are at the end of the string 
    if (!ugetc(printed))
    {
      if (currentline->next==NULL)
      {
        // the under blank bit 
        if (draw)
        {
          if (y1<y+h-4)
          {
            //only allow drawing in the 2-pixel border between text and frame
            set_clip_rect(screen, x+2, y+2, x+w-3, y+h-3);
            //left side
            rectfill(screen, x+2, y1, x+3, y+h-3, deselect);
            //right side
            rectfill(screen, x+w-4, y1, x+w-3, y+h-3, deselect);
            //only allow drawing in the text area
            set_clip_rect(screen, x+4, y+4, x+w-5, y+h-5);
            //bottom middle
            rectfill(screen, x+4, y1, x+w-5, y+h-5, deselect);
          }
  
          //only allow drawing in the 2-pixel border between text and frame
          set_clip_rect(screen, x+2, y+2, x+w-3, y+h-3);
          //bottom
          rectfill(screen, x+2, y+h-4, x+w-3, y+h-3, deselect);
          //allow drawing anywhere
          set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
        }
  
        *listsize = line;
        //allow drawing anywhere
//        set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
        return;
      }
      else
      {
        currentline=currentline->next;
        printed = currentline->text;
        scanned = currentline->text;
      }
    }
  }
}

int prevuchar(char* thetext, int nextchar)
{
  int oldcounter=0,counter=0;
  while (counter<nextchar)
  {
    oldcounter=counter;
    counter+=uwidth(thetext);
    thetext+=uwidth(thetext);
  }
  return oldcounter;
}

int nextuchar(char* thetext, int prevchar)
{
  int counter=prevchar;
  thetext+=prevchar;
  counter+=uwidth(thetext);
  thetext+=uwidth(thetext);
  return counter;
}

int othernr(int currentnr)
{
  switch (currentnr)
  {
    case '\n':
      return '\r';
      break;
    case '\r':
      return '\n';
      break;
  }
  return -1;
}

int is_nr(int testnr)
{
  switch (testnr)
  {
    case '\n':
    case '\r':
      return 1;
      break;
  }
  return 0;
}

void _focus_on_cursor(DIALOG *d, int ypos, int *yofs, int list_height, int frame_height, int xpos, int *xofs, int list_width, int frame_width)
{
  int temp_xofs=*xofs;
  int temp_yofs=*yofs;
  //if the bottom line of text is above the bottom of
  //the frame and the vertical offset is > 0...
  if (*yofs>0&&*yofs>list_height-frame_height)
  {
    *yofs=list_height-frame_height;
  }
  //if the cursor is off the screen...
  if (ypos<=*yofs)                                          //above screen
  {
    *yofs=ypos;
    //below screen
  }
  else if (ypos+text_height(font)>*yofs+frame_height)
  {
    *yofs=ypos+text_height(font)-frame_height;
  }

  if ((xpos<*xofs)||(xpos-*xofs>=frame_width))
  {
    *xofs=xpos-(frame_width/2);
  }

  if (*xofs<0)
  {
    *xofs=0;
  }
  if ((temp_xofs!=(*xofs))||(temp_yofs!=(*yofs)))
  {
    d->flags|=D_DIRTY;
  }
}

void _set_cursor_visibility(int *showcur, DIALOG *d, clock_t *t, int visible)
{
  if (visible==-1)
  {
    *showcur^=1;
  }
  else
  {
    *showcur=visible;
  }
//  d->flags |= D_DIRTY;
  object_message(d, MSG_BLINKCURSOR, 0);
  *t=clock()+(CLOCKS_PER_SEC/2);
}

int uchar_width(FONT *cfont, int c)
{
  char temptext[256];
  usetc(temptext+usetc(temptext, c), 0);
  return text_length(cfont, temptext);
}

int starts_crlf(char *text, int crlf_style)
{
  switch (crlf_style)
  {
    case eb_crlf_n:
      if (ugetc(text)=='\n')
      {
        return 1;
      }
      break;
    case eb_crlf_r:
      if (ugetc(text)=='\r')
      {
        return 1;
      }
      break;
    case eb_crlf_nr:
      if (ugetc(text)=='\n')
      {
        if (ugetc(text+uwidth(text))=='\r')
        {
          return 1;
        }
      }
      break;
    case eb_crlf_rn:
      if (ugetc(text)=='\r')
      {
        if (ugetc(text+uwidth(text))=='\n')
        {
          return 1;
        }
      }
      break;
    case eb_crlf_any:
      if (starts_crlf(text,eb_crlf_nr))
      {
        return eb_crlf_nr;
      }
      else if (starts_crlf(text,eb_crlf_rn))
      {
        return eb_crlf_rn;
      }
      else if (starts_crlf(text,eb_crlf_n))
      {
        return eb_crlf_n;
      }
      else if (starts_crlf(text,eb_crlf_r))
      {
        return eb_crlf_r;
      }
      break;
  }
  return 0;
}

int _draw_crlf(int x, int y, int type, int style, int fg, int bg, int selfg, int selbg, int selstart, int selend, int currchar)
{
  int width=0;
  char s[16];
  int temp=selend;

  if (selstart>selend)
  {
    selend=selstart;
    selstart=temp;
  }

  switch (style)                                            //1=text, 2=graphic
  {
    case 1:
      switch (type)
      {
        case eb_crlf_n:
          usetc(s+usetc(s, 'n'), 0);
          hl_textout_ex(screen, font, s, x+width, y, fg, bg, selfg, selbg, selstart, selend, currchar);
          width += text_length(font, s);
          break;
        case eb_crlf_nr:
          usetc(s+usetc(s, 'n'), 0);
          hl_textout_ex(screen, font, s, x+width, y, fg, bg, selfg, selbg, selstart, selend, currchar);
          width += text_length(font, s);
          usetc(s+usetc(s, 'r'), 0);
          hl_textout_ex(screen, font, s, x+width, y, fg, bg, selfg, selbg, selstart, selend, currchar);
          width += text_length(font, s);
          break;
        case eb_crlf_r:
          usetc(s+usetc(s, 'r'), 0);
          hl_textout_ex(screen, font, s, x+width, y, fg, bg, selfg, selbg, selstart, selend, currchar);
          width += text_length(font, s);
          break;
        case eb_crlf_rn:
          usetc(s+usetc(s, 'r'), 0);
          hl_textout_ex(screen, font, s, x+width, y, fg, bg, selfg, selbg, selstart, selend, currchar);
          width += text_length(font, s);
          usetc(s+usetc(s, 'n'), 0);
          hl_textout_ex(screen, font, s, x+width, y, fg, bg, selfg, selbg, selstart, selend, currchar);
          width += text_length(font, s);
          break;
      }
      break;
    case 2:
      if (currchar>=selstart&&currchar<selend)
      {
        fg=selfg;
        bg=selbg;
      }
      switch (type)                                         //for now, they all look the same
      {
        case eb_crlf_n:
        case eb_crlf_nr:
        case eb_crlf_r:
        case eb_crlf_rn:
          rectfill(screen, x, y, x+4, y+text_height(font)-1, bg);
          hline(screen, x, y+(text_height(font)/2), x+4, fg);
          putpixel(screen, x+1, y+(text_height(font)/2)-1, fg);
          putpixel(screen, x+1, y+(text_height(font)/2)+1, fg);
          if (text_height(font)>6)
          {
            rectfill(screen, x+5, y, x+6, y+text_height(font)-1, bg);
            hline(screen, x+5, y+(text_height(font)/2), x+6, fg);
            vline(screen, x+6, y+(text_height(font)/2)-3, y+(text_height(font)/2)-1, fg);
            putpixel(screen, x+2, y+(text_height(font)/2)-2, fg);
            putpixel(screen, x+2, y+(text_height(font)/2)+2, fg);
          }
          else
          {
            vline(screen, x+4, y+(text_height(font)/2)-2, y+(text_height(font)/2)-1, fg);
          }
          width=text_height(font)>6?7:5;
          break;
      }
      break;
    default:
      break;
  }
  return width;
}

int _crlf_width(int type, int style)
{
  int width=0;
  char s[16];
  switch (style)                                            //1=text, 2=graphic
  {
    case 1:
      switch (type)
      {
        case eb_crlf_n:
          usetc(s+usetc(s, 'n'), 0);
          width += text_length(font, s);
          break;
        case eb_crlf_nr:
          usetc(s+usetc(s, 'n'), 0);
          width += text_length(font, s);
          usetc(s+usetc(s, 'r'), 0);
          width += text_length(font, s);
          break;
        case eb_crlf_r:
          usetc(s+usetc(s, 'r'), 0);
          width += text_length(font, s);
          break;
        case eb_crlf_rn:
          usetc(s+usetc(s, 'r'), 0);
          width += text_length(font, s);
          usetc(s+usetc(s, 'n'), 0);
          width += text_length(font, s);
          break;
      }
      break;
    case 2:
      switch (type)                                         //for now, they all look the same
      {
        case eb_crlf_n:
        case eb_crlf_nr:
        case eb_crlf_r:
        case eb_crlf_rn:
          width=text_height(font)>6?7:5;
          break;
      }
      break;
    default:
      break;
  }
  return width;
}

void _draw_tab(int x, int y, int tabstop, int printtab, int fg, int bg, int selfg, int selbg, int selstart, int selend, int currchar)
{
  int temp=selend;
  if (selstart>selend)
  {
    selend=selstart;
    selstart=temp;
  }

  if (currchar>=selstart&&currchar<selend)
  {
    fg=selfg;
    bg=selbg;
  }
  rectfill(screen, x, y, tabstop, y+text_height(font)-1,bg);
  if (printtab)
  {
    hline(screen, x, y+(text_height(font)/2), tabstop-1, fg);
    putpixel(screen, tabstop-3, y+(text_height(font)/2)-1, fg);
    putpixel(screen, tabstop-3, y+(text_height(font)/2)+1, fg);
    if (text_height(font)>6)
    {
      putpixel(screen, tabstop-4, y+(text_height(font)/2)-2, fg);
      putpixel(screen, tabstop-4, y+(text_height(font)/2)+2, fg);
      vline(screen, tabstop-1, y+(text_height(font)/2)-3, y+(text_height(font)/2)+3, fg);
    }
    else
    {
      vline(screen, tabstop-1, y+(text_height(font)/2)-2, y+(text_height(font)/2)+2, fg);
    }
  }
}

//_draw_scrollable_frame(d, d->d1, d->d2, height, fg_color, d->bg);
//d1=lines of text
//d2=top line
//height=visible lines of text
/* _draw_scrollable_frame:
 *  Helper function to draw a frame for all objects with vertical scrollbars.
 */
void _draw_scrollable_frame_vh(DIALOG *d, int list_height, int y_offset, int frame_height, int list_width, int x_offset, int frame_width, int vscroll_bar_style, int hscroll_bar_style, int fg_color, int bg)
{
  int vs_thumb_len, hs_thumb_wid, len, wid;
  BITMAP *pattern;
  int hh, ww;
  int vs_thumb_x, vs_thumb_y;
  int hs_thumb_x, hs_thumb_y;
  int draw_vscroll=(((list_height > frame_height)|| (vscroll_bar_style==eb_scrollbar_on))&& (vscroll_bar_style!=eb_scrollbar_off));
  int draw_hscroll=(((list_width > frame_width)|| (hscroll_bar_style==eb_scrollbar_on))&& (hscroll_bar_style!=eb_scrollbar_off));
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
      //    if (draw_vscroll&&list_height>frame_height-hscroll_h) {
      //x pos of thumb
      vs_thumb_x = d->x+d->w-11;
      //y pos of thumb
      vs_thumb_y = d->y+2;
      //length of thumb
      vs_thumb_len = (hh * frame_height + list_height/2) / list_height;
      //if thumb is not at top, draw a blank spot there
      if (y_offset > 0)
      {
        len = ((hh * y_offset) + list_height/2) / list_height;
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
      //    if (draw_hscroll&&list_width>frame_width-vscroll_w) {
      //x pos of thumb
      hs_thumb_x = d->x+2;
      //y pos of thumb
      hs_thumb_y = d->y+d->h-11;
      //width of thumb
      hs_thumb_wid = (ww * frame_width + list_width/2) / list_width;
      //if thumb is not at left, draw a blank spot there
      if (x_offset > 0)
      {
        wid = ((ww * x_offset) + list_width/2) / list_width;
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
void _handle_scrollable_scroll_click_vh(DIALOG *d, int list_height, int *y_offset, int frame_height, int list_width, int *x_offset, int frame_width, int vscroll_bar_style, int hscroll_bar_style, int handle_what)
{
  int vs_thumb_y, vs_thumb_mouse_y;
  int hs_thumb_x, hs_thumb_mouse_x;
  int hh, ww;
  int vs_thumb_len, hs_thumb_wid;
  int len, wid;

  int draw_vscroll=(((list_height > frame_height)|| (vscroll_bar_style==eb_scrollbar_on))&& (vscroll_bar_style!=eb_scrollbar_off));
  int draw_hscroll=(((list_width > frame_width)|| (hscroll_bar_style==eb_scrollbar_on))&& (hscroll_bar_style!=eb_scrollbar_off));
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
      case eb_handle_vscroll:
        //length of thumb
        vs_thumb_len = (hh * frame_height + list_height/2) / list_height;
        len = (((hh * (*y_offset)) + list_height/2) / list_height)+2;
        //        len = (hh * (*y_offset) + list_height/2) / list_height + 2;

        if ((gui_mouse_y() >= d->y+len) && (gui_mouse_y() <= d->y+len+vs_thumb_len))
        {
          vs_thumb_mouse_y = gui_mouse_y() - len + 2;
          while (gui_mouse_b())
          {
            vs_thumb_y = (list_height * (gui_mouse_y() - vs_thumb_mouse_y) + hh/2) / hh;
            if (vs_thumb_y > list_height-frame_height)
            {
              vs_thumb_y = list_height-frame_height;
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

            // let other objects continue to animate 
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

          if (vs_thumb_y > list_height-frame_height)
          {
            vs_thumb_y = list_height-frame_height;
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
      case eb_handle_hscroll:
        //length of thumb
        hs_thumb_wid = (ww * frame_width + list_width/2) / list_width;
        wid = (((ww * (*x_offset)) + list_width/2) / list_width)+2;
        //        len = (hh * (*y_offset) + list_height/2) / list_height + 2;

        if ((gui_mouse_x() >= d->x+wid) && (gui_mouse_x() <= d->x+wid+hs_thumb_wid))
        {
          hs_thumb_mouse_x = gui_mouse_x() - wid + 2;
          while (gui_mouse_b())
          {
            hs_thumb_x = (list_width * (gui_mouse_x() - hs_thumb_mouse_x) + ww/2) / ww;
            if (hs_thumb_x > list_width-frame_width)
            {
              hs_thumb_x = list_width-frame_width;
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

            // let other objects continue to animate 
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

          if (hs_thumb_x > list_width-frame_width)
          {
            hs_thumb_x = list_width-frame_width;
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

    // let other objects continue to animate 
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

  // two loops to avoid bank switches 
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

/* uinsert:
 *  Inserts a character at the specified index within a string, sliding
 *  following data along to make room. Returns how far the data was moved.
 */
int eb_insert(char *dest, int index, char *source)
{
  //   int w = ustrsize(source);
  //   char *dest="Frogs!"
  ASSERT(source);
  ASSERT(dest);

  dest += uoffset(dest, index);
  memmove(dest+ustrsize(source), dest, ustrsize(dest));
  memcpy(dest, source, ustrsize(source));
  //   usetc(dest, c);

  return ustrsize(source);
}

int eb_delete(char *s, int index, int w)
{
  ASSERT(s);
  s += index;
  memmove(s, s+w, ustrsizez(s+w));

  return -w;
}

/* d_editbox_proc:
 *  A text box object. The dp field points to a char * which is the text
 *  to be displayed in the text box. If the text is long, there will be
 *  a vertical scrollbar on the right hand side of the object which can
 *  be used to scroll through the text. The default is to print the text
 *  with word wrapping, but if the D_SELECTED flag is set, the text will
 *  be printed with character wrapping. The d1 field is used internally
 *  to store the number of lines of text, and d2 is used to store how far
 *  it has scrolled through the text.
 */
int d_editbox_proc(int msg, DIALOG *d, int c)
{
  int height, width, vsbar_w=0, hsbar_h=0, ret = D_O_K;
  int start, ln=0;
  int used, delta;
  int fg_color;
  static clock_t tics;
  char temptext[256];
  int firstchar=0, lastchar=0;
  char lastbreakchar=0;
  int firstxpos=0, prevxpos=0;
  //  char tabchar=128;
  //  char crlfchar=129;
  //  char crlfchar=0x00B6;
  int tempchar;
  int tempselstart, tempselend;
  int templine=0;
  int from_backspace=0;
  int ignore_mouse=0;
  int tabsize;
  static textline *sharedtext_data=(textline *)malloc(sizeof(textline));
  //  static char *clipboard;
  editbox_data *ebd=(editbox_data *)(d->dp);
  textline *currentline;

  static int firstrun=1;

  FONT *tempfont=font;

  char *ccptr=ebd->text_data->text;
  char *ccptr2=ebd->text_data->text;
  //  ebd->clipboard=clipboard;
  tabsize=ebd->defaulttabsize==0?30:(ebd->tabunits==0?ebd->defaulttabsize:ebd->defaulttabsize*uchar_width(font, ' '));
  ebd->vsbarstyle=eb_scrollbar_on;
  ebd->hsbarstyle=eb_scrollbar_optional;
  if (ebd->font)
  {
    font=ebd->font;
  }

  get_palette(current_pal);
  if (firstrun==1||memcmp(current_pal, old_pal, sizeof(PALETTE)))
  {
    memcpy(old_pal, current_pal, sizeof(PALETTE));
    create_color_table(&negative_table, current_pal, return_negative_color, NULL);
    color_map = &negative_table;
    if (firstrun==1)
    {
      if (ebd->clipboardsize>0)
      {
        ebd->clipboard=(char*)malloc(ebd->clipboardsize);
      }
    }
    firstrun=0;
  }

  ASSERT(d);

  fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
  // calculate the actual height 
  height = d->h-8;
  width = d->w-8;

  if (((ebd->lines*text_height(font) > height)&& (ebd->vsbarstyle!=eb_scrollbar_off))|| (ebd->vsbarstyle==eb_scrollbar_on))
  {
    vsbar_w = 12;
  }
  else
  {
    vsbar_w = 0;
  }

  if (((ebd->list_width > width)&& (ebd->hsbarstyle!=eb_scrollbar_off))|| (ebd->hsbarstyle==eb_scrollbar_on))
  {
    hsbar_h = 12;
  }
  else
  {
    hsbar_h = 0;
  }

  ln = (d->h-8)/text_height(font);

  if (msg!=MSG_IDLE&&msg!=MSG_DRAW)
  {
    al_trace("entering switch code (%d)\n",ebd->maxlines);
    al_trace("msg=%d\n",msg);
    al_trace("clipboard contents:\n%s\n",ebd->clipboard);
  }
  switch (msg)
  {
    case MSG_START:
      //just for testing...
      d->dp2=create_bitmap(d->w, d->h);
      ebd->tabdisplaystyle=1;
      ebd->crlfdisplaystyle=2;
      ebd->customtabs=0;
      ebd->customtabpos=(int*)calloc(3, sizeof(int));
      ebd->customtabpos[0]=0;
      ebd->maxchars=0;
      //      ebd->maxlines=0;
      ebd->insertmode=0;
      //      ebd->clipboardsize=0;

      //reset counter variables
      ebd->list_width=0;
      ebd->currchar=0;
      ebd->currxpos=0;
      ebd->numchars=0;
      ebd->fakexpos=0;
      ebd->selstart=0;
      ebd->selend=0;
      ebd->xofs=0;
      ebd->wrapping=eb_wrap_none;
      ebd->newcrlftype=eb_crlf_n;
      if (ebd->text_data==NULL)
      {
        al_trace("ebd->text==NULL\n");
        ebd->text_data=sharedtext_data;
        ebd->text_data->text=(char*)calloc(1,65536);
        sprintf(ebd->text_data->text, "Editbox!\n");
      }
      else if (ebd->text_data->text==NULL)
      {
        al_trace("ebd->text==NULL\n");
        ebd->text_data=sharedtext_data;
        ebd->text_data->text=(char*)calloc(1,65536);
        sprintf(ebd->text_data->text, "Editbox!\n");
      }


      ebd->yofs=0;
      _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
      // measure how many lines of text we contain 
      // DONT DRAW anything 
      _draw_editbox(ebd->text_data, &ebd->lines, 0, ebd->xofs, ebd->yofs, ebd->wrapping, tabsize, d->x, d->y, d->w, d->h, (d->flags & D_DISABLED), 0,0,0, &ebd->list_width, 0, 0, 0, 0, ebd->tabdisplaystyle, ebd->crlfdisplaystyle);
      currentline=ebd->text_data;
      while (currentline!=NULL)
      {
        ccptr=currentline->text;
        while (ugetc(ccptr)!=0)
        {
          (ebd->numchars)++;
          ccptr+=uwidth(ccptr);
        }
        currentline=currentline->next;
      }
      break;

    case MSG_END:
      destroy_bitmap((BITMAP *)d->dp2);
      break;

      // update the cursor visibility in response to idle messages
    case MSG_IDLE:
      if ((d->flags &D_GOTFOCUS)&&(clock()>tics))
      {
        _set_cursor_visibility(&ebd->showcursor, d, &tics, -1);
      }
      break;

    case MSG_DRAW:
      // tell the object to sort of draw, but only calculate the listsize 
                                                            // DONT DRAW anything 
      _draw_editbox(ebd->text_data, &ebd->lines, 0, ebd->xofs, ebd->yofs, ebd->wrapping, tabsize, d->x, d->y, d->w, d->h, (d->flags & D_DISABLED), 0, 0, 0, &ebd->list_width, 0, 0, 0, 0, ebd->tabdisplaystyle, ebd->crlfdisplaystyle);
      //just in case the width or height changed, recalc scroll bar sizes...
      if (((ebd->lines*text_height(font) > height)&& (ebd->vsbarstyle!=eb_scrollbar_off))|| (ebd->vsbarstyle==eb_scrollbar_on))
      {
        vsbar_w = 12;
      }
      else
      {
        vsbar_w = 0;
      }

      if (((ebd->list_width > width)&& (ebd->hsbarstyle!=eb_scrollbar_off))|| (ebd->hsbarstyle==eb_scrollbar_on))
      {
        hsbar_h = 12;
      }
      else
      {
        hsbar_h = 0;
      }

      ln = (d->h-8)/text_height(font);

      // now do the actual drawing 
      _draw_editbox(ebd->text_data, &ebd->lines, 1, ebd->xofs, ebd->yofs, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h-hsbar_h, (d->flags & D_DISABLED), fg_color, d->bg, gui_mg_color, &ebd->list_width, ebd->selstart, ebd->selend, ebd->selfg, ebd->selbg, ebd->tabdisplaystyle, ebd->crlfdisplaystyle);

      // draw the frame around 
      _draw_scrollable_frame_vh(d, (ebd->lines)*text_height(font), ebd->yofs, height-hsbar_h, ebd->list_width, ebd->xofs, width-vsbar_w, ebd->vsbarstyle, ebd->hsbarstyle, fg_color, d->bg);

//      blit(screen, screen, d->x, d->y, rand()%SCREEN_W, rand()%SCREEN_H, d->w, d->h);
      blit(screen, (BITMAP *)d->dp2, d->x, d->y, 0, 0, d->w, d->h);
//      blit((BITMAP *)d->dp2, screen, 0, 0, rand()%SCREEN_W, rand()%SCREEN_H, d->w, d->h);

      //draw the cursor
      if (ebd->showcursor &&(d->flags &D_GOTFOCUS) &&(ebd->fakexpos-ebd->xofs>=0) &&(ebd->selstart==ebd->selend))
      {
        //only allow drawing in the text area
        set_clip_rect(screen, d->x+4, d->y+4, d->x+d->w-5-vsbar_w, d->y+d->h-5-hsbar_h);
        vline(screen, ebd->fakexpos+d->x+4-ebd->xofs, (ebd->currtextline*text_height(font)-ebd->yofs)+d->y+4, ((ebd->currtextline+1)*text_height(font)-ebd->yofs)+d->y+3, vc(8));
        //allow drawing anywhere
        set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
      }
      break;
      
    case MSG_BLINKCURSOR:
//      blit(screen, screen, d->x, d->y, rand()%SCREEN_W, rand()%SCREEN_H, d->w, d->h);
//      blit(screen, (BITMAP *)d->dp2, d->x, d->y, 0, 0, d->w, d->h);
      scare_mouse();
      blit((BITMAP *)d->dp2, screen, 0, 0, d->x, d->y, d->w, d->h);

      //draw the cursor
      if (ebd->showcursor &&(d->flags &D_GOTFOCUS) &&(ebd->fakexpos-ebd->xofs>=0) &&(ebd->selstart==ebd->selend))
      {
        //only allow drawing in the text area
        set_clip_rect(screen, d->x+4, d->y+4, d->x+d->w-5-vsbar_w, d->y+d->h-5-hsbar_h);
        vline(screen, ebd->fakexpos+d->x+4-ebd->xofs, (ebd->currtextline*text_height(font)-ebd->yofs)+d->y+4, ((ebd->currtextline+1)*text_height(font)-ebd->yofs)+d->y+3, vc(8));
        //allow drawing anywhere
        set_clip_rect(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
      }
      unscare_mouse();
        
      break;

    case MSG_CLICK:
      {
      // figure out if it's on the text or the scrollbar 
      if (is_in_rect(gui_mouse_x()-d->x, gui_mouse_y()-d->y, 0, 0, d->w-vsbar_w-1, d->h-hsbar_h-1))
      {
/*        
        ignore_mouse=1;
        tempchar=-1;
        while(gui_mouse_b())
        {
          // clicked on the text area 
          ebd->currtextline=((ebd->yofs+gui_mouse_y()-4-d->y)/text_height(font));
          if (ebd->currtextline<0)
          {
            ebd->currtextline=0;
          }
          if (ebd->currtextline>=ebd->lines-1)
          {
            ebd->currtextline=ebd->lines-1;
          }
          firstxpos=gui_mouse_x()-4-d->x+ebd->xofs;
          _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
          prevxpos=ebd->fakexpos=0;
          ccptr=*ebd->text+ebd->currchar;
          while (ebd->currchar<=lastchar)
          {
            if (ebd->fakexpos>firstxpos)
            {
              if (abs(ebd->fakexpos-firstxpos)>abs(firstxpos-prevxpos))
              {
                ebd->currchar=prevuchar(*ebd->text,ebd->currchar);
                ebd->fakexpos=prevxpos;
              }
              break;
            }
            prevxpos=ebd->fakexpos;
            usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
            if (ugetc(ccptr) == '\t')
            {
              ebd->fakexpos=(((ebd->fakexpos/tabsize)+1)*tabsize);
            }
            else
            {
              if (ugetc(ccptr)=='\n')
              {
                ccptr += uwidth(ccptr);
                if (ugetc(ccptr)=='\r')
                {
                  ebd->currchar = nextuchar(*ebd->text, ebd->currchar);
                  ebd->fakexpos+=_crlf_width(eb_crlf_nr, ebd->crlfdisplaystyle);
                }
                else
                {
                  ebd->fakexpos+=_crlf_width(eb_crlf_n, ebd->crlfdisplaystyle);
                }
              }
              else if (ugetc(ccptr)=='\r')
              {
                ccptr += uwidth(ccptr);
                if (ugetc(ccptr)=='\n')
                {
                  ebd->currchar = nextuchar(*ebd->text, ebd->currchar);
                  ebd->fakexpos+=_crlf_width(eb_crlf_rn, ebd->crlfdisplaystyle);
                }
                else
                {
                  ebd->fakexpos+=_crlf_width(eb_crlf_r, ebd->crlfdisplaystyle);
                }
              }
              else
              {
                ebd->fakexpos+=text_length(font, temptext);
              }
            }

            ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
            ccptr=*ebd->text+ebd->currchar;
          }
          ebd->selend=ebd->currchar;
          if (ignore_mouse)
          {
            ignore_mouse=0;
            ebd->selstart=ebd->selend;
          }
          ebd->currxpos=ebd->fakexpos;
          _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
          _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
          if (ebd->currchar!=tempchar)
          {
            tempchar=ebd->currchar;
            object_message(d, MSG_DRAW, 0);
          }
        }
        ret = D_O_K;
*/
      }
      else
      {
        // clicked on the scroll area
        // is it on the vertical scroll bar?
        if (is_in_rect(gui_mouse_x()-d->x, gui_mouse_y()-d->y, d->w-vsbar_w, 0, d->w-1, d->h-hsbar_h-1))
        {
          _handle_scrollable_scroll_click_vh(d, (ebd->lines)*text_height(font), &ebd->yofs, height-hsbar_h, ebd->list_width, &ebd->xofs, width-vsbar_w, ebd->vsbarstyle, ebd->hsbarstyle, eb_handle_vscroll);
        } else if (is_in_rect(gui_mouse_x()-d->x, gui_mouse_y()-d->y, 0, d->h-hsbar_h, d->w-vsbar_w-1, d->h-1))
        {
          _handle_scrollable_scroll_click_vh(d, (ebd->lines)*text_height(font), &ebd->yofs, height-hsbar_h, ebd->list_width, &ebd->xofs, width-vsbar_w, ebd->vsbarstyle, ebd->hsbarstyle, eb_handle_hscroll);
        }
      }
      }
      break;

    case MSG_CHAR:
      {
/*
      used = D_USED_CHAR;

      if (ebd->lines > 0)
      {
        switch (c>>8)
        {
          case KEY_C:
            if (key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])
            {
              //copy text to clipboard
              al_trace("entering copy code\n");
              al_trace("clipboard contents:\n%s\n",ebd->clipboard);
              if (ebd->selstart!=ebd->selend)
              {
                tempselstart=ebd->selstart<ebd->selend?ebd->selstart:ebd->selend;
                tempselend=ebd->selstart<ebd->selend?ebd->selend:ebd->selstart;
                if ((ebd->clipboardsize<(tempselend-tempselstart)+2)|| (always_resize_clipboard))
                {
                  if (ebd->clipboard)
                  {
                    free(ebd->clipboard);
                  }
                  ebd->clipboardsize=(tempselend-tempselstart)+1;
                  ebd->clipboard=(char *)malloc(ebd->clipboardsize);
                }
                memset(ebd->clipboard,0,ebd->clipboardsize-1);
                ebd->clipboard[ebd->clipboardsize-1]=0;
                memcpy(ebd->clipboard, *ebd->text+tempselstart, tempselend-tempselstart);
                al_trace("%d %d--->%d %d\n",ebd->selstart, ebd->selend, tempselstart, tempselend);
                al_trace("clipboard contents:\n%s\n",ebd->clipboard);
                al_trace("exiting copy code\n\n\n");
              }
            }
            else
            {
              used=D_O_K;
            }
            break;
          case KEY_X:
            if (key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])
            {
              //copy text to clipboard and delete
              if (ebd->selstart!=ebd->selend)
              {
                object_message(d, MSG_CHAR, KEY_C<<8);
                object_message(d, MSG_CHAR, KEY_DEL<<8);
              }
            }
            else
            {
              used=D_O_K;
            }
            break;
          case KEY_V:
            al_trace("entering paste code\n");
            al_trace("clipboard contents:\n%s\n",ebd->clipboard);
            //            tempselstart=ebd->selstart<ebd->selend?ebd->selstart:ebd->selend;
            //            tempselend=ebd->selstart<ebd->selend?ebd->selend:ebd->selstart;
            if (key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])
            {
              //paste text from clipboard
              for (tempchar=0; tempchar<ustrsize(ebd->clipboard); tempchar++)
              {
                al_trace("src1:\n%c(%d)\n",ebd->clipboard[tempchar],ebd->clipboard[tempchar]);
              }
              al_trace("\n\n");
              al_trace("src:\n%s\n\n\n\n",ebd->clipboard);
              tempchar=eb_insert(*ebd->text, ebd->currchar, ebd->clipboard);
              ebd->numchars+=tempchar;
              ebd->currchar+=tempchar;
              if (!ebd->postpaste_dontmove)                 //put cursor at end of paste area
              {
                tempchar=ebd->currchar;
                _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &firstchar, &lastchar, ebd->currchar);
                ebd->currxpos=0;
                while (firstchar<ebd->currchar)
                {
                  ccptr=*ebd->text+firstchar;
                  usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
                  if (ugetc(ccptr) == '\t')
                  {
                    ebd->currxpos=(((ebd->currxpos/tabsize)+1)*tabsize);
                  }
                  else
                  {
                    ebd->currxpos+=text_length(font, temptext);
                  }
                  firstchar=nextuchar(*ebd->text,firstchar);
                }
                ebd->fakexpos=ebd->currxpos;
              }
              _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
              _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
            }
            else
            {
              used=D_O_K;
            }
            al_trace("exiting paste code\n\n");
            break;
          case KEY_HOME:
            if (key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])
            {
              ebd->currchar=ebd->currxpos=ebd->currtextline=ebd->yofs=0;
            }
            else
            {
              _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
            }

            ebd->fakexpos=ebd->currxpos=0;

            ebd->selend=ebd->currchar;
            if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
            {
              ebd->selstart=ebd->selend;
            }

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

            break;
          case KEY_END:
            if (key[KEY_ZC_LCONTROL]||key[KEY_ZC_RCONTROL])
            {
              ebd->currtextline=ebd->lines-1;
            }                                               // else {
            _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
            prevxpos=ebd->currxpos=0;
            ccptr=*ebd->text+ebd->currchar;
            while ((ebd->currchar<=lastchar)&&!is_nr(ugetc(ccptr)))
            {
              prevxpos=ebd->currxpos;
              usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
              if (ugetc(ccptr) == '\t')
              {
                ebd->currxpos=(((ebd->currxpos/tabsize)+1)*tabsize);
              }
              else
              {
                ebd->currxpos+=text_length(font, temptext);
              }
              ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
              ccptr=*ebd->text+ebd->currchar;
            }
            //            }

            ebd->fakexpos=ebd->currxpos;

            ebd->selend=ebd->currchar;
            if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
            {
              ebd->selstart=ebd->selend;
            }

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

            break;
          case KEY_PGUP:
            ebd->yofs-=(ln*text_height(font));
            // make sure that the list stays in bounds 
            if (((ebd->yofs)/text_height(font)) > ebd->lines-ln)
            {
              ebd->yofs = (ebd->lines-ln)*text_height(font);
            }
            if (ebd->yofs < 0)
            {
              ebd->yofs = 0;
            }
            ebd->currtextline=ebd->currtextline-ln;
            if (ebd->currtextline > ebd->lines-1)
            {
              ebd->currtextline = ebd->lines-1;
            }
            if (ebd->currtextline < 0)
            {
              ebd->currtextline = 0;
            }
            _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
            firstxpos=ebd->currxpos;
            prevxpos=ebd->fakexpos=0;
            ccptr=*ebd->text+ebd->currchar;
            while ((ebd->currchar<lastchar)&&!is_nr(ugetc(ccptr)))
            {
              if (ebd->fakexpos>firstxpos)
              {
                if (abs(ebd->fakexpos-firstxpos)>abs(firstxpos-prevxpos))
                {
                  ebd->currchar=prevuchar(*ebd->text,ebd->currchar);
                  ebd->fakexpos=prevxpos;
                }
                break;
              }
              prevxpos=ebd->fakexpos;
              usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
              if (ugetc(ccptr) == '\t')
              {
                ebd->fakexpos=(((ebd->fakexpos/tabsize)+1)*tabsize);
              }
              else
              {
                ebd->fakexpos+=text_length(font, temptext);
              }

              ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
              ccptr=*ebd->text+ebd->currchar;
            }

            ebd->selend=ebd->currchar;
            if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
            {
              ebd->selstart=ebd->selend;
            }

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
            break;
          case KEY_PGDN:
            ebd->yofs+=(ln*text_height(font));
            // make sure that the list stays in bounds 
            if (((ebd->yofs)/text_height(font)) > ebd->lines-ln)
            {
              ebd->yofs = (ebd->lines-ln)*text_height(font);
            }
            if (ebd->yofs < 0)
            {
              ebd->yofs = 0;
            }
            ebd->currtextline=ebd->currtextline+ln;
            if (ebd->currtextline > ebd->lines-1)
            {
              ebd->currtextline = ebd->lines-1;
            }
            if (ebd->currtextline < 0)
            {
              ebd->currtextline = 0;
            }
            _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
            firstxpos=ebd->currxpos;
            prevxpos=ebd->fakexpos=0;
            ccptr=*ebd->text+ebd->currchar;
            while ((ebd->currchar<lastchar)&&!is_nr(ugetc(ccptr)))
            {
              if (ebd->fakexpos>firstxpos)
              {
                if (abs(ebd->fakexpos-firstxpos)>abs(firstxpos-prevxpos))
                {
                  ebd->currchar=prevuchar(*ebd->text,ebd->currchar);
                  ebd->fakexpos=prevxpos;
                }
                break;
              }
              prevxpos=ebd->fakexpos;
              usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
              if (ugetc(ccptr) == '\t')
              {
                ebd->fakexpos=(((ebd->fakexpos/tabsize)+1)*tabsize);
              }
              else
              {
                ebd->fakexpos+=text_length(font, temptext);
              }

              ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
              ccptr=*ebd->text+ebd->currchar;
            }

            ebd->selend=ebd->currchar;
            if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
            {
              ebd->selstart=ebd->selend;
            }

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

            break;

          case KEY_UP:
            if (ebd->currtextline>0)
            {
              (ebd->currtextline)--;
              _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
              firstxpos=ebd->currxpos;
              prevxpos=ebd->fakexpos=0;
              ccptr=*ebd->text+ebd->currchar;
              while ((ebd->currchar<=lastchar)&&!is_nr(ugetc(ccptr)))
              {
                prevxpos=ebd->fakexpos;
                usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
                if (ugetc(ccptr) == '\t')
                {
                  ebd->fakexpos=(((ebd->fakexpos/tabsize)+1)*tabsize);
                }
                else
                {
                  ebd->fakexpos+=text_length(font, temptext);
                }
                ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
                ccptr=*ebd->text+ebd->currchar;
                if (ebd->fakexpos>firstxpos)
                {
                  if (abs(ebd->fakexpos-firstxpos)>abs(firstxpos-prevxpos))
                  {
                    ebd->currchar=prevuchar(*ebd->text,ebd->currchar);
                    ebd->fakexpos=prevxpos;
                  }
                  break;
                }
              }
              ebd->selend=ebd->currchar;
              if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
              {
                ebd->selstart=ebd->selend;
              }
            }

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

            break;
          case KEY_DOWN:
            if (ebd->currtextline<ebd->lines-1)
            {
              (ebd->currtextline)++;
              _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
              firstxpos=ebd->currxpos;
              ebd->fakexpos=0;
              ccptr=*ebd->text+ebd->currchar;
              while ((ebd->currchar<=lastchar)&&!is_nr(ugetc(ccptr)))
              {
                prevxpos=ebd->fakexpos;
                usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
                if (ugetc(ccptr) == '\t')
                {
                  ebd->fakexpos=(((ebd->fakexpos/tabsize)+1)*tabsize);
                }
                else
                {
                  ebd->fakexpos+=text_length(font, temptext);
                }
                ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
                ccptr=*ebd->text+ebd->currchar;
                if (ebd->fakexpos>firstxpos)
                {
                  if (abs(ebd->fakexpos-firstxpos)>abs(firstxpos-prevxpos))
                  {
                    ebd->currchar=prevuchar(*ebd->text,ebd->currchar);
                    ebd->fakexpos=prevxpos;
                  }
                  break;
                }
              }
              ebd->selend=ebd->currchar;
              if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
              {
                ebd->selstart=ebd->selend;
              }
            }

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

            break;
          case KEY_RIGHT:
            if (ebd->currchar<ebd->numchars)
            {
              ebd->currxpos=ebd->fakexpos;
              ccptr=*ebd->text+ebd->currchar;
              if (is_nr(ugetc(ccptr)))
              {
                lastbreakchar=ugetc(ccptr);
                ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
                ccptr += uwidth(ccptr);
                if (ugetc(ccptr) == othernr(lastbreakchar))
                {
                  ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
                }
                if (ebd->currtextline<ebd->lines-1)
                {
                  (ebd->currtextline)++;
                  _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &ebd->currchar, &lastchar, -1);
                  ebd->currxpos=ebd->xofs=0;
                }
              }
              else
              {
                usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
                if (ugetc(ccptr) == '\t')
                {
                  ebd->currxpos=(((ebd->currxpos/tabsize)+1)*tabsize);
                }
                else
                {
                  ebd->currxpos+=text_length(font, temptext);
                }
                templine=ebd->currtextline;
                ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
                _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &firstchar, &lastchar, ebd->currchar);
                if (ebd->currtextline!=templine)
                {
                  ebd->currxpos=ebd->xofs=0;
                }
              }
              ebd->selend=ebd->currchar;
              if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
              {
                ebd->selstart=ebd->selend;
              }

            }
            ebd->fakexpos=ebd->currxpos;

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
            break;
          case KEY_BACKSPACE:
            if (ebd->selstart!=ebd->selend)
            {
              if (ebd->selstart>ebd->selend)
              {
                tempchar=ebd->selend;
                ebd->selend=ebd->selstart;
                ebd->selstart=tempchar;
              }
              eb_delete(*ebd->text, ebd->selstart, ebd->selend-ebd->selstart);
              ebd->numchars-=(ebd->selend-ebd->selstart);
              ebd->currchar=ebd->selend=ebd->selstart;
            }
            else
            {
              if (ebd->currchar>0)
              {
                from_backspace=1;
                object_message(d, MSG_CHAR, KEY_LEFT<<8);
                from_backspace=0;
                object_message(d, MSG_CHAR, KEY_DEL<<8);
              }
            }
            break;
          case KEY_LEFT:
            if (ebd->currchar>0)
            {
              ebd->currxpos=ebd->fakexpos;
              tempchar=ebd->currchar;
              //if the previous char is a \n or \r
              if (is_nr(ugetc(*ebd->text+prevuchar(*ebd->text,tempchar))))
              {
                //go back to the first \n or \r in this series
                while (is_nr(ugetc(*ebd->text+prevuchar(*ebd->text,tempchar))))
                {
                  if (tempchar==0)
                  {
                    break;
                  }
                  tempchar=prevuchar(*ebd->text,tempchar);
                }
                //then, find the last \n, \r, \n\r, or \r\n in this series
                lastchar=ebd->currchar;
                while (tempchar<lastchar&&is_nr(ugetc(*ebd->text+tempchar)))
                {
                  ebd->currchar=tempchar;
                  tempchar=nextuchar(*ebd->text,tempchar);
                  if (ugetc(*ebd->text+tempchar)== othernr(ugetc(*ebd->text+ebd->currchar)))
                  {
                    tempchar=nextuchar(*ebd->text,tempchar);
                  }
                }
                // ctl should always be > 0 at this point, but we check anyway
                if (ebd->currtextline>0)
                {
                  (ebd->currtextline)--;
                }
              }
              else
              {
                ebd->currchar=prevuchar(*ebd->text,ebd->currchar);
                _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &firstchar, &lastchar, ebd->currchar);
              }

              ebd->currxpos=0;
              _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &firstchar, &lastchar, -1);
              while (firstchar<ebd->currchar)
              {
                ccptr=*ebd->text+firstchar;
                usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
                if (ugetc(ccptr) == '\t')
                {
                  ebd->currxpos=(((ebd->currxpos/tabsize)+1)*tabsize);
                }
                else
                {
                  ebd->currxpos+=text_length(font, temptext);
                }
                firstchar=nextuchar(*ebd->text,firstchar);
              }

            }
            ebd->fakexpos=ebd->currxpos;

            if (!from_backspace)
            {
              ebd->selend=ebd->currchar;
              if (!key[KEY_LSHIFT]&&!key[KEY_RSHIFT])
              {
                ebd->selstart=ebd->selend;
              }
              _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
              _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
            }
            break;

          case KEY_DEL:
            if (ebd->selstart!=ebd->selend)
            {
              if (ebd->selstart>ebd->selend)
              {
                tempchar=ebd->selend;
                ebd->selend=ebd->selstart;
                ebd->selstart=tempchar;
              }
              eb_delete(*ebd->text, ebd->selstart, ebd->selend-ebd->selstart);
              ebd->numchars-=(ebd->selend-ebd->selstart);

              ebd->currchar=ebd->selend=ebd->selstart;
              ebd->currxpos=0;
              _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &firstchar, &lastchar, ebd->currchar);
              while (firstchar<ebd->currchar)
              {
                ccptr=*ebd->text+firstchar;
                usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
                if (ugetc(ccptr) == '\t')
                {
                  ebd->currxpos=(((ebd->currxpos/tabsize)+1)*tabsize);
                }
                else
                {
                  ebd->currxpos+=text_length(font, temptext);
                }
                firstchar=nextuchar(*ebd->text,firstchar);
              }
              ebd->fakexpos=ebd->currxpos;
            }
            else
            {
              if (ebd->currchar<ebd->numchars)
              {
                ccptr=*ebd->text+ebd->currchar;
                if (is_nr(ugetc(ccptr)))
                {
                  lastbreakchar=ugetc(ccptr);
                  ccptr=*ebd->text+nextuchar(*ebd->text,ebd->currchar);
                  if (ugetc(ccptr) == othernr(lastbreakchar))
                  {
                    uremove(*ebd->text, ebd->currchar);
                    (ebd->numchars)--;
                  }
                }

                uremove(*ebd->text, ebd->currchar);
                (ebd->numchars)--;

                tempchar=prevuchar(*ebd->text,ebd->currchar);
                ccptr=*ebd->text+tempchar;
                ccptr2=*ebd->text+ebd->currchar;

                //if the current character and the previous character
                //are \n and \r in any order...
                if (is_nr(ugetc(ccptr))&& ugetc(ccptr2)==othernr(ugetc(ccptr)))
                {
                  //check the character before the previous character
                  tempchar=prevuchar(*ebd->text,tempchar);
                  ccptr2=*ebd->text+tempchar;
                  //if it doesn't create a \n\r or \r\n sequence...
                  if (ugetc(ccptr2)!=othernr(ugetc(ccptr)))
                  {
                    tempchar=ebd->currchar;
                    //invert all \n's and \r's in a row
                    while (is_nr(ugetc(*ebd->text+tempchar)))
                    {
                      usetat(*ebd->text, tempchar, othernr(ugetc(*ebd->text+tempchar)));
                      tempchar=nextuchar(*ebd->text, tempchar);
                    }
                  }
                }
              }
            }
            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

            break;

          case KEY_ENTER:
            if (ebd->selstart!=ebd->selend)
            {
              if (ebd->selstart>ebd->selend)
              {
                tempchar=ebd->selend;
                ebd->selend=ebd->selstart;
                ebd->selstart=tempchar;
              }
              eb_delete(*ebd->text, ebd->selstart, ebd->selend-ebd->selstart);
              ebd->numchars-=(ebd->selend-ebd->selstart);
              ebd->currchar=ebd->selend=ebd->selstart;
            }
            ccptr=*ebd->text+ebd->currchar;
            if (is_nr(ugetc(ccptr)))
            {
              ccptr2=*ebd->text+nextuchar(*ebd->text, ebd->currchar);
              if (ugetc(ccptr2) == othernr(ugetc(ccptr)))
              {
                //insert crlf pair based on next 2 chars
                uinsert(*ebd->text, ebd->currchar, ugetc(ccptr));
                ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                ccptr2=*ebd->text+nextuchar(*ebd->text, ebd->currchar);
                uinsert(*ebd->text, ebd->currchar, ugetc(ccptr2));
                ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                ebd->numchars+=2;
              }
              else
              {
                //insert cr or lf based on next char
                uinsert(*ebd->text, ebd->currchar, ugetc(ccptr));
                ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                (ebd->numchars)++;
              }
            }
            else
            {
              tempchar=prevuchar(*ebd->text, ebd->currchar);
              ccptr=*ebd->text+tempchar;
              if (is_nr(ugetc(ccptr)))
              {
                ccptr2=*ebd->text+prevuchar(*ebd->text, tempchar);
                if (ugetc(ccptr2) == othernr(ugetc(ccptr)))
                {
                  //insert crlf pair based on prev 2 chars
                  uinsert(*ebd->text, ebd->currchar, ugetc(ccptr2));
                  ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                  uinsert(*ebd->text, ebd->currchar, ugetc(ccptr));
                  ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                  ebd->numchars+=2;
                }
                else
                {
                  //insert cr or lf based on prev char
                  uinsert(*ebd->text, ebd->currchar, ugetc(ccptr));
                  ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                  (ebd->numchars)++;
                }
              }
              else
              {
                //insert cr, lf, or crlf pair based on preference
                switch (ebd->newcrlftype)
                {
                  case eb_crlf_n:
                    uinsert(*ebd->text, ebd->currchar, '\n');
                    ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                    (ebd->numchars)++;
                    break;
                  case eb_crlf_r:
                    uinsert(*ebd->text, ebd->currchar, '\r');
                    ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                    (ebd->numchars)++;
                    break;
                  case eb_crlf_nr:
                    uinsert(*ebd->text, ebd->currchar, '\n');
                    ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                    uinsert(*ebd->text, ebd->currchar, '\r');
                    ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                    ebd->numchars+=2;
                    break;
                  case eb_crlf_rn:
                    uinsert(*ebd->text, ebd->currchar, '\r');
                    ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                    uinsert(*ebd->text, ebd->currchar, '\n');
                    ebd->currchar=nextuchar(*ebd->text, ebd->currchar);
                    ebd->numchars+=2;
                    break;
                }
              }
            }

            (ebd->currtextline)++;
            ebd->fakexpos=ebd->currxpos=0;

            object_message(d, MSG_DRAW, 0);

            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
            break;
          case KEY_TAB:
            if (ebd->selstart!=ebd->selend)
            {
              if (ebd->selstart>ebd->selend)
              {
                tempchar=ebd->selend;
                ebd->selend=ebd->selstart;
                ebd->selstart=tempchar;
              }
              eb_delete(*ebd->text, ebd->selstart, ebd->selend-ebd->selstart);
              ebd->numchars-=(ebd->selend-ebd->selstart);
              ebd->currchar=ebd->selend=ebd->selstart;
            }
            uinsert(*ebd->text, ebd->currchar, '\t');
            ebd->numchars += uwidth(*ebd->text+ebd->currchar);
            ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
            ebd->currxpos=(((ebd->currxpos/tabsize)+1)*tabsize);
            ebd->fakexpos=ebd->currxpos;
            _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
            _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
            break;
          default:
            used = D_O_K;
            break;
        }

        // make sure that the list stays in bounds 
        if (((ebd->yofs)/text_height(font)) > ebd->lines-ln)
        {
          ebd->yofs = (ebd->lines-ln)*text_height(font);
        }
        if (ebd->yofs < 0)
        {
          ebd->yofs = 0;
        }

      }
      else
      {
        used = D_O_K;
      }

      ret = used;
*/
      }
      break;

    case MSG_UCHAR:
      {
/*
      if ((c >= ' ') && (uisok(c)))
      {
        if (ebd->selstart!=ebd->selend)
        {
          if (ebd->selstart>ebd->selend)
          {
            tempchar=ebd->selend;
            ebd->selend=ebd->selstart;
            ebd->selstart=tempchar;
          }
          eb_delete(*ebd->text, ebd->selstart, ebd->selend-ebd->selstart);
          ebd->numchars-=(ebd->selend-ebd->selstart);
          ebd->currchar=ebd->selend=ebd->selstart;
        }
        uinsert(*ebd->text, ebd->currchar, c);
        ebd->numchars += uwidth(*ebd->text+ebd->currchar);
        ebd->currxpos=ebd->fakexpos;
        ccptr=*ebd->text+ebd->currchar;
        usetc(temptext+usetc(temptext, ugetc(ccptr)), 0);
        ebd->currxpos+=text_length(font, temptext);
        if (ebd->currxpos-ebd->xofs>d->w-vsbar_w-8)
        {
          ebd->xofs=ebd->currxpos-d->w+vsbar_w+9;
          //          ebd->xofs+=text_length(font, temptext);
        }
        templine=ebd->currtextline;
        ebd->currchar=nextuchar(*ebd->text,ebd->currchar);
        _find_char_in_textbox_line(ebd->text_data, ebd->wrapping, tabsize, d->x, d->y, d->w-vsbar_w, d->h, &ebd->currtextline, &firstchar, &lastchar, ebd->currchar);
        if (ebd->currtextline!=templine)
        {
          ebd->currxpos=0;
        }

        ebd->fakexpos=ebd->currxpos;

        _focus_on_cursor(d, ebd->currtextline*text_height(font), &(ebd->yofs), ebd->lines*text_height(font), d->h-hsbar_h-8, ebd->currxpos, &ebd->xofs, ebd->list_width, d->w-vsbar_w-8);
        _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);

        if (ebd->font)
        {
          font=tempfont;
        }
        return D_USED_CHAR;
      }
*/
      }
      break;

    case MSG_WHEEL:
      {
/*
      delta = (ln > 3) ? 3 : 1;

      // scroll, making sure that the list stays in bounds 
      start = ebd->yofs;
      ebd->yofs = ((c > 0) ? MAX(0, ((ebd->yofs)/text_height(font))-delta) : MIN(ebd->lines-ln, ((ebd->yofs)/text_height(font))+delta))*text_height(font);

      // if we changed something, better redraw... 
      if (ebd->yofs != start)
      {
        d->flags |= D_DIRTY;
      }

      ret = D_O_K;
*/
      }
      break;
    case MSG_WANTFOCUS:
      ret = D_WANTFOCUS;
      _set_cursor_visibility(&ebd->showcursor, d, &tics, 1);
      break;

    default:
      ret = D_O_K;
  }
  if (msg!=MSG_IDLE&&msg!=MSG_DRAW)
  {
    al_trace("exiting switch code\n");
    al_trace("clipboard contents:\n%s\n\n",ebd->clipboard);
  }
  if (ebd->font)
  {
    font=tempfont;
  }
  return ret;
}

/***  The End  ***/
