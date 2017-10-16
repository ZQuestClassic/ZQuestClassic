/*
  ZQ_CLASS.CC
*/


/************************/
/****** ZMAP class ******/
/************************/


void reset_dmap(int index)
{
 bound(index,0,MAXDMAPS-1);
 byte *di = ((byte*)(DMaps+index));
 for(unsigned i=0; i<sizeof(dmap); i++)
   *(di++)=0;
}


class zmap {
 mapscr *screens;
 int currmap,copymap;
 int currscr,copyscr;
 int scrpos[MAXMAPS+1];

 // editing screens
 mapscr undomapscr,copymapscr;
 bool can_undo,can_paste;

public:

 zmap() { can_undo=can_paste=false; }
 bool CanUndo() { return can_undo; }
 bool CanPaste() { return can_paste; }
 int  CopyScr() { return (copymap<<8)+copyscr; }
 void Ugo();
 void Uhuilai();
 void Copy();
 void Paste();
 void PasteEnemies();
 bool isDungeon()
 {
  return ( (screens[currscr].data[0]&0x1FF)==66 &&
           (screens[currscr].data[15]&0x1FF)==75 );
 }
 void clearall()
 {
   Color=0;
   for(int i=0; i<MAXMAPS; i++) {
     setCurrMap(i);
     clearmap();
   }
   setCurrMap(0);
 }
 void clearmap()
 {
   if(currmap<MAXMAPS) {
    for(int i=0; i<130; i++)
     clearscr(i);
    setCurrScr(0);
   }
 }
 void clearscr(int scr);
 int  load(char *path);
 int  save(char *path);
 void draw(BITMAP* dest,int x,int y,int flags);
 void scroll(int dir);
 mapscr *CurrScr()        { return screens+currscr; }
 mapscr *Scr(int scr)     { return screens+scr; }
 int  getCurrMap()        { return currmap; }
 void setCurrMap(int index)
 {
   int oldmap=currmap;
   scrpos[currmap]=currscr;
   currmap=bound(index,0,MAXMAPS);
   screens=TheMaps+(currmap*130);
   currscr=scrpos[currmap];
   if(currmap!=oldmap)
     can_undo=false;
 }
 int  getCurrScr()        { return currscr; }
 void setCurrScr(int scr)
 {
   int oldscr=currscr;
   currscr=bound(scr,0,129);
   setcolor(screens[currscr].color&15);
   if(currscr!=oldscr)
     can_undo=false;
 }
 void setcolor(int c)
 {
   if(screens[currscr].valid&mVALID) {
     screens[currscr].color=c;
     if(Color!=c)
       loadlvlpal(Color=c);
     }
 }
 void resetflags()
 {
  byte *di=&(screens[currscr].valid);
  for(int i=1; i<48; i++)
    *(di+i)=0;
 }
 void Template(int floor)
 {
  if(!(screens[currscr].valid&mVALID))
    screens[currscr].color=Color;
  screens[currscr].valid|=mVALID;

  for(int i=0; i<32; i++)
    screens[currscr].data[i]=TheMaps[TEMPLATE].data[i];
  for(int i=144; i<176; i++)
    screens[currscr].data[i]=TheMaps[TEMPLATE].data[i];
  for(int y=2; y<=9; y++) {
    int j=y<<4;
    screens[currscr].data[j]=TheMaps[TEMPLATE].data[j++];
    screens[currscr].data[j]=TheMaps[TEMPLATE].data[j++];
    j+=12;
    screens[currscr].data[j]=TheMaps[TEMPLATE].data[j++];
    screens[currscr].data[j]=TheMaps[TEMPLATE].data[j++];
    }

  if(floor!=-1) {
    for(int y=2; y<9; y++)
     for(int x=2; x<14; x++) {
      int i=(y<<4)+x;
      screens[currscr].data[i] &= 0xF000;
      screens[currscr].data[i] |= floor&0x0FFF;
      }
    }
  for(int i=0; i<4; i++)
    putdoor(i,screens[currscr].door[i]);
 }
 void putdoor(int side,int doortype);
 void dowarp(int type);
 void dowarp(int ring,int index);
} Map;




void zmap::clearscr(int scr)
{
 byte *di=((byte*)screens)+(scr*sizeof(mapscr));
 for(unsigned i=0; i<sizeof(mapscr); i++)
  *(di++) = 0;
 screens[scr].valid=mVERSION;
}



static char *loaderror[] = {"OK","File not found","Incomplete data",
  "Invalid version","Invalid file"};

