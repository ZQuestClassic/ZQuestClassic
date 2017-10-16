/*
  ZQ_TILES.CC

*/


/*********************************/
/*****    Tiles & Combos    ******/
/*********************************/

void little_x(BITMAP *dest, int x, int y, int c, int s)
{
  line(dest,x,y,x+s,y+s,c);
  line(dest,x+s,y,x,y+s,c);
}


void zoomtile8(BITMAP *dest,int tile,int x,int y,int cset,int flip,int m)
{
  cset <<= 4;
  byte *si = tilebuf+(tile<<6);
  for(int cy=0; cy<8; cy++)
    for(int cx=0; cx<8; cx++)
    {
      int dx = ((flip&1)?7-cx:cx)*m;
      int dy = ((flip&2)?7-cy:cy)*m;
      rectfill(dest,x+dx,y+dy,x+dx+m-2,y+dy+m-2,*si+cset);
      if(*si==0)
        little_x(dest,x+dx+2,y+dy+2,vc(7),2);
      si++;
    }
}


void zoomtile16(BITMAP *dest,int tile,int x,int y,int cset,int flip,int m)
{
  if(tile>=MIN16) {
    cset <<= 4;
    byte *si = tilebuf+(tile<<8);

    for(int cy=0; cy<16; cy++)
      for(int cx=0; cx<16; cx++)
      {
        int dx = ((flip&1)?15-cx:cx)*m;
        int dy = ((flip&2)?15-cy:cy)*m;
        rectfill(dest,x+dx,y+dy,x+dx+m-2,y+dy+m-2,*si+cset);
        if(*si==0)
          little_x(dest,x+dx+2,y+dy+2,vc(7),2);
        si++;
      }
    }
  else {
    tile <<= 2;
    int t1=tile;
    int t2=tile+1;
    int t3=tile+2;
    int t4=tile+3;
    if(flip&1) {
      swap(t1,t3);
      swap(t2,t4);
      }
    if(flip&2) {
      swap(t1,t2);
      swap(t3,t4);
      }
    zoomtile8(dest,t1,x,y,cset,flip,m);
    zoomtile8(dest,t2,x,y+m*8,cset,flip,m);
    zoomtile8(dest,t3,x+m*8,y,cset,flip,m);
    zoomtile8(dest,t4,x+m*8,y+m*8,cset,flip,m);
    }
}


void draw_button(BITMAP *dest,int x,int y,int w,int h,char *text,int bg,int fg)
{
 rect(dest,x+1,y+1,x+w-1,y+h-1,fg);
 rectfill(dest,x+1,y+1,x+w-3,y+h-3,bg);
 rect(dest,x,y,x+w-2,y+h-2,fg);
 text_mode(-1);
 textout_centre(dest,font,text,(x+x+w)>>1,((y+y+h)>>1)-4,fg);
}


bool do_button(int x,int y,int w,int h,char *text,int bg,int fg)
{
 bool over=false;
 while(mouse_b) {
  vsync();
  if(isinRect(mouse_x,mouse_y,x,y,x+w-1,y+h-1)) {
    if(!over) {
      scare_mouse();
      draw_button(screen,x,y,w,h,text,fg,bg);
      unscare_mouse();
      over=true;
      }
    }
  else {
    if(over) {
      scare_mouse();
      draw_button(screen,x,y,w,h,text,bg,fg);
      unscare_mouse();
      over=false;
      }
    }
  }
 return over;
}


bool do_button_reset(int x,int y,int w,int h,char *text,int bg,int fg)
{
 bool over=false;
 while(mouse_b) {
  vsync();
  if(isinRect(mouse_x,mouse_y,x,y,x+w-1,y+h-1)) {
    if(!over) {
      scare_mouse();
      draw_button(screen,x,y,w,h,text,fg,bg);
      unscare_mouse();
      over=true;
      }
    }
  else {
    if(over) {
      scare_mouse();
      draw_button(screen,x,y,w,h,text,bg,fg);
      unscare_mouse();
      over=false;
      }
    }
  }

 if(over) {
   vsync();
   scare_mouse();
   draw_button(screen,x,y,w,h,text,bg,fg);
   unscare_mouse();
   }

 return over;
}


//*************** tile flood fill stuff **************

int  tf_t;
byte tf_c;
byte tf_u;

void tile_floodfill_rec(int x,int y)
{
  while(x>0 && (tilebuf[(tf_t<<8)+(y<<4)+x-1] == tf_u))
    x--;

  while(x<=15 && (tilebuf[(tf_t<<8)+(y<<4)+x] == tf_u)) {

    tilebuf[(tf_t<<8)+(y<<4)+x] = tf_c;

    if(y>0 && (tilebuf[(tf_t<<8)+((y-1)<<4)+x] == tf_u))
      tile_floodfill_rec(x,y-1);

    if(y<15 && (tilebuf[(tf_t<<8)+((y+1)<<4)+x] == tf_u))
      tile_floodfill_rec(x,y+1);

    x++;
    }
}

