/*
   GUI.cc
   Extra GUI stuff by Jer.
*/


int close_dlg() { return D_CLOSE; }


/*****************************************/
/***********  drop list proc  ************/
/*****************************************/


int d_dropcancel_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_CLICK || msg==MSG_DCLICK)
    return D_CLOSE;
  return D_O_K;
}


static DIALOG droplist_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)     (d2)      (dp) */
 { d_list_proc,       0,    0,    0,    0,    0,       0,      0,       D_EXIT,     0,       0,        NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          0,       KEY_ESC,  close_dlg },
 { d_dropcancel_proc, 0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL },
 { d_dropcancel_proc, 0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL },
 { d_dropcancel_proc, 0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL },
 { d_dropcancel_proc, 0,    0,    0,    0,    0,       0,      0,       0,          0,       0,        NULL },
 { NULL }
};


typedef char *(*getfuncptr)(int, int *);


int droplist(DIALOG *d)
{
  int d1 = d->d1;
  int listsize, h;

  (*(getfuncptr)d->dp)(-1, &listsize);
  h = min(listsize,8) * text_height(font) + 4;

  droplist_dlg[0] = *d;
  droplist_dlg[0].proc = d_list_proc;
  droplist_dlg[0].flags = D_EXIT;
  droplist_dlg[0].h = h;
  droplist_dlg[0].d2 = listsize<=8 ? 0 : max(d1-3,0);

  droplist_dlg[2].x = 0;
  droplist_dlg[2].y = 0;
  droplist_dlg[2].w = d->x - 1;
  droplist_dlg[2].h = screen->h;

  droplist_dlg[3].x = d->x - 1;
  droplist_dlg[3].y = 0;
  droplist_dlg[3].w = d->w + 3;
  droplist_dlg[3].h = d->y - 1;

  droplist_dlg[4].x = d->x + d->w + 1;
  droplist_dlg[4].y = 0;
  droplist_dlg[4].w = screen->w - d->w - d->x;
  droplist_dlg[4].h = screen->h;

  droplist_dlg[5].x = d->x - 1;
  droplist_dlg[5].y = d->y + h + 1;
  droplist_dlg[5].w = d->w + 3;
  droplist_dlg[5].h = screen->h - h - d->y;

  if(popup_dialog(droplist_dlg,0)==0)
    return droplist_dlg[0].d1;
  return d1;
}


int d_droplist_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
  case MSG_CLICK:
    if(isinRect(mouse_x,mouse_y,d->x+d->w-10,d->y,d->x+d->w,d->y+d->h))
      goto dropit;
    break;

  case MSG_CHAR:
    if(c>>8 == KEY_ENTER)
      goto dropit;
    break;
  }
  return d_list_proc(msg,d,c);

  dropit:
  while(mouse_b);
  d->d2 = d->d1 = droplist(d);
  scare_mouse();
  d_list_proc(MSG_DRAW,d,0);
  unscare_mouse();
  while(mouse_b);
  if(msg==MSG_CLICK)
    return D_WANTFOCUS;
  return D_USED_CHAR;
}



/*****************************************/
/************  ABC list proc  ************/
/*****************************************/


int d_abclist_proc(int msg,DIALOG *d,int c)
{
  if(msg==MSG_CHAR && isalpha(c&0xFF))
  {
    int max,dummy,h;
    h = (d->h-3) / text_height(font);

    (*(getfuncptr)d->dp)(-1, &max);
    c&=0xDF;

    for(int i=0; i<max; i++)
    {
      if(((((*(getfuncptr)d->dp)(i,&dummy))[0])&0xDF)==c)
      {
        d->d1 = i;
        d->d2 = vbound(i-(h>>1),0,max-h);
        goto gotit;
      }
    }

    gotit:
    scare_mouse();
    d_list_proc(MSG_DRAW,d,0);
    unscare_mouse();
    return D_USED_CHAR;
  }

  return d_list_proc(msg,d,c);
}




/* end of gui.cc */
