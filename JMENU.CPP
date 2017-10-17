//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  jmenu.cc
//
//  The Allegro menu routines don't support joystick input, so here's
//  their code with a little joystick stuff added.
//
//--------------------------------------------------------

#include <ctype.h>
#include <string.h>
#include "zc_alleg.h"
#include "zelda.h"
#include "jwin.h"
#include <allegro\internal\aintern.h>

typedef struct MENU_INFO            /* information about a popup menu */
{
  MENU *menu;                      /* the menu itself */
  struct MENU_INFO *parent;        /* the parent menu, or NULL for root */
  int bar;                         /* set if it is a top level menu bar */
  int size;                        /* number of items in the menu */
  int sel;                         /* selected item */
  int x, y, w, h;                  /* screen position of the menu */
  int (*proc)();                   /* callback function */
  BITMAP *saved;                   /* saved what was underneath it */
} MENU_INFO;



/* get_menu_pos:
  *  Calculates the coordinates of an object within a top level menu bar.
  */
static void get_menu_pos(MENU_INFO *m, int c, int *x, int *y, int *w)
{
  int c2;

  if (m->bar) {
    *x = m->x+1;

    for (c2=0; c2<c; c2++)
      *x += gui_strlen(m->menu[c2].text) + 16;

    *y = m->y+1;
    *w = gui_strlen(m->menu[c].text) + 16;
  }
  else {
    *x = m->x+1;
    *y = m->y+c*(text_height(font)+4)+1;
    *w = m->w-2;
  }
}



/* draw_menu_item:
  *  Draws an item from a popup menu onto the screen.
  */
static void draw_menu_item(MENU_INFO *m, int c)
{
  int fg, bg;
  int i, x, y, w;
  char buf[80], *tok;

  if (m->menu[c].flags & D_DISABLED) {
    if (c == m->sel) {
      fg = gui_mg_color;
      bg = gui_fg_color;
    }
    else {
      fg = gui_mg_color;
      bg = gui_bg_color;
    }
  }
  else {
    if (c == m->sel) {
      fg = gui_bg_color;
      bg = gui_fg_color;
    }
    else {
      fg = gui_fg_color;
      bg = gui_bg_color;
    }
  }

  get_menu_pos(m, c, &x, &y, &w);

  rectfill(screen, x, y, x+w-1, y+text_height(font)+3, bg);
  text_mode(bg);

  if (m->menu[c].text[0]) {
    for (i=0; (m->menu[c].text[i]) && (m->menu[c].text[i] != '\t'); i++)
      buf[i] = m->menu[c].text[i];
    buf[i] = 0;

    gui_textout(screen, buf, x+8, y+1, fg, FALSE);

    if (m->menu[c].text[i] == '\t') {
      tok = m->menu[c].text+i+1;
      gui_textout(screen, tok, x+w-gui_strlen(tok)-8, y+1, fg, FALSE);
    }
  }
  else
    hline(screen, x, y+text_height(font)/2+2, x+w, fg);

  if (m->menu[c].flags & D_SELECTED) {
    line(screen, x+1, y+text_height(font)/2+1, x+3, y+text_height(font)+1, fg);
    line(screen, x+3, y+text_height(font)+1, x+6, y+2, fg);
  }
}



/* draw_menu:
  *  Draws a popup menu onto the screen.
  */
static void draw_menu(MENU_INFO *m)
{
  int c;

  rect(screen, m->x, m->y, m->x+m->w-1, m->y+m->h-1, gui_fg_color);
  vline(screen, m->x+m->w, m->y+1, m->y+m->h, gui_fg_color);
  hline(screen, m->x+1, m->y+m->h, m->x+m->w, gui_fg_color);

  for (c=0; m->menu[c].text; c++)
    draw_menu_item(m, c);
}



/* menu_mouse_object:
  *  Returns the index of the object the mouse is currently on top of.
  */
static int menu_mouse_object(MENU_INFO *m)
{
  int c;
  int x, y, w;

  for (c=0; c<m->size; c++) {
    get_menu_pos(m, c, &x, &y, &w);

    if ((gui_mouse_x() >= x) && (gui_mouse_x() < x+w) &&
        (gui_mouse_y() >= y) && (gui_mouse_y() < y+(text_height(font)+4)))
      return (m->menu[c].text[0]) ? c : -1;
  }

  return -1;
}