int zmap::load(char *path)
{
 FILE *f=fopen(path,"rb");
 if(!f)
  return 1;
 for(int i=0; i<130; i++) {
  if(fread(screens+i,sizeof(mapscr),1,f)<1) {
   fclose(f);
   clearall();
   setCurrScr(0);
   return 2;
   }
  }
 fclose(f);
 if(!(screens[0].valid&mVERSION)) {
   clearall();
   setCurrScr(0);
   return 3;
   }
 setCurrScr(0);
 return 0;
}


int zmap::save(char *path)
{
 FILE *f=fopen(path,"wb");
 if(!f)
  return 1;
 for(int i=0; i<130; i++) {
  if(fwrite(screens+i,sizeof(mapscr),1,f)<1) {
   fclose(f);
   return 2;
   }
  }
 fclose(f);
 return 0;
}


void putcombo(BITMAP *dest,combo c,word cmbdat,int x,int y,int cset,int flags)
{
  if(c.tile==0) {
    rectfill(dest,x,y,x+15,y+15,0);
    rectfill(dest,x+3,y+3,x+12,y+12,vc(4));
    return;
    }

  puttile16(dest,c.tile,x,y,cset,c.flip);

  for(int i=0; i<4; i++) {
    int tx=((i&2)<<2)+x;
    int ty=((i&1)<<3)+y;
    if((flags&cWALK) && (c.walk&(1<<i)))
      rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
    }

  if((flags&cFLAGS)&&(cmbdat&0xF000))
    rectfill(dest,x,y,x+15,y+15,vc((cmbdat&0xF000)>>12) );
}



inline void putcombo(BITMAP* dest,int x,int y,word cmbdat,int flags)
{
  putcombo(dest,combobuf[cmbdat&0x1FF],cmbdat,x,y,(cmbdat&0x0E00)>>9,flags);
}


inline word cmb(int comboindex,int cset,int flags)
{ return (comboindex&0x1FF)+((cset&7)<<9)+(flags<<12); }


inline word tcmb(int pos)
{ return TheMaps[TEMPLATE].data[pos]&0x1FF; }



void zmap::draw(BITMAP* dest,int x,int y,int flags)
{
 if(!(screens[currscr].valid&mVALID)) {
  rectfill(dest,x,y,x+255,y+175,dvc(1));
  return;
  }

 int dark=screens[currscr].flags&4;
 for(int i=0; i<176; i++)
  putcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,screens[currscr].data[i],flags|dark);

 int cs=2;

 switch(screens[currscr].door[up]) {
   case dLOCKED:      puttile16(dest,combobuf[tcmb(41)].tile,x+120,y+16,cs,combobuf[tcmb(41)].flip); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     puttile16(dest,combobuf[tcmb(57)].tile,x+120,y+16,cs,combobuf[tcmb(57)].flip); break;
   case dBOMB:        puttile16(dest,combobuf[tcmb(73)].tile,x+120,y+16,cs,combobuf[tcmb(73)].flip); break;
   case dWALK:        puttile16(dest,combobuf[tcmb(89)].tile,x+120,y+16,cs,combobuf[tcmb(89)].flip); break;
   }
 switch(screens[currscr].door[down]) {
   case dLOCKED:      puttile16(dest,combobuf[tcmb(137)].tile,x+120,y+144,cs,combobuf[tcmb(137)].flip); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     puttile16(dest,combobuf[tcmb(121)].tile,x+120,y+144,cs,combobuf[tcmb(121)].flip); break;
   case dBOMB:        puttile16(dest,combobuf[tcmb(105)].tile,x+120,y+144,cs,combobuf[tcmb(105)].flip); break;
   case dWALK:        puttile16(dest,combobuf[tcmb(89)].tile,x+120,y+144,cs,combobuf[tcmb(89)].flip); break;
   }
 switch(screens[currscr].door[left]) {
   case dLOCKED:      puttile16(dest,combobuf[tcmb(68)].tile,x+16,y+80,cs,combobuf[tcmb(68)].flip); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     puttile16(dest,combobuf[tcmb(84)].tile,x+16,y+80,cs,combobuf[tcmb(84)].flip); break;
   case dBOMB:        puttile16(dest,combobuf[tcmb(100)].tile,x+16,y+80,cs,combobuf[tcmb(100)].flip); break;
   case dWALK:        puttile16(dest,combobuf[tcmb(89)].tile,x+16,y+80,cs,combobuf[tcmb(89)].flip); break;
   }
 switch(screens[currscr].door[right]) {
   case dLOCKED:      puttile16(dest,combobuf[tcmb(75)].tile,x+224,y+80,cs,combobuf[tcmb(75)].flip); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     puttile16(dest,combobuf[tcmb(91)].tile,x+224,y+80,cs,combobuf[tcmb(91)].flip); break;
   case dBOMB:        puttile16(dest,combobuf[tcmb(107)].tile,x+224,y+80,cs,combobuf[tcmb(107)].flip); break;
   case dWALK:        puttile16(dest,combobuf[tcmb(89)].tile,x+224,y+80,cs,combobuf[tcmb(89)].flip); break;
   }

 if(!(flags&cDEBUG))
 {
   for(int j=168; j<176; j++)
    for(int i=0; i<256; i++)
      if(((i^j)&1)==0)
        putpixel(dest,x+i,y+j,vc(0));
 }

 if(dark)
 {
   for(int j=0; j<80; j++)
    for(int i=0; i<80-j; i++)
      if(((i^j)&1)==0)
        putpixel(dest,x+i,y+j,vc(0));
 }

 if(screens[currscr].item)
   putitem(dest,screens[currscr].itemx+x,screens[currscr].itemy+y,screens[currscr].item);
}


