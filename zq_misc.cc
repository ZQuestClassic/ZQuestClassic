/*
  ZQ_MISC.CC
*/

inline void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }


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


enum { ftBIN=1, ftBMP, ftTIL };
char *imgstr[4] = { "Not loaded", "Binary/ROM", "Image", "ZC Tiles" };

int filetype(char *path)
{
  if(path==NULL || strlen(get_filename(path))==0)
    return 0;

  char ext[40];
  strcpy(ext,get_extension(path));
  strupr(ext);

  if(strcmp(ext,"BMP")==0) return ftBMP;
  if(strcmp(ext,"PCX")==0) return ftBMP;
  if(strcmp(ext,"TGA")==0) return ftBMP;
  if(strcmp(ext,"TIL")==0) return ftTIL;
  if(strcmp(ext,"TXT")==0) return 0;
  if(strcmp(ext,"ZIP")==0) return 0;
  return ftBIN;
}


void load_mice()
{
 for(int i=0; i<MAXMICE; i++) {
  mouse_bmp[i] = create_bitmap(16,16);
  blit((BITMAP*)data[BMP_MOUSE].dat,mouse_bmp[i],i*17+1,1,0,0,16,16);
  }
}


void dump_pal()
{
  for(int i=0; i<256; i++)
    rectfill(screen,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}


inline int bit(int val,int b)
{
 return (val>>b)&1;
}


int bound(int &x,int low,int high)
{
 if(x<low) x=low;
 if(x>high) x=high;
 return x;
}

int wrap(int x,int low,int high)
{
 if(x<low) x=high;
 if(x>high) x=low;
 return x;
}


int readfile(char *path,void *buf,int count)
{
 FILE *f=fopen(path,"rb");
 if(!f)
  return 0;
 int r=fread(buf,1,count,f);
 fclose(f);
 return r;
}

int writefile(char *path,void *buf,int count)
{
 FILE *f=fopen(path,"wb");
 if(!f)
  return 0;
 int r=fwrite(buf,1,count,f);
 fclose(f);
 return r;
}


/***  from allegro's guiproc.c  ***/
void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
   int x = ((x1+y1) & 1) ? 1 : 0;
   int c;

   for (c=x1; c<=x2; c++) {
      putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
      putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
   }

   for (c=y1+1; c<y2; c++) {
      putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
      putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
   }
}


RGB _RGB(byte *si)
{
  RGB x;
  x.r = si[0];
  x.g = si[1];
  x.b = si[2];
  return x;
}

RGB _RGB(int r,int g,int b)
{
  RGB x;
  x.r = r;
  x.g = g;
  x.b = b;
  return x;
}

RGB invRGB(RGB s)
{
  RGB x;
  x.r = 63-s.r;
  x.g = 63-s.g;
  x.b = 63-s.b;
  return x;
}


inline RGB NESpal(int i)
{
 return _RGB(nes_pal+(i*3));
}


RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio)
{
  RGB x;
  x.r = ( r1*(64-ratio) + r2*ratio ) >> 6;
  x.g = ( g1*(64-ratio) + g2*ratio ) >> 6;
  x.b = ( b1*(64-ratio) + b2*ratio ) >> 6;
  return x;
}

void load_cset(RGB *pal,int cset_index,int dataset)
{
  byte *si = colordata + CSET(dataset)*3;
  for(int i=0; i<16; i++) {
    pal[CSET(cset_index)+i] = _RGB(si);
    si+=3;
    }
}


void set_pal()
{
  set_palette_range(RAMpal,0,192,true);
}


void loadfullpal()
{
  for(int i=0; i<192; i++)
    RAMpal[i] = _RGB(colordata+i*3);

  set_pal();
}



void loadlvlpal(int level)
{
  byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;

  for(int i=0; i<16*3; i++) {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
    }

  for(int i=0; i<16; i++) {
    RAMpal[CSET(9)+i] = _RGB(si);
    si+=3;
    }

  set_pal();
}



void loadfadepal(int dataset)
{

 byte *si = colordata + CSET(dataset)*3;

 for(int i=0; i<16*3; i++) {
   RAMpal[CSET(2)+i] = _RGB(si);
   si+=3;
   }

 set_pal();
}




#define vc(x)  ((x)+224)    // offset to 'VGA color' x
#define dvc(x) ((x)+192)    // offset to dark 'VGA color' x
#define lc1(x) ((x)+208)    // offset to 'level bg color' x
#define lc2(x) ((x)+240)    // offset to 'level fg color' x

void setup_lcolors()
{
  for(int i=0; i<14; i++) {
    RAMpal[lc1(i)] = _RGB(colordata+(CSET(i*pdLEVEL+poLEVEL)+2)*3);
    RAMpal[lc2(i)] = _RGB(colordata+(CSET(i*pdLEVEL+poLEVEL)+16+1)*3);
    }
  set_palette(RAMpal);
}