void tile_floodfill(int tile,int x,int y,byte c)
{
  tf_t = tile;
  tf_c = c;
  tf_u = tilebuf[(tile<<8)+(y<<4)+x];
  if(tf_u != tf_c)
    tile_floodfill_rec(x,y);
}



//***************** tile editor  stuff *****************

int c1=1;
int c2=0;
int bgc=dvc(9);
enum { t_pen, t_brush, t_recolor, t_select };
int tool = t_pen;
int tool_cur = -1;
int drawing=0;

void update_tool_cursor()
{
 if(isinRect(mouse_x,mouse_y,80,32,206,158)) {
   if(tool_cur==-1)
     set_mouse_sprite(mouse_bmp[9+tool]);
   tool_cur=tool;
   }
 else if(tool_cur != -1) {
   set_mouse_sprite(mouse_bmp[0]);
   tool_cur = -1;
   }
}

void draw_edit_scr(int tile,int flip,int cs,byte *oldtile)
{
 clear_to_color(screen2,bgc);

 swap(oldtile,tilebuf);
 if(tile>=MIN16)
   temptile=true;
 puttile16(screen2,0,224,48,cs,flip);
 overtile16(screen2,0,248,48,cs,flip);
 temptile=false;
 swap(oldtile,tilebuf);

 puttile16(screen2,tile,224,80,cs,flip);
 overtile16(screen2,tile,248,80,cs,flip);
 zoomtile16(screen2,tile,80,32,cs,flip,8);

 rect(screen2,223,47,240,64,dvc(14));
 rect(screen2,247,47,264,64,dvc(14));
 rect(screen2,223,79,240,96,dvc(14));
 rect(screen2,247,79,264,96,dvc(14));
 rect(screen2,79,31,207,159,dvc(14));
 rect(screen2,103,175,136,208,dvc(14));

 textprintf(screen2,font,224,112,vc(11),"tile: %d",tile);
 textprintf(screen2,font,224,120,vc(11),"flip: %d",flip);
 textprintf(screen2,font,224,128,vc(11),"cset: %d",cs);

 for(int i=0; i<16; i++) {
   int x=((i&3)<<3)+104;
   int y=((i>>2)<<3)+176;
   rectfill(screen2,x,y,x+7,y+7,CSET(cs)+i);
   }
 little_x(screen2,106,178,vc(7),2);

 textprintf(screen2,font,152,180,vc(11),"l:  %d",c1);
 textprintf(screen2,font,152,196,vc(11),"r:  %d",c2);

 rectfill(screen2,171,180,178,187,CSET(cs)+c1);
 if(c1==0) little_x(screen2,173,182,vc(7),2);
 rectfill(screen2,171,196,178,203,CSET(cs)+c2);
 if(c2==0) little_x(screen2,173,198,vc(7),2);

 draw_button(screen2,224,168,60,16,"OK",dvc(9),vc(14));
 draw_button(screen2,224,192,60,16,"Cancel",dvc(9),vc(14));
 draw_button(screen2,24,184,60,16,"Edit",dvc(9),vc(14));

 blit(mouse_bmp[9],screen2,0,0,48,40,16,16);
 blit(mouse_bmp[10],screen2,0,0,48,64,16,16);
 blit(mouse_bmp[11],screen2,0,0,48,88,16,16);
 blit(mouse_bmp[12],screen2,0,0,48,112,16,16);

 rect(screen2,47,tool*24+39,64,tool*24+56,vc(14));

 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 update_tool_cursor();
 unscare_mouse();
 SCRFIX();
}


void normalize(int tile,int flip)
{
  if(flip&1)
  {
    for(int y=0; y<16; y++)
      for(int x=0; x<8; x++)
        swap(tilebuf[(tile<<8)+(y<<4)+x],tilebuf[(tile<<8)+(y<<4)+15-x]);
  }
  if(flip&2)
  {
    for(int y=0; y<8; y++)
      for(int x=0; x<16; x++)
        swap(tilebuf[(tile<<8)+(y<<4)+x],tilebuf[(tile<<8)+((15-y)<<4)-x]);
  }
}