void zmap::scroll(int dir)
{
 if(currmap<MAXMAPS) {
   switch(dir) {
   case up:    if(currscr>15)  setCurrScr(currscr-16); break;
   case down:  if(currscr<114) setCurrScr(currscr+16); break;
   case left:  if(currscr&15)  setCurrScr(currscr-1);  break;
   case right: if((currscr&15)<15 && currscr<129) setCurrScr(currscr+1); break;
   }
 }
}




void zmap::putdoor(int side,int door)
{
 screens[currscr].door[side]=door;
 word *di=screens[currscr].data;
 switch(side) {
 case up:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK: di[7]=cmb(tcmb(7),2,0);
               di[8]=cmb(tcmb(8),2,0);
               di[23]=cmb(tcmb(23),2,0);
               di[24]=cmb(tcmb(24),2,0); break;
   default:
               di[7]=cmb(tcmb(37),2,0);
               di[8]=cmb(tcmb(38),2,0);
               di[23]=cmb(tcmb(53),2,0);
               di[24]=cmb(tcmb(54),2,0); break;
   } break;
 case down:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK:
               di[151]=cmb(tcmb(151),2,0);
               di[152]=cmb(tcmb(152),2,0);
               di[167]=cmb(tcmb(167),2,0);
               di[168]=cmb(tcmb(168),2,0); break;
   default:
               di[151]=cmb(tcmb(117),2,0);
               di[152]=cmb(tcmb(118),2,0);
               di[167]=cmb(tcmb(133),2,0);
               di[168]=cmb(tcmb(134),2,0); break;
   } break;
 case left:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK:
               di[64]=cmb(tcmb(64),2,0); di[65]=cmb(tcmb(65),2,0);
               di[80]=cmb(tcmb(80),2,0); di[81]=cmb(tcmb(81),2,0);
               di[96]=cmb(tcmb(96),2,0); di[97]=cmb(tcmb(97),2,0); break;
   default:
               di[64]=cmb(tcmb(66),2,0); di[65]=cmb(tcmb(67),2,0);
               di[80]=cmb(tcmb(82),2,0); di[81]=cmb(tcmb(83),2,0);
               di[96]=cmb(tcmb(98),2,0); di[97]=cmb(tcmb(99),2,0); break;
   } break;
 case right:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK:
               di[78]=cmb(tcmb(78),2,0);   di[79]=cmb(tcmb(79),2,0);
               di[94]=cmb(tcmb(94),2,0);   di[95]=cmb(tcmb(95),2,0);
               di[110]=cmb(tcmb(110),2,0); di[111]=cmb(tcmb(111),2,0); break;
   default:
               di[78]=cmb(tcmb(76),2,0);   di[79]=cmb(tcmb(77),2,0);
               di[94]=cmb(tcmb(92),2,0);   di[95]=cmb(tcmb(93),2,0);
               di[110]=cmb(tcmb(108),2,0); di[111]=cmb(tcmb(109),2,0); break;
   } break;
 }
}



void zmap::Ugo()
{
 undomapscr=screens[currscr];
 can_undo=true;
}


void zmap::Uhuilai()
{
 if(can_undo)
  swap(screens[currscr],undomapscr);
}

