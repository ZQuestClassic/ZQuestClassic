//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_tiles.cc
//
//  Tile editor stuff, etc., for ZQuest.
//
//--------------------------------------------------------


/*********************************/
/*****    Tiles & Combos    ******/
/*********************************/

void go_tiles()
{
  int *si = (int*)tilebuf;
  int *di = (int*)undotilebuf;
  for(int i=0; i<NEWTILE_SIZE2/4; i++)
    *(di++) = *(si++);
}

void comeback_tiles()
{
  int *si = (int*)undotilebuf;
  int *di = (int*)tilebuf;
  for(int i=0; i<NEWTILE_SIZE2/4; i++)
    *(di++) = *(si++);
}

void go_combos()
{
  newcombo *si = combobuf;
  newcombo *di = undocombobuf;
  for(int i=0; i<MAXCOMBOS; i++)
    *(di++) = *(si++);
}

void comeback_combos()
{
  newcombo *si = undocombobuf;
  newcombo *di = combobuf;
  for(int i=0; i<MAXCOMBOS; i++)
    *(di++) = *(si++);
}





void little_x(BITMAP *dest, int x, int y, int c, int s)
{
  line(dest,x,y,x+s,y+s,c);
  line(dest,x+s,y,x,y+s,c);
}


void zoomtile16(BITMAP *dest,int tile,int x,int y,int cset,int flip,int m)
{
  cset <<= 4;

  unpack_tile(tile);
  byte *si = unpackbuf;

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

//    circle(BITMAP *bmp, int x, int y, int radius, int color);
//    circlefill(BITMAP *bmp, int x, int y, int radius, int color);

void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg, int value) {
  for (int k=0; k<7; k++) {
    if ((k==0)||(Map.CurrScr()->layermap[k-1])) {
      circle(dest, x+(k*25)+4, y+4, 4, fg);
      circlefill(dest, x+(k*25)+4, y+4, 2, (value==k)?fg:bg);
    }
  }
}

void do_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int &value) {
  while(mouse_b) {
    vsync();
    for (int k=0; k<7; k++) {
      if ((k==0)||(Map.CurrScr()->layermap[k-1])) {
        if(isinRect(mouse_x,mouse_y,x+(k*25),y,x+(k*25)+8,y+8)) { //if on radio button
          value=k;
          scare_mouse();
          draw_layerradio(dest,x,y,bg,fg,value);
          refresh(rMENU);
          unscare_mouse();
        }
      }
    }
  }
}

void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg, bool value)
{
 rect(dest,x, y, x+8, y+8, fg);
 line(dest,x+1,y+1,x+7,y+7,value?fg:bg);
 line(dest,x+1,y+7,x+7,y+1,value?fg:bg);
/*
 if (gotfocus) {
   //top
   putpixel(dest,x+1,y+1,fg);
   putpixel(dest,x+3,y+1,fg);
   putpixel(dest,x+5,y+1,fg);
   putpixel(dest,x+7,y+1,fg);

   //bottom
   putpixel(dest,x+1,y+7,fg);
   putpixel(dest,x+3,y+7,fg);
   putpixel(dest,x+5,y+7,fg);
   putpixel(dest,x+7,y+7,fg);

   //left
   putpixel(dest,x+1,y+3,fg);
   putpixel(dest,x+1,y+5,fg);

   //right
   putpixel(dest,x+7,y+3,fg);
   putpixel(dest,x+7,y+5,fg);
 }
*/
}


bool do_checkbox(BITMAP *dest,int x,int y,int bg,int fg,int &value)
{
 bool over=false;
 while(mouse_b) {
  vsync();
  if(isinRect(mouse_x,mouse_y,x,y,x+8,y+8)) { //if on checkbox
    if(!over) { //if wasn't here before
      scare_mouse();
      value=!value;
      draw_checkbox(dest,x,y,bg,fg,value);
      refresh(rMENU);
      unscare_mouse();
      over=true;
      }
    }
  else { //if not on checkbox
    if(over) { //if was here before
      scare_mouse();
      value=!value;
      draw_checkbox(dest,x,y,bg,fg,value);
      refresh(rMENU);
      unscare_mouse();
      over=false;
      }
    }
  }
 return over;
}


//*************** tile flood fill stuff **************

byte tf_c;
byte tf_u;

void tile_floodfill_rec(int x,int y)
{
  while(x>0 && (unpackbuf[(y<<4)+x-1] == tf_u))
    x--;

  while(x<=15 && (unpackbuf[(y<<4)+x] == tf_u))
  {
    unpackbuf[(y<<4)+x] = tf_c;

    if(y>0 && (unpackbuf[((y-1)<<4)+x] == tf_u))
      tile_floodfill_rec(x,y-1);

    if(y<15 && (unpackbuf[((y+1)<<4)+x] == tf_u))
      tile_floodfill_rec(x,y+1);

    x++;
  }
}