void edit_tile(int tile,int flip,int &cs)
{
 byte oldtile[256],undotile[256];
 for(int i=0; i<256; i++)
   oldtile[i]=undotile[i]=tilebuf[(tile<<8)+i];

 bool bdown=false;
 int done=0;
 drawing=0;
 tool_cur = -1;
 if(tool>t_pen && tile<MIN16)
   tool=t_pen;

 draw_edit_scr(tile,flip,cs,oldtile);
 while(mouse_b);

 do {
   bool redraw=false;

   if(keypressed()) {
     switch(readkey()>>8) {
     case KEY_ENTER: done=2; break;
     case KEY_ESC:   done=1; break;
     case KEY_H:     flip^=1; redraw=true; break;
     case KEY_V:     flip^=2; redraw=true; break;
     case KEY_N:     normalize(tile,flip); flip=0; redraw=true; break;
     case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  redraw=true; break;
     case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; redraw=true; break;
     case KEY_SPACE: if(bgc==dvc(9)) bgc=vc(7); else bgc=dvc(9);
                     redraw=true; break;
     case KEY_U:     for(int i=0; i<256; i++)
                       swap(undotile[i],tilebuf[(tile<<8)+i]);
                     redraw=true;
                     break;
     }
   }

   if(mouse_b==1 && !bdown) {
     if(isinRect(mouse_x,mouse_y,80,32,206,158)) {
       drawing=1;
       for(int i=0; i<256; i++)
         undotile[i]=tilebuf[(tile<<8)+i];
       }
     if(isinRect(mouse_x,mouse_y,224,168,283,183))
       if(do_button(224,168,60,16,"OK",dvc(9),vc(14)))
         done=2;
     if(isinRect(mouse_x,mouse_y,224,192,283,207))
       if(do_button(224,192,60,16,"Cancel",dvc(9),vc(14)))
         done=1;
     if(isinRect(mouse_x,mouse_y,24,184,83,199))
       if(do_button(24,184,60,16,"Edit",dvc(9),vc(14))) {
         do_menu(colors_menu,24,144);
         text_mode(-1);
         redraw=true;
         }
     if(isinRect(mouse_x,mouse_y,104,176,135,207)) {
       int x=(mouse_x-104)>>3;
       int y=(mouse_y-176)>>3;
       c1 = (y<<2)+x;
       redraw=true;
       }
     if(isinRect(mouse_x,mouse_y,48,40,63,55)) {
       tool=t_pen;
       redraw=true;
       }
     if(isinRect(mouse_x,mouse_y,48,64,63,79) && tile>=MIN16) {
       tool=t_brush;
       redraw=true;
       }
     if(isinRect(mouse_x,mouse_y,48,88,63,103) && tile>=MIN16) {
       tool=t_recolor;
       redraw=true;
       }
     if(isinRect(mouse_x,mouse_y,48,112,63,127) && tile>=MIN16) {
       tool=t_select;
       redraw=true;
       }
     bdown=true;
     }

   if(mouse_b&2 && !bdown) {
     if(isinRect(mouse_x,mouse_y,80,32,206,158)) {
       drawing=2;
       for(int i=0; i<256; i++)
         undotile[i]=tilebuf[(tile<<8)+i];
       }
     if(isinRect(mouse_x,mouse_y,104,176,135,207)) {
       int x=(mouse_x-104)>>3;
       int y=(mouse_y-176)>>3;
       c2 = (y<<2)+x;
       redraw=true;
       }
     bdown=true;
     }

   if(drawing && isinRect(mouse_x,mouse_y,80,32,206,158)) {
     int x=(mouse_x-80)>>3;
     int y=(mouse_y-32)>>3;

     switch(tool) {
     case t_pen:
       if(tile>=MIN16) {
         if(flip&1) x=15-x;
         if(flip&2) y=15-y;
         tilebuf[((y<<4)+x)+(tile<<8)]=(drawing==1)?c1:c2;
         }
       else {
         int t=0;
         if(x&8) t|=2;
         if(y&8) t|=1;
         x&=7;
         y&=7;
         if(flip&1) { x=7-x; t^=2; }
         if(flip&2) { y=7-y; t^=1; }
         t+=tile<<2;
         tilebuf[((y<<3)+x)+(t<<6)]=(drawing==1)?c1:c2;
         }
       break;
     case t_brush:
       tile_floodfill(tile,x,y,(drawing==1)?c1:c2);
       drawing=0;
       break;
     case t_recolor:
       tf_u = tilebuf[(tile<<8)+(y<<4)+x];
       for(int i=0; i<256; i++) {
         if(tilebuf[(tile<<8)+i]==tf_u)
           tilebuf[(tile<<8)+i]=(drawing==1)?c1:c2;
         }
       drawing=0;
       break;
     }
     redraw=true;
   }

   if(mouse_b==0) {
     bdown=false;
     drawing=0;
     }

   if(redraw)
     draw_edit_scr(tile,flip,cs,oldtile);
   else {
     vsync();
     scare_mouse();
     update_tool_cursor();
     unscare_mouse();
     SCRFIX();
     }

 } while(!done);
 while(mouse_b);

 if(done==1) {
   for(int i=0; i<256; i++)
     tilebuf[(tile<<8)+i]=oldtile[i];
   }
 else {
   usetiles=true;
   saved=false;
   }
 set_mouse_sprite(mouse_bmp[0]);
}



/*  Grab Tile Code  */

void *imagebuf=NULL;
long imagesize=0;
int  imagetype=0;
int imagex,imagey,selx,sely;
int bp=4,grabmode=16;
PALETTE imagepal;