void zmap::Copy()
{
 if(screens[currscr].valid&mVALID) {
  copymapscr=screens[currscr];
  can_paste=true;
  copymap=currmap;
  copyscr=currscr;
  }
}


void zmap::Paste()
{
 if(can_paste) {
  Ugo();
  screens[currscr]=copymapscr;
  }
}

void zmap::PasteEnemies()
{
 if(can_paste) {
  for(int i=0; i<10; i++)
    screens[currscr].enemy[i]=copymapscr.enemy[i];
  }
}


void zmap::dowarp(int type)
{
 if(type==0)
 {
   int dmap=screens[currscr].warpdmap;
   int scr=screens[currscr].warpscr;
   switch(screens[currscr].warptype) {
    case 1:
    case 2:
    case 3:
      setCurrMap(DMaps[dmap].map);
      if(DMaps[dmap].type==dmDNGN)
        setCurrScr(scr+DMaps[dmap].xoff);
      else
        setCurrScr(scr);
      break;
   }
 }
 else if(type==1)
 {
   int dmap=screens[currscr].warpdmap2;
   int scr=screens[currscr].warpscr2;
   switch(screens[currscr].warptype2) {
    case 1:
    case 2:
    case 3:
      setCurrMap(DMaps[dmap].map);
      if(DMaps[dmap].type==dmDNGN)
        setCurrScr(scr+DMaps[dmap].xoff);
      else
        setCurrScr(scr);
      break;
   }
 }
 else
 {
   int dmap=misc.wind[type-2].dmap;
   int scr=misc.wind[type-2].scr;
   setCurrMap(DMaps[dmap].map);
   if(DMaps[dmap].type==dmDNGN)
     setCurrScr(scr+DMaps[dmap].xoff);
   else
     setCurrScr(scr);
 }
}

void zmap::dowarp(int ring,int index)
{
  int dmap=misc.warp[ring].dmap[index];
  int scr=misc.warp[ring].scr[index];
  setCurrMap(DMaps[dmap].map);
  if(DMaps[dmap].type==dmDNGN)
    setCurrScr(scr+DMaps[dmap].xoff);
  else
    setCurrScr(scr);
}



/******************************/
/******** ZQuest stuff ********/
/******************************/

int msg_count=0;

char *MsgString(int index)
{
 bound(index,0,MAXMSGS-1);
 char *s=MsgStrings[index].s;
 while(*s==' ')
  s++;
 return s;
}

void reset_msgstr(int index)
{
 bound(index,0,MAXMSGS-1);
 char *s=MsgStrings[index].s;
 for(int i=0; i<76; i++)
   *(s++)=0;
}

void init_msgstrs()
{
 for(int i=0; i<MAXMSGS; i++)
   reset_msgstr(i);
 strcpy(MsgStrings[0].s,"(none)");
 msg_count=1;
}


const char zqsheader[30]="Zelda Classic String Table\n\x01";

bool save_msgstrs(char *path)
{
 FILE *f=fopen(path,"wb");
 if(!f)
  return false;
 if(fwrite(zqsheader,1,30,f)<30) {
  fclose(f);
  return false;
  }
 if(fwrite(&msg_count,2,1,f)<1) {
  fclose(f);
  return false;
  }
 if(fwrite(MsgStrings,sizeof(MsgStr),MAXMSGS,f)<MAXMSGS) {
  fclose(f);
  return false;
  }
 fclose(f);
 return true;
}


int load_msgstrs(char *path)
{
 FILE *f=fopen(path,"rb");
 if(!f)
  return 1;

 char buf[30];
 if(fread(buf,1,30,f)<30) {
  fclose(f);
  return 2;
  }
 if(strcmp(buf,zqsheader)) {
  fclose(f);
  return 4;
  }

 if(fread(&msg_count,2,1,f)<1) {
  fclose(f);
  return 2;
  }
 if(fread(MsgStrings,sizeof(MsgStr),MAXMSGS,f)<MAXMSGS) {
  fclose(f);
  return 2;
  }
 fclose(f);
 return 0;
}


bool save_pals(char *path)
{
 FILE *f=fopen(path,"wb");
 if(!f)
   return false;
 if(fwrite(colordata,1,psTOTAL,f)<psTOTAL) {
   fclose(f);
   return false;
   }
 fclose(f);
 return true;
}