void tile_floodfill(int tile,int x,int y,byte c)
{
  unpack_tile(tile);
  tf_c = c;
  tf_u = unpackbuf[(y<<4)+x];
  if(tf_u != tf_c)
    tile_floodfill_rec(x,y);
  pack_tile(unpackbuf,tile);
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
 puttile16(screen2,0,224,48,cs,flip);
 overtile16(screen2,0,248,48,cs,flip);
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

 text_mode(bgc);
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


void normalize(int tile,int tile2, int flip)
{

  if (tile>tile2) {
    swap(tile, tile2);
  }
  for (int temptile=tile; temptile<=tile2; temptile++) {
    unpack_tile(temptile);

    if(flip&1)
    {
      for(int y=0; y<16; y++)
        for(int x=0; x<8; x++)
          swap(unpackbuf[(y<<4)+x],unpackbuf[(y<<4)+15-x]);
    }
    if(flip&2)
    {
      for(int y=0; y<8; y++)
        for(int x=0; x<16; x++)
          swap(unpackbuf[(y<<4)+x],unpackbuf[((15-y)<<4)+x]);
    }

    pack_tile(unpackbuf,temptile);
  }
}

void rotate_tile(int tile, bool backward)
{
  unpack_tile(tile);
  byte tempunpackbuf[256];
  byte tempx, tempy;
  for (tempx=0; tempx<16; tempx++) {
    for (tempy=0; tempy<16; tempy++) {
      if (!backward) {
        tempunpackbuf[(tempy<<4)+tempx]=unpackbuf[((15-tempx)<<4)+tempy];
      } else {
        tempunpackbuf[((15-tempx)<<4)+tempy]=unpackbuf[(tempy<<4)+tempx];
      }
    }
  }
  pack_tile(tempunpackbuf,tile);
}

void wrap_tile(int tile, int offset)
{
  byte buf[256];

  unpack_tile(tile);
  if(abs(offset) == 16)
  {
    for(int i=0; i<256; i++)
      buf[(i+offset)&0xFF] = unpackbuf[i];
  }
  else
  {
    for(int i=0; i<256; i++)
      buf[((i+offset)&15)+(i&0xF0)] = unpackbuf[i];
  }
  pack_tile(buf,tile);
}


void edit_tile(int tile,int flip,int &cs)
{
 byte oldtile[128],undotile[128];
 for(int i=0; i<128; i++)
   oldtile[i]=undotile[i]=tilebuf[(tile<<7)+i];

 bool bdown=false;
 int done=0;
 drawing=0;
 tool_cur = -1;

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
     case KEY_R:     go_tiles(); rotate_tile(tile,(key[KEY_LSHIFT] || key[KEY_RSHIFT])); redraw=true; saved=false; usetiles=true; break;
     case KEY_N:     normalize(tile,tile,flip); flip=0; redraw=true; break;
     case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  redraw=true; break;
     case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; redraw=true; break;
     case KEY_SPACE: if(bgc==dvc(9)) bgc=vc(7); else bgc=dvc(9);
                     redraw=true; break;
     case KEY_U:     for(int i=0; i<128; i++)
                       swap(undotile[i],tilebuf[(tile<<7)+i]);
                     redraw=true;
                     break;
     case KEY_UP:
            for(int i=0; i<128; i++)
              undotile[i]=tilebuf[(tile<<7)+i];
            wrap_tile(tile, -16);
            redraw=true;
            break;
     case KEY_DOWN:
            for(int i=0; i<128; i++)
              undotile[i]=tilebuf[(tile<<7)+i];
            wrap_tile(tile, 16);
            redraw=true;
            break;
     case KEY_LEFT:
            for(int i=0; i<128; i++)
              undotile[i]=tilebuf[(tile<<7)+i];
            wrap_tile(tile, -1);
            redraw=true;
            break;
     case KEY_RIGHT:
            for(int i=0; i<128; i++)
              undotile[i]=tilebuf[(tile<<7)+i];
            wrap_tile(tile, 1);
            redraw=true;
            break;
     }
   }

   if(mouse_b==1 && !bdown)
   {
     if(isinRect(mouse_x,mouse_y,80,32,206,158))
     {
       drawing=1;
       for(int i=0; i<128; i++)
         undotile[i]=tilebuf[(tile<<7)+i];
     }

     if(isinRect(mouse_x,mouse_y,224,168,283,183))
       if(do_button(224,168,60,16,"OK",dvc(9),vc(14)))
         done=2;

     if(isinRect(mouse_x,mouse_y,224,192,283,207))
       if(do_button(224,192,60,16,"Cancel",dvc(9),vc(14)))
         done=1;

     if(isinRect(mouse_x,mouse_y,24,184,83,199))
       if(do_button(24,184,60,16,"Edit",dvc(9),vc(14)))
       {
         popup_menu(colors_menu,24,144);
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
     if(isinRect(mouse_x,mouse_y,48,64,63,79)) {
       tool=t_brush;
       redraw=true;
       }
     if(isinRect(mouse_x,mouse_y,48,88,63,103)) {
       tool=t_recolor;
       redraw=true;
       }
     if(isinRect(mouse_x,mouse_y,48,112,63,127)) {
       tool=t_select;
       redraw=true;
       }
     bdown=true;
     }

   if(mouse_b&2 && !bdown) {
     if(isinRect(mouse_x,mouse_y,80,32,206,158)) {
       drawing=2;
       for(int i=0; i<128; i++)
         undotile[i]=tilebuf[(tile<<7)+i];
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

     switch(tool)
     {
     case t_pen:
       if(flip&1) x=15-x;
       if(flip&2) y=15-y;
       unpack_tile(tile);
       unpackbuf[((y<<4)+x)]=(drawing==1)?c1:c2;
       pack_tile(unpackbuf,tile);
       break;

     case t_brush:
       tile_floodfill(tile,x,y,(drawing==1)?c1:c2);
       drawing=0;
       break;

     case t_recolor:
       unpack_tile(tile);
       tf_u = unpackbuf[(y<<4)+x];

       for(int i=0; i<256; i++)
         if(unpackbuf[i]==tf_u)
           unpackbuf[i]=(drawing==1)?c1:c2;

       pack_tile(unpackbuf,tile);
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
   for(int i=0; i<128; i++)
     tilebuf[(tile<<7)+i]=oldtile[i];
   }
 else
 {
   byte buf[128];
   // put back old tile
   for(int i=0; i<128; i++)
   {
     buf[i] = tilebuf[(tile<<7)+i];
     tilebuf[(tile<<7)+i] = oldtile[i];
   }
   // go
   go_tiles();
   // replace old tile with new one again
   for(int i=0; i<128; i++)
     tilebuf[(tile<<7)+i] = buf[i];

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
int bp=4,grabmode=16,romofs=0;
int grabmask=0;
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




void draw_grab_scr(int tile,int cs,byte *newtile,int bg,int white,int txt, int width, int height)
{
 clear_to_color(screen2,bg);
 text_mode(-1);

 switch(imagetype)
 {
 case ftBMP:
   blit((BITMAP*)imagebuf,screen2,imagex<<4,imagey<<4,0,0,320,160);
   draw_button(screen2,140,192,56,16,"Pal",dvc(9),vc(14));
   break;

 case ftZGP:
 case ftQSU:
 case ftTIL: {
   byte *hold = tilebuf;
   tilebuf = (byte*)imagebuf;
   int t=imagey*20;
   for(int i=0; i<200; i++) // 10 rows, down to y=160
   {
     if(t<<7 <= imagesize-128)
       puttile16(screen2,t,(i%20)<<4,(i/20)<<4,cs,0);
     t++;
   }
   tilebuf = hold;
   }
   break;

 case ftBIN: {
   int ofs = (19*imagex + imagey) * 128*bp + romofs;
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
 puttile16(screen2,0,208,168,cs,0);
 overtile16(screen2,0,232,168,cs,0);
 swap(newtile,tilebuf);

 puttile16(screen2,tile,208,192,cs,0);
 overtile16(screen2,tile,232,192,cs,0);

 rectfill(screen2,184,168,191,175,grabmask&1?vc(12):vc(7));
 rectfill(screen2,192,168,199,175,grabmask&2?vc(12):vc(7));
 rectfill(screen2,184,176,191,183,grabmask&4?vc(12):vc(7));
 rectfill(screen2,192,176,199,183,grabmask&8?vc(12):vc(7));

 rect(screen2,183,167,200,184,dvc(14));
 rect(screen2,207,167,224,184,dvc(14));
 rect(screen2,231,167,248,184,dvc(14));
 rect(screen2,207,191,224,208,dvc(14));
 rect(screen2,231,191,248,208,dvc(14));

 switch(imagetype)
 {
 case 0:     textprintf(screen2,font,8,216,txt,"%s",imgstr[imagetype]); break;
 case ftBMP: textprintf(screen2,font,8,216,txt,"%s  %dx%d",imgstr[imagetype],((BITMAP*)imagebuf)->w,((BITMAP*)imagebuf)->h); break;
 case ftZGP:
 case ftQSU:
 case ftTIL:
 case ftBIN: textprintf(screen2,font,8,216,txt,"%s  %ld KB",imgstr[imagetype],imagesize>>10); break;
 }

 textprintf(screen2,font,8,168,txt,"sel: %d %d",selx,sely);
 textprintf(screen2,font,8,176,txt,"pos: %d %d",imagex,imagey);
 textprintf(screen2,font,8,192,txt,"cset: %d",cs);
 textprintf(screen2,font,8,200,txt,"step: %d",grabmode);
 if(imagetype==ftBIN)
 {
   textprintf(screen2,font,104,192,txt,"bp:  %d",bp);
   textprintf(screen2,font,104,200,txt,"ofs: %Xh",romofs);
 }

 textprintf(screen2,font,8,224,txt,"%s",imagepath);
 rectfill(screen2,256,224,319,231,bg);
 draw_button(screen2,260,168,56,16,"OK",dvc(9),vc(14));
 draw_button(screen2,260,192,56,16,"Cancel",dvc(9),vc(14));
 draw_button(screen2,260,216,56,16,"File",dvc(9),vc(14));
 draw_button(screen2,120,168,56,16,"Leech",dvc(9),vc(14));

 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 rect(screen,selx,sely,selx+((width-1)*16)+15,sely+((height-1)*16)+15,white);
 unscare_mouse();
 SCRFIX();
}


void load_imagebuf()
{
  FILE *f;
  if(imagebuf)
  {
    if(imagetype==ftBMP)
      destroy_bitmap((BITMAP*)imagebuf);
    else
      free(imagebuf);
    imagebuf=NULL;
  }

  imagex=imagey=selx=sely=romofs=0;
  bp=4;
  imagetype=filetype(imagepath);

      short version;
      byte build;
      word combos_used;
      word tiles_used;

  switch(imagetype)
  {
  case ftBMP:
    imagebuf = load_bitmap(imagepath,imagepal);
    if(!imagebuf)
      imagetype=0;
    break;

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

  case ftTIL:
      imagesize = file_size(imagepath);
      f = fopen(imagepath,"rb");
      if(fread(&version,1,sizeof(short),f) != sizeof(short))
        goto error;
      if(fread(&build,1,sizeof(byte),f) != sizeof(byte))
        goto error;
      if(fread(&tiles_used,1,sizeof(word),f) != sizeof(word))
        goto error;
      imagebuf = malloc(dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)));
      if(fread(imagebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)!=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used))) {
        free(imagebuf);
        imagesize=0;
        imagetype=0;
      }
      error:
      fclose(f);
    
    if(imagetype==ftTIL && imagesize==TILEBUF_SIZE)
      pack_tiles((byte*)imagebuf);
    imagesize=tiles_used*128;
    break;

  case ftZGP:
    imagesize = file_size(imagepath);

    //open the file
    f=fopen(imagepath,"rb");
    if(!f)
      goto error2;

    //read the version and make sure it worked
    if(fread(&version,1,sizeof(short),f) != sizeof(short))
      goto error2;

    //read the build and make sure it worked
    if(fread(&build,1,sizeof(byte),f) != sizeof(byte))
      goto error2;

    //read the tile info and make sure it worked
    if(fread(&tiles_used,1,sizeof(word),f) != sizeof(word))
      goto error2;
    imagebuf = malloc(dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)));
    if(fread(imagebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)!=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used))) {
      free(imagebuf);
      imagesize=0;
      imagetype=0;
    }
    imagesize=tiles_used*128;


    //yay!  it worked!  close the file and say everything was ok.
    error2:
      fclose(f);

  break;

  case ftQSU:
    imagesize = file_size(imagepath);

    f = fopen(imagepath,"rb");

    zquestheader tmpheader;
    if(!f)
      goto error3;
    if(fread(&tmpheader,1,sizeof(zquestheader),f)!=sizeof(zquestheader)) {
      goto error3;
      }