void puttileROM(BITMAP *dest,int x,int y,byte *src,int cs)
{
  byte buf[64];

  for(int l=0; l<8; l++)
  {
    int  bx=l<<3;
    byte b=src[(bp&1)?l:l<<1];

    for(int i=7; i>=0; i--)
    {
      buf[bx+i] = (b&1)+(cs<<4);
      b>>=1;
    }
  }

  src++;

  for(int p=1; p<bp; p++)
  {
    for(int l=0; l<8; l++)
    {
      int  bx=l<<3;
      byte b=src[(bp&1)?l:l<<1];

      for(int i=7; i>=0; i--)
      {
        buf[bx+i] |= (b&1)<<p;
        b>>=1;
      }
    }
    if(p&1)
      src+=15;
    else
      src++;
  }

  int c=0;
  for(int j=0; j<8; j++)
    for(int i=0; i<8; i++)
      putpixel(dest,x+i,y+j,buf[c++]);
}




void draw_grab_scr(int tile,int cs,byte *newtile,int bg,int white,int txt)
{
 clear_to_color(screen2,bg);
 text_mode(-1);

 switch(imagetype)
 {
 case ftBMP:
   blit((BITMAP*)imagebuf,screen2,imagex<<4,imagey<<4,0,0,320,160);
   draw_button(screen2,140,192,56,16,"Pal",dvc(9),vc(14));
   break;

 case ftTIL: {
   byte *hold = tilebuf;
   tilebuf = (byte*)imagebuf;
   int t=imagey*20;
   for(int i=0; i<200; i++) // 10 rows, down to y=160
   {
     if(t<<8 <= imagesize-256)
       puttile16(screen2,t,(i%20)<<4,(i/20)<<4,cs,0);
     t++;
   }
   tilebuf = hold;
   }
   break;

 case ftBIN: {
   int ofs = (19*imagex + imagey) * 128*bp;
   byte *buf = (byte*)imagebuf;
   for(int y=0; y<152; y+=8)
     for(int x=0; x<128; x+=8)
     {
       if(ofs>imagesize-8*bp)
         goto down;
       puttileROM(screen2,x,y,buf+ofs,cs);
       ofs+=8*bp;
     }
   for(int y=0; y<152; y+=8)
     for(int x=0; x<128; x+=8)
     {
       if(ofs>imagesize-8*bp)
         goto down;
       puttileROM(screen2,x+128,y,buf+ofs,cs);
       ofs+=8*bp;
     }
   }
   down:
   break;
 }

 swap(newtile,tilebuf);
 temptile=true;
 puttile16(screen2,0,208,168,cs,0);
 overtile16(screen2,0,232,168,cs,0);
 temptile=false;
 swap(newtile,tilebuf);

 puttile16(screen2,tile,208,192,cs,0);
 overtile16(screen2,tile,232,192,cs,0);

 rect(screen2,207,167,224,184,dvc(14));
 rect(screen2,231,167,248,184,dvc(14));
 rect(screen2,207,191,224,208,dvc(14));
 rect(screen2,231,191,248,208,dvc(14));

 switch(imagetype)
 {
 case 0:     textprintf(screen2,font,8,216,txt,"%s",imgstr[imagetype]); break;
 case ftBMP: textprintf(screen2,font,8,216,txt,"%s  %dx%d",imgstr[imagetype],((BITMAP*)imagebuf)->w,((BITMAP*)imagebuf)->h); break;
 case ftTIL:
 case ftBIN: textprintf(screen2,font,8,216,txt,"%s  %ld KB",imgstr[imagetype],imagesize>>10); break;
 }

 textprintf(screen2,font,8,168,txt,"sel: %d %d",selx,sely);
 textprintf(screen2,font,8,176,txt,"ofs: %d %d",imagex,imagey);
 textprintf(screen2,font,8,192,txt,"cset: %d",cs);
 textprintf(screen2,font,8,200,txt,"step: %d",grabmode);

 textprintf(screen2,font,8,224,txt,"%s",imagepath);
 rectfill(screen2,256,224,319,231,bg);
 draw_button(screen2,260,168,56,16,"OK",dvc(9),vc(14));
 draw_button(screen2,260,192,56,16,"Cancel",dvc(9),vc(14));
 draw_button(screen2,260,216,56,16,"File",dvc(9),vc(14));

 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 rect(screen,selx,sely,selx+15,sely+15,white);
 unscare_mouse();
 SCRFIX();
}


void load_imagebuf()
{
  if(imagebuf)
  {
    if(imagetype==ftBMP)
      destroy_bitmap((BITMAP*)imagebuf);
    else
      free(imagebuf);
    imagebuf=NULL;
  }

  imagex=imagey=selx=sely=0;
  imagetype=filetype(imagepath);

  switch(imagetype)
  {
  case ftBMP:
    imagebuf = load_bitmap(imagepath,imagepal);
    if(!imagebuf)
      imagetype=0;
    break;

  case ftTIL:
  case ftBIN:
    imagesize = file_size(imagepath);
    if(imagesize)
    {
      imagebuf = malloc(imagesize);
      if(readfile(imagepath,imagebuf,imagesize)!=imagesize)
      {
        free(imagebuf);
        imagesize=0;
        imagetype=0;
      }
    }
    break;
  }
}


