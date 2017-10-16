/*
  zelda.cc
  Jeremy Craner, 1999
  Main code for Zelda Classic. Originally written in SPHINX C--,
  now rewritten in DJGPP with Allegro.
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <dir.h>
#include <ctype.h>
#include <allegro.h>

BEGIN_GFX_DRIVER_LIST
 GFX_DRIVER_VESA3
 GFX_DRIVER_VESA2L
 GFX_DRIVER_VESA2B
 GFX_DRIVER_VESA1
 GFX_DRIVER_MODEX
END_GFX_DRIVER_LIST

DECLARE_COLOR_DEPTH_LIST(COLOR_DEPTH_8);


int db=0;


extern "C" {
char * getcwd(char *_buf, size_t _size);
}



/**********************************/
/*** Classes, Definitions, Etc. ***/
/**********************************/

#include "zelda.h"
#include "sprite.cc"
#include "link.cc"

/**********************************/
/******** Global Variables ********/
/**********************************/

BITMAP *framebuf,*scrollbuf,*tmp_bmp,*tmp_scr,*fps_undo;
DATAFILE *data,*sfxdata,*mididata;
SAMPLE wav_refill;
FONT *zfont;
PALETTE RAMpal;
byte *tilebuf,*colordata;
combo *combobuf,*spritebuf;
sprite_list  guys,items,Ewpns,Lwpns;
word msgclk=0,msgstr=0,msgpos,Bwpn=0,Bpos=0;
int tilecnt=1000,homescr,currscr,frame=0,currmap=0,dlevel,warpscr,worldscr;
int newscr_clk=0,opendoors=0,currdmap=0,fadeclk=-1,currgame=0,listpos=0;
int lastentrance,prices[3],loadside;
int digi_volume,midi_volume,currmidi,wand_x,wand_y,hasitem,whistleclk,pan_style;
int Akey,Bkey,Skey,Abtn,Bbtn,Sbtn,Quit=0;
bool Vsync, Paused=false, Advance=false, ShowFPS=false, Showpal=false, Playing;
bool refreshpal,blockpath,wand_dead,debug=false,loaded_guys,freeze_guys,
     loaded_enemies,drawguys,details=false,DXtitle,debug_enabled,watch;
bool darkroom=false;
bool Udown,Ddown,Ldown,Rdown,Adown,Bdown,Sdown,SystemKeys=true,NESquit;
short visited[6];
byte guygrid[176];
movingblock mblock[4];
mapscr tmpscr[2];
gamedata game;
char sig_str[44];

LinkClass link;

int VidMode,resx,resy,scrx,scry;
bool sbig; // big screen
bool toogam=false;

int mouse_down;  // used to hold the last reading of 'mouse_b' status

// quest file data
zquestheader QHeader;
MsgStr       *MsgStrings;
dmap         *DMaps;
miscQdata    QMisc;
mapscr       *TheMaps;


volatile int lastfps=0;
volatile int framecnt=0;
volatile int myvsync=0;


/**********************************/
/******* Other Source Files *******/
/**********************************/

#include "aglogo.cc"
#include "qst.cc"
#include "pal.cc"
#include "tiles.cc"
#include "maps.cc"
#include "subscr.cc"
#include "title.cc"
#include "guys.cc"
#include "ending.cc"
#include "midi.cc"
#include "gui.cc"


/**********************************/
/******** System functions ********/
/**********************************/

static char cfg_sect[] = "zeldadx";

void load_game_configs()
{
 Akey = get_config_int(cfg_sect,"key_a",KEY_ALT);
 Bkey = get_config_int(cfg_sect,"key_b",KEY_CONTROL);
 Skey = get_config_int(cfg_sect,"key_s",KEY_ENTER);
 Abtn = get_config_int(cfg_sect,"btn_a",2);
 Bbtn = get_config_int(cfg_sect,"btn_b",1);
 Sbtn = get_config_int(cfg_sect,"btn_s",3);
 digi_volume = get_config_int(cfg_sect,"sfx",248);
 midi_volume = get_config_int(cfg_sect,"music",255);
 pan_style = get_config_int(cfg_sect,"pan",1);
 Vsync = (bool)get_config_int(cfg_sect,"vsync",1);
 ShowFPS = (bool)get_config_int(cfg_sect,"showfps",0);
 NESquit = (bool)get_config_int(cfg_sect,"NESquit",0);
 DXtitle = (bool)get_config_int(cfg_sect,"title",1);
 VidMode = get_config_int(cfg_sect,"mode",GFX_AUTODETECT);
 resx = get_config_int(cfg_sect,"resx",320);
 resy = get_config_int(cfg_sect,"resy",240);
 sbig = get_config_int(cfg_sect,"sbig",0);
 strcpy(qstpath,get_config_string(cfg_sect,"qst_dir",""));
 if(strlen(qstpath)==0)
 {
   getcwd(qstpath,280);
   fix_filename_case(qstpath);
   fix_filename_slashes(qstpath);
   put_backslash(qstpath);
 }
 else
   chop_path(qstpath);
}

void save_game_configs()
{
 set_config_int(cfg_sect,"key_a",Akey);
 set_config_int(cfg_sect,"key_b",Bkey);
 set_config_int(cfg_sect,"key_s",Skey);
 set_config_int(cfg_sect,"btn_a",Abtn);
 set_config_int(cfg_sect,"btn_b",Bbtn);
 set_config_int(cfg_sect,"btn_s",Sbtn);
 set_config_int(cfg_sect,"sfx",digi_volume);
 set_config_int(cfg_sect,"music",midi_volume);
 set_config_int(cfg_sect,"pan",pan_style);
 set_config_int(cfg_sect,"vsync",(int)Vsync);
 set_config_int(cfg_sect,"showfps",(int)ShowFPS);
 set_config_int(cfg_sect,"NESquit",(int)NESquit);
 set_config_int(cfg_sect,"title",(int)DXtitle);
 set_config_int(cfg_sect,"mode",VidMode);
 set_config_int(cfg_sect,"resx",resx);
 set_config_int(cfg_sect,"resy",resy);
 set_config_int(cfg_sect,"sbig",sbig);
 chop_path(qstpath);
 set_config_string(cfg_sect,"qst_dir",qstpath);
}

void fps_callback()
{
  lastfps=framecnt;
  framecnt=0;
}
END_OF_FUNCTION(fps_callback);


void myvsync_callback()
{
  myvsync++;
}
END_OF_FUNCTION(myvsync_callback);



void go()
{
  scare_mouse();
  blit(screen,tmp_scr,scrx,scry,0,0,320,240);
  unscare_mouse();
}

void comeback()
{
  scare_mouse();
  blit(tmp_scr,screen,0,0,scrx,scry,320,240);
  unscare_mouse();
}