/*
    if(fread(&dummy,1,sizeof(MsgStr)*MAXMSGS,f)!=sizeof(MsgStr)*MAXMSGS) {
      goto error3;
      }
    if(fread(&dummy,1,sizeof(dmap)*MAXDMAPS,f)!=sizeof(dmap)*MAXDMAPS) {
      goto error3;
      }
    if(fread(&dummy,1,sizeof(miscQdata),f)!=sizeof(miscQdata)) {
      goto error3;
      }
    if(fread(&dummy,1,sizeof(itemdata)*MAXITEMS,f)!=sizeof(itemdata)*MAXITEMS) {
      goto error3;
      }
    if(fread(&dummy,1,sizeof(wpndata)*MAXWPNS,f)!=sizeof(wpndata)*MAXWPNS) {
      goto error3;
      }

    for(int i=0; i<header.map_count; i++) {
      if(fread(&dummy,1,sizeof(mapscr)*MAPSCRS,f)!=sizeof(mapscr)*MAPSCRS) {
        goto error3;
        }
      }


    if(fread(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
      goto error3;
      }
    if(fread(&dummy,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used) {
      goto error3;
      }
    if(fread(&dummy,1,psTOTAL,f)!=psTOTAL) {
      goto error3;
      }
*/


    fseek(f, sizeof(MsgStr)*MAXMSGS, SEEK_CUR);
    fseek(f, sizeof(dmap)*MAXDMAPS, SEEK_CUR);
    fseek(f, sizeof(miscQdata), SEEK_CUR);
    fseek(f, sizeof(itemdata)*MAXITEMS, SEEK_CUR);
    fseek(f, sizeof(wpndata)*MAXWPNS, SEEK_CUR);
    fseek(f, sizeof(mapscr)*MAPSCRS*tmpheader.map_count, SEEK_CUR);
    if(fread(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
      goto error3;
      }
    fseek(f, sizeof(newcombo)*combos_used, SEEK_CUR);
    fseek(f, newpsTOTAL, SEEK_CUR);

    if(tmpheader.data_flags[0]) {
      if(fread(&tiles_used,1,sizeof(word),f) != sizeof(word))
        goto error3;
      imagebuf = malloc(dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)));
      if(fread(imagebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)!=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used))) {
        free(imagebuf);
        imagesize=0;
        imagetype=0;
      }
      imagesize=tiles_used*128;
    }

    error3:
      fclose(f);
      return;

    break;
  }
}