void grab(byte *tbuf)
{
  switch(imagetype)
  {
  case ftTIL:
  case ftBIN:
    for(int y=0; y<16; y++)
      for(int x=0; x<16; x++)
      {
        *tbuf = getpixel(screen2,x+selx,y+sely)&15;
        tbuf++;
      }
    break;

  case ftBMP:
    for(int y=0; y<16; y++)
      for(int x=0; x<16; x++)
      {
        *tbuf = getpixel(screen2,x+selx,y+sely);
        tbuf++;
      }
    break;
  }
}


void grab_tile(int tile,int &cs)
{
 byte newtile[256];
 for(int i=0; i<256; i++)
   newtile[i]=0;

 bool bdown=false;
 int done=0;
 int pal=0;
 int f=0;
 int black=bgc,white=vc(15);

 if(imagebuf==NULL)
   load_imagebuf();

 draw_grab_scr(tile,cs,newtile,black,white,vc(11));
 while(mouse_b);

 do {
   bool redraw=false;

   if(keypressed())
   {
     redraw=true;
     switch(readkey()>>8)
     {
     case KEY_ESC:   done=1; break;
     case KEY_DOWN:  imagey++; break;
     case KEY_UP:    imagey--; break;
     case KEY_RIGHT: imagex++; break;
     case KEY_LEFT:  imagex--; break;
     case KEY_PGDN:  imagey+=4; break;
     case KEY_PGUP:  imagey-=4; break;
     case KEY_HOME:  imagex=imagey=0; break;
     case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  break;
     case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; break;
     case KEY_S: if(grabmode==1) grabmode=8; else if(grabmode==8) grabmode=16; else grabmode=1; break;
     default: redraw=false;
     }

     if(imagex<0) imagex=0;
     if(imagey<0) imagey=0;
   }

   if(mouse_b==1 && !bdown)
   {
     bdown=true;
     if(mouse_y<=144)
     {
       while(mouse_b)
       {
         selx=min((mouse_x/grabmode)*grabmode,304);
         sely=min((mouse_y/grabmode)*grabmode,144);
         grab(newtile);
         draw_grab_scr(tile,cs,newtile,black,white,pal?white:vc(11));
       }
     }
     else if(isinRect(mouse_x,mouse_y,260,168,311,183))
     {
       if(do_button(260,168,56,16,"OK",dvc(9),vc(14)))
         done=2;
     }
     else if(isinRect(mouse_x,mouse_y,260,192,311,207))
     {
       if(do_button(260,192,56,16,"Cancel",dvc(9),vc(14)))
         done=1;
     }
     else if(isinRect(mouse_x,mouse_y,260,216,311,231))
     {
       if(do_button(260,216,56,16,"File",dvc(9),vc(14)))
       {
         if(getname_nogo("Load Image/TIL/ROM File",NULL,imagepath,true))
         {
           set_palette(RAMpal);
           pal=0;
           white=vc(15);
           black=bgc;
           strcpy(imagepath,temppath);
           load_imagebuf();
         }
         redraw=true;
       }
     }
     else if(imagetype==ftBMP && isinRect(mouse_x,mouse_y,140,192,196,208))
     {
       if(do_button(140,192,56,16,"Pal",dvc(9),vc(14)))
       {
         pal^=1;
         if(pal)
           get_bw(imagepal,black,white);
         else
         {
           white=vc(15);
           black=bgc;
         }
         set_palette_range(pal?imagepal:RAMpal,0,255,false);
         redraw=true;
       }
     }
   }

   if(mouse_b==0)
     bdown=false;

   if(redraw)
     draw_grab_scr(tile,cs,newtile,black,white,pal?white:vc(11));
   else
     vsync();

   scare_mouse();
   blit(screen2,screen,selx,sely,selx,sely,16,16);
   if(f&8)
     rect(screen,selx,sely,selx+15,sely+15,white);
   unscare_mouse();
   SCRFIX();
   f++;

 } while(!done);

 if(done==2)
 {
   saved=false;
   usetiles=true;
   for(int i=0; i<256; i++)
     tilebuf[(tile<<8)+i] = newtile[i];
 }

 if(pal)
   set_palette(RAMpal);
}



void draw_tiles(int first,int cs)
{
 clear(screen2);
 for(int i=0; i<260; i++) // 13 rows, leaving 32 pixels from y=208 to y=239
   puttile16(screen2,first+i,(i%20)<<4,(i/20)<<4,cs,0);

}