void refresh_pal()
{
 loadfullpal();
 loadlvlpal(Color);
 setup_lcolors();
}




#define MAXITEMS   40
char *item_string[MAXITEMS] = {
"(none)", "blue rupee", "heart", "bombs", "clock",
"sword", "white sword", "magic sword", "magic shield", "key",
"blue candle", "red candle", "letter", "arrow", "silver arrow",
"bow", "bait", "blue ring", "red ring", "power bracelet",
"triforce", "map", "compass", "wood boomerang", "magic boomerang",
"wand", "raft", "ladder", "heart container", "blue potion",
"red potion", "whistle", "magic book", "magic key", "(fairy)",
"fire boomerang", "Excalibur", "mirror shield", "20 rupies", "50 rupies" };

#define MAXROOMTYPES   rMAX
char *roomtype_string[MAXROOMTYPES] = {
"(none)","special item","pay for info","secret money","gamble",
"door repair","heart container","feed the Goriya","level 9 entrance",
"potion shop","shop","more bombs","leave money or life","10 rupies",
"3-stair warp","Ganon","Zelda" };

char *catchall_string[MAXROOMTYPES] = {
" ","Sp.Item","Info Type","Amount"," ","Repair Fee"," "," "," "," ",
"Shop Type","Price","Price"," ","Warp Ring"," "," " };


#define MAXWARPTYPES   4
char *warptype_string[MAXWARPTYPES] = {
"cave/item room","passageway","entrance/exit","scrolling warp"
};


#define MAXCOMBOTYPES  cMAX
char *combotype_string[MAXCOMBOTYPES] = {
"-","stairs","cave","water","armos","grave","dock",
"undefined","push-wait","push-heavy","push-hw","l statue","r statue" };


#define MAXFLAGS    16
char *flag_string[MAXFLAGS] = {
" 0 (none)"," 1 push up/down"," 2 push 4-way"," 3 burn"," 4 burn->stair",
" 5 bomb"," 6 bomb->cave"," 7 fairy"," 8 raft"," 9 armos->stair",
"10 armos->item","11","12","13","14","15 Zelda"
};


#define MAXGUYS    9
// eMAXGUYS is defined in zdefs.h
char *guy_string[eMAXGUYS] = {
"(none)","abei","ama","merchant","molblin","fire","fairy","goriya","Zelda","",
/*10*/ "octorok - red, slow","octorok - blue, slow","octorok - red, fast","octorok - blue, fast","tektite - red",
/*15*/ "tektite - blue","leever - red","leever - blue","molblin - red","molblin - black",
/*20*/ "lynel - red","lynel - blue","peahat","zora (sp)","rock (sp)",
/*25*/ "ghini","-<ghini 2>","-<armos>","keese - blue","keese - red",
/*30*/ "keese - level color","stalfos","gel","zol","rope",
/*35*/ "goriya - red","goriya - blue","-<trap>","wall master","darknut - red",
/*40*/ "darknut - blue","bubble","vire","like like","gibdo",
/*45*/ "pols voice","wizzrobe - red","wizzrobe - blue","aquamentus","moldorm",
/*50*/ "dodongo","manhandla","gleeok - 2 heads","gleeok - 3 heads","gleeok - 4 heads",
/*55*/ "digdogger - 1 kid","digdogger - 3 kids","gohma - red","gohma - blue","lanmola - red",
/*60*/ "lanmola - blue","patra - big circle","patra - oval","-<Ganon>","stalfos 2",
/*65*/ "rope 2","bubble - red","bubble - blue","-<fire shooter>","-<item fairy>",
/*70*/ "-<fire>"
};

#define MAXPATTERNS  2
char *pattern_string[MAXPATTERNS] = {
"random","enter from sides"
};


#define MAXMIDIS_ZQ  4+MAXMIDIS
char *midi_string[MAXMIDIS_ZQ] = {
"(none)",
"Overworld",
"Dungeon",
"Level 9",
};



#define cWALK      1
#define cFLAGS     2
#define cDARK      4
#define cDEBUG     128

#define rMAP       1
#define rCOMBOS    2
#define rSCRMAP    4
#define rMENU      8
#define rCOMBO     16
#define rALL       0x0FF
#define rCLEAR     0x100

void refresh(int flags);
void domouse();

int onNew();
int onOpen();
int onSave();
int onSaveAs();
int onImport();

int onUndo();
int onCopy();
int onPaste();
int onDelete();
int onDeleteMap();

int onTemplate();
int onDoors();
int onCSetFix();
int onFlags();
int onShowPal();

int playTune();
int playMIDI();
int stopMIDI();

int onUp();
int onDown();
int onLeft();
int onRight();
int onPgUp();
int onPgDn();

int onHelp();