static DIALOG leech_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 8,    20,   303,  216-72,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  27,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Leech Options" },
// 2
 { d_button_proc,     200,  210-72,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_button_proc,     60,   210-72,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 5
 { d_text_proc,       20,   40,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Update Status Every:  " },
 { d_edit_proc,      186,   40,   48,   8,    vc(12),  vc(1),  0,       0,          5,             0,       NULL },
//7
 { d_text_proc,       20,   60,   80,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Duplicates" },
 { d_text_proc,       20,   70,  168,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Check only new tiles:" },
 { d_check_proc,     188,   70,   16,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) " " },
 { d_text_proc,       20,   80,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " Normal: " },
 { d_text_proc,       20,   90,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " H-Flip: " },
 { d_text_proc,       20,  100,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) " V-Flip: " },
 { d_text_proc,       20,  110,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "HV-Flip: " },
//14
 { d_radio_proc,      92,   80,   64,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Ignore" },
 { d_radio_proc,      164,  80,   56,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Check" },
 { d_radio_proc,      228,  80,   72,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Discard" },

 { d_radio_proc,      92,   90,   64,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Ignore" },
 { d_radio_proc,      164,  90,   56,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Check" },
 { d_radio_proc,      228,  90,   72,   8,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Discard" },

 { d_radio_proc,      92,  100,   64,   8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Ignore" },
 { d_radio_proc,      164, 100,   56,   8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Check" },
 { d_radio_proc,      228, 100,   72,   8,    vc(14),  vc(1),  0,       0,          2,             0,       (void *) "Discard" },

 { d_radio_proc,      92,  110,   64,   8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Ignore" },
 { d_radio_proc,      164, 110,   56,   8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Check" },
 { d_radio_proc,      228, 110,   72,   8,    vc(14),  vc(1),  0,       0,          3,             0,       (void *) "Discard" },
 { NULL }
};

void leech_tiles(byte *dest,int start)
{
  int currtile=start;
  int height, width;
  byte testtile[128];
  char updatestring[6];
  bool canadd;
  bool temp_canadd;
  bool duplicate;
  int total_duplicates_found=0, total_duplicates_discarded=0;
  int duplicates_found[4]={0,0,0,0};//, duplicates_discarded[4]={0,0,0,0};
  BITMAP *status;
  status = create_bitmap(240,120);
  clear(status);
  sprintf(updatestring, "%d", LeechUpdate);
  leech_dlg[6].dp=updatestring;

  leech_dlg[9].flags=(OnlyCheckNewTilesForDuplicates!=0) ? D_SELECTED : 0;

  for (int i=0; i<12; i++) {
    leech_dlg[i+14].flags=0;
  }
  for (int i=0; i<4; i++) {
    leech_dlg[(DuplicateAction[i])+14+(i*3)].flags=D_SELECTED;
  }

  int ret = popup_dialog(leech_dlg,3);
  if (ret==2) {
    return;
  }
  LeechUpdate=atoi(updatestring);
  for (int j=0; j<4; j++) {
    for (int i=0; i<3; i++) {
      if (leech_dlg[i+14+(j*3)].flags&D_SELECTED) {
        DuplicateAction[j]=i;
      }
    }
  }

  OnlyCheckNewTilesForDuplicates=leech_dlg[9].flags&D_SELECTED?1:0;

  switch(imagetype) {
    case ftBIN: break;
    case ftZGP:
    case ftQSU:
    case ftTIL:
      width=imagesize/128;
      height=1;
      break;
    case ftBMP:
      width=((((BITMAP*)imagebuf)->w)+15)/16;
      height=((((BITMAP*)imagebuf)->h)+15)/16;
      break;
  }

  if (currtile+(width*height)>NEWMAXTILES) {
    if(alert("Too many tiles.",
             "Truncation may occur.",NULL,"&Okay","&Cancel",'o','c')==2)
    {
      return;
    }
  }

  go_tiles();
  saved=false;
  usetiles=true;
  for (int ty=0; ty<height; ty++) { //for every row
    for (int tx=0; tx<width; tx++) { //for every column (tile)
      if ((((ty*width)+tx)%LeechUpdate)==0) { //update status
        rectfill(status,0,0,239,119,dvc(9));
        rect(status,0,0,239,119,vc(15));
        textprintf_centre(status,font,120,2,vc(14),"Status");
        textprintf_centre(status,font,120,14,vc(7),"Checking %d of %d",((ty*width)+tx), (width*height));
        textprintf_centre(status,font,120,24,vc(7),"%d tiles imported",currtile-start);
        textprintf_centre(status,font,120,36,vc(12),"Duplicates");
        textprintf_centre(status,font,120,46,vc(7),"%d/%d found/discarded",total_duplicates_found, total_duplicates_discarded);
        textprintf_centre(status,font,120,66,vc(7),"%d normal %s",duplicates_found[0],((DuplicateAction[0]<2)?"found":"discarded"));
        textprintf_centre(status,font,120,76,vc(7),"%d flipped (h) %s",duplicates_found[1],((DuplicateAction[1]<2)?"found":"discarded"));
        textprintf_centre(status,font,120,86,vc(7),"%d flipped (v) %s",duplicates_found[2],((DuplicateAction[2]<2)?"found":"discarded"));
        textprintf_centre(status,font,120,96,vc(7),"%d flipped (hv) %s",duplicates_found[3],((DuplicateAction[3]<2)?"found":"discarded"));
        textprintf_centre(status,font,120,108,vc(7),"Press any key to stop.");
        scare_mouse();
        blit(status,screen,0, 0, 40, 60-36, 240, 120);
        unscare_mouse();
        SCRFIX();
      }
      canadd=true;
      if (currtile>=NEWMAXTILES) { //if we've maxed out on our tiles...
        return;
      }
      switch(imagetype) {
        case ftBIN: break;
        case ftZGP:
        case ftQSU:
        case ftTIL:
          for(int y=0; y<16; y++) { //snag a tile
            for(int x=0; x<8; x++) {
              testtile[(y*8)+x]=((byte*)imagebuf)[(tx*128)+(y*8)+x];
            }
          }
        case ftBMP:
          for(int y=0; y<16; y++) { //snag a tile
            for(int x=0; x<16; x+=2) {
              testtile[(y*8)+(x/2)]=
                (getpixel(((BITMAP*)imagebuf),(tx*16)+x,(ty*16)+y)&15)+
                 ((getpixel(((BITMAP*)imagebuf),(tx*16)+x+1,(ty*16)+y)&15)<<4);
            }
          }
          break;
      }

      if (DuplicateAction[0]+DuplicateAction[1]
         +DuplicateAction[2]+DuplicateAction[3]>0) {
        temp_canadd=true;
        for (int checktile=((OnlyCheckNewTilesForDuplicates!=0)?start:0); ((temp_canadd==true)&&(checktile<currtile)); checktile++) { //check all tiles before this one
          for (int flipping=0; ((temp_canadd==true)&&(flipping<4)); flipping++) {
            if (DuplicateAction[flipping]>0) {
              if (keypressed()) {
                return;
              }
              duplicate=true;
              switch (flipping) {
                case 0: //normal
                  for(int y=0; ((duplicate==true)&&(y<16)); y++) { //check tile
                    for(int x=0; ((duplicate==true)&&(x<16)); x+=2) {
                      if ((dest[(checktile*128)+(y*8)+(x/2)])!=(testtile[(y*8)+(x/2)])) {
                        duplicate=false;
                      }
                    }
                  }
                  break;
                case 1: //horizontal
                  for(int y=0; ((duplicate==true)&&(y<16)); y++) { //check tile
                    for(int x=0; ((duplicate==true)&&(x<16)); x+=2) {
                      if ((dest[(checktile*128)+(y*8)+((14-x)/2)])!=(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15))) {
                        duplicate=false;
                      }
                    }
                  }
                  break;
                case 2: //vertical
                  for(int y=0; ((duplicate==true)&&(y<16)); y++) { //check tile
                    for(int x=0; ((duplicate==true)&&(x<16)); x+=2) {
                      if ((dest[(checktile*128)+((15-y)*8)+(x/2)])!=(testtile[(y*8)+(x/2)])) {
                        duplicate=false;
                      }
                    }
                  }
                  break;
                case 3: //both
                  for(int y=0; ((duplicate==true)&&(y<16)); y++) { //check tile
                    for(int x=0; ((duplicate==true)&&(x<16)); x+=2) {
                      if ((dest[(checktile*128)+((15-y)*8)+((14-x)/2)])!=(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15))) {
                        duplicate=false;
                      }
                    }
                  }
                  break;
              }
              if (duplicate==true) {
                duplicates_found[flipping]++;
                total_duplicates_found++;
                if (DuplicateAction[flipping]>1) {
                  total_duplicates_discarded++;
                  temp_canadd=false;
                }
              }
            }
            canadd=canadd&&temp_canadd;
          }
        }
      }

      if (canadd==true) {
        for(int y=0; y<16; y++) {
          for(int x=0; x<8; x++) {
            dest[(currtile*128)+(y*8)+x]=testtile[(y*8)+x];
          }
        }
        currtile++;
      }
    }
  }
  return;
}

void grab(byte *dest,byte *def, int width, int height)
{
  switch(imagetype)
  {
  case ftZGP:
  case ftQSU:
  case ftTIL:
  case ftBIN:
  case ftBMP:
    for (int ty=0; ty<height; ty++) {
      for (int tx=0; tx<width; tx++) {
        for(int y=0; y<16; y++) {
          for(int x=0; x<16; x+=2) {
            if(y<8 && x<8 && grabmask&1) {
              dest[(((ty*20)+tx)*128)+(y*8)+(x/2)]=def[(((ty*20)+tx)*128)+(y*8)+(x/2)];
            } else if(y<8 && x>7 && grabmask&2) {
              dest[(((ty*20)+tx)*128)+(y*8)+(x/2)]=def[(((ty*20)+tx)*128)+(y*8)+(x/2)];
            } else if(y>7 && x<8 && grabmask&4) {
              dest[(((ty*20)+tx)*128)+(y*8)+(x/2)]=def[(((ty*20)+tx)*128)+(y*8)+(x/2)];
            } else if(y>7 && x>7 && grabmask&8) {
              dest[(((ty*20)+tx)*128)+(y*8)+(x/2)]=def[(((ty*20)+tx)*128)+(y*8)+(x/2)];
            } else {
              dest[(((ty*20)+tx)*128)+(y*8)+(x/2)]=
                (getpixel(screen2,(tx*16)+x+selx,(ty*16)+y+sely)&15)+
                ((getpixel(screen2,(tx*16)+x+1+selx,(ty*16)+y+sely)&15) << 4);
            }
          }
        }
      }
    }
    break;

  default:
    for(int i=0; i<128*200; i++)
      dest[i]=0;
    break;
  }
}

void grab_tile(int tile,int &cs)
{
 byte newtile[128*200];

 bool bdown=false;
 int done=0;
 int pal=0;
 int f=0;
 int black=bgc,white=vc(15);
 int selwidth=1, selheight=1;
 int selx2, sely2;
 bool xreversed=false, yreversed=false;
 
 if(imagebuf==NULL)
   load_imagebuf();

 draw_grab_scr(tile,cs,newtile,black,white,vc(11), selwidth, selheight);
 grab(newtile,tilebuf+(tile<<7), selwidth, selheight);
 draw_grab_scr(tile,cs,newtile,black,white,vc(11), selwidth, selheight);
 while(mouse_b);

 do {
   bool redraw=false;

   if(keypressed())
   {
     redraw=true;
     switch(readkey()>>8)
     {
     case KEY_ESC:   done=1; break;
     case KEY_ENTER: done=2; break;
     case KEY_DOWN:  imagey++; break;
     case KEY_UP:    imagey--; break;
     case KEY_RIGHT: imagex++; break;
     case KEY_LEFT:  imagex--; break;
     case KEY_PGDN:  imagey+=10; break;
     case KEY_PGUP:  imagey-=10; break;
     case KEY_HOME:  imagex=imagey=0; break;
     case KEY_PLUS_PAD:  cs = (cs<11) ? cs+1:0;  break;
     case KEY_MINUS_PAD: cs = (cs>0)  ? cs-1:11; break;
     case KEY_S: if(grabmode==1) grabmode=8; else if(grabmode==8) grabmode=16; else grabmode=1; break;
     case KEY_1: imagex=(imagex*bp)>>3; bp=1; imagex<<=3; break;
     case KEY_2: imagex=(imagex*bp)>>3; bp=2; imagex<<=2; break;
     case KEY_4: imagex=(imagex*bp)>>3; bp=4; imagex<<=1; break;
     case KEY_8: imagex=(imagex*bp)>>3; bp=8; break;
     case KEY_Z: if(romofs>0) romofs--; break;
     case KEY_X: romofs++; break;
     default: redraw=false;
     }

     if(imagex<0) imagex=0;
     if(imagey<0) imagey=0;
     draw_grab_scr(tile,cs,newtile,black,white,pal?white:vc(11), selwidth, selheight);
     grab(newtile,tilebuf+(tile<<7), selwidth, selheight);
   }

   //boogie!
   if(mouse_b==1 && !bdown)
   {
     bool regrab=false;

     bdown=true;
     if(mouse_y<160)
     {
       while(mouse_b)
       {
         if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT])) {
           selx=min((mouse_x/grabmode)*grabmode,304);
           sely=min((mouse_y/grabmode)*grabmode,144);
           selx2=selx;
           sely2=sely;
           selwidth=1;
           selheight=1;
           xreversed=false;
           yreversed=false;
         } else {
           if (xreversed) {
             swap(selx, selx2);
             xreversed=false;
           }
           if (yreversed) {
             swap(sely, sely2);
             yreversed=false;
           }
           selx2=min((mouse_x/grabmode)*grabmode,304);
           sely2=min((mouse_y/grabmode)*grabmode,144);
           selwidth=1+(abs(selx2-selx))/16;
           selheight=1+(abs(sely2-sely))/16;
           if (selx2<selx) {
             swap(selx, selx2);
             xreversed=true;
           }
           if (sely2<sely) {
             swap(sely, sely2);
             yreversed=true;
           }
         }
//         grab(newtile,tilebuf+(tile<<7), 1, 1);
         grab(newtile,tilebuf+(tile<<7), selwidth, selheight);
         draw_grab_scr(tile,cs,newtile,black,white,pal?white:vc(11), selwidth, selheight);
       }
     }
     else if(isinRect(mouse_x,mouse_y,260,168,311,183))
     {
       if(do_button(260,168,56,16,"OK",dvc(9),vc(14)))
         done=2;
     }
     else if(isinRect(mouse_x,mouse_y,120,168,171,183))
     {
       if(do_button(120,168,56,16,"Leech",dvc(9),vc(14))) {
         leech_tiles(tilebuf,tile);
         done=1;
       }
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
           draw_grab_scr(tile,cs,newtile,black,white,vc(11), selwidth, selheight);
//           grab(newtile,tilebuf+(tile<<7), 1, 1);
           grab(newtile,tilebuf+(tile<<7), selwidth, selheight);
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
     else if(isinRect(mouse_x,mouse_y,184,168,191,175))
     {
       regrab=true; grabmask^=1;
     }
     else if(isinRect(mouse_x,mouse_y,192,168,199,175))
     {
       regrab=true; grabmask^=2;
     }
     else if(isinRect(mouse_x,mouse_y,184,176,191,183))
     {
       regrab=true; grabmask^=4;
     }
     else if(isinRect(mouse_x,mouse_y,192,176,199,183))
     {
       regrab=true; grabmask^=8;
     }

     if(regrab)
     {
//       grab(newtile,tilebuf+(tile<<7), 1, 1);
       grab(newtile,tilebuf+(tile<<7), selwidth, selheight);
       redraw=true;
     }
   }

   if(mouse_b==0)
     bdown=false;

   if(redraw)
     draw_grab_scr(tile,cs,newtile,black,white,pal?white:vc(11), selwidth, selheight);
   else
     vsync();

   scare_mouse();
   blit(screen2,screen,selx,sely,selx,sely,selwidth*16,selheight*16);
   if(f&8) {
/*
     for (int x=0; x<selwidth; x++) {
       for (int y=0; y<selheight; y++) {
         rect(screen,selx,sely,selx+15,sely+15,white);
       }
     }
*/
//     rect(screen,selx,sely,selx+15,sely+15,white);
     rect(screen,selx,sely,selx+((selwidth-1)*16)+15,sely+((selheight-1)*16)+15,white);
   }
   unscare_mouse();
   SCRFIX();
   f++;

 } while(!done);

 if(done==2)
 {
   go_tiles();
   saved=false;
   usetiles=true;
   for (int y=0; y<selheight; y++) {
     for (int x=0; x<selwidth; x++) {
       for(int i=0; i<128; i++) {
         tilebuf[(tile<<7)+(((20*y)+x)*128)+i] = newtile[(((20*y)+x)*128)+i];
       }
     }
   }
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


void tile_info_0(int tile,int flip,int cs,int copy,int copycnt,int page)
{
 rectfill(screen2,0,208,319,239,dvc(9));
 rect(screen2,0,208,319,239,vc(15));
 if(copy>=0)
 {
   puttile16(screen2,copy,8,216,cs,flip);
   if(copycnt>1) {
     textprintf_centre(screen2,sfont,16,210,vc(11),"%d-",copy);
     textprintf_centre(screen2,sfont,16,233,vc(11),"%d",copy+copycnt-1);
   }
   else
     textprintf_centre(screen2,sfont,16,233,vc(11),"%d",copy);
 }
 else
   rectfill(screen2,8,216,23,231,vc(1));
 puttile16(screen2,tile,32,216,cs,flip);
 text_mode(dvc(9));
 textprintf(screen2,font,56,216,vc(11),"tile:%-5d",tile);
 textprintf(screen2,font,56,224,vc(11),"f:%d cs:%d",flip,cs);
 draw_button(screen2,140,216,44,16,"Edit",dvc(9),vc(14));
 draw_button(screen2,185,216,44,16,"Grab",dvc(9),vc(14));
 draw_button(screen2,230,216,44,16,"Done",dvc(9),vc(14));
 textprintf(screen2,font,300,212,vc(14),"^");
 textprintf(screen2,font,278,220,vc(11),"p:%-3d",page);
 textprintf(screen2,font,300,228,vc(14),"v");
 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 unscare_mouse();
 SCRFIX();
}

void tile_info_1(int oldtile,int oldflip,int tile,int flip,int cs,int copy,int page)
{
 rectfill(screen2,0,208,319,239,dvc(9));
 rect(screen2,0,208,319,239,vc(15));
 text_mode(dvc(9));
 textprintf(screen2,font,4,212,vc(11),"Old tile: %-5d  New tile: %-5d",oldtile,tile);
 textprintf(screen2,font,4,220,vc(11),"    flip: %d          flip: %d",oldflip,flip);
 if(copy>=0) {
   puttile16(screen2,copy,124,220,cs,flip);
//   rectfill(screen2,152,216,159,223,dvc(9));
   }
 puttile16(screen2,oldtile,8,220,cs,oldflip);
 puttile16(screen2,tile,144,220,cs,flip);
 textprintf(screen2,font,300,212,vc(14),"^");
 textprintf(screen2,font,268,220,vc(11),"p: %-3d",page);
 textprintf(screen2,font,300,228,vc(14),"v");
 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 unscare_mouse();
 SCRFIX();
}


void reset_tile(int t)
{
 for(int i=0; i<128; i++)
   tilebuf[(t<<7)+i]=0;
}


static MENU select_tile_rc_menu[] =
{ 
   { "Copy",    NULL,  NULL },
   { "Paste",   NULL,  NULL },
   { "Delete",  NULL,  NULL },
   { "" },
   { "Edit",    NULL,  NULL },
   { "Grab",    NULL,  NULL },
   { NULL }
};


void copy_tiles(int &tile,int &tile2,int &copy,int &copycnt)
{
  go_tiles();
  if(tile2<tile)
    swap(tile,tile2);
  //if copying to an earlier tile, copy from left to right
  //otherwise, copy from right to left
  for(int t=(tile<copy)?0:(copycnt-1);
      (tile<copy)?(t<copycnt):(t>=0);
      (tile<copy)?(t++):(t--))
    for(int i=0; i<128; i++)
      tilebuf[((tile+t)<<7)+i]=tilebuf[((copy+t)<<7)+i];
  copy=-1;
  tile2=tile;
  saved=false;
  usetiles=true;
}

void move_tiles(int &tile,int &tile2,int &copy,int &copycnt)
{
  go_tiles();
  if(tile2<tile)
    swap(tile,tile2);
  for(int t=(tile<copy)?0:(copycnt-1);
      (tile<copy)?(t<copycnt):(t>=0);
      (tile<copy)?(t++):(t--))
    for(int i=0; i<128; i++) {
      tilebuf[((tile+t)<<7)+i]=tilebuf[((copy+t)<<7)+i];
      tilebuf[((copy+t)<<7)+i]=0;
      }

  for(int u=0; u<MAXCOMBOS; u++)
      if ((combobuf[u].tile>=copy)&&(combobuf[u].tile<copy+copycnt)) {
        combobuf[u].tile=combobuf[u].tile-copy+tile;
      }
  copy=-1;
  tile2=tile;
  saved=false;
  usetiles=true;
}

void delete_tiles(int &tile,int &tile2)
{
  char buf[40];
  if(tile==tile2)
    sprintf(buf,"Delete tile %d?",tile);
  else
    sprintf(buf,"Delete tiles %d-%d?",min(tile,tile2),max(tile,tile2));

  if(alert(buf,NULL,NULL,"&Yes","&No",'y','n')==1)
  {
    go_tiles();
    for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
      reset_tile(i);
    tile=tile2=min(tile,tile2);
    saved=false;
    usetiles=true;
  }
}


void overlay_tile(int dest,int src)
{
  byte buf[256];
  go_tiles();

  unpack_tile(dest);
  for(int i=0; i<256; i++)
    buf[i] = unpackbuf[i];

  unpack_tile(src);
  for(int i=0; i<256; i++)
    if(unpackbuf[i])
      buf[i] = unpackbuf[i];

  pack_tile(buf,dest);
  saved=false;
}


void sel_tile(int &tile, int &tile2, int &first, int type, int s)
{
  tile+=s;
  bound(tile,0,NEWMAXTILES-1);
  if(type!=0 || !(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
    tile2 = tile;
  first = tile - (tile%260);
}


int select_tile(int &tile,int &flip,int type,int &cs)
{
 reset_combo_animations();
 bound(tile,0,NEWMAXTILES-1);
 int done=0;
 int oflip=flip;
 int otile=tile;
 int first=(tile/260)*260;
 int copy=-1;
 int tile2=tile,copycnt=0;
 bound(first,0,260*125);

 if(type==0)
   select_tile_rc_menu[5].text = "Grab";

 go();

 draw_tiles(first,cs);
 if(type==0)
   tile_info_0(tile,flip,cs,copy,copycnt,first/260);
 else
   tile_info_1(otile,oflip,tile,flip,cs,copy,first/260);

 go_tiles();
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
     case KEY_UP:    sel_tile(tile,tile2,first,type,-20); redraw=true; break;
     case KEY_DOWN:  sel_tile(tile,tile2,first,type,20);  redraw=true; break;
     case KEY_LEFT:  sel_tile(tile,tile2,first,type,-1);  redraw=true; break;
     case KEY_RIGHT: sel_tile(tile,tile2,first,type,1);   redraw=true; break;
     case KEY_PGUP:  sel_tile(tile,tile2,first,type,-260); redraw=true; break;
     case KEY_PGDN:  sel_tile(tile,tile2,first,type,260);  redraw=true; break;
     case KEY_HOME:  sel_tile(tile,tile2,first,type,-(tile%20));   redraw=true; break;
     case KEY_END:   sel_tile(tile,tile2,first,type,19-(tile%20)); redraw=true; break;
     case KEY_O:     if(type==0 && copy>=0) { overlay_tile(tile,copy); redraw=true; } break;
     case KEY_E:     if(type==0) { edit_tile(tile,flip,cs); redraw=true; } break;
     case KEY_G:     if(type==0) { grab_tile(tile,cs); redraw=true; } break;
     case KEY_C:     copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; redraw=true; break;
     case KEY_R:     go_tiles(); rotate_tile(tile,(key[KEY_LSHIFT] || key[KEY_RSHIFT])); redraw=true; saved=false; usetiles=true; break;
     case KEY_N:     go_tiles(); normalize(tile,tile2,flip); flip=0; redraw=true; saved=false; usetiles=true; break;
     case KEY_H:     flip^=1; redraw=true; break;
     case KEY_F12:   onSnapshot();
     case KEY_V:     if(copy==-1)
                       flip^=2;
                     else
                     {
                       copy_tiles(tile,tile2,copy,copycnt);
                       saved=false;
                       usetiles=true;
                     }
                     redraw=true;
                     break;
     case KEY_DEL:   delete_tiles(tile,tile2); redraw=true; break;
     case KEY_U:     comeback_tiles(); redraw=true; break;
     case KEY_M:     if(copy!=-1) {
                       move_tiles(tile,tile2,copy,copycnt);
                       saved=false;
                       usetiles=true;
                     }
                     redraw=true;
                     break;
     }
   }

   if(mouse_b&1) {
     int x=mouse_x;
     int y=mouse_y;
     if(y<208)
     {
       int t = (y>>4)*20 + (x>>4) + first;

       if(type==0 && (key[KEY_LSHIFT] || key[KEY_RSHIFT]))
         tile2=t;
       else tile=tile2=t;
     }
     else if(x>300 && !bdown)
     {
       if(y<224 && first>0) { first-=260; redraw=true; }
       if(y>=224 && first<260*125) { first+=260; redraw=true; }
       bdown=true;
     }


/*
     if(type==1) {
       if(!bdown && isinRect(x,y,40,216,63,231))
         done=1;
       if(!bdown && isinRect(x,y,184,216,199,231))
         done=2;
       }
*/
     if(type==1) {
       if(!bdown && isinRect(x,y,8,220,23,235))
         done=1;
       if(!bdown && isinRect(x,y,144,220,159,235))
         done=2;
       }
     else if(!bdown && isinRect(x,y,140,216,184,231)) {
       if(do_button(140,216,44,16,"Edit",dvc(9),vc(14))) {
         edit_tile(tile,flip,cs);
         redraw=true;
         }
       }
     else if(!bdown && isinRect(x,y,185,216,234,231)) {
       if(do_button(185,216,44,16,"Grab",dvc(9),vc(14))) {
         grab_tile(tile,cs);
         redraw=true;
         }
       }
     else if(!bdown && isinRect(x,y,230,216,284,231))
       if(do_button(230,216,44,16,"Done",dvc(9),vc(14)))
         done=1;
     bdown=true;
     }

   bool r_click = false;
   if(mouse_b&2 && !bdown && type==0)
   {
     int x=mouse_x&0xFF0;
     int y=mouse_y&0xF0;
     if(y<208)
     {
       int t = (y>>4)*20 + (x>>4) + first;
       if(t<min(tile,tile2) || t>max(tile,tile2))
         tile=tile2=t;
     }
     bdown = r_click = true;
     f=8;
   }

   if(mouse_b==0)
     bdown=false;


   REDRAW:

//   if(redraw)
   draw_tiles(first,cs);
   if(f&8)
   {
     for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
     {
       if(i>=first && i<first+260)
       {
         int x=(i%20)<<4;
         int y=((i-first)/20)<<4;
         rect(screen2,x,y,x+15,y+15,vc(15));
       }
     }
   }
   if(type==0)
     tile_info_0(tile,flip,cs,copy,copycnt,first/260);
   else
     tile_info_1(otile,oflip,tile,flip,cs,copy,first/260);

   f++;

   if(r_click)
   {
     select_tile_rc_menu[1].flags = (copy==-1) ? D_DISABLED : 0;
     int m = popup_menu(select_tile_rc_menu,mouse_x,mouse_y);
     redraw=true;
     switch(m) {
     case 0: copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; break;
     case 1: copy_tiles(tile,tile2,copy,copycnt); break;
     case 2: delete_tiles(tile,tile2); break;
     case 4: edit_tile(tile,flip,cs); break;
     case 5: grab_tile(tile,cs); break;
     default: redraw=false; break;
     }
     r_click = false;
     goto REDRAW;
   }

 } while(!done);

 while(mouse_b);
 comeback();
 setup_combo_animations();
 return done-1;
}



int onTiles()
{
 static int t=0;
 int f=0;
 int c=CSet;
 select_tile(t,f,0,c);
 refresh(rALL);
 return D_O_K;
}




void draw_combo(int x,int y,int c,int cs,int page)
{
  if(c<1024)
    put_combo(screen2,x,y,(cs<<8)+c,0,page,0);
  else
    rectfill(screen2,x,y,x+15,y+15,0);
}


void draw_combos(int page,int cs,bool cols)
{
 clear(screen2);
 if(cols==false) {
   for(int i=0; i<256; i++) // 13 rows, leaving 32 pixels from y=208 to y=239
     draw_combo((i%20)<<4,(i/20)<<4,i,cs,page);
   }
 else {
   int c = 0;
   for(int i=0; i<256; i++) {
     draw_combo((i%20)<<4,(i/20)<<4,c,cs,page);
     c++;
     if((i&3)==3)
       c+=48;
     if((i%20)==19)
       c-=256;
     }
   }
 for(int x=64; x<320; x+=64)
   vline(screen2,x,0,207,vc(15));
}


void combo_info(int tile,int tile2,int cs,int copy,int copycnt,int page)
{
 rectfill(screen2,0,208,319,239,dvc(9));
 rect(screen2,0,208,319,239,vc(15));
 if(copy>=0)
 {
   put_combo(screen2,16,216,(copy&255)+(cs<<8),0,copy>>8,0);
   if(copycnt>1) {
     textprintf_centre(screen2,sfont,24,210,vc(11),"%d-",copy);
     textprintf_centre(screen2,sfont,24,233,vc(11),"%d",copy+copycnt-1);
   }
   else
     textprintf_centre(screen2,sfont,24,233,vc(11),"%3d",copy);
 }
 else
   rectfill(screen2,16,216,31,231,vc(1));
 put_combo(screen2,40,216,(tile&255)+(cs<<8),0,tile>>8,0);
 text_mode(dvc(9));
// textprintf(screen2,font,56,216,vc(11),"page: %d",page);

 if(tile>tile2)
   swap(tile,tile2);
 char buf[8];
 buf[0]=0;
 if(tile2!=tile)
   sprintf(buf,"-%d",tile2);

 textprintf(screen2,font,72,216,vc(11),"combo:");
 textprintf(screen2,font,72,224,vc(11),"%d%s",tile,buf);

 draw_button(screen2,174,216,44,16,"Edit",dvc(9),vc(14));
 draw_button(screen2,224,216,44,16,"Done",dvc(9),vc(14));
 textprintf(screen2,font,304,212,vc(14),"^");
 textprintf(screen2,font,286,220,vc(11),"p:%2d",page);
 textprintf(screen2,font,304,228,vc(14),"v");
 vsync();
 scare_mouse();
 blit(screen2,screen,0,0,0,0,320,240);
 unscare_mouse();
 SCRFIX();
}



void sel_combo(int &tile, int &tile2, int s, bool cols)
{
  int page = tile&0xFF00;
  tile &= 0xFF;

  if(!cols)
    tile += s;
  else
  {
    if(s==-20)
      tile-=4;
    if(s==20)
      tile+=4;
    if(s==-1)
      tile-=1;
    if(s==1)
      tile+=1;
  }
/*
    if(s==1)
    {
      if((tile&3)==3)
        tile+=48;
      else
        tile++;
    }
    if(s==-1)
    {
      if((tile&3)==0)
        tile-=48;
      else
        tile--;
    }
  }
*/
  bound(tile,0,255);
  tile += page;
  if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
    tile2 = tile;
}


word ctable[MAXCOMBOS];

int combo_screen()
{
 reset_combo_animations();
 static int tile=0;
 static int page=0;
 static bool cols=false;
 int tile2=tile;
 int done=0;
 int cs = CSet;
 int copy=-1;
 int copycnt=0;

 select_tile_rc_menu[5].text = NULL;

 for(int i=0; i<MAXCOMBOS; i++)
//   for (int x=0; x<9; x++)
     combobuf[i].foo=i;

 go();
 draw_combos(page,cs,cols);
 combo_info(tile,tile2,cs,copy,copycnt,page);
 go_combos();
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
     case KEY_UP:    sel_combo(tile,tile2,-20,cols); redraw=true; break;
     case KEY_DOWN:  sel_combo(tile,tile2,20,cols);  redraw=true; break;
     case KEY_LEFT:  sel_combo(tile,tile2,-1,cols);  redraw=true; break;
     case KEY_RIGHT: sel_combo(tile,tile2,1,cols);   redraw=true; break;
     case KEY_PGUP:  if(page>0) { page--; tile=tile2=(page<<8)+(tile&0xFF); } redraw=true; break;
     case KEY_PGDN:  if(page<63) { page++; tile=tile2=(page<<8)+(tile&0xFF); } redraw=true; break;
     case KEY_U:     comeback_combos(); redraw=true; break;
     case KEY_E:     go_combos();  edit_combo(tile,false,cs); redraw=true; setup_combo_animations(); break;
     case KEY_C:     go_combos(); copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; redraw=true; break;

     case KEY_H:     for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
                       combobuf[i].flip^=1;
                     redraw=true;
                     saved=false;
                     break;

     case KEY_M:     tile=tile2=min(tile,tile2);
                     if(copy>=0 && tile!=copy)
                     {
                       go_combos();
                       newcombo hold;
                       for(int i=0; i<copycnt; i++)
                       {
                         if(tile<copy)
                         {
                           hold = combobuf[copy+copycnt-1];
                           for(int i=copy+copycnt-1; i>tile; i--)
                             combobuf[i] = combobuf[i-1];
                           combobuf[tile] = hold;
                         }
                         else
                         {
                           hold = combobuf[copy];
                           for(int i=copy; i<tile+copycnt-1; i++)
                             combobuf[i] = combobuf[i+1];
                           combobuf[tile] = hold;
                         }
                       }
                       saved=false;
                     }
                     copy=-1;
                     redraw=true;
                     break;

     case KEY_S:     tile=tile2=min(tile,tile2);
                     if(copy>=0 && tile!=copy)
                     {
                       go_combos();
                       for(int i=0; i<copycnt; i++)
                         swap(combobuf[copy+i],combobuf[tile+i]);
                       saved=false;
                       setup_combo_animations();
                     }
                     redraw=true;
                     copy=-1;
                     break;

     case KEY_V:     if(copy==-1)
                     {
                       go_combos();
                       for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
                         combobuf[i].flip^=2;
                       saved=false;
                     }
                     else if(min(tile,tile2)!=copy)
                     {
                       go_combos();
                       tile=tile2=min(tile,tile2);
                       for(int i=0; i<copycnt; i++)
                         combobuf[tile+i]=combobuf[copy+i];
                       saved=false;
                       setup_combo_animations();
                     }
                     copy=-1;
                     redraw=true;
                     break;

     case KEY_DEL:   {
                     char buf[40];
                     if(tile==tile2)
                       sprintf(buf,"Delete combo %d?",tile);
                     else
                       sprintf(buf,"Delete combos %d-%d?",min(tile,tile2),max(tile,tile2));

                     if(alert(buf,NULL,NULL,"&Yes","&No",'y','n')==1)
                     {
                       go_combos();
                       for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
                         clear_combo(i);
                       tile=tile2=min(tile,tile2);
                       redraw=true;
                       saved=false;
                       setup_combo_animations();
                     }
                     } break;
     }
   }

   if(mouse_b&1) {
     int x=mouse_x;
     int y=mouse_y;
     if(y<208)
     {
       int t;
       if(!cols)
         t = (y>>4)*20 + (x>>4);
       else
         t = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
       bound(t,0,255);
       t+=page<<8;

       if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
         tile2=t;
       else tile=tile2=t;
     }
     else if(x>300 && !bdown)
     {
       if(y<224 && page>0) { page--; redraw=true; }
       if(y>=224 && page<63) { page++; redraw=true; }
       bdown=true;
     }

//     if(!bdown && isinRect(x,y,190,216,234,231)) {
     if(!bdown && isinRect(x,y,174,216,214,232)) {
       if(do_button(174,216,44,16,"Edit",dvc(9),vc(14))) {
         edit_combo(tile,false,cs);
         redraw=true;
         }
       }
//     else if(!bdown && isinRect(x,y,240,216,284,231))
     else if(!bdown && isinRect(x,y,224,216,268,232))
       if(do_button(224,216,44,16,"Done",dvc(9),vc(14)))
         done=1;
     bdown=true;
     }


   bool r_click = false;
   if(mouse_b&2 && !bdown)
   {
     int x=mouse_x;
     int y=mouse_y;
     if(y<208)
     {
       int t;
       if(!cols)
         t = (y>>4)*20 + (x>>4);
       else
         t = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
       bound(t,0,255);
       t+=page<<8;

       if(t<min(tile,tile2) || t>max(tile,tile2))
         tile=tile2=t;
     }
     bdown = r_click = true;
     f=8;
   }



   REDRAW:

   if(mouse_b==0)
     bdown=false;

   if(redraw)
     draw_combos(page,cs,cols);
   combo_info(tile,tile2,cs,copy,copycnt,page);

   if(f&8)
   {
     int x,y;
     scare_mouse();
     for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
     {
       if((i>>8)==page)
       {
         int t=i&255;
         if(!cols)
         {
           x=(t%20)<<4;
           y=(t/20)<<4;
         }
         else
         {
           x=((t&3) + ((t/52)<<2)) << 4;
           y=((t%52)>>2) << 4;
         }
         rect(screen,x,y,x+15,y+15,vc(15));
       }
     }
     unscare_mouse();
     SCRFIX();
   }
   f++;

   if(r_click)
   {
     select_tile_rc_menu[1].flags = (copy==-1) ? D_DISABLED : 0;
     int m = popup_menu(select_tile_rc_menu,mouse_x,mouse_y);
     redraw=true;
     switch(m)
     {
     case 0: go_combos(); copy=min(tile,tile2); copycnt=abs(tile-tile2)+1; break;
     case 1:
       if(min(tile,tile2)!=copy)
       {
         go_combos();
         tile=tile2=min(tile,tile2);
         for(int i=0; i<copycnt; i++)
           combobuf[tile+i]=combobuf[copy+i];
         saved=false;
       }
       copy=-1;
       break;

     case 2:
       {
       char buf[40];
       if(tile==tile2)
         sprintf(buf,"Delete combo %d?",tile);
       else
         sprintf(buf,"Delete combos %d-%d?",min(tile,tile2),max(tile,tile2));

       if(alert(buf,NULL,NULL,"&Yes","&No",'y','n')==1)
       {
         go_combos();
         for(int i=min(tile,tile2); i<=max(tile,tile2); i++)
           clear_combo(i);
         tile=tile2=min(tile,tile2);
         redraw=true;
         saved=false;
         }
       }
       break;

     case 4: go_combos(); edit_combo(tile,false,cs); break;
     default: redraw=false; break;
     }
     r_click = false;
     goto REDRAW;
   }

 } while(!done);

 for(int p=0; p<MAXCOMBOS; p+=256)
 {
   for(int i=0; i<256; i++)
   {
     int pos=0;
     for(int j=0; j<256; j++)
     {
       if(combobuf[j+p].foo==i+p)
       {
         pos=j+p;
         goto down;
       }
     }
     down:
     ctable[i+p]=pos;
   }
 }

 int sc = vbound(header.map_count,0,MAXMAPS)*MAPSCRS;
 for(int s=0; s<sc; s++)
   for(int i=0; i<176; i++)
   {
     int oc = ((TheMaps[s].data[i]&0xFF) + (TheMaps[s].cpage<<8)) & 2047;
     TheMaps[s].data[i] &= 0xFF00;
     TheMaps[s].data[i] |= ctable[oc]&0xFF;
   }

 while(mouse_b);
 comeback();
 setup_combo_animations();
 return done-1;
}



int onCombos()
{
// reset_combo_animations();
 combo_screen();
// setup_combo_animations();
 refresh(rALL);
 return D_O_K;
}



int edit_combo_cset;

int d_ctile_proc(int msg,DIALOG *d,int c)
{
 if(msg==MSG_CLICK) {
   int t=curr_combo.tile;
   int f=curr_combo.flip;
   if(select_tile(t,f,1,edit_combo_cset)) {
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
 {
   newcombo *hold = combobuf;
   combobuf = &curr_combo;
   putcombo(combo_bmp,0,0,edit_combo_cset<<8,0);
   combobuf = hold;

   text_mode(d->bg);
   textprintf(screen,font,d->x,d->y,d->fg,"Tile: %-4d",curr_combo.tile);
   textprintf(screen,font,d->x,d->y+8,d->fg,"Flip: %d",curr_combo.flip);
   textprintf(screen,font,d->x,d->y+16,d->fg,"CSet2:");
 }
 return D_O_K;
}



typedef struct combotype_struct {
  char *s;
  int i;
} combotype_struct;

combotype_struct bict[cMAX];
int bict_cnt=-1;

void build_bict_list()
{
  bict[0].s = "(none)";
  bict[0].i = 0;
  bict_cnt=1;

  for(int i=1; i<cMAX; i++)
  {
    bict[bict_cnt].s = combotype_string[i];
    bict[bict_cnt].i = i;
    bict_cnt++;
  }

  for(int i=1; i<bict_cnt-1; i++)
    for(int j=i+1; j<bict_cnt; j++)
      if(stricmp(bict[i].s,bict[j].s)>0)
        swap(bict[i],bict[j]);

}

char *combotypelist(int index, int *list_size)
{
 if(index<0) {
   *list_size = bict_cnt;
   return NULL;
   }
 return bict[index].s;
}

/*
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
*/

int onCmb_dlg_h()
{
  curr_combo.flip^=1;
  return D_REDRAW;
}

int onCmb_dlg_v()
{
  curr_combo.flip^=2;
  return D_REDRAW;
}


static DIALOG combo_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 64,   48,   192,  152+16,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_combo_loader,    72,   72,   0,    0,    vc(11),  vc(1),  0,       0,          0,             0,       NULL },
// 2
 { d_bitmap_proc,     160,  72,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_button_proc,     90,   176+16,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  176+16,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { d_edit_proc,       128,  89,   48,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
// 6
 { d_wflag_proc,      192,  72,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      192,  80,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      200,  72,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      200,  80,   8,    8,    vc(12),  vc(7),  0,       0,          0,             1,       NULL },
// 10
 { d_ctile_proc,      160,  72,   16,   16,   0,       0,      0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  56,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Edit Combo" },
 { d_text_proc,       72,   91+24,  48,   8,    vc(11),  vc(1),  0,       0,          0,             0,       (void *) "Type:" },
 { d_abclist_proc,    72,  108+16,  120+48,  59,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       onHelp },
// 15
 { d_wflag_proc,      224,  72,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      232,  72,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      224,  80,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL },
 { d_wflag_proc,      232,  80,   8,    8,    vc(11),  vc(7),  0,       0,          0,             1,       NULL },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'h',     0,          0,             0,       onCmb_dlg_h },
 { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      'v',     0,          0,             0,       onCmb_dlg_v },
 { d_text_proc,       72,   89+8,   48,   8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "A.Frames:" },
 { d_text_proc,       72,   89+16,   48,   8,    vc(11),  vc(1),  0,       0,          2,             0,       (void *) "A.Speed:" },
 { d_edit_proc,       72+80,  89+8,   48,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { d_edit_proc,       72+72,  89+16,   48,   8,    vc(12),  vc(1),  0,       0,          2,             0,       NULL },
 { NULL }
};



void edit_combo(int c,bool freshen,int cs)
{
 if (bict_cnt==-1) {
   build_bict_list();
 }
 reset_combo_animations();

 curr_combo = combobuf[c];

 char cset_str[8];
 char frm[8];
 char spd[8];

 char csets = curr_combo.csets & 15;
 if(csets&8)
   csets |= 0xF0;

 sprintf(cset_str,"%d",csets);
 sprintf(frm,"%d",curr_combo.frames);
 sprintf(spd,"%d",curr_combo.speed);
 combo_dlg[5].dp = cset_str;
 combo_dlg[23].dp = frm;
 combo_dlg[24].dp = spd;

 combo_dlg[2].dp=combo_bmp;
 for(int i=0; i<4; i++)
   combo_dlg[i+6].flags = curr_combo.walk&(1<<i) ? D_SELECTED : 0;
 for(int i=0; i<4; i++)
   combo_dlg[i+15].flags = curr_combo.csets&(16<<i) ? D_SELECTED : 0;

 int index=0;

 for(int j=0; j<bict_cnt; j++)
   if(bict[j].i == curr_combo.type)
     index=j;

 combo_dlg[13].d1 = index;
 combo_dlg[13].dp = combotypelist;
 combo_dlg[1].fg = cs;
 edit_combo_cset = cs;


 int ret=popup_dialog(combo_dlg,4);
 if(freshen)
   refresh(rALL);

 if(ret==3) {
   saved=false;
   for(int i=0; i<4; i++)
     if(combo_dlg[i+6].flags & D_SELECTED)
       curr_combo.walk |= 1<<i;
     else
       curr_combo.walk &= ~(1<<i);
   csets = atoi(cset_str) & 15;
   for(int i=0; i<4; i++)
     if(combo_dlg[i+15].flags & D_SELECTED)
       csets |= 16<<i;
   curr_combo.csets = csets;
   curr_combo.type = bict[combo_dlg[13].d1].i;
   curr_combo.frames = atoi(frm);
   curr_combo.speed = atoi(spd);
   combobuf[c] = curr_combo;
   }

 cs = edit_combo_cset;

 if(freshen)
   refresh(rALL);

 setup_combo_animations();

}





int d_itile_proc(int msg,DIALOG *d,int c)
{
  switch(msg)
  {
  case MSG_CLICK:
    {
    int cs = d->d2;
    int f  = 0;
    if(select_tile(d->d1,f,1,cs))
      return D_REDRAW;
    }
    break;

  case MSG_DRAW:
    BITMAP *buf = create_bitmap(16,16);
    if(buf)
    {
      clear(buf);
      overtile16(buf,d->d1,0,0,d->fg,0);
      blit(buf,screen,0,0,d->x,d->y,16,16);
      destroy_bitmap(buf);
    }
    break;
  }
  return D_O_K;
}



static DIALOG icon_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 80,   80,   160,  80,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Game Icons" },
 { d_itile_proc,      108,  102,  16,   16,   0,       0,      0,       0,          0,             6,       NULL },
 { d_itile_proc,      138,  102,  16,   16,   0,       0,      0,       0,          0,             7,       NULL },
 { d_itile_proc,      168,  102,  16,   16,   0,       0,      0,       0,          0,             8,       NULL },
 { d_itile_proc,      198,  102,  16,   16,   0,       0,      0,       0,          0,             9,       NULL },
 { d_button_proc,     90,   136,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  136,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
 { NULL }
};



int onIcons()
{
  PALETTE pal;
  pal = RAMpal;

  for(int i=0; i<4; i++)
  {
    icon_dlg[i+2].d1 = misc.icons[i];
    icon_dlg[i+2].fg = i+6;
    load_cset(pal, i+6, pSprite(i+spICON1));
  }

  set_palette(pal);
  int ret = popup_dialog(icon_dlg,7);

  if(ret==6)
    for(int i=0; i<4; i++)
      if(misc.icons[i] != icon_dlg[i+2].d1)
      {
        misc.icons[i] = icon_dlg[i+2].d1;
        saved=false;
      }

  set_palette(RAMpal);
  return D_O_K;
}