void tile_info_0(int tile,int flip,int cs,int copy)
{
 rectfill(screen2,0,208,319,239,dvc(9));
 rect(screen2,0,208,319,239,vc(15));
 vline(screen2,292,209,238,vc(15));
 if(copy>=0)
   puttile16(screen2,copy,8,216,cs,flip);
 else
   rectfill(screen2,8,216,23,231,vc(1));
 puttile16(screen2,tile,32,216,cs,flip);
 text_mode(dvc(9));
 textprintf(screen2,font,56,216,vc(11),"tile: %-3d",tile);
 textprintf(screen2,font,56,224,vc(11),"flip: %d",flip);
 draw_button(screen2,140,216,44,16,"Edit",dvc(9),vc(14));
 draw_button(screen2,190,216,44,16,"Grab",dvc(9),tile<MIN16?vc(9):vc(14));
 draw_button(screen2,240,216,44,16,"Done",dvc(9),vc(14));
 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 unscare_mouse();
 SCRFIX();
}

void tile_info_1(int oldtile,int oldflip,int tile,int flip,int cs,int copy)
{
 rectfill(screen2,0,208,319,239,dvc(9));
 rect(screen2,0,208,319,239,vc(15));
 vline(screen2,292,209,238,vc(15));
 text_mode(dvc(9));
 textprintf(screen2,font,8,216,vc(11),"Old    tile: %-3d  New    tile: %-3d",oldtile,tile);
 textprintf(screen2,font,8,224,vc(11),"       flip: %d           flip: %d",oldflip,flip);
 if(copy>=0) {
   puttile16(screen2,copy,160,216,cs,flip);
   rectfill(screen2,152,216,159,223,dvc(9));
   }
 puttile16(screen2,oldtile,40,216,cs,oldflip);
 puttile16(screen2,tile,184,216,cs,flip);
 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 unscare_mouse();
 SCRFIX();
}


int tilesel_cs=6;

int select_tile(int &tile,int &flip,int type)
{
 bound(tile,0,599);
 int done=0;
 int cs = type==1 ? CSet : tilesel_cs;
 int oflip=flip;
 int otile=tile;
 int first=(tile/20-7)*20;
 int copy=-1;
 bound(first,0,340);

 go();

 draw_tiles(first,cs);
 if(type==0)
   tile_info_0(tile,flip,cs,copy);
 else
   tile_info_1(otile,oflip,tile,flip,cs,copy);

 while(mouse_b);
 bool bdown=false;
 int f=0;

 do {
   bool redraw=false;

   if(keypressed()) {
     switch(readkey()>>8) {
     case KEY_ENTER: done=2; break;
     case KEY_ESC:   done=1; break;
     case KEY_F1:    onHelp(); break;
     case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  redraw=true; break;
     case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; redraw=true; break;
     case KEY_UP:    if(first>0)  { first-=20; redraw=true; } break;
     case KEY_DOWN:  if(first<340) { first+=20; redraw=true; } break;
     case KEY_E:     edit_tile(tile,flip,cs); redraw=true; break;
     case KEY_G:     grab_tile(tile,cs); redraw=true; break;
     case KEY_C:     copy=tile; redraw=true; break;
     case KEY_N:     normalize(tile,flip); flip=0; redraw=true; saved=false; usetiles=true; break;
     case KEY_H:     flip^=1; redraw=true; break;
     case KEY_V:     if(copy==-1)
                     { flip^=2;
                       redraw=true;
                       break;
                     }
                     for(int i=0; i<256; i++)
                       tilebuf[(tile<<8)+i]=tilebuf[(copy<<8)+i];
                     copy=-1; redraw=true;
                     saved=false;
                     usetiles=true;
                     break;
     }
   }

   if(mouse_y==0 && first>0 && !(f&3))   { first-=20; redraw=true; }
   if(mouse_y>207 && mouse_x>291 && first<340 && !(f&3)) { first+=20; redraw=true; }

   if(mouse_b&1) {
     int x=mouse_x;
     int y=mouse_y;
     if(y<208)
       tile=(y>>4)*20+(x>>4)+first;

     if(type==1) {
       if(!bdown && isinRect(x,y,40,216,63,231))
         done=1;
       if(!bdown && isinRect(x,y,184,216,199,231))
         done=2;
       }
     else if(!bdown && isinRect(x,y,140,216,184,231)) {
       if(do_button(140,216,44,16,"Edit",dvc(9),vc(14))) {
         edit_tile(tile,flip,cs);
         redraw=true;
         }
       }
     else if(!bdown && isinRect(x,y,190,216,234,231) && tile>=MIN16) {
       if(do_button(190,216,44,16,"Grab",dvc(9),vc(14))) {
         grab_tile(tile,cs);
         redraw=true;
         }
       }
     else if(!bdown && isinRect(x,y,240,216,284,231))
       if(do_button(240,216,44,16,"Done",dvc(9),vc(14)))
         done=1;
     bdown=true;
     }

   if(mouse_b==0)
     bdown=false;

   if(redraw)
     draw_tiles(first,cs);
   if(type==0)
     tile_info_0(tile,flip,cs,copy);
   else tile_info_1(otile,oflip,tile,flip,cs,copy);

   if(f&8) {
     int x=(tile%20)<<4;
     int y=((tile-first)/20)<<4;
     if(y>=0 && y<208) {
       scare_mouse();
       rect(screen,x,y,x+15,y+15,vc(15));
       unscare_mouse();
       SCRFIX();
       }
     }
   f++;

 } while(!done);

 if(type==0)
   tilesel_cs=cs;
 while(mouse_b);
 comeback();
 return done-1;
}