int onScrFlags();
int onGuy();
int onString();
int onRType();
int onCatchall();
int onItem();
int onWarp();
int onWarp2();
int onPath();
int onEnemies();
int onEnemyFlags();
int onUnderCombo();

int onHeader();
int onRules();
int onStrings();
int onDmaps();
int onTiles();
int onCombos();
int onMidis();
int onShopTypes();
int onInfoTypes();
int onWarpRings();
int onWhistle();
int onMapColors();
int onTriPieces();

int onColors_Main();
int onColors_Levels();
int onColors_Sprites();

int onImport_Map();
int onImport_DMaps();
int onImport_Msgs();
int onImport_Combos();
int onImport_Tiles();
int onImport_Pals();

int onExport_Map();
int onExport_DMaps();
int onExport_Msgs();
int onExport_Combos();
int onExport_Tiles();
int onExport_Pals();

int onMap1();
int onMap2();
int onMap3();
int onMap4();
int onMap5();
int onMap6();
int onMap7();
int onMap8();
int onMap9();
int onMap10();
int onMapCount();

int onViewPic();
int onEditTemplate();

int onDefault_Pals();
int onDefault_Tiles();
int onDefault_Combos();
int onDefault_MapColors();

void edit_combo(int c,bool freshen);

int gocnt=0;

void go()
{
  switch(gocnt) {
  case 0:
   scare_mouse();
   blit(screen,menu,0,0,0,0,320,240);
   unscare_mouse();
   break;
  case 1:
   scare_mouse();
   blit(screen,menu2,0,0,0,0,320,240);
   unscare_mouse();
   break;
  default: return;
  }
  gocnt++;
}

void comeback()
{
  switch(gocnt) {
  case 1:
   scare_mouse();
   blit(menu,screen,0,0,0,0,320,240);
   unscare_mouse();
   break;
  case 2:
   scare_mouse();
   blit(menu2,screen,0,0,0,0,320,240);
   unscare_mouse();
   break;
  default: return;
  }
  gocnt--;
}



int checksave()
{
 if(saved)
   return 1;
 char buf[80];
 char *name = get_filename(filepath);
 if(name[0]==0)
   sprintf(buf,"Save this quest file?");
 else
   sprintf(buf,"Save changes to %s?",name);
 switch(alert3(buf,NULL,NULL,"&Yes","&No","Cancel",'y','n',27)) {
 case 1:
   onSave();
   return 1;
 case 2:
   return 1;
 }
 return 0;
}


int onExit()
{
 if(checksave()==0)
   return D_O_K;
 if(alert("Really want to quit?", NULL, NULL, "&Yes", "&No", 'y', 'n') == 2)
   return D_O_K;
 return D_CLOSE;
}


int onAbout()
{
 char buf1[40];
 char buf2[40];
 char buf3[40];
 sprintf(buf1,"ZQuest %s",VerStr(ZELDA_VERSION));
 if(debug)
 {
   sprintf(buf2,"ZQuest Editor: %04X",INTERNAL_VERSION);
   sprintf(buf3,"This qst file: %04X",header.internal&0xFFFF);
   alert(buf1,buf2,buf3,"OK", NULL, 13, 27);
 }
 else
 {
   alert(buf1,"Zelda Classic Quest Editor","by Phantom Menace", "OK", NULL, 13, 27);
 }
 return D_O_K;
}


int onW()
{
 Flags^=cWALK;
 refresh(rMAP+rMENU);
 return D_O_K;
}

int onF()
{
 Flags^=cFLAGS;
 refresh(rMAP);
 return D_O_K;
}

int onL()
{
 loadfadepal(Color*pdLEVEL+poFADE3);
 readkey();
 loadlvlpal(Color);
 return D_O_K;
}

int onPlus()
{
 CSet=wrap(CSet+1,0,5);
 refresh(rCOMBOS+rMENU+rCOMBO);
 return D_O_K;
}

int onMinus()
{
 CSet=wrap(CSet-1,0,5);
 refresh(rCOMBOS+rMENU+rCOMBO);
 return D_O_K;
}

void setFlagColor()
{
 RAMpal[dvc(0)]=RAMpal[vc(Flag)];
 set_palette_range(RAMpal,dvc(0),dvc(0),false);
}

int onTimes()
{
 Flag=(Flag+1)&15;
 setFlagColor();
 refresh(rMENU);
 return D_O_K;
}

int onDivide()
{
 Flag=(Flag-1)&15;
 setFlagColor();
 refresh(rMENU);
 return D_O_K;
}

int on0();
int on1();
int on2();
int on3();
int on4();
int on5();
int on6();
int on7();
int on8();
int on9();
int on10();
int on11();
int on12();
int on13();
int on14();

int onD();
int onIncMap();
int onDecMap();

int onDumpScr();


#define C(x)   ((x)-'a'+1)