/* mouse_in_parent_menu:
  *  Recursively checks if the mouse is inside a menu or any of its parents.
  */
static int mouse_in_parent_menu(MENU_INFO *m)
{
  int c;

  if (!m)
    return FALSE;

  c = menu_mouse_object(m);
  if ((c >= 0) && (c != m->sel))
    return TRUE;

  return mouse_in_parent_menu(m->parent);
}



/* fill_menu_info:
  *  Fills a menu info structure when initialising a menu.
  */
static void fill_menu_info(MENU_INFO *m, MENU *menu, MENU_INFO *parent, int bar, int x, int y, int minw, int minh)
{
  int c, i;
  int extra = 0;
  char buf[80], *tok;

  m->menu = menu;
  m->parent = parent;
  m->bar = bar;
  m->x = x;
  m->y = y;
  m->w = 2;
  m->h = (m->bar) ? (text_height(font)+6) : 2;
  m->proc = NULL;
  m->sel = -1;

  /* calculate size of the menu */
  for (m->size=0; m->menu[m->size].text; m->size++) {
    for (i=0; (m->menu[m->size].text[i]) && (m->menu[m->size].text[i] != '\t'); i++)
      buf[i] = m->menu[m->size].text[i];
    buf[i] = 0;

    c = gui_strlen(buf);

    if (m->bar) {
      m->w += c+16;
    }
    else {
      m->h += text_height(font)+4;
      m->w = MAX(m->w, c+16);
    }

    if (m->menu[m->size].text[i] == '\t') {
      tok = m->menu[m->size].text+i+1;
      c = gui_strlen(tok);
      extra = MAX(extra, c);
    }
  }

  if (extra)
    m->w += extra+16;

  m->w = MAX(m->w, minw);
  m->h = MAX(m->h, minh);
}



/* menu_key_shortcut:
  *  Returns true if c is indicated as a keyboard shortcut by a '&' character
  *  in the specified string.
  */
static int menu_key_shortcut(int c, char *s)
{
  while (*s) {
    if (*s == '&') {
      ++s;
      if ((*s != '&') && (tolower(*s) == tolower(c & 0xff)))
        return TRUE;
    }
    ++s;
  }

  return FALSE;
}




/* menu_alt_key:
  *  Searches a menu for keyboard shortcuts, for the alt+letter to bring
  *  up a menu.
  */
int menu_alt_key(int k, MENU *m)
{
  char *s;
  int c;

  if (k & 0xff)
    return 0;

  //k = key_ascii_table[k>>8];

  for (c=0; m[c].text; c++) {
    s = m[c].text;
    while (*s) {
      if (*s == '&') {
        ++s;
        if ((*s != '&') && (tolower(*s) == tolower(k)))
          return k;
      }
      ++s;
    }
  }

  return 0;
}



/* _j_do_menu:
  *  The core menu control function, called by do_menu() and d_menu_proc().
  */