int onTiles()
{
 int t=0,f=0;
 select_tile(t,f,0);
 refresh(rALL);
 return D_O_K;
}



void draw_combo(int x,int y,int c,int cs)
{
  if(c<512)
    putcombo(screen2,x,y,(cs<<9)+c,0);
  else
    rectfill(screen2,x,y,x+15,y+15,0);
}


void draw_combos(int first,int cs,bool cols)
{
 clear(screen2);
 if(cols==false) {
   for(int i=0; i<260; i++) // 13 rows, leaving 32 pixels from y=208 to y=239
     draw_combo((i%20)<<4,(i/20)<<4,i+first,cs);
   }
 else {
   int c = first/5;
   for(int i=0; i<260; i++) {
     draw_combo((i%20)<<4,(i/20)<<4,c,cs);
     c++;
     if((i&3)==3)
       c+=100;
     if((i%20)==19)
       c-=516;
     }
   }
 for(int x=64; x<320; x+=64)
   vline(screen2,x,0,207,vc(15));
}


void combo_info(int tile,int cs,int copy)
{
 rectfill(screen2,0,208,319,239,dvc(9));
 rect(screen2,0,208,319,239,vc(15));
 vline(screen2,288,209,238,vc(15));
 if(copy>=0)
   putcombo(screen2,8,216,(cs<<9)+copy,0);
 else
   rectfill(screen2,8,216,23,231,vc(1));
 putcombo(screen2,32,216,(cs<<9)+tile,0);
 text_mode(dvc(9));
 textprintf(screen2,font,56,216,vc(11),"combo: %-3d",tile);
 draw_button(screen2,140,216,60,16,"Edit",dvc(9),vc(14));
 draw_button(screen2,212,216,60,16,"Done",dvc(9),vc(14));
 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 unscare_mouse();
 SCRFIX();
}

word ctable[512],ctable2[512];

int combo_screen()
{
 int done=0;
 int cs = CSet;
 int first=0;
 int copy=-1;
 int tile=0;
 bool cols=false;
 bound(first,0,340);

 for(word i=0; i<512; i++)
   ctable[i]=i;

 go();
 draw_combos(first,cs,cols);
 combo_info(tile,cs,copy);
 while(mouse_b);
 bool bdown=false;
 int f=0;

 do {
   bool redraw=false;

   if(keypressed()) {
     switch(readkey()>>8) {
     case KEY_ENTER: done=2; break;
     case KEY_ESC:   done=1; break;
     case KEY_F1:    onHelp(); break;
     case KEY_SPACE: cols=!cols; redraw=true; break;
     case KEY_PLUS_PAD:  cs = (cs<5) ? cs+1:0;  redraw=true; break;
     case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:5; redraw=true; break;
     case KEY_UP:    if(first>0)  { first-=20; redraw=true; } break;
     case KEY_DOWN:  if(first<260) { first+=20; redraw=true; } break;
     case KEY_E:     edit_combo(tile,false); redraw=true; break;
     case KEY_C:     copy=tile; redraw=true; break;

     case KEY_V:     if(copy>=0) {
                       combo hold = combobuf[copy];
                       word  thold = ctable[copy];

                       if(tile==copy) { }
                       else if(tile<copy) {
                         for(int i=copy; i>tile; i--) {
                           combobuf[i] = combobuf[i-1];
                           ctable[i] = ctable[i-1];
                           }
                         combobuf[tile] = hold;
                         ctable[tile] = thold;
                         }
                       else {
                         for(int i=copy; i<tile; i++) {
                           combobuf[i] = combobuf[i+1];
                           ctable[i] = ctable[i+1];
                           }
                         combobuf[tile] = hold;
                         ctable[tile] = thold;
                         }

                       copy=-1;
                       redraw=true;
                       saved=false;
                       }
                     break;

     case KEY_S:     if(copy>=0) {
                       swap(combobuf[copy],combobuf[tile]);
                       swap(ctable[copy],ctable[tile]);
                       copy=-1;
                       redraw=true;
                       saved=false;
                       }
                     break;
     case KEY_O:     if(copy>=0) {
                       combobuf[tile]=combobuf[copy];
                       copy=-1;
                       redraw=true;
                       saved=false;
                       }
                     break;
     }
   }

   if(mouse_y==0 && first>0 && !(f&3))   { first-=20; redraw=true; }
   if(mouse_y>207 && mouse_x>287 && first<260 && !(f&3)) { first+=20; redraw=true; }

   if(mouse_b&1) {
     int x=mouse_x;
     int y=mouse_y;
     if(y<208) {
       if(!cols) {
         tile=(y>>4)*20+(x>>4)+first;
         }
       else {
         tile=((x>>6)*104)+((x>>4)&3)+((y>>4)<<2) + first/5;
         }
       bound(tile,0,511);
       }

     if(!bdown && isinRect(x,y,140,216,200,231)) {
       if(do_button(140,216,60,16,"Edit",dvc(9),vc(14))) {
         edit_combo(tile,false);
         redraw=true;
         }
       }
     else if(!bdown && isinRect(x,y,212,216,272,231))
       if(do_button(212,216,60,16,"Done",dvc(9),vc(14)))
         done=1;
     bdown=true;
     }

   if(mouse_b==0)
     bdown=false;

   if(redraw)
     draw_combos(first,cs,cols);
   combo_info(tile,cs,copy);

   if(f&8) {
     int x,y;
     if(!cols) {
       x=(tile%20)<<4;
       y=((tile-first)/20)<<4;
       }
     else {
       x=((tile&3) + ((tile/104)<<2) ) << 4;
       y=( ((tile%104)>>2) - first/20 ) << 4;
       }
     if(y>=0 && y<208) {
       scare_mouse();
       rect(screen,x,y,x+15,y+15,vc(15));
       unscare_mouse();
       SCRFIX();
       }
     }
   f++;

 } while(!done);

 for(int i=0; i<512; i++) {
   int pos=0;
   for(int j=0; j<512; j++)
     if(ctable[j]==i)
       pos=j;
   ctable2[i]=pos;
   }

 for(int s=0; s<=TEMPLATE; s++)
   for(int i=0; i<176; i++) {
     int oc = TheMaps[s].data[i]&0x01FF;
     TheMaps[s].data[i]&=0xFE00;
     TheMaps[s].data[i]|=ctable2[oc]&0x1FF;
     }

 while(mouse_b);
 comeback();
 return done-1;
}