void dump_pal()
{
  for(int i=0; i<256; i++)
    rectfill(framebuf,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}


void show_paused()
{
  char buf[7] = "PAUSED";
  for(int i=0; buf[i]!=0; i++)
    buf[i]+=0x60;

  text_mode(-1);
  if(sbig)
    textout(screen,zfont,buf,scrx+40-120,scry+224+104,-1);
  else
    textout(screen,zfont,buf,scrx+40,scry+224,-1);
}


void show_fps()
{
  char buf[20];

  text_mode(-1);
  sprintf(buf,"%2d/60",lastfps);
  for(int i=0; buf[i]!=0; i++)
    if(buf[i]!=' ')
      buf[i]+=0x60;

  if(sbig)
    textout(screen,zfont,buf,scrx+40-120,scry+216+104,-1);
  else
    textout(screen,zfont,buf,scrx+40,scry+216,-1);
}


// sets the video mode and initializes the palette and mouse sprite
bool game_vid_mode(int mode,int wait)
{
  set_color_depth(8);

  switch(mode) {
  case GFX_AUTODETECT:
  case GFX_VESA3:   if(set_gfx_mode(GFX_VESA3,resx,resy,0,0)==0)
                    {
                      VidMode=GFX_VESA3;
                      break;
                    }
  case GFX_VESA2L:  if(set_gfx_mode(GFX_VESA2L,resx,resy,0,0)==0)
                    {
                      VidMode=GFX_VESA2L;
                      break;
                    }
  case GFX_VESA2B:  if(set_gfx_mode(GFX_VESA2B,resx,resy,0,0)==0)
                    {
                      VidMode=GFX_VESA2B;
                      break;
                    }
  case GFX_VESA1:   if(set_gfx_mode(GFX_VESA1,resx,resy,0,0)==0)
                    {
                      VidMode=GFX_VESA1;
                      break;
                    }
  case GFX_MODEX:   if(set_gfx_mode(GFX_MODEX,320,240,0,0)==0)
                    {
                      VidMode=GFX_MODEX;
                      resx=320;
                      resy=240;
                      sbig=false;
                      break;
                    }
  default:          return false;
  }

  scrx=(resx-320)>>1;
  scry=(resy-240)>>1;

  set_mouse_sprite((BITMAP*)data[BMP_MOUSE].dat);
  for(int i=240; i<256; i++)
    RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
  set_palette(RAMpal);
  clear_to_color(screen,0);

  rest(wait);
  return true;
}


void init_NES_mode()
{
  init_colordata();
  loadfullpal();
  for(int i=0; i<TILEBUF_SIZE/4; i++)
    ((long*)tilebuf)[i]=((long*)data[TIL_NES].dat)[i];
}


void updatescr()
{
  if(Showpal)
    dump_pal();
  if(Vsync)
    vsync();
  else if(!debug)
    while(!myvsync);
  myvsync=0;
  if(refreshpal) {
    refreshpal=false;
    RAMpal[253] = _RGB(0,0,0);
    RAMpal[254] = _RGB(63,63,63);
    set_palette_range(RAMpal,0,255,false);
    }

  if(sbig)
    stretch_blit(framebuf,screen,0,0,256,224,scrx+32-128,scry+8-112,512,448);
  else
    blit(framebuf,screen,0,0,scrx+32,scry+8,256,224);

  if(ShowFPS)
    show_fps();
  if(Paused)
    show_paused();
  if(details) {
    text_mode(0);
    textprintf(screen,font,0,224,1,"%d",sfx_count());
    textprintf(screen,font,240,232,1,"pos: %-4d",(int)midi_pos);
    }
  if(toogam)
  {
   text_mode(-1);
   if(sbig)
    textout(screen,font,"no walls",scrx+40-120,scry+224+104,1);
   else
    textout(screen,font,"no walls",scrx+40,scry+224,1);
  }

  framecnt++;
}



#define FILENAME8_3   0
#define FILENAME8__   1

void extract_name(char *path,char *name,int type)
{
 int l=strlen(path);
 int i=l;
 while(i>0 && path[i-1]!='/' && path[i-1]!='\\')
  i--;
 int n=0;
 if(type==FILENAME8__) {
   while(i<l && n<8 && path[i]!='.')
     name[n++]=path[i++];
   }
 else {
   while(i<l && n<12 )
     name[n++]=path[i++];
   }
 name[n]=0;
}



int onSnapshot()
{
  struct ffblk f;
  char buf[20];
  int num=0;
  do {
    sprintf(buf,"zelda%03d.bmp",++num);
  } while(num<999 && !findfirst(buf,&f,0));
  save_bmp(buf,framebuf,RAMpal);
  return D_O_K;
}



void f_Save()
{
  if(!Playing)
    return;

  midi_pause();
  pause_all_sfx();
  system_pal();

  onSave();

  game_pal();
  midi_resume();
  resume_all_sfx();
  rAbtn();
  rBbtn();
  rSbtn();
  if(key[KEY_ESC])
    key[KEY_ESC]=0;
}


void f_Quit(int type)
{
  if(type==qQUIT && !Playing)
    return;

  midi_pause();
  pause_all_sfx();
  system_pal();

  switch(type) {
  case qQUIT:  onQuit(); break;
  case qRESET: onReset(); break;
  case qEXIT:  onExit(); break;
  }
  if(Quit) {
    kill_sfx();
    stop_midi();
    clear(screen);
    }
  else {
    game_pal();
    midi_resume();
    resume_all_sfx();
    }

  rAbtn();
  rBbtn();
  rSbtn();
  if(key[KEY_ESC])
    key[KEY_ESC]=0;
}


void syskeys()
{
  poll_joystick();

  if(mouse_b && !mouse_down) System();
  mouse_down=mouse_b;

  if(ReadKey(KEY_ESC))   System();
  if(ReadKey(KEY_F1))    Vsync=!Vsync;
  if(ReadKey(KEY_F2))    ShowFPS=!ShowFPS;
  if(ReadKey(KEY_F3) && Playing)    Paused=!Paused;
  if(ReadKey(KEY_F4) && Playing)  { Paused=true; Advance=true; }
  if(ReadKey(KEY_F6))    f_Quit(qQUIT);
  if(ReadKey(KEY_F7))    f_Quit(qRESET);
  if(ReadKey(KEY_F8))    f_Quit(qEXIT);
  if(ReadKey(KEY_F12))   onSnapshot();

  if(debug_enabled && ReadKey(KEY_TAB))
    debug = !debug;

  if(!debug || !SystemKeys)
    return;

  if(ReadKey(KEY_D)) {
    details=!details;
    rectfill(screen,0,0,319,7,0);
    rectfill(screen,0,8,31,239,0);
    rectfill(screen,288,8,319,239,0);
    rectfill(screen,32,232,287,239,0);
    }

  if(ReadKey(KEY_W))   link.setClock((watch=true));
  if(ReadKey(KEY_V))   Vsync=!Vsync;
  if(ReadKey(KEY_P))   Paused=!Paused;
  if(ReadKey(KEY_A)) { Paused=true; Advance=true; }
  if(ReadKey(KEY_G))   db=(db==999)?0:999;

  if(ReadKey(KEY_F9))   Showpal=!Showpal;
  if(ReadKey(KEY_F10))  ;
  if(ReadKey(KEY_F11)) { toogam=!toogam; link.setClock(toogam); }

  if(ReadKey(KEY_PGUP)) master_volume(-1,midi_volume+8);
  if(ReadKey(KEY_PGDN)) master_volume(-1,midi_volume==255?248:midi_volume-8);
  if(ReadKey(KEY_HOME)) master_volume(digi_volume+8,-1);
  if(ReadKey(KEY_END))  master_volume(digi_volume==255?248:digi_volume-8,-1);

  if(ReadKey(KEY_PLUS_PAD))   game.life=min(game.life+1,game.maxlife);
  if(ReadKey(KEY_MINUS_PAD))  game.life=max(game.life-1,0);
  if(ReadKey(KEY_ASTERISK))   game.life=game.maxlife;

  if(ReadKey(KEY_I))   link.setClock(!link.getClock());
  if(ReadKey(KEY_S))   game.misc^=iSHIELD;
  if(ReadKey(KEY_0))   game.sword=0;
  if(ReadKey(KEY_1))   game.sword=1;
  if(ReadKey(KEY_2))   game.sword=2;
  if(ReadKey(KEY_3))   game.sword=3;
  if(ReadKey(KEY_4))   game.sword=4;
  if(ReadKey(KEY_5)) { game.ring=0; ringcolor(); }
  if(ReadKey(KEY_6)) { game.ring=1; ringcolor(); }
  if(ReadKey(KEY_7)) { game.ring=2; ringcolor(); }

  if(ReadKey(KEY_COMMA))  jukebox(currmidi-1);
  if(ReadKey(KEY_STOP))   jukebox(currmidi+1);
}


void advanceframe()
{
 while(Paused&&!Advance&&!Quit) {
   updatescr();   // to keep fps constant
   syskeys();     // have to call this, otherwise we'll get an infinite loop
   }
 if(Quit)
   return;

 Advance=false;
 frame++;
 updatescr();
 syskeys();
 sfx_cleanup();
}




void blackscr(int fcnt,bool showsubscr)
{
 while(fcnt>0) {
  clear(framebuf);
  if(showsubscr)
    putsubscr(framebuf,0,0);
  advanceframe();
  if(Quit)
    return;
  fcnt--;
  }
}


void openscreen()
{
 putscr(scrollbuf,0,0,&tmpscr[0]);
 int c=0, x=128;
 for(int i=0; i<80; i++) {
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  rectfill(framebuf,0,56,x,223,0);
  rectfill(framebuf,256-x,56,255,223,0);
  x--;
  switch(++c) {
   case 5: c=0;
   case 0:
   case 2:
   case 3: x--; break;
   }
  advanceframe();
  if(Quit)
    return;
  }
}


void ALLOFF()
{
 if(items.idCount(iPile))
   loadlvlpal(DMaps[currdmap].color);
 items.clear();
 guys.clear();
 Lwpns.clear();
 Ewpns.clear();
 link.resetflags(false);
 for(int i=0; i<4; i++)
   mblock[i].clk=0;
 msgstr=0;
 fadeclk=-1;
 drawguys=Udown=Ddown=Ldown=Rdown=Adown=Bdown=Sdown=true;
 if(watch)
   link.setClock(false);
 watch=freeze_guys=loaded_guys=loaded_enemies=wand_dead=blockpath=false;
 stop_sfx(WAV_BRANG);
 for(int i=0; i<176; i++)
   guygrid[i]=0;
 sle_clk=0;
}

fix LinkX()   { return link.getX(); }
fix LinkY()   { return link.getY(); }
int LinkDir() { return link.getDir(); }

void setClock(bool state) { link.setClock(state); }
void CatchBrang() { link.Catch(); }

int TriforceCount()
{
 int c=0;
 for(int i=1; i<=8; i++)
   if(game.lvlitems[i]&liTRIFORCE)
     c++;
 return c;
}


/****************************/
/**********  GUI  ***********/
/****************************/


int onContinue()
{
  return D_CLOSE;
}

int onEsc()
{
  return key[KEY_ESC]?D_CLOSE:D_O_K;
}

int onVsync()
{
  Vsync = !Vsync;
  return D_O_K;
}

int onNESquit()
{
  NESquit = !NESquit;
  return D_O_K;
}

int onShowFPS()
{
  ShowFPS = !ShowFPS;
  scare_mouse();

  if(ShowFPS)
    show_fps();
  else if(sbig)
    stretch_blit(fps_undo,screen,0,0,64,16,scrx+40-120,scry+216+96,128,32);
  else
    blit(fps_undo,screen,0,0,scrx+40,scry+216,64,16);
  if(Paused)
    show_paused();

  unscare_mouse();
  return D_O_K;
}



void kb_getkey(DIALOG *d)
{
 d->flags|=D_SELECTED;
 scare_mouse();
 d_button_proc(MSG_DRAW,d,0);
 unscare_mouse();
 clear_keybuf();
 int k = next_press_key();
 clear_keybuf();
 if(k>1 && k<123)
   *((int*)d->dp3) = k;
 d->flags&=~D_SELECTED;
 scare_mouse();
 d_button_proc(MSG_DRAW,d,0);
 unscare_mouse();
}


int d_kbutton_proc(int msg,DIALOG *d,int c)
{
 switch(msg) {
 case MSG_CHAR:
   if((c>>8)==KEY_ENTER) {
     kb_getkey(d);
     return D_REDRAW;
     }
   break;
 case MSG_CLICK:
   kb_getkey(d);
   while(mouse_b);
   return D_REDRAW;
 }
 d->flags&=~D_SELECTED;
 return d_button_proc(msg,d,c);
}

void j_getbtn(DIALOG *d)
{
 d->flags|=D_SELECTED;
 scare_mouse();
 d_button_proc(MSG_DRAW,d,0);
 unscare_mouse();
 int b = next_press_btn();
 if(b>=0)
   *((int*)d->dp3) = b;
 d->flags&=~D_SELECTED;
 scare_mouse();
 d_button_proc(MSG_DRAW,d,0);
 unscare_mouse();
}


int d_jbutton_proc(int msg,DIALOG *d,int c)
{
 switch(msg) {
 case MSG_CHAR:
   if((c>>8)==KEY_ENTER) {
     j_getbtn(d);
     return D_REDRAW;
     }
   break;
 case MSG_CLICK:
   j_getbtn(d);
   while(mouse_b);
   return D_REDRAW;
 }
 d->flags&=~D_SELECTED;
 return d_button_proc(msg,d,c);
}

const char *key_str[128] = {
"","esc","1","2","3","4","5","6","7","8","9","0",
"-","=","bkspc","tab","q","w","e","r","t","y","u","i","o",
"p","[","]","enter","l ctrl","a","s","d","f","g","h","j","k",
"l",";","\"","~","l shift","\\","z","x","c","v","b","n","m",
",",".","/","r shift","num *","l alt","space","caps lck","f1","f2","f3",
"f4","f5","f6","f7","f8","f9","f10","num lck","scrl lck","home","up",
"pgup","num -","left","num 5","right","num +","end","down","pgdown",
"ins","del","","","","f11","f12","","","","","r click","","","","","","",
"","","","","","","","","","",
"","","","","","","","","","",
"r ctrl","r alt","num /","","","","",""
};

char *pan_str[4] = { "MONO", " 1/2", " 3/4", "FULL" };

static char str_a[41],str_b[41],str_s[41];


int d_stringloader(int msg,DIALOG *d,int c)
{
 if(msg==MSG_DRAW) {
   if(d->w < 2) {
     sprintf(str_a,"%3d %s",d->w?Abtn:Akey,d->w?"":key_str[Akey]);
     sprintf(str_b,"%3d %s",d->w?Bbtn:Bkey,d->w?"":key_str[Bkey]);
     sprintf(str_s,"%3d %s",d->w?Sbtn:Skey,d->w?"":key_str[Skey]);
     }
   else {
     sprintf(str_a,"%3d",midi_volume);
     sprintf(str_b,"%3d",digi_volume);
     strcpy(str_s,pan_str[pan_style]);
     }
   }
 return D_O_K;
}


int set_vol(void *dp3, int d2)
{
 switch(((int*)dp3)[0]) {
 case 0:  midi_volume = min(d2<<3,255); break;
 case 1:  digi_volume = min(d2<<3,255); break;
 }
 scare_mouse();
 text_mode(gui_bg_color);
 textprintf(screen,font,((int*)dp3)[1],((int*)dp3)[2],vc(11),"%3d",min(d2<<3,255));
 unscare_mouse();
 return D_O_K;
}


int set_pan(void *dp3, int d2)
{
 pan_style = vbound(d2,0,3);
 scare_mouse();
 text_mode(gui_bg_color);
 textout(screen,font,pan_str[pan_style],((int*)dp3)[1],((int*)dp3)[2],vc(11));
 unscare_mouse();
 return D_O_K;
}


static DIALOG key_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_stringloader,      0,    0,    0 },
 { d_shadow_box_proc,   60,   56,   200,  140,  vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_ctext_proc,        160,  64,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "Keyboard Buttons" },
 { d_text_proc,         88,   82,   160,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Button  Code Key" },
 { d_text_proc,         160,  100,  60,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_a },
 { d_text_proc,         160,  120,  60,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_b },
 { d_text_proc,         160,  140,  60,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_s },
 { d_kbutton_proc,      80,   95,   60,   16,   vc(14),  vc(1),   0,       0,         0,        0,       "A",     NULL, &Akey},
 { d_kbutton_proc,      80,   115,  60,   16,   vc(14),  vc(1),   0,       0,         0,        0,       "B",     NULL, &Bkey},
 { d_kbutton_proc,      80,   135,  60,   16,   vc(14),  vc(1),   0,       0,         0,        0,       "Start", NULL, &Skey},
 { d_button_proc,       140,  166,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { NULL }
};


static DIALOG btn_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_stringloader,      0,    0,    1 },
 { d_shadow_box_proc,   60,   56,   200,  140,  vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_ctext_proc,        160,  64,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "Joystick Buttons" },
 { d_text_proc,         88,   82,   160,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Button  Joy. Button" },
 { d_text_proc,         160,  100,  60,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_a },
 { d_text_proc,         160,  120,  60,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_b },
 { d_text_proc,         160,  140,  60,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_s },
 { d_jbutton_proc,      80,   95,   60,   16,   vc(14),  vc(1),   0,       0,         0,        0,       "A",     NULL, &Abtn},
 { d_jbutton_proc,      80,   115,  60,   16,   vc(14),  vc(1),   0,       0,         0,        0,       "B",     NULL, &Bbtn},
 { d_jbutton_proc,      80,   135,  60,   16,   vc(14),  vc(1),   0,       0,         0,        0,       "Start", NULL, &Sbtn},
 { d_button_proc,       140,  166,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { NULL }
};


int midi_dp[3] = {0,132,104};
int digi_dp[3] = {1,132,120};
int pan_dp[3]  = {0,124,136};


static DIALOG sound_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)  (dp2)    (dp3) */
 { d_stringloader,      0,    0,    2 },
 { d_shadow_box_proc,   48,   80,   224,  96,   vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_ctext_proc,        160,  88,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "Sound Settings" },
 { d_text_proc,         132,  104,  40,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_a },
 { d_text_proc,         132,  120,  40,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_b },
 { d_text_proc,         124,  136,  40,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_s },
 { d_slider_proc,       164,  104,  96,   8,    vc(15),  vc(1),   0,       0,         32,       0,       NULL, set_vol, midi_dp },
 { d_slider_proc,       164,  120,  96,   8,    vc(15),  vc(1),   0,       0,         32,       0,       NULL, set_vol, digi_dp },
 { d_slider_proc,       164,  136,  96,   8,    vc(15),  vc(1),   0,       0,         3,        0,       NULL, set_pan, pan_dp },
 { d_text_proc,         60,   104,  48,   8,    vc(7),   vc(1),   0,       0,         0,        0,       "BG  Vol" },
 { d_text_proc,         60,   120,  48,   8,    vc(7),   vc(1),   0,       0,         0,        0,       "SFX Vol" },
 { d_text_proc,         60,   136,  48,   8,    vc(7),   vc(1),   0,       0,         0,        0,       "SFX Pan" },
 { d_button_proc,       140,  152,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { NULL }
};