int _j_do_menu(MENU *menu, MENU_INFO *parent, int bar, int x, int y, int repos, int *dret, int minw, int minh)
{
  MENU_INFO m;
  MENU_INFO *i;
  int c, c2;
  int ret = -1;
  int mouse_on = gui_mouse_b();
  int joy_on = joy_left + joy_right + joy_up + joy_down + joy_b1 + joy_b2;
  int old_sel;
  int mouse_sel;
  int _x, _y;
  int redraw = TRUE;

  scare_mouse();

  fill_menu_info(&m, menu, parent, bar, x, y, minw, minh);

  if (repos) {
    m.x = MID(0, m.x, SCREEN_W-m.w-1);
    m.y = MID(0, m.y, SCREEN_H-m.h-1);
  }

  /* save screen under the menu */
  m.saved = create_bitmap(m.w+1, m.h+1);

  if (m.saved)
    blit(screen, m.saved, m.x, m.y, 0, 0, m.w+1, m.h+1);
  else
    errno = ENOMEM;

  m.sel = mouse_sel = menu_mouse_object(&m);
  if ((m.sel < 0) && (!mouse_on) && (!bar))
    m.sel = 0;

  unscare_mouse();

  do {
    old_sel = m.sel;

    c = menu_mouse_object(&m);
    if ((gui_mouse_b()) || (c != mouse_sel))
      m.sel = mouse_sel = c;

    if (gui_mouse_b()) {                            /* if button pressed */
      if ((gui_mouse_x() < m.x) || (gui_mouse_x() > m.x+m.w) ||
          (gui_mouse_y() < m.y) || (gui_mouse_y() > m.y+m.h)) {
        if (!mouse_on)                            /* dismiss menu? */
          break;

        if (mouse_in_parent_menu(m.parent))       /* back to parent? */
          break;
      }

      if ((m.sel >= 0) && (m.menu[m.sel].child))   /* bring up child? */
        ret = m.sel;

      mouse_on = TRUE;
      clear_keybuf();
    }
    else {                                          /* button not pressed */
      if (mouse_on)                                 /* selected an item? */
        ret = m.sel;

      mouse_on = FALSE;

      poll_joystick();
      if (joy_left + joy_right + joy_up + joy_down + joy_b1 + joy_b2 == 0)
        joy_on = FALSE;

      c = 0;

      if (keypressed())                            /* keyboard input */
        c = readkey();
      else if (!joy_on)                            /* joystick input */
        {
          if (joy_up)           { c = KEY_UP    << 8; joy_on = TRUE; }
          if (joy_down)         { c = KEY_DOWN  << 8; joy_on = TRUE; }
          if (joy_left)         { c = KEY_LEFT  << 8; joy_on = TRUE; }
          if (joy_right)        { c = KEY_RIGHT << 8; joy_on = TRUE; }
          if (joy_b1 | joy_b2)  { c = KEY_ENTER << 8; joy_on = TRUE; }
        }

        if (c) {

        if ((c & 0xff) == 27) {
          ret = -1;
          goto getout;
        }

        switch (c >> 8) {

          case KEY_LEFT:
          if (m.parent) {
            if (m.parent->bar) {
              simulate_keypress(KEY_LEFT<<8);
              simulate_keypress(KEY_DOWN<<8);
            }
            ret = -1;
            goto getout;
          }
          /* fall through */

          case KEY_UP:
          if ((((c >> 8) == KEY_LEFT) && (m.bar)) ||
              (((c >> 8) == KEY_UP) && (!m.bar))) {
            c = m.sel;
            do {
              --c;
              if (c < 0)
                c = m.size - 1;
            } while ((!(m.menu[c].text[0])) && (c != m.sel));
            m.sel = c;
          }
          break;

          case KEY_RIGHT:
          if (((m.sel < 0) || (!m.menu[m.sel].child)) &&
              (m.parent) && (m.parent->bar)) {
            simulate_keypress(KEY_RIGHT<<8);
            simulate_keypress(KEY_DOWN<<8);
            ret = -1;
            goto getout;
          }
          /* fall through */

          case KEY_DOWN:
          if ((m.sel >= 0) && (m.menu[m.sel].child) &&
              ((((c >> 8) == KEY_RIGHT) && (!m.bar)) ||
               (((c >> 8) == KEY_DOWN) && (m.bar)))) {
            ret = m.sel;
          }
          else if ((((c >> 8) == KEY_RIGHT) && (m.bar)) ||
                   (((c >> 8) == KEY_DOWN) && (!m.bar))) {
              c = m.sel;
              do {
                ++c;
                if (c >= m.size)
                  c = 0;
              } while ((!(m.menu[c].text[0])) && (c != m.sel));
              m.sel = c;
            }
            break;

          case KEY_SPACE:
          case KEY_ENTER:
          if (m.sel >= 0)
            ret = m.sel;
          break;

          default:
          if ((!m.parent) && ((c & 0xff) == 0))
            c = menu_alt_key(c, m.menu);
          for (c2=0; m.menu[c2].text; c2++) {
            if (menu_key_shortcut(c, m.menu[c2].text)) {
              ret = m.sel = c2;
              break;
            }
          }
          if (m.parent) {
            i = m.parent;
            for (c2=0; i->parent; c2++)
              i = i->parent;
            c = menu_alt_key(c, i->menu);
            if (c) {
              while (c2-- > 0)
                simulate_keypress(27);
              simulate_keypress(c);
              ret = -1;
              goto getout;
            }
          }
          break;
        }
      }
    }

    if ((redraw) || (m.sel != old_sel)) {           /* selection changed? */
      scare_mouse();

      if (redraw) {
        draw_menu(&m);
        redraw = FALSE;
      }
      else {
        if (old_sel >= 0)
          draw_menu_item(&m, old_sel);

        if (m.sel >= 0)
          draw_menu_item(&m, m.sel);
      }

      unscare_mouse();
    }

    if ((ret >= 0) && (m.menu[ret].flags & D_DISABLED))
      ret = -1;

    if (ret >= 0) {                                 /* child menu? */
      if (m.menu[ret].child) {
        if (m.bar) {
          get_menu_pos(&m, ret, &_x, &_y, &c);
          _x += 6;
          _y += text_height(font)+7;
        }
        else {
          //	       _x = m.x+m.w*2/3;
          _x = m.x+m.w-4;
          _y = m.y + (text_height(font)+4)*ret + text_height(font)/4+2;
        }
        c = _j_do_menu(m.menu[ret].child, &m, FALSE, _x, _y, TRUE, NULL, 0, 0);
        if (c < 0) {
          ret = -1;
          mouse_on = FALSE;
          mouse_sel = menu_mouse_object(&m);
        }
      }
    }

    if ((m.bar) && (!gui_mouse_b()) && (!keypressed()) &&
        ((gui_mouse_x() < m.x) || (gui_mouse_x() > m.x+m.w) ||
         (gui_mouse_y() < m.y) || (gui_mouse_y() > m.y+m.h)))
      break;

  } while (ret < 0);

getout:

  if (dret)
    *dret = 0;

  /* callback function? */
  if ((!m.proc) && (ret >= 0)) {
    active_menu = &m.menu[ret];
    m.proc = active_menu->proc;
  }

  if (ret >= 0) {
    if (parent)
      parent->proc = m.proc;
    else  {
      if (m.proc) {
        c = m.proc();
        if (dret)
          *dret = c;
      }
    }
  }

  /* restore screen */
  if (m.saved) {
    scare_mouse();
    blit(m.saved, screen, 0, 0, m.x, m.y, m.w+1, m.h+1);
    unscare_mouse();
    destroy_bitmap(m.saved);
  }

  return ret;
}