int onCombos()
{
 combo_screen();
 refresh(rALL);
 return D_O_K;
}


int d_ctile_proc(int msg,DIALOG *d,int c)
{
 if(msg==MSG_CLICK) {
   int t=curr_combo.tile;
   int f=curr_combo.flip;
   if(select_tile(t,f,1)) {
     curr_combo.tile=t;
     curr_combo.flip=f;
     return D_REDRAW;
     }
   }
 return D_O_K;
}

int d_combo_loader(int msg,DIALOG *d,int c)
{
 if(msg==MSG_DRAW)
   putcombo(combo_bmp,curr_combo,0,0,0,CSet,0);
 return D_O_K;
}


char *combotypelist(int index, int *list_size)
{
 if(index>=0) {
   if(index>=MAXCOMBOTYPES)
    index=MAXCOMBOTYPES-1;
   return combotype_string[index];
   }
 *list_size=MAXCOMBOTYPES;
 return NULL;
}

static DIALOG combo_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 64,   48,   192,  152,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_combo_loader },
// 2
 { d_bitmap_proc,     96,   72,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     90,   176,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  176,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { d_button_proc,     170,  72,   60,   16,   vc(14),  vc(1),  'd',     D_EXIT,     0,             0,       "Delete" },
// 6
 { d_wflag_proc,      128,  72,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      128,  80,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      136,  72,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      136,  80,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
// 10
 { d_ctile_proc,      96,   72,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Edit Combo" },
 { d_text_proc,       72,   96,   48,   8,    vc(11),  vc(1),  0,       0,          0,             0,       "Type:" },
 { d_droplist_proc,   120,  94,   120,  11,   vc(14),  vc(1),  0,       0,          0,             0,       "Edit Combo" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onHelp },
 { NULL }
};



void edit_combo(int c,bool freshen)
{
 curr_combo = combobuf[c];

 combo_dlg[2].dp=combo_bmp;
 for(int i=0; i<4; i++)
   combo_dlg[i+6].flags = curr_combo.walk&(1<<i) ? D_SELECTED : 0;

 combo_dlg[13].d1 = curr_combo.type;
 combo_dlg[13].dp = combotypelist;

 int ret=popup_dialog(combo_dlg,-1);
 if(freshen)
   refresh(rALL);

 if(ret==5)
   if(alert("Delete combo?", NULL, NULL, "&Yes", "&No", 'y', 'n') == 1) {
     saved=false;
     curr_combo.tile =
     curr_combo.flip =
     curr_combo.walk =
     curr_combo.type =
     curr_combo.attr = 0;
     for(int i=0; i<5; i++)
       curr_combo.e[i] = 0;
     combobuf[c] = curr_combo;
     }

 if(ret==3) {
   saved=false;
   for(int i=0; i<4; i++) {
     if(combo_dlg[i+6].flags & D_SELECTED)
       curr_combo.walk |= 1<<i;
     else
       curr_combo.walk &= ~(1<<i);
     }
   curr_combo.type = combo_dlg[13].d1;
   combobuf[c] = curr_combo;
   }
 if(freshen)
   refresh(rALL);
}