int load_pals(char *path)
{
 FILE *f=fopen(path,"rb");
 if(!f)
   return false;
 if(fread(colordata,1,psTOTAL,f)<psTOTAL) {
   fclose(f);
   return false;
   }
 fclose(f);
 loadfullpal();
 loadlvlpal(Color);
 return true;
}



bool save_dmaps(char *path)
{
 return writefile(path,DMaps,sizeof(dmap)*MAXDMAPS)==sizeof(dmap)*MAXDMAPS;
}


bool load_dmaps(char *path)
{
  int size = sizeof(dmap)*MAXDMAPS;
  dmap *buf = (dmap*)malloc(size);
  if(readfile(path,buf,size)!=size) {
    free(buf);
    return false;
    }
  for(int i=0;i<MAXDMAPS; i++)
    DMaps[i]=buf[i];
  free(buf);
  return true;
}



bool save_combos(char *path)
{
 return writefile(path,combobuf,8192)==8192;
}


bool load_combos(char *path)
{
  combo *buf = (combo*)malloc(8192);
  if(readfile(path,buf,8192)!=8192) {
    free(buf);
    return false;
    }
  for(int i=0;i<512; i++)
    combobuf[i]=buf[i];
  free(buf);
  return true;
}


void reset_tiles()
{
  byte *si=(byte*)data[TIL_NES].dat;
  for(int i=0; i<TILEBUF_SIZE; i++)
    tilebuf[i]=*(si++);
  usetiles=false;
}


bool save_tiles(char *path)
{
 return writefile(path,tilebuf,TILEBUF_SIZE)==TILEBUF_SIZE;
}


bool load_tiles(char *path)
{
  struct ffblk f;
  if(findfirst(path,&f,0))
    return false;

  if(f.ff_fsize!=TILEBUF_SIZE)
    return false;

  if(readfile(path,tilebuf,TILEBUF_SIZE)!=TILEBUF_SIZE) {
    reset_tiles();
    return false;
    }

  return true;
}




void setMapCount(int c)
{
  bound(c,1,MAXMAPS);
  header.map_count=c;
  int currmap=Map.getCurrMap();
  Map.setCurrMap(bound(currmap,0,c-1)); // for bound checking
}



void reset_mapcolors()
{
  misc.colors = (zcolors){ 1,17, 18,19, 2, 18, 0,3, 0,2, 19,18,18,17 };
}



int init_quest()
{
 strcpy(header.id_str,QH_IDSTR);
 header.zelda_version = ZELDA_VERSION;
 header.internal = INTERNAL_VERSION;
 header.quest_number = 0;
 header.author[0] = 0;
 header.title[0] = 0;
 header.pwdkey = 0;
 header.password[0] = 0;

 for(int i=0; i<ZQ_MAXDATA; i++)
   header.data_flags[i]=0;

 Map.clearall();
 setMapCount(1);
 init_msgstrs();
 header.str_count=1;

 if(data) {
  for(int i=0; i<512; i++)
    combobuf[i]=((combo*)data[DAT_COMBO].dat)[i];
  reset_tiles();
  }

 init_colordata();

 for(int i=0; i<MAXDMAPS; i++)
   reset_dmap(i);
 for(int i=0; i<MAXMIDIS; i++)
   reset_midi(customMIDIs+i);

 byte *di = (byte*)&misc;
 for(unsigned i=0; i<sizeof(miscQdata); i++)
   *(di++)=0;

 for(int i=0; i<8; i++)
   misc.triforce[i]=i+1;

 reset_mapcolors();
 TheMaps[TEMPLATE] = *((mapscr*)data[DAT_TEMPLATE].dat);

 refresh_pal();
 saved=true;
 return 0;
}


void set_questpwd(char *pwd)
{
  if(strlen(pwd)==0)
  {
    header.pwdkey=0;
    for(int i=0; i<30; i++)
      header.password[i]=rand();
  }
  else
  {
    short key=(rand()&0xFFF0)+11;
    header.pwdkey=key;
    memcpy(header.password,pwd,30);
    for(int i=0; i<30; i++)
    {
      header.password[i] += key;
      int t=key>>15;
      key = (key<<1)+t;
    }
  }
}


void get_questpwd(char *pwd)
{
  if(header.pwdkey==0)
    pwd[0]=0;
  else
  {
    short key = header.pwdkey;
    memcpy(pwd,header.password,30);
    pwd[30]=0;
    for(int i=0; i<30; i++)
    {
      pwd[i] -= key;
      int t=key>>15;
      key = (key<<1)+t;
    }
  }
}