static DIALOG about_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_shadow_box_proc,   72,   64,   176,  120,  vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_button_proc,       140,  158,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { d_ctext_proc,        160,  78,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "Zelda Classic" },
 { d_ctext_proc,        160,  86,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       ver_str },
 { d_text_proc,         88,   106,  140,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Coded by:" },
 { d_text_proc,         88,   114,  140,  8,    vc(11),  vc(1),   0,       0,         0,        0,       "  Phantom Menace" },
 { d_text_proc,         88,   126,  140,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Produced by:" },
 { d_text_proc,         88,   134,  140,  8,    vc(11),  vc(1),   0,       0,         0,        0,       "  Armageddon Games" },
 { NULL }
};


static DIALOG quest_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_shadow_box_proc,   72,   40,   176,  160,  vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_button_proc,       140,  176,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { d_ctext_proc,        160,  48,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "Quest Info" },
 { d_text_proc,         88,   64,   140,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Number:" },
 { d_text_proc,         152,  64,   24,   8,    vc(11),  vc(1),   0,       0,         0,        0,       str_a },
 { d_text_proc,         88,   80,   140,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Version:" },
 { d_text_proc,         160,  80,   64,   8,    vc(11),  vc(1),   0,       0,         0,        0,       QHeader.version },
 { d_text_proc,         88,   96,   144,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Title:" },
 { d_textbox_proc,      88,   105,  144,  24,   vc(11),  vc(1),   0,       0,         0,        0,       QHeader.title },
 { d_text_proc,         88,   136,  144,  8,    vc(7),   vc(1),   0,       0,         0,        0,       "Author:" },
 { d_textbox_proc,      88,   145,  144,  24,   vc(11),  vc(1),   0,       0,         0,        0,       QHeader.author },
 { NULL }
};


static DIALOG vidmode_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_shadow_box_proc,   80,   80,   160,  80,   vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_button_proc,       140,  134,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { d_ctext_proc,        160,  88,   160,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "Video Mode" },
 { d_ctext_proc,        160,  104,  160,  8,    vc(11),  vc(1),   0,       0,         0,        0,       str_a },
 { d_ctext_proc,        160,  112,  140,  8,    vc(11),  vc(1),   0,       0,         0,        0,       str_b },
 { NULL }
};


static DIALOG triforce_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 72,   64,   176,  104,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  72,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Triforce Pieces" },
// 2
 { d_check_proc,      116,  90,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "1" },
 { d_check_proc,      116,  100,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "2" },
 { d_check_proc,      116,  110,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "3" },
 { d_check_proc,      116,  120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "4" },
 { d_check_proc,      172,  90,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "5" },
 { d_check_proc,      172,  100,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "6" },
 { d_check_proc,      172,  110,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "7" },
 { d_check_proc,      172,  120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "8" },
// 10
 { d_button_proc,     90,   144,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  144,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};


static DIALOG equip_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   56,   208,  144,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  64,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Equipment" },
 { d_text_proc,       72,   80,   160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "Sword" },
 { d_text_proc,       72,   136,  160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "Ring" },
 { d_text_proc,       168,  80,   160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "Equip" },
// 5
 { d_radio_proc,      72,   88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "None" },
 { d_radio_proc,      72,   96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Wooden" },
 { d_radio_proc,      72,   104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "White" },
 { d_radio_proc,      72,   112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Magic" },
 { d_radio_proc,      72,   120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Excalibur" },
// 10
 { d_radio_proc,      72,   144,  160,  8,    vc(14),  vc(1),  0,       0,          1,             0,       "None" },
 { d_radio_proc,      72,   152,  160,  8,    vc(14),  vc(1),  0,       0,          1,             0,       "Blue" },
 { d_radio_proc,      72,   160,  160,  8,    vc(14),  vc(1),  0,       0,          1,             0,       "Red" },
// 13
 { d_check_proc,      168,  88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Shield  " },
 { d_check_proc,      168,  96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Bow     " },
 { d_check_proc,      168,  104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Raft    " },
 { d_check_proc,      168,  112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Ladder  " },
 { d_check_proc,      168,  120,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Book    " },
 { d_check_proc,      168,  128,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Key     " },
 { d_check_proc,      168,  136,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Bracelet" },
 { d_check_proc,      168,  144,  160,  8,    vc(14),  vc(1),  0,       D_DISABLED, 0,             0,       "MirrShld" },
// 21
 { d_button_proc,     90,   176,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  176,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};


static DIALOG items_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 56,   56,   208,  160,  vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  64,   128,  8,    vc(15),  vc(1),  0,       0,          0,             0,       "Items" },
 { d_text_proc,       72,   80,   160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "B-rang" },
 { d_text_proc,       72,   128,  160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "Candle" },
 { d_text_proc,       168,  80,   160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "Arrows" },
 { d_text_proc,       168,  128,  160,  8,    vc(11),  vc(1),  0,       0,          0,             0,       "Potion" },
// 6
 { d_radio_proc,      72,   88,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "None" },
 { d_radio_proc,      72,   96,   160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Wooden" },
 { d_radio_proc,      72,   104,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Magic" },
 { d_radio_proc,      72,   112,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Fire" },
// 10
 { d_radio_proc,      72,   136,  160,  8,    vc(14),  vc(1),  0,       0,          1,             0,       "None" },
 { d_radio_proc,      72,   144,  160,  8,    vc(14),  vc(1),  0,       0,          1,             0,       "Blue" },
 { d_radio_proc,      72,   152,  160,  8,    vc(14),  vc(1),  0,       0,          1,             0,       "Red" },
// 13
 { d_radio_proc,      168,  88,   160,  8,    vc(14),  vc(1),  0,       0,          2,             0,       "None" },
 { d_radio_proc,      168,  96,   160,  8,    vc(14),  vc(1),  0,       0,          2,             0,       "Wooden" },
 { d_radio_proc,      168,  104,  160,  8,    vc(14),  vc(1),  0,       0,          2,             0,       "Silver" },
// 16
 { d_radio_proc,      168,  136,  160,  8,    vc(14),  vc(1),  0,       0,          3,             0,       "None" },
 { d_radio_proc,      168,  144,  160,  8,    vc(14),  vc(1),  0,       0,          3,             0,       "Blue" },
 { d_radio_proc,      168,  152,  160,  8,    vc(14),  vc(1),  0,       0,          3,             0,       "Red" },
// 19
 { d_check_proc,      72,   168,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Whistle " },
 { d_check_proc,      72,   176,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Bait    " },
 { d_check_proc,      168,  168,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Letter  " },
 { d_check_proc,      168,  176,  160,  8,    vc(14),  vc(1),  0,       0,          0,             0,       "Wand    " },
// 23
 { d_button_proc,     90,   192,  60,   16,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0,       "O&K" },
 { d_button_proc,     170,  192,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};



static DIALOG credits_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_shadow_box_proc,   40,   48,   240,  152,  vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_box_proc,          48,   56,   223,  111,  vc(15),  vc(1),   0,       0,         0,        0,       NULL },
 { d_bitmap_proc,       49,   57,   222,  110,  vc(15),  vc(1),   0,       0,         0,        0,       NULL },
 { d_button_proc,       140,  176,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { NULL }
};



int onCredits()
{
  go();

  BITMAP *win = create_bitmap(222,110);
  if(!win)
    return D_O_K;

  int c=0;
  int l=0;
  int ol=-1;
  RLE_SPRITE *rle = (RLE_SPRITE*)(data[RLE_CREDITS].dat);
  RGB *pal = (RGB*)(data[PAL_CREDITS].dat);
  PALETTE tmppal;

  clear(win);
  draw_rle_sprite(win,rle,0,0);
  credits_dlg[1].fg = gui_mg_color;
  credits_dlg[2].dp = win;
  set_palette_range(black_palette,0,127,false);

  DIALOG_PLAYER *p = init_dialog(credits_dlg,-1);

  while(update_dialog(p))
  {
    c++;
    l = max((c>>1)-30,0);

    if(l > rle->h)
      l = c = 0;
    if(l > rle->h - 112)
      l = rle->h - 112;

    clear(win);
    draw_rle_sprite(win,rle,0,0-l);

    if(c<=64)
      fade_interpolate(black_palette,pal,tmppal,c,0,127);

    if(Vsync)
      vsync();
    else
      while(!myvsync);
    myvsync=0;
    if(c<=64)
      set_palette_range(tmppal,0,127,false);
    if(l!=ol)
    {
      scare_mouse();
      d_bitmap_proc(MSG_DRAW,credits_dlg+2,0);
      unscare_mouse();
      SCRFIX();
      ol=l;
    }
  }

  shutdown_dialog(p);
  destroy_bitmap(win);
  comeback();
  return D_O_K;
}


char *midilist(int index, int *list_size)
{
  if(index<0)
  {
    *list_size=0;
    for(int i=0; i<MAXMUSIC; i++)
      if(tunes[i].midi)
        (*list_size)++;
    return NULL;
  }

  int i=0,m=0;
  while(m<=index && i<=MAXMUSIC)
  {
    if(tunes[i].midi)
      m++;
    i++;
  }
  i--;
  if(i==MAXMUSIC && m<index)
    return "(null)";
  return tunes[i].title;
}


///// MIDI info stuff ....

static char *text;
static midi_info *mi;

void get_info(int index);

int d_midilist_proc(int msg,DIALOG *d,int c)
{
  int d2 = d->d2;
  int ret = d_droplist_proc(msg,d,c);
  if(d2!=d->d2)
    get_info(d->d2);
  return ret;
}


static DIALOG midi_dlg[] =
{
 /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)    (d1)      (d2)     (dp)     (dp2) (dp3) */
 { d_shadow_box_proc,   8,    40,   304,  168,  vc(14),  vc(1),   0,       0,         0,        0,       NULL },
 { d_ctext_proc,        160,  48,   240,  8,    vc(15),  vc(1),   0,       0,         0,        0,       "MIDI Info" },
 { d_text_proc,         48,   64,   40,   8,    vc(7),   vc(1),   0,       0,         0,        0,       "Tune:" },
 { d_midilist_proc,     96,   62,   176,  11,   vc(14),  vc(1),   0,       0,         0,        0,       midilist },
 { d_textbox_proc,      16,   80,   287,  96,   vc(11),  vc(1),   0,       0,         0,        0,       NULL },
 { d_button_proc,       140,  184,  40,   16,   vc(14),  vc(1),   0,       D_EXIT,    0,        0,       "OK" },
 { NULL }
};


bool dialog_running;

void get_info(int index)
{
  int i=0,m=0;
  while(m<=index && i<=MAXMUSIC)
  {
    if(tunes[i].midi)
      m++;
    i++;
  }
  i--;

  if(i==MAXMUSIC && m<index)
    strcpy(text,"(null)");
  else
  {
    get_midi_info(tunes[i].midi,mi);
    get_midi_text(tunes[i].midi,mi,text);
  }

  midi_dlg[4].dp = text;
  midi_dlg[4].d1 = midi_dlg[4].d2 = 0;

  if(dialog_running)
  {
    scare_mouse();
    d_textbox_proc(MSG_DRAW,midi_dlg+4,0);
    unscare_mouse();
  }
}


int onMIDICredits()
{
  text = (char*)malloc(4096);
  mi = (midi_info*)malloc(sizeof(midi_info));

  if(!text || !mi)
  {
    alert("Not enough memory",NULL,NULL,"OK",NULL,13,27);
    return D_O_K;
  }

  midi_dlg[3].d1 =
  midi_dlg[3].d2 = 0;
  dialog_running=false;
  get_info(0);

  dialog_running=true;
  PopUp_dialog(midi_dlg,-1);
  dialog_running=false;

  free(text);
  free(mi);
  return D_O_K;
}



void fix_dialog(DIALOG *d)
{
  for( ; d->proc != NULL; d++)
  {
    d->x += scrx;
    d->y += scry;
  }
}


int PopUp_dialog(DIALOG *d,int f)
{
  go();
  int ret=do_dialog(d,f);
  comeback();
  return ret;
}


int onAbout()
{
  VerStr(ZELDA_VERSION);
  PopUp_dialog(about_dlg,-1);
  return D_O_K;
}


int onQuest()
{
  if(QHeader.quest_number==0)
    sprintf(str_a,"Custom");
  else
    sprintf(str_a,"%d",QHeader.quest_number);
  quest_dlg[6].d2 = quest_dlg[8].d2 = 0;
  PopUp_dialog(quest_dlg,-1);
  return D_O_K;
}


int onVidMode()
{
  switch(VidMode) {
  case GFX_MODEX:  sprintf(str_a,"VGA Mode X"); break;
  case GFX_VESA1:  sprintf(str_a,"VESA 1.x"); break;
  case GFX_VESA2B: sprintf(str_a,"VESA2 Banked"); break;
  case GFX_VESA2L: sprintf(str_a,"VESA2 Linear"); break;
  case GFX_VESA3:  sprintf(str_a,"VESA3"); break;
  default:         sprintf(str_a,"Unknown... ?"); break;
  }
  sprintf(str_b,"%dx%d 8-bit",resx,resy);
  PopUp_dialog(vidmode_dlg,-1);
  return D_O_K;
}


int onKeyboard()
{
  PopUp_dialog(key_dlg,-1);
  return D_O_K;
}

int onJoystick()
{
  PopUp_dialog(btn_dlg,-1);
  return D_O_K;
}

int onSound()
{
  pan_style = vbound(pan_style,0,3);
  sound_dlg[6].d2 = midi_volume==255?32:midi_volume>>3;
  sound_dlg[7].d2 = digi_volume==255?32:digi_volume>>3;
  sound_dlg[8].d2 = pan_style;
  PopUp_dialog(sound_dlg,-1);
  master_volume(digi_volume,midi_volume);
  return D_O_K;
}

int onQuit()
{
  if(Playing && alert("Quit current game?",NULL,NULL,"&Yes","&No",'y','n')==1) {
    Quit=qQUIT;
    return D_CLOSE;
  }
  return D_O_K;
}

int onReset()
{
  if(alert("  Reset system?  ",NULL,NULL,"&Yes","&No",'y','n')==1) {
    Quit=qRESET;
    return D_CLOSE;
  }
  return D_O_K;
}

int onExit()
{
  if(alert("  Exit to DOS?  ",NULL,NULL,"&Yes","&No",'y','n')==1) {
    Quit=qEXIT;
    return D_CLOSE;
  }
  return D_O_K;
}

int onSave()
{
  if(Playing) {
    saves[currgame] = game;
    alert("  Game saved.  ",NULL,NULL,"OK",NULL,13,27);
    }
  return D_O_K;
}


int onTitle_NES() { DXtitle=false; return D_O_K; }
int onTitle_DX() { DXtitle=true; return D_O_K; }

int onDebug()
{
  if(debug_enabled)
    debug=!debug;
  return D_O_K;
}

int onTriforce()
{
 for(int i=1; i<=8; i++)
   triforce_dlg[i+1].flags = (game.lvlitems[i] & liTRIFORCE) ? D_SELECTED : 0;

 if(PopUp_dialog(triforce_dlg,-1)==10) {
   for(int i=1; i<=8; i++) {
     game.lvlitems[i] &= ~liTRIFORCE;
     game.lvlitems[i] |= (triforce_dlg[i+1].flags & D_SELECTED) ? liTRIFORCE : 0;
     }
   }
 return D_O_K;
}

bool rc = false;

int onEquipment()
{
 for(int i=0; i<5; i++)
   equip_dlg[i+5].flags = (game.sword == i) ? D_SELECTED : 0;
 for(int i=0; i<3; i++)
   equip_dlg[i+10].flags = (game.ring == i) ? D_SELECTED : 0;
 for(int i=0; i<8; i++)
   equip_dlg[i+13].flags = (game.misc&(1<<i)) ? D_SELECTED : 0;

 if(PopUp_dialog(equip_dlg,-1)==21) {
   for(int i=0; i<5; i++)
     if(equip_dlg[i+5].flags & D_SELECTED)
       game.sword=i;
   for(int i=0; i<3; i++)
     if(equip_dlg[i+10].flags & D_SELECTED)
       game.ring=i;
   for(int i=0; i<8; i++)
     if(equip_dlg[i+13].flags & D_SELECTED)
       game.misc |= (1<<i);
     else
       game.misc &= ~(1<<i);
   rc=true;
   }
 return D_O_K;
}


int onItems()
{
 for(int i=0; i<4; i++)
   items_dlg[i+6].flags = (game.brang == i) ? D_SELECTED : 0;
 for(int i=0; i<3; i++)
   items_dlg[i+10].flags = (game.candle == i) ? D_SELECTED : 0;
 for(int i=0; i<4; i++)
   items_dlg[i+13].flags = (game.arrow == i) ? D_SELECTED : 0;
 for(int i=0; i<3; i++)
   items_dlg[i+16].flags = (game.potion == i) ? D_SELECTED : 0;

 items_dlg[19].flags = (game.whistle) ? D_SELECTED : 0;
 items_dlg[20].flags = (game.bait) ? D_SELECTED : 0;
 items_dlg[21].flags = (game.letter) ? D_SELECTED : 0;
 items_dlg[22].flags = (game.wand) ? D_SELECTED : 0;

 if(PopUp_dialog(items_dlg,-1)==23) {
   for(int i=0; i<4; i++)
     if(items_dlg[i+6].flags & D_SELECTED)
       game.brang=i;
   for(int i=0; i<3; i++)
     if(items_dlg[i+10].flags & D_SELECTED)
       game.candle=i;
   for(int i=0; i<3; i++)
     if(items_dlg[i+13].flags & D_SELECTED)
       game.arrow=i;
   for(int i=0; i<3; i++)
     if(items_dlg[i+16].flags & D_SELECTED)
       game.potion=i;

   game.whistle = items_dlg[19].flags & D_SELECTED ? 1:0;
   game.bait    = items_dlg[20].flags & D_SELECTED ? 1:0;
   game.letter  = items_dlg[21].flags & D_SELECTED ? 1:0;
   game.wand    = items_dlg[22].flags & D_SELECTED ? 1:0;
   }

 return D_O_K;
}



static DIALOG getnum_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 80,   80,   160,  64,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,   152,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
 { d_text_proc,       112,  104,  48,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Value:" },
 { d_edit_proc,       168,  104,  48,   8,    vc(12),  vc(1),  0,       0,          6,             0,       NULL },
 { d_button_proc,     90,   120,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  120,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};


int getnumber(char *prompt,int initialval)
{
 char buf[20];
 sprintf(buf,"%d",initialval);
 getnum_dlg[1].dp=prompt;
 getnum_dlg[3].dp=buf;
 if(popup_dialog(getnum_dlg,3)==4)
   return atoi(buf);
 return initialval;
}


int onLife()
{
 game.life = vbound(getnumber("Life",game.life),0,game.maxlife);
 return D_O_K;
}

int onHeartC()
{
 game.maxlife = vbound(getnumber("Heart Containers",game.maxlife>>3),3,16) << 3;
 game.life = vbound(game.life,0,game.maxlife);
 return D_O_K;
}

int onRupies()
{
 game.rupies = vbound(getnumber("Rupies",game.rupies),0,255);
 return D_O_K;
}

int onMaxBombs()
{
 game.maxbombs = vbound(getnumber("Max Bombs",game.maxbombs),8,99);
 game.bombs = game.maxbombs;
 return D_O_K;
}


int onRefill() { game.life = game.maxlife; return D_O_K; }
int onClock()  { link.setClock(!link.getClock()); return D_O_K; }


int onQstPath()
{
  chop_path(qstpath);
  go();
  file_select("Zelda Classic Quest Files (.qst)",qstpath,"qst");
  comeback();
  chop_path(qstpath);
  return D_O_K;
}

static MENU game_menu[] =
{ 
   { "&Continue\tESC",       onContinue,  NULL },
   { "" },
   { "&Quit\tF6",            onQuit,      NULL },
   { "" },
   { "&Reset\tF7",           onReset,     NULL },
   { "E&xit\tF8",            onExit,      NULL },
   { NULL }
};


static MENU title_menu[] =
{ 
   { "&Original",            onTitle_NES, NULL },
   { "&Zelda Classic",       onTitle_DX,  NULL },
   { NULL }
};


static MENU settings_menu[] =
{
   { "&Keyboard...",         onKeyboard,  NULL },
   { "&Joystick...",         onJoystick,  NULL },
   { "&Sound...",            onSound,     NULL },
   { "&Title Screen\t\x86",  NULL,        title_menu },
   { "" },
   { "&Vsync\tF1",           onVsync,     NULL },
   { "Show &FPS\tF2",        onShowFPS,   NULL },
   { "NES &Quit",            onNESquit,   NULL },
   { NULL }
};


static MENU misc_menu[] =
{ 
   { "&About...",            onAbout,     NULL },
   { "&Credits...",          onCredits,   NULL },
   { "&MIDI Info...",        onMIDICredits, NULL },
   { "&Quest Info...",       onQuest,     NULL },
   { "&Video Mode...",       onVidMode,   NULL },
   { "" },
   { "Quest &Dir...",        onQstPath,   NULL },
   { "" },
   { "Take &Snapshot\tF12",  onSnapshot,  NULL },
   { NULL }
};


static MENU debug_menu[] =
{ 
   { "&Debug\tTAB",          onDebug,     NULL },
   { "" },
   { "Refill\t[*]",          onRefill,    NULL },
   { "Clock\t[i]",           onClock,     NULL },
   { "" },
   { "Equipment...",         onEquipment, NULL },
   { "Items...",             onItems,     NULL },
   { "Triforce...",          onTriforce,  NULL },
   { "" },
   { "Life...",              onLife,      NULL },
   { "Heart Containers...",  onHeartC,    NULL },
   { "Rupies...",            onRupies,    NULL },
   { "Max Bombs...",         onMaxBombs,  NULL },
   { NULL }
};


static MENU the_menu[] =
{ 
   { "&Game",                NULL,        game_menu },
   { "&Settings",            NULL,        settings_menu },
   { "&Debug",               NULL,        debug_menu },
   { "&Misc",                NULL,        misc_menu },
   { NULL }
};


static DIALOG system_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key)    (flags)  (d1)      (d2)     (dp) */
 { d_menu_proc,       0,    0,    0,    0,    0,    0,    0,       0,       0,        0,       the_menu },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       onVsync },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F2,   0,       onShowFPS },
// { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F5,   0,       onSave },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F6,   0,       onQuit },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F7,   0,       onReset },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F8,   0,       onExit },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F12,  0,       onSnapshot },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_ESC,  0,       onEsc },
 { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_TAB,  0,       onDebug },
 { NULL }
};


void color_layer(RGB *src,RGB *dest,char r,char g,char b,char pos,int from,int to)
{
  PALETTE tmp;
  for(int i=0; i<256; i++) {
    tmp[i].r=r;
    tmp[i].g=g;
    tmp[i].b=b;
  }
  fade_interpolate(src,tmp,dest,pos,from,to);
}


void system_pal()
{
  PALETTE pal;
  copy_pal((RGB*)data[PAL_GUI].dat,pal);

  // set up the new palette
  for(int i=128; i<192; i++) {
    pal[i].r = i-128;
    pal[i].g = i-128;
    pal[i].b = i-128;
    }

  gui_fg_color=vc(14);
  gui_bg_color=vc(1);
  gui_mg_color=vc(9);

  for(int i=0; i<240; i+=2) {
    int v = (i>>3)+2;
    int c = (i>>3)+192;
    pal[c] = _RGB(v,v,v+(v>>1));
    hline(tmp_scr,0,i,319,c);
    hline(tmp_scr,0,i+1,319,c);
  }

  // gray scale the current frame
  for(int y=0; y<224; y++)
    for(int x=0; x<256; x++) {
      int c = framebuf->line[y][x];
      int gray = min(int(RAMpal[c].r*0.3 + RAMpal[c].g*0.5 + RAMpal[c].b*0.2),63);
      tmp_scr->line[y+8][x+32] = gray+128;
      }
  // save the fps_undo section
  blit(tmp_scr,fps_undo,40,216,0,0,64,16);

  // display everything
  pal[vc(1)]  = _RGB(0x00,0x00,0x14);
  pal[vc(7)]  = _RGB(0x20,0x20,0x20);
  pal[vc(9)]  = _RGB(0x20,0x20,0x24);
  pal[vc(11)] = _RGB(0x30,0x30,0x30);
  pal[vc(14)] = _RGB(0x3F,0x38,0x28);
  vsync();
  color_layer(pal, pal, 24,16,16, 28, 128,191);
  set_palette_range(pal,0,255,false);
  if(sbig)
    stretch_blit(tmp_scr,screen,0,0,320,240,scrx-160,scry-120,640,480);
  else
    blit(tmp_scr,screen,0,0,scrx,scry,320,240);
  if(ShowFPS)
    show_fps();
  if(Paused)
    show_paused();
}

void game_pal()
{
  clear(tmp_scr);
  blit(framebuf,tmp_scr,0,0,32,8,256,224);
  vsync();
  if(sbig)
    stretch_blit(tmp_scr,screen,0,0,320,240,scrx-160,scry-120,640,480);
  else
    blit(tmp_scr,screen,0,0,scrx,scry,320,240);
  set_palette_range(RAMpal,0,255,false);
}

static char bar_str[] = "";

void System()
{
  mouse_down=mouse_b;
  midi_pause();
  pause_all_sfx();

  system_pal();

  misc_menu[3].flags =
  game_menu[2].flags = Playing ? 0 : D_DISABLED;
  misc_menu[6].flags = !Playing ? 0 : D_DISABLED;

  clear_keybuf();
  show_mouse(screen);

  DIALOG_PLAYER *p=init_dialog(system_dlg,-1);
  do {
    if(mouse_down && !mouse_b)
      mouse_down=0;

    title_menu[0].flags = DXtitle ? 0 : D_SELECTED;
    title_menu[1].flags = title_menu[0].flags ^ D_SELECTED;

    settings_menu[5].flags = Vsync?D_SELECTED:0;
    settings_menu[6].flags = ShowFPS?D_SELECTED:0;
    settings_menu[7].flags = NESquit?D_SELECTED:0;

    if(debug_enabled) {
      debug_menu[0].flags = debug?D_SELECTED:0;
      debug_menu[1].text = (Playing && debug) ? bar_str : NULL;
      debug_menu[3].flags = link.getClock()?D_SELECTED:0;
      }
    if(mouse_b && !mouse_down)
      break;
  } while(update_dialog(p));
  mouse_down=mouse_b;
  shutdown_dialog(p);
  show_mouse(NULL);

  if(Quit) {
    kill_sfx();
    stop_midi();
    clear(screen);
    }
  else {
    game_pal();
    midi_resume();
    resume_all_sfx();
    if(rc)
      ringcolor();
    }

  rc=false;
  clear_keybuf();
  text_mode(0);
}


/*****************************/
/**** Custom Sound System ****/
/*****************************/

/*

#define MAXMUSIC MUSIC_COUNT+MAXMIDIS

music tunes[MAXMUSIC] = {
 // (title)             (s) (ls) (le) (l) (vol) (midi)
 { "Z1 Dungeon",         0,  -1,  -1,  1,  168,  NULL },
 { "Z1 Ending",          0,  129, 225, 1,  132,  NULL },
 { "Z1 Game Over",       0,  -1,  -1,  1,  144,  NULL },
 { "Z1 Level 9",         0,  -1,  -1,  1,  255,  NULL },
 { "Z1 Overworld",       0,  17,  -1,  1,  200,  NULL },
 { "Z1 Title",           0,  -1,  -1,  0,  160,  NULL },
 { "Z1 Triforce",        0,  -1,  -1,  0,  160,  NULL },

// { "Z2 Great Palace",    0,  14,  94,  1,  144,  NULL },
// { "Z3 Ganon",           4,  16,  -1,  1,  152,  NULL }
// { "Z5 Gerudo Valley",   4, 349,  -1,  1,  144,  NULL },

};

*/

inline int mixvol(int v1,int v2)
{
  return (min(v1,255)*min(v2,255)) >> 8;
}


void jukebox(int index,int loop)
{
 stop_midi();
 if(index<0)         index=MAXMUSIC-1;
 if(index>=MAXMUSIC) index=0;

 stop_midi();
 set_volume(-1, mixvol(tunes[index].volume,midi_volume));
 play_midi(tunes[index].midi,loop);
 if(tunes[index].start>0)
   midi_seek(tunes[index].start);

 midi_loop_end = tunes[index].loop_end;
 midi_loop_start = tunes[index].loop_start;

 if(details) {
   rectfill(screen,0,232,319,239,0);
   textprintf(screen,font,0,232,2,"%d - %s",index+1,tunes[index].title);
   }

 currmidi=index;
}



void jukebox(int index)
{
 if(index<0)         index=MAXMUSIC-1;
 if(index>=MAXMUSIC) index=0;

 if(index==currmidi && midi_pos>=0) // do nothing if it's already playing
   return;

 jukebox(index,tunes[index].loop);
}



void play_DmapMidi()
{
  int m=DMaps[currdmap].midi;
  switch(m) {
  case 1: jukebox(MUSIC_OVERWORLD); break;
  case 2: jukebox(MUSIC_DUNGEON); break;
  case 3: jukebox(MUSIC_LEVEL9); break;
  default:
    if(m>=4 && m<4+MAXMIDIS)
      jukebox(m-4+MUSIC_COUNT);
    else
      stop_midi();
  }
}



void master_volume(int dv,int mv)
{
 if(dv>=0) digi_volume=max(min(dv,255),0);
 if(mv>=0) midi_volume=max(min(mv,255),0);
 int i = min(max(currmidi,0),MAXMUSIC-1);
 set_volume(digi_volume,mixvol(tunes[i].volume,midi_volume));
 if(details) {
   textprintf(screen,font,0,200,vc(1),"%3d",digi_volume);
   textprintf(screen,font,0,208,vc(1),"%3d",midi_volume);
   }
}



/*****************/
/*****  SFX  *****/
/*****************/


// array of voices, one for each sfx sample in the data file
// 0+ = voice #
// -1 = voice not allocated
int sfx_voice[WAV_COUNT];


// returns number of voices currently allocated
int sfx_count()
{
 int c=0;
 for(int i=0; i<WAV_COUNT; i++)
   if(sfx_voice[i]!=-1)
     c++;
 return c;
}


// clean up finished samples
void sfx_cleanup()
{
 for(int i=0; i<WAV_COUNT; i++)
   if(sfx_voice[i]!=-1 && voice_get_position(sfx_voice[i])<0) {
     deallocate_voice(sfx_voice[i]);
     sfx_voice[i]=-1;
     }
}


// allocates a voice for the sample "wav_index" (index into zelda.dat)
// if a voice is already allocated (and/or playing), then it just returns true
// Returns true:  voice is allocated
//         false: unsuccessful
bool sfx_init(int index)
{
 // check index
 if(index<0 || index>=WAV_COUNT)
   return false;

 if(sfx_voice[index]==-1) {
   // allocate voice
   if(index!=WAV_REFILL)
     sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[index].dat);
   else
     sfx_voice[index]=allocate_voice(&wav_refill);
   }

 return sfx_voice[index] != -1;
}




// plays an sfx sample
void sfx(int index,int pan,bool loop)
{
 if(!sfx_init(index))
   return;

 voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
 voice_set_pan(sfx_voice[index],pan);

 int pos=voice_get_position(sfx_voice[index]);
 voice_set_position(sfx_voice[index],index==WAV_REFILL?wav_refill.loop_start:0);
 if(pos<=0)
   voice_start(sfx_voice[index]);
}



// start it (in loop mode) if it's not already playing,
// otherwise just leave it in its current position
void cont_sfx(int index)
{
 if(!sfx_init(index))
   return;

 if(voice_get_position(sfx_voice[index])<=0) {
   voice_set_position(sfx_voice[index],0);
   voice_set_playmode(sfx_voice[index],PLAYMODE_LOOP);
   voice_start(sfx_voice[index]);
   }
}


// adjust parameters while playing
void adjust_sfx(int index,int pan,bool loop)
{
 if(index<0 || index>=WAV_COUNT || sfx_voice[index]==-1)
   return;

 voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
 voice_set_pan(sfx_voice[index],pan);
}


// pauses a voice
void pause_sfx(int index)
{
 if(index>=0 && index<WAV_COUNT && sfx_voice[index]!=-1)
   voice_stop(sfx_voice[index]);
}


// resumes a voice
void resume_sfx(int index)
{
 if(index>=0 && index<WAV_COUNT && sfx_voice[index]!=-1)
   voice_start(sfx_voice[index]);
}


// pauses all active voices
void pause_all_sfx()
{
 for(int i=0; i<WAV_COUNT; i++)
   if(sfx_voice[i]!=-1)
     voice_stop(sfx_voice[i]);
}

// resumes all paused voices
void resume_all_sfx()
{
 for(int i=0; i<WAV_COUNT; i++)
   if(sfx_voice[i]!=-1)
     voice_start(sfx_voice[i]);
}


// stops an sfx and deallocates the voice
void stop_sfx(int index)
{
 if(index<0 || index>=WAV_COUNT)
   return;

 if(sfx_voice[index]!=-1) {
   deallocate_voice(sfx_voice[index]);
   sfx_voice[index]=-1;
   }
}


void kill_sfx()
{
 for(int i=0; i<WAV_COUNT; i++)
   if(sfx_voice[i]!=-1) {
     deallocate_voice(sfx_voice[i]);
     sfx_voice[i]=-1;
     }
}


int pan(int x)
{
 switch(pan_style)
 {
 case 0: return 128;
 case 1: return vbound((x>>1)+68,0,255);
 case 2: return vbound(((x*3)>>2)+36,0,255);
 }
 return vbound(x,0,255);
}



/*******************************/
/******* Input Handlers ********/
/*******************************/


bool joybtn(int b)
{
 switch(b) {
   case 1: return joy_b1;
   case 2: return joy_b2;
   case 3: return joy_b3;
   case 4: return joy_b4;
   case 5: return joy_b5;
   case 6: return joy_b6;
   case 7: return joy_b7;
   case 8: return joy_b8;
   }
 return false;
}


int next_press_key()
{
 char k[128];
 for(int i=0; i<128; i++)
   k[i]=key[i];
 do {
   for(int i=0; i<128; i++)
     if(key[i]!=k[i])
       return i;
 } while(1);
}


int next_press_btn()
{
 clear_keybuf();
 bool b[9];
 for(int i=1; i<=8; i++)
   b[i]=joybtn(i);
 do {
   poll_joystick();
   if(keypressed()) {
     switch(readkey()>>8) {
     case KEY_ESC:   return -1;
     case KEY_SPACE: return 0;
     }
   }
   for(int i=1; i<=8; i++)
     if(joybtn(i)!=b[i])
       return i;
 } while(1);
}


inline bool Up()    { return key[KEY_UP]||joy_up; }
inline bool Down()  { return key[KEY_DOWN]||joy_down; }
inline bool Left()  { return key[KEY_LEFT]||joy_left; }
inline bool Right() { return key[KEY_RIGHT]||joy_right; }
inline bool cAbtn()  { return key[Akey]||joybtn(Abtn); }
inline bool cBbtn()  { return key[Bkey]||joybtn(Bbtn); }
inline bool cSbtn()  { return key[Skey]||joybtn(Sbtn); }

bool rButton(bool(proc)(),bool &flag)
{
 if(!proc())
   flag=false;
 else if(!flag) {
   flag=true;
   return true;
   }
 return false;
}

inline bool rUp()    { return rButton(Up,Udown); }
inline bool rDown()  { return rButton(Down,Ddown); }
inline bool rLeft()  { return rButton(Left,Ldown); }
inline bool rRight() { return rButton(Right,Rdown); }
inline bool rAbtn()  { return rButton(cAbtn,Adown); }
inline bool rBbtn()  { return rButton(cBbtn,Bdown); }
inline bool rSbtn()  { return rButton(cSbtn,Sdown); }


bool ReadKey(int k)
{
 if(key[k]) {
  key[k]=0;
  return true;
  }
 return false;
}



/**************************/
/***** Main Game Code *****/
/**************************/



int init_game()
{
  srand(frame);

  // copy saved data to RAM data
  game = saves[currgame];

  // load quest
  chop_path(qstpath);

  int ret;
  if(game.quest<255) {
    sprintf(qstpath,"%s%s.qst",qstpath,ordinal(game.quest));
    ret = loadquest(qstpath,&QHeader,&QMisc,tunes+MUSIC_COUNT);
    }
  else {
    sprintf(qstpath,"%s%s",qstpath,get_filename(game.qstpath));
    ret = loadquest(qstpath,&QHeader,&QMisc,tunes+MUSIC_COUNT);
    }

  if(!game.title[0] || !game.hasplayed)
  {
    strcpy(game.version,QHeader.version);
    strcpy(game.title,QHeader.title);
  }
  else
  {
    if(!ret && strcmp(game.title,QHeader.title))
      ret = qe_match;
  }

  if(ret) {
    system_pal();
    char buf1[40],buf2[40];

    sprintf(buf1,"Error loading %s:",get_filename(qstpath));
    sprintf(buf2,"%s",qst_error[ret]);
    alert(buf1,buf2,qstpath,"OK",NULL,13,27);

    Quit=qERROR;
    return 1;
    }

  homescr=currscr=DMaps[0].cont;
  currdmap=warpscr=worldscr=0;
  currmap=DMaps[0].map;
  dlevel=DMaps[0].level;
  sle_x=sle_y=newscr_clk=opendoors=Bwpn=Bpos=0;
  fadeclk=-1;

  for(int i=0; i<6; i++)
    visited[i]=-1;
  game.lvlitems[9]&=~liBOSS;

  ALLOFF();
  whistleclk=-1;
  darkroom=false;
  loadscr(0,currscr,up);
  putscr(scrollbuf,0,0,&tmpscr[0]);
  link.init();
  link.resetflags(true);

  copy_pal((RGB*)data[PAL_GUI].dat,RAMpal);
  loadfullpal();
  loadlvlpal(DMaps[0].color);

  game.hasplayed=1;
  game.life=3*8;
  selectBwpn(0);

  ringcolor();
  for(int i=0; i<128; i++)
    key[i]=0;

  Playing=true;
  openscreen();
  if(!Quit)
    play_DmapMidi();
  return 0;
}



int cont_game()
{
  homescr = currscr = (dlevel) ? lastentrance : DMaps[currdmap].cont;

  for(int i=0; i<6; i++)
    visited[i]=-1;
  if(dlevel==0)
    game.lvlitems[9]&=~liBOSS;

  ALLOFF();
  whistleclk=-1;
  darkroom=false;
  loadscr(0,currscr,up);
  putscr(scrollbuf,0,0,&tmpscr[0]);

  loadfullpal();
  loadlvlpal(DMaps[currdmap].color);

  link.init();

  game.life=3*8;

  ringcolor();
  for(int i=0; i<128; i++)
    key[i]=0;

  Playing=true;
  openscreen();
  if(!Quit) {
    play_DmapMidi();
    if(dungeon())
      link.stepforward(12);
    newscr_clk=frame;
    }
  return 0;
}


void restart_level()
{
  blackscr(16,true);
  homescr = currscr = (dlevel) ? lastentrance : DMaps[currdmap].cont;

  for(int i=0; i<6; i++)
    visited[i]=-1;

  ALLOFF();
  whistleclk=-1;
  darkroom=false;
  loadscr(0,currscr,up);
  putscr(scrollbuf,0,0,&tmpscr[0]);

  loadfullpal();
  loadlvlpal(DMaps[currdmap].color);
  link.init();
  ringcolor();

  openscreen();
  if(!Quit)
  {
    play_DmapMidi();
    if(dungeon())
      link.stepforward(12);
    newscr_clk=frame;
  }
}



void game_loop()
{
  if(fadeclk>=0) {
    if(fadeclk==0 && currscr<128)
      blockpath=false;
    fadeclk--;
    }

  for(int i=0; i<4; i++)
    mblock[i].animate(0);
  items.animate();
  guys.animate();
  roaming_item();
  Ewpns.animate();
  if(link.animate(0)) {
    if(!Quit)
      Quit=qGAMEOVER;
    return;
    }
  Lwpns.animate();
  check_collisions();

  dryuplake();
  putmsg();
  domoney();
  putsubscr(framebuf,0,0);
  blit(scrollbuf,framebuf,0,0,0,56,256,168);
  link.draw_under(framebuf);
  for(int i=0; i<4; i++)
    mblock[i].draw(framebuf);
  if(drawguys) {
   if(frame&1) {
    guys.draw(framebuf,true);
    Lwpns.draw(framebuf,true);
    Ewpns.draw(framebuf,true);
    items.draw(framebuf,true);
    }
   else {
    items.draw(framebuf,false);
    Lwpns.draw(framebuf,false);
    guys.draw(framebuf,false);
    Ewpns.draw(framebuf,false);
    }
   guys.draw2(framebuf,true);
   }
  link.draw(framebuf);

  // draw likelike over Link
  for(int i=0; i<guys.Count(); i++)
  {
    if(guys.spr(i)->id == eLIKE)
      if(((eLikeLike*)guys.spr(i))->haslink)
        guys.spr(i)->draw(framebuf);
    if(guys.spr(i)->id == eWALLM)
      if(((eWallM*)guys.spr(i))->haslink)
        guys.spr(i)->draw(framebuf);
  }

  if(debug) {
    if(ReadKey(KEY_R)) game.drupy=255-game.rupies;
    if(ReadKey(KEY_B)) game.bombs=game.maxbombs;
    }
  advanceframe();

  text_mode(0);
  if(details) {
   for(int i=0; i<24; i++)
    if(i<guys.Count()) {
      int c=vc((guys.spr(i)->id)&15);
      if(c==vc(0))
        c=CSET(6)+1;
      c=1;
      textprintf(screen,font,296,(i<<3)+16,c,"%3d",((enemy*)guys.spr(i))->clk);
      }
    else
      textout(screen,font,"   ",296,(i<<3)+16,0);
   }
}


void fix_dialogs()
{
  system_dlg[0].x = scrx-(sbig?160:0);
  system_dlg[0].y = scry-(sbig?120:0);
  if(system_dlg[0].x>0) system_dlg[0].x++;
  if(system_dlg[0].y>0) system_dlg[0].y++;
  if(system_dlg[0].x<0) system_dlg[0].x=0;
  if(system_dlg[0].y<0) system_dlg[0].y=0;

  fix_dialog(key_dlg);
  fix_dialog(btn_dlg);
  fix_dialog(sound_dlg);
  fix_dialog(about_dlg);
  fix_dialog(quest_dlg);
  fix_dialog(vidmode_dlg);
  fix_dialog(triforce_dlg);
  fix_dialog(equip_dlg);
  fix_dialog(items_dlg);
  fix_dialog(gamemode_dlg);
  fix_dialog(getnum_dlg);
  fix_dialog(credits_dlg);
  fix_dialog(midi_dlg);

  digi_dp[1] += scrx;
  digi_dp[2] += scry;
  midi_dp[1] += scrx;
  midi_dp[2] += scry;
}


/**************************/
/********** Main **********/
/**************************/


char cfg_error[] =
"\"ag.cfg\" not found. Please run \"setup.exe\"\n(You can download it at www.armageddongames.com)\n";


int main(int argc, char* argv[])
{
  printf("--- Loading Zelda Classic %s ---\n",VerStr(ZELDA_VERSION));

  // check for the ag.cfg file and 1st.qst
  {
  struct ffblk f;
  if(findfirst("ag.cfg",&f,0)) {
    printf(cfg_error);
    return 1;
    }
  if(findfirst("1st.qst",&f,0)) {
    printf("\"1st.qst\" not found.");
    return 1;
    }
  }
  set_config_file("ag.cfg");


  // allocate quest data buffers
  printf("Allocating quest data buffers... ");
  qstpath = (char*)malloc(280);
  if(!qstpath || !get_qst_buffers()) {
    printf("Error\n");
    return 1;
    }
  printf("OK\n");


  // initialize Allegro
  printf("Initializing Allegro... ");
  allegro_init();
  install_keyboard();
  install_timer();
  install_mouse();
  install_joystick(JOY_TYPE_AUTODETECT);

  LOCK_VARIABLE(lastfps);
  LOCK_VARIABLE(framecnt);
  LOCK_FUNCTION(fps_callback);
  if(install_int_ex(fps_callback,SECS_TO_TIMER(1))) {
    allegro_exit();
    printf("Couldn't allocate timer\n");
    return 2;
    }
  LOCK_VARIABLE(myvsync);
  LOCK_FUNCTION(myvsync_callback);
  if(install_int_ex(myvsync_callback,BPS_TO_TIMER(60))) {
    allegro_exit();
    printf("Couldn't allocate timer\n");
    return 3;
    }
  printf("OK\n");


  // allocate bitmap buffers
  printf("Allocating bitmap buffers... ");
  framebuf  = create_bitmap(256,224);
  scrollbuf = create_bitmap(512,406);
  tmp_scr   = create_bitmap(320,240);
  tmp_bmp   = create_bitmap(32,32);
  fps_undo  = create_bitmap(64,16);
  if(!framebuf || !scrollbuf || !tmp_bmp || !fps_undo || !tmp_scr) {
    allegro_exit();
    printf("Error\n");
    return 4;
    }
  clear(scrollbuf);
  clear(framebuf);
  printf("OK\n");


  // load game configurations
  load_game_configs();

  if(used_switch(argc,argv,"-v0")) Vsync=false;
  if(used_switch(argc,argv,"-v1")) Vsync=true;

  resolve_password(zeldapwd);
  debug = debug_enabled = used_switch(argc,argv,"-d")
    && !strcmp(get_config_string(cfg_sect,"debug",""),zeldapwd);

  if(!debug_enabled)
  {
    the_menu[2] = the_menu[3];
    the_menu[3].text = NULL;
  }


  // load the data file

  resolve_password(datapwd);
  packfile_password(datapwd);

  printf("Loading data file... ");
  set_color_conversion(COLORCONV_NONE);
  if((data=load_datafile("#"))==NULL)
    if((data=load_datafile("zelda.dat"))==NULL)
    {
      allegro_exit();
      printf("Error loading zelda.dat\n");
      return 1;
    }

  sprintf(sig_str,"Zelda Classic %s - Jeremy Craner, 1999",VerStr(DATA_VERSION));

  if(strncmp((char*)data[0].dat,sig_str,36)) {
    allegro_exit();
    printf("Incompatible version of zelda.dat.\nPlease upgrade to %s\n",
      VerStr(DATA_VERSION));
    return 1;
    }
  printf("OK\n");

  sfxdata = (DATAFILE*)data[WAV].dat;
  mididata = (DATAFILE*)data[MUSIC].dat;

  wav_refill = *((SAMPLE*)sfxdata[WAV_REFILL].dat);
  wav_refill.loop_start = 256;
  wav_refill.loop_end = 2048;

  font = (FONT*)data[FONT_GUI].dat;
  zfont = (FONT*)data[FONT_NES].dat;
  text_mode(0);


  // load saved games
  printf("Loading saved games... ");
  if(load_savedgames()!=0)
  {
    allegro_exit();
    printf("Insufficient memory\n");
    return 1;
  }
  printf("OK\n");

  // initialise sound driver

  printf("Initializing sound driver... ");
  if(!used_switch(argc,argv,"-s"))
  {
   if(install_sound(DIGI_AUTODETECT,DIGI_AUTODETECT,NULL))
   {
    allegro_exit();
    printf("%s\n",allegro_error);
    return 5;
   }
   printf("OK\n");
  }
  else
   printf("skipped\n");

  for(int i=0; i<WAV_COUNT; i++)
    sfx_voice[i]=-1;
  for(int i=0; i<MUSIC_COUNT; i++)
    tunes[i].midi = (MIDI*)mididata[i].dat;
  master_volume(digi_volume,midi_volume);


  // AG logo
  if(!debug_enabled)
    aglogo(scrollbuf);

  // set video mode
  int mode = VidMode; // from config file

  int res_arg = used_switch(argc,argv,"-res");
  if(res_arg && argc>res_arg+2)
  {
    resx = atoi(argv[res_arg+1]);
    resy = atoi(argv[res_arg+2]);
    sbig = strcmp(argv[res_arg+3],"big")==0;// && resx>=640;
    mode = GFX_AUTODETECT;
  };

  if(used_switch(argc,argv,"-modex"))  mode=GFX_MODEX;
  if(used_switch(argc,argv,"-vesa1"))  mode=GFX_VESA1;
  if(used_switch(argc,argv,"-vesa2b")) mode=GFX_VESA2B;
  if(used_switch(argc,argv,"-vesa2l")) mode=GFX_VESA2L;
  if(used_switch(argc,argv,"-vesa3"))  mode=GFX_VESA3;

  if(!game_vid_mode(mode,250))
  {
    allegro_exit();
    printf("%s\n",allegro_error);
    return 6;
  }

  fix_dialogs();

  Quit = debug_enabled ? qQUIT : qRESET;

  while(Quit!=qEXIT)
  {
    titlescreen();
    while(!Quit)
      game_loop();
    Playing=Paused=false;
    switch(Quit)
    {
    case qQUIT:
    case qGAMEOVER: game_over(); break;
    case qWON:      ending(); break;
    }
    kill_sfx();
    stop_midi();
    clear(screen);
  }

  stop_midi();
  kill_sfx();
  set_gfx_mode(GFX_TEXT,80,25,0,0);
  save_savedgames();
  save_game_configs();
  unload_datafile(data);
  destroy_bitmap(framebuf);
  destroy_bitmap(scrollbuf);
  destroy_bitmap(tmp_scr);
  destroy_bitmap(tmp_bmp);
  destroy_bitmap(fps_undo);
  rest(250);
  allegro_exit();
  printf("Armageddon Games web site: http://www.armageddongames.com\n");
  return 0;
}

/*** end of zelda.cc ***/

