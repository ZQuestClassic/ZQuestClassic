//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_misc.cc
//
//  Misc. stuff for ZQuest.
//
//--------------------------------------------------------


//inline void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }
inline void SCRFIX() {}


inline int popup_menu(MENU *menu,int x,int y)
{
  while(mouse_b);
  return do_menu(menu,x,y);
}




enum { ftBIN=1, ftBMP, ftTIL, ftZGP, ftQSU };
char *imgstr[6] = { "Not loaded", "Binary/ROM", "Image", "ZC Tiles",
                    "ZC Tiles", "ZC Tiles" };

int filetype(char *path)
{
  if(path==NULL || strlen(get_filename(path))==0)
    return 0;

  char ext[40];
  strcpy(ext,get_extension(path));
  strupr(ext);

  if(strcmp(ext,"BMP")==0) return ftBMP;
  if(strcmp(ext,"GIF")==0) return ftBMP;
  if(strcmp(ext,"PCX")==0) return ftBMP;
  if(strcmp(ext,"TGA")==0) return ftBMP;
  if(strcmp(ext,"TIL")==0) return ftTIL;
  if(strcmp(ext,"ZGP")==0) return ftZGP;
  if(strcmp(ext,"QSU")==0) return ftQSU;
  if(strcmp(ext,"DAT")==0) return 0;
  if(strcmp(ext,"HTM")==0) return 0;
  if(strcmp(ext,"HTML")==0) return 0;
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

void load_arrows()
{
 for(int i=0; i<MAXARROWS; i++) {
  arrow_bmp[i] = create_bitmap(16,16);
  blit((BITMAP*)data[BMP_ARROWS].dat,arrow_bmp[i],i*17+1,1,0,0,16,16);
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

void reset_pal_cycling();
void cycle_palette();

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


void loadlvlpal(int level)
{
  // full pal
  for(int i=0; i<192; i++)
    RAMpal[i] = _RGB(colordata+i*3);

  // level pal
  byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;

  for(int i=0; i<16*3; i++)
  {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
  }

  for(int i=0; i<16; i++)
  {
    RAMpal[CSET(9)+i] = _RGB(si);
    si+=3;
  }

  reset_pal_cycling();
  set_pal();
}



void loadfadepal(int dataset)
{
  byte *si = colordata + CSET(dataset)*3;

  for(int i=0; i<16*3; i++)
  {
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
  loadlvlpal(Color);
  setup_lcolors();
}




#define ITEMCNT   iMax
char *item_string[ITEMCNT] = {
"(none)", "blue rupee", "heart", "bombs", "clock",
"wooden sword", "white sword", "magic sword", "magic shield", "key",
"blue candle", "red candle", "letter", "arrow", "silver arrow",
"bow", "bait", "blue ring", "red ring", "power bracelet",
"triforce", "map", "compass", "wooden boomerang", "magic boomerang",
"wand", "raft", "ladder", "heart container", "blue potion",
"red potion", "whistle", "magic book", "magic key", "(fairy)",
"fire boomerang", "Master sword", "mirror shield", "20 rupies", "50 rupies",
"200 rupies", "small wallet (500)", "large wallet (999)", "dust pile",
"big triforce", "selection", "misc 1", "misc 2", "super bomb","HC piece",
"cross", "flippers", "hookshot", "lens of truth", "hammer", "boots",
"L2 bracelet", "golden arrow", "magic container", "magic jar (sm)",
"magic jar (lg)", "golden ring" };


#define WPNCNT    wMAX
char *weapon_string[WPNCNT] = {
"wooden sword", "white sword", "magic sword", "master sword",
"wooden boomerang", "magic boomerang", "fire boomerang", "bomb",
"super bomb", "explosion", "arrow", "silver arrow", "fire", "whirlwind",
"bait", "wand", "magic", "fire ball", "rock", "enemy arrow", "enemy sword",
"enemy magic", "MISC: spawn", "MISC: death", "MISC: swim", "Hammer",
"Hookshot Head", "Hookshot Chain", "Hookshot Handle", "MISC: SArw Sparkle",
"MISC: GArw Sparkle", "MISC: MBrg Sparkle", "MISC: FBrg Sparkle",
"MISC: Smack", "golden arrow", "enemy flame", "enemy wind", "MISC: Magic Meter",
"DF 1a", "DF 1b","DF Spark 1a", "DF Spark 1b"
};


#define MAXROOMTYPES   rMAX
char *roomtype_string[MAXROOMTYPES] = {
"(none)","special item","pay for info","secret money","gamble",
"door repair","heart container","feed the Goriya","level 9 entrance",
"potion shop","shop","more bombs","leave money or life","10 rupies",
"3-stair warp","Ganon","Zelda", "item pond", "magic upgrade" };

char *catchall_string[MAXROOMTYPES] = {
" ","Sp.Item","Info Type","Amount"," ","Repair Fee"," "," "," ","Shop Type",
"Shop Type","Price","Price"," ","Warp Ring"," "," ", "Item Pond Type", " " };


#define MAXWARPTYPES   wtMAX-1 // minus whistle
char *warptype_string[MAXWARPTYPES] = {
"cave/item room","passageway","entrance/exit","scrolling warp",
"insta-warp","i-warp w/ blkout","i-warp w/ openscrn","i-warp w/ zap FX",
"no warp"
};


#define MAXCOMBOTYPES  cMAX
char *combotype_string[MAXCOMBOTYPES] = {
"-","stairs","cave","water","armos","grave","dock",
"undefined","push-wait","push-heavy","push-hw","l statue","r statue",
"slow walk","convey up","convey down","convey left","convey right",
"swim warp","dive warp","ladder only","trigger temp","trigger perm",
"win game","slash","slash->item", "push-heavy2", "push-hw2", "pound",
"HS grab", "HS bridge", "Damage 1/2", "Damage 1", "Damage 2", "Damage 4",
"c statue", "trap - horizontal", "trap - vertical", "trap - 4-way",
"trap - left/right", "trap - up/down", "pit", "hookshot over", "overhead",
"no fly zone", "m. mirror +", "m. mirror /", "m. mirror \\", "m. prism (3-way)",
"m. prism (4-way)", "m. sponge", "cave 2"
};


#define MAXFLAGS    mfMAX
char *flag_string[MAXFLAGS] = {
" 0 (none)"," 1 push up/down"," 2 push 4-way"," 3 whistle"," 4 burn",
" 5 arrow"," 6 bomb"," 7 fairy"," 8 raft"," 9 armos->secret",
"10 armos->item","11 super bomb","12 raft branch",
"13 dive for item",
"14 all-purpose flag",
"15 Zelda",
"16 secret tile 0",
"17 secret tile 1",
"18 secret tile 2",
"19 secret tile 3",
"20 secret tile 4",
"21 secret tile 5",
"22 secret tile 6",
"23 secret tile 7",
"24 secret tile 8",
"25 secret tile 9",
"26 secret tile 10",
"27 secret tile 11",
"28 secret tile 12",
"29 secret tile 13",
"30 secret tile 14",
"31 secret tile 15",
"32 trap horizontal",
"33 trap vertical",
"34 trap 4-way",
"35 trap left/right",
"36 trap up/down",
"37 enemy 1",
"38 enemy 2",
"39 enemy 3",
"40 enemy 4",
"41 enemy 5",
"42 enemy 6",
"43 enemy 7",
"44 enemy 8",
"45 enemy 9",
"46 enemy 10"
};


#define zqMAXGUYS    9
// eMAXGUYS is defined in zdefs.h
char *guy_string[eMAXGUYS] = {
"(none)","abei","ama","merchant","molblin","fire","fairy","goriya","Zelda","",
/*10*/ "octorok - red, slow","octorok - blue, slow","octorok - red, fast","octorok - blue, fast","tektite - red",
/*15*/ "tektite - blue","leever - red","leever - blue","molblin - red","molblin - black",
/*20*/ "lynel - red","lynel - blue","peahat","zora (sp)","rock (sp)",
/*25*/ "ghini","ghini 2","armos","keese - blue","keese - red",
/*30*/ "keese - level color","stalfos","gel","zol","rope",
/*35*/ "goriya - red","goriya - blue","-<trap>","wall master","darknut - red",
/*40*/ "darknut - blue","bubble","vire","like like","gibdo",
/*45*/ "pols voice","wizzrobe - red","wizzrobe - blue","aquamentus - right","moldorm",
/*50*/ "dodongo","manhandla","gleeok - 1 head","gleeok - 2 heads","gleeok - 3 heads","gleeok - 4 heads",
/*55*/ "digdogger - 1 kid","digdogger - 3 kids","digdogger kid","gohma - red","gohma - blue","lanmola - red",
/*60*/ "lanmola - blue","patra - big circle","patra - oval","-<Ganon>","stalfos 2",
/*65*/ "rope 2","bubble - red","bubble - blue","fire shooter","-<item fairy>",
/*70*/ "fire","octorok - on crack", "darknut - death knight", "gel tribble", "zol tribble",
/*75*/ "keese tribble", "vire tribble", "darknut - super", "aquamentus - left", "manhandla 2",
/*80*/ "-<trap h>", "-<trap v>", "-<trap lr>", "-<trap ud>", "wizzrobe - fire",
/*85*/ "wizzrobe - wind", "-ceiling master", "-floor master", "patra (bs)", "patra 2",
/*90*/ "patra 3", "bat", "wizzrobe - bat", "-wizzrobe - bat2", "gleeok2 - 1 head", "gleeok2 - 2 heads",
/*95*/ "gleeok2 - 3 heads","gleeok2 - 4 heads", "wizzrobe - mirror", "dodongo (bs)", "-dodongo - fire", "trigger"
};

#define MAXPATTERNS  2
char *pattern_string[MAXPATTERNS] = {
"random","enter from sides"
};

char *short_pattern_string[MAXPATTERNS] = {
"random", "sides"
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
#define cCSET      8
#define cNODARK    16
#define cNOITEM    32
#define cCTYPE     64
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
int onOpen2();
int onSave();
int onUpgrade();
int onSaveAs();

/*I guess this is a legacy import function.
  Compiles fine with it commented out.

  int onImport();
*/

int onUndo();
int onCopy();
int onPaste();
int onPasteAll();
int onPasteToAll();
int onPasteAllToAll();
int onDelete();
int onDeleteMap();

int onTemplate();
int onDoors();
int onCSetFix();
int onFlags();
int onShowPal();
int onReTemplate();

int playTune();
int playMIDI();
int stopMIDI();

int onUp();
int onDown();
int onLeft();
int onRight();
int onPgUp();
int onPgDn();

int  onHelp();
void doHelp(int bg,int fg);

int onScrFlags();
int onGuy();
int onEndString();
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
int onSecretCombo();

int onHeader();
int onRules();
int onRules2();
int onCheats();
int onStrings();
int onDmaps();
int onTiles();
int onCombos();
int onMidis();
int onShopTypes();
int onInfoTypes();
int onWarpRings();
int onWhistle();
int onMiscColors();
int onMapStyles();
int onTriPieces();
int onIcons();
int onInit();
int onLayers();
int onScreenPalette();
int xtoi(char *hexstr);

int onColors_Main();
int onColors_Levels();
int onColors_Sprites();

int onImport_Map();
int onImport_DMaps();
int onImport_Msgs();
int onImport_Combos();
int onImport_Tiles();
int onImport_Pals();
int onImport_ZGP();
int onImport_UnencodedQuest();

int onExport_Map();
int onExport_DMaps();
int onExport_Msgs();
int onExport_Combos();
int onExport_Tiles();
int onExport_Pals();
int onExport_ZGP();
int onExport_UnencodedQuest();


int onGotoMap();
int onMapCount();

int onViewPic();
int onViewMap();
int onComboPage();

int onDefault_Pals();
int onDefault_Tiles();
int onDefault_Combos();
int onDefault_Sprites();
int onDefault_MapStyles();

int onCustomItems();
int onCustomWpns();
int onCustomGuys();

int onTest();
int onTestOptions();

int onOptions();

void edit_combo(int c,bool freshen,int cs);

void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg,bool value);
void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int value);

int onSpacebar()
{


  return D_O_K;
}


int onSnapshot()
{
  struct ffblk f;
  char buf[20];
  int num=0;
  do {
    sprintf(buf,"zelda%03d.bmp",++num);
  } while(num<999 && !findfirst(buf,&f,0));
  blit(screen,screen2,0,0,0,0,320,240);
  save_bmp(buf,screen2,RAMpal);
  return D_O_K;
}



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
 if(debug)
 {
   if (IS_BETA) {
     sprintf(buf1,"ZQuest %s Beta (Build %d) - DEBUG",VerStr(ZELDA_VERSION), VERSION_BUILD);
   } else {
     sprintf(buf1,"ZQuest %s (Build %d) - DEBUG",VerStr(ZELDA_VERSION), VERSION_BUILD);
   }
//   sprintf(buf1,"ZQuest %s - DEBUG",VerStr(ZELDA_VERSION));
   sprintf(buf2,"ZQuest Editor: %04X",INTERNAL_VERSION);
   sprintf(buf3,"This qst file: %04X",header.internal&0xFFFF);
   alert(buf1,buf2,buf3,"OK", NULL, 13, 27);
 }
 else
 {
   if (IS_BETA) {
     sprintf(buf1,"ZQuest %s Beta (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
   } else {
     sprintf(buf1,"ZQuest %s (Build %d)",VerStr(ZELDA_VERSION), VERSION_BUILD);
   }
//   sprintf(buf1,"ZQuest %s",VerStr(ZELDA_VERSION));
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

int onQ()
{
 if(Flags&cCSET)
 {
   Flags ^= cCSET;
   Flags |= cCTYPE;
 }
 else if(Flags&cCTYPE)
 {
   Flags ^= cCTYPE;
 }
 else
 {
   Flags |= cCSET;
 }

 refresh(rMAP);
 return D_O_K;
}

int onL()
{
 if(get_bit(header.rules,qr1_FADE))
 {
   int last = CSET(5)-1;

   if(get_bit(header.rules,qr1_FADECS5))
     last += 16;

   byte *si = colordata + CSET(Color*pdLEVEL+poFADE1)*3;
   for(int i=0; i<16; i++)
   {
     int light = si[0]+si[1]+si[2];
     si+=3;
     fade_interpolate(RAMpal,black_palette,RAMpal,light?32:64,CSET(2)+i,CSET(2)+i);
   }
   fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(3),last);
   set_palette(RAMpal);

   readkey();

   load_cset(RAMpal,5,5);
   loadlvlpal(Color);
 }
 else
 {
   loadfadepal(Color*pdLEVEL+poFADE3);
   readkey();
   loadlvlpal(Color);
 }
 return D_O_K;
}


int onM()
{
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
 Flag=(Flag+1);
 if (Flag==mfMAX) {
   Flag=0;
 }
 setFlagColor();
 refresh(rMENU);
 return D_O_K;
}

int onDivide()
{
 if (Flag==0) {
   Flag=mfMAX;
 }
 Flag=(Flag-1);
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