/* j_do_menu:
  *  Displays and animates a popup menu at the specified screen position,
  *  returning the index of the item that was selected, or -1 if it was
  *  dismissed. If the menu crosses the edge of the screen it will be moved.
  */
int j_do_menu(MENU *menu, int x, int y)
{
  int ret = _j_do_menu(menu, NULL, FALSE, x, y, TRUE, NULL, 0, 0);

  do {
  } while (gui_mouse_b());

  return ret;
}



/* d_jmenu_proc:
  *  Dialog procedure for adding drop down menus to a GUI dialog. This
  *  displays the top level menu items as a horizontal bar (eg. across the
  *  top of the screen), and pops up child menus when they are clicked.
  *  When it executes one of the menu callback routines, it passes the
  *  return value back to the dialog manager, so these can return D_O_K,
  *  D_CLOSE, D_REDRAW, etc.
  */
int d_jmenu_proc(int msg, DIALOG *d, int c)
{
  MENU_INFO m;
  int ret = D_O_K;
  int x;

  switch (msg) {

    case MSG_START:
    fill_menu_info(&m, (MENU*)d->dp, NULL, TRUE, d->x-1, d->y-1, d->w+2, d->h+2);
    d->w = m.w-2;
    d->h = m.h-2;
    break;

    case MSG_DRAW:
    fill_menu_info(&m, (MENU*)d->dp, NULL, TRUE, d->x-1, d->y-1, d->w+2, d->h+2);
    draw_menu(&m);
    break;

    case MSG_XCHAR:
    x = menu_alt_key(c, (MENU*)d->dp);
    if (!x)
      break;

    ret |= D_USED_CHAR;
    simulate_keypress(x);
    /* fall through */

    case MSG_GOTMOUSE:
    case MSG_CLICK:
    _j_do_menu((MENU*)d->dp, NULL, TRUE, d->x-1, d->y-1, FALSE, &x, d->w+2, d->h+2);
    ret |= x;
    do {
    } while (gui_mouse_b());
    break;
  }

  return ret;
}



 