static DIALOG pwd_dlg[] =
{
 /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
 { d_shadow_box_proc, 48,   80,  224,   80,   vc(14),  vc(1),  0,       0,          0,             0,       NULL },
 { d_ctext_proc,      160,  88,  152,   8,    vc(15),  vc(1),  0,       0,          0,             0,       "Requires Authorization" },
 { d_text_proc,       64,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "File name:" },
// 3 (filename)
 { d_text_proc,       148,  104, 128,   8,    vc(11),  vc(1),  0,       0,          24,            0,       NULL },
 { d_text_proc,       64,   112,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       "Password:" },
// 5 (password)
 { d_edit_proc,       140,  112, 120,   8,    vc(12),  vc(1),  0,       0,          24,            0,       NULL },
 { d_button_proc,     90,   136,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       "OK" },
 { d_button_proc,     170,  136,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       "Cancel" },
 { NULL }
};


bool quest_access(char *filename)
{
  if(debug || header.pwdkey==0)
    return true;

  char pwd[32];
  char prompt[32]="";

  pwd_dlg[3].dp=get_filename(filename);
  get_questpwd(pwd);
  pwd_dlg[5].dp=prompt;
  popup_dialog(pwd_dlg,5);

  return strcmp(pwd,prompt)==0;
}



// wrapper to reinitialize everything on an error
int load_quest(char *filename)
{
 int ret=loadquest(filename,&header,&misc,customMIDIs);

 if(ret>1 && ret<5)
   init_quest();
 else if(!quest_access(filename))
 {
   init_quest();
   ret=6;
 }
 else
 {
   msg_count = header.str_count;
   usetiles = header.data_flags[0];
   setMapCount(header.map_count); // force a bound check
   refresh_pal();
 }
 return ret;
}



bool write_midi(MIDI *m,PACKFILE *f)
{
   int c;

   pack_mputw(m->divisions,f);

   for(c=0; c<MIDI_TRACKS; c++)
   {
      pack_mputl(m->track[c].len,f);

      if(m->track[c].len > 0)
      {
         if(!pfwrite(m->track[c].data,m->track[c].len,f))
            return false;
      }
   }
   return true;
}



int save_quest(char *filename)
{
 strcpy(header.id_str,QH_IDSTR);
 header.zelda_version = ZELDA_VERSION;
 header.internal = INTERNAL_VERSION;
 header.str_count = msg_count;
 header.data_flags[0] = usetiles;
 for(int i=0; i<MAXMIDIS; i++)
   set_bit(header.data_flags+ZQ_MIDIS,i,int(customMIDIs[i].midi!=NULL));


 PACKFILE *f = pack_fopen(filename,F_WRITE_PACKED);
 if(!f)
   return 1;
 if(!pfwrite(&header,sizeof(zquestheader),f)) {
   pack_fclose(f);
   return 2;
   }
 if(!pfwrite(MsgStrings,sizeof(MsgStr)*MAXMSGS,f)) {
   pack_fclose(f);
   return 3;
   }
 if(!pfwrite(DMaps,sizeof(dmap)*MAXDMAPS,f)) {
   pack_fclose(f);
   return 4;
   }
 if(!pfwrite(&misc,sizeof(miscQdata),f)) {
   pack_fclose(f);
   return 5;
   }
 for(int i=0; i<header.map_count; i++) {
   if(!pfwrite(TheMaps+(i*130),sizeof(mapscr)*130,f)) {
     pack_fclose(f);
     return 6+i;
     }
   }
 if(!pfwrite(TheMaps+(TEMPLATE),sizeof(mapscr),f)) {
   pack_fclose(f);
   return 15;
   }
 if(!pfwrite(combobuf,sizeof(combo)*512,f)) {
   pack_fclose(f);
   return 16;
   }
 if(!pfwrite(colordata,psTOTAL,f)) {
   pack_fclose(f);
   return 17;
   }
 if(header.data_flags[0])
   if(!pfwrite(tilebuf,TILEBUF_SIZE,f))
   {
     pack_fclose(f);
     return 18;
   }
 for(int i=0; i<MAXMIDIS; i++)
 {
   if(get_bit(header.data_flags+ZQ_MIDIS,i))
   {
     if(!pfwrite(customMIDIs+i,sizeof(music),f) ||
        !write_midi(customMIDIs[i].midi,f) )
     {
       pack_fclose(f);
       return 19+i;
     }

   }
 }

 pack_fclose(f);
 return 0;
}



