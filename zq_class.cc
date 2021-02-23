//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_class.cc
//
//  Contains zmap class and other main code for ZQuest.
//
//--------------------------------------------------------

/************************/
/****** ZMAP class ******/
/************************/


void reset_dmap(int index)
{
 bound(index,0,MAXDMAPS-1);
 byte *di = ((byte*)(DMaps+index));
 for(unsigned i=0; i<sizeof(dmap); i++)
   *(di++)=0;
 sprintf(DMaps[index].title, "                    ");
 sprintf(DMaps[index].intro, "                                                                        ");

}

void mapfix_0x166(mapscr *scr)
{
   byte buf[440];

   byte *di = (byte*)scr;
   for(unsigned i=0; i<440; i++, di++)
   {
     buf[i] = *di;
     *di = 0;
   }

   di = (byte*)scr;
   for(unsigned i=0; i<48; i++, di++)
     *di = buf[i];

   di += 40;
   for(unsigned i=0; i<352; i++, di++)
     *di = buf[i+48];


   scr->flags  = ((oldmapscr*)scr)->__flags;
   scr->flags2 = ((oldmapscr*)scr)->__flags2;
   scr->catchall  = ((oldmapscr*)scr)->__catchall;
   scr->secret[0] =
   scr->secret[1] =
   scr->secret[2] =
   scr->secret[3] = ((oldmapscr*)scr)->__secret;
   ((oldmapscr*)scr)->__flags =
   ((oldmapscr*)scr)->__flags2 =
   ((oldmapscr*)scr)->__secret =
   ((oldmapscr*)scr)->__catchall = 0;
}

void setMapCount(int c);

class zmap {
 mapscr *screens;
 int currmap,copymap;
 int currscr,copyscr;
 int scrpos[MAXMAPS+1];

 // editing screens
// mapscr undomapscr
 mapscr copymapscr;
 mapscr undomap[128];
 bool can_undo,can_paste,can_undo_map,can_paste_map,screen_copy;

public:

 zmap() { can_undo=can_paste=false; }
 bool CanUndo() { return can_undo; }
 bool CanPaste() { return can_paste; }
 int  CopyScr() { return (copymap<<8)+copyscr; }
 void Ugo();
 void Uhuilai();
 void Copy();
 void Paste();
 void PasteAll();
 void PasteToAll();
 void PasteAllToAll();
 void PasteEnemies();
 bool isDungeon(int scr)
 {
   for(int i=0; i<4; i++)
     if((screens[scr].data[i]&0xFF)!=(screens[TEMPLATE].data[i]&0xFF))
       return false;
   return true;
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
 void reset_templates() {
   short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
   byte *tmpptr2=(byte*)tmpptr+1;//skip build
   word *tilestoskip=(word*)tmpptr2; //check number of tiles
   byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
   //skip past tile section
   byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
   word *combostoread=(word*)tmpptr4; //check number of combos
   byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
   //skip past combo section
   byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
   tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
   tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
   tmpptr6=tmpptr6+(MAXLEVELS*PALNAMESIZE); //skip past palette names
   tmpptr6=tmpptr6+(sizeof(itemdata)*MAXITEMS); //skip past item info
   tmpptr6=tmpptr6+(sizeof(wpndata)*MAXITEMS); //skip past weapon info
   tmpptr6=tmpptr6+(sizeof(byte)*8); //skip past triforce info
   tmpptr6=tmpptr6+(sizeof(word)*4); //skip past game icon info
   byte *num_maps=tmpptr6+(sizeof(zcolors)); //skip past misc color info
                                             //and read number of maps
   tmpptr6=num_maps+(sizeof(byte)); //skip past number of maps
   memcpy(TheMaps+128, tmpptr6, (sizeof(mapscr)*4)); //read template info
   if (header.map_count<(*num_maps)) {
     setMapCount(*num_maps);
//     header.map_count=(*num_maps);
   }
   if ((*num_maps)>1) { //dungeon templates
   tmpptr6=tmpptr6+(sizeof(mapscr)*4); //skip past number of maps
   memcpy(TheMaps+132+128, tmpptr6, (sizeof(mapscr)*4)); //read template info
   }



 }
 void clearmap()
 {
   if(currmap<MAXMAPS) {
    for(int i=0; i<MAPSCRS; i++)
     clearscr(i);
    setCurrScr(0);
    reset_templates();
   }
 }
 void clearscr(int scr);
 void savescreen();
 int  load(char *path);
 int  save(char *path);
 void check_alignments(BITMAP* dest,int x,int y,int scr=-1);
 void draw(BITMAP *dest,int x,int y,int flags,int scr=-1);
 void draw_template(BITMAP *dest,int x,int y);
 void draw_secret(BITMAP *dest, int pos);
 void scroll(int dir);
 mapscr *CurrScr()                     { return screens+currscr; }
 mapscr *Scr(int scr)                  { return screens+scr; }
 mapscr *AbsoluteScr(int scr)          { return TheMaps+scr; }
 mapscr *AbsoluteScr(int map, int scr) { return TheMaps+(map*MAPSCRS)+scr; }
 int  getCurrMap()                     { return currmap; }
 void setCurrMap(int index)
 {
   int oldmap=currmap;
   scrpos[currmap]=currscr;
   currmap=bound(index,0,MAXMAPS);
   screens=TheMaps+(currmap*MAPSCRS);
   currscr=scrpos[currmap];
   if(currmap!=oldmap)
     can_undo=false;
 }
 int  getCurrScr()        { return currscr; }
 void setCurrScr(int scr)
 {
   int oldscr=currscr;
   currscr=bound(scr,0,MAPSCRS-1);
   setcolor(screens[currscr].color);
   if(currscr!=oldscr)
     can_undo=false;
 }
 void setcolor(int c)
 {
   if(screens[currscr].valid&mVALID)
   {
     screens[currscr].color = c;
     if(Color!=c)
     {
       Color = c;
       loadlvlpal(c);
     }
   }
 }
 int getcolor()
 {
   return screens[currscr].color;
 }

 void resetflags()
 {
  byte *di=&(screens[currscr].valid);
  for(int i=1; i<48; i++)
    *(di+i)=0;
 }
 word tcmb(int pos)
 {
  return (screens[TEMPLATE].data[pos]&0xFF) + (screens[TEMPLATE].cpage<<8);
 }
 word tcmbdat(int pos)
 {
  return screens[TEMPLATE].data[pos];
 }
 int tcmbflag(int pos)
 {
  return screens[TEMPLATE].sflag[pos];
 }
 void put_vert_door(BITMAP *dest,int tmppos,int x,int y,int cs);
 void over_vert_door(BITMAP *dest,int tmppos,int x,int y,int cs);
 void put_horiz_door(BITMAP *dest,int tmppos,int x,int y,int cs);

 void TemplateAll()
 {
   for(int i=0; i<128; i++)
   {
     if((screens[i].valid&mVALID) && isDungeon(i))
       Template(-1,i);
   }
 }
 void Template(int floor)
 {
   Template(floor,currscr);
 }

 void Template(int floor,int scr)
 {
  if(scr==TEMPLATE)
    return;

  if(!(screens[scr].valid&mVALID))
    screens[scr].color=Color;
  screens[scr].valid|=mVALID;

  for(int i=0; i<32; i++) {
    screens[scr].data[i]=screens[TEMPLATE].data[i];
    screens[scr].sflag[i]=screens[TEMPLATE].sflag[i];
  }
  for(int i=144; i<176; i++) {
    screens[scr].data[i]=screens[TEMPLATE].data[i];
    screens[scr].sflag[i]=screens[TEMPLATE].sflag[i];
  }
  for(int y=2; y<=9; y++) {
    int j=y<<4;
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    j+=12;
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    screens[scr].data[j]=screens[TEMPLATE].data[j];
    screens[scr].sflag[j]=screens[TEMPLATE].sflag[j++];
    }

  if(floor!=-1) {
    for(int y=2; y<9; y++)
     for(int x=2; x<14; x++) {
      int i=(y<<4)+x;
      screens[scr].data[i] &= 0xF000;
      screens[scr].data[i] |= floor&0x0FFF;
      }
    }
  for(int i=0; i<4; i++)
    putdoor(scr,i,screens[scr].door[i]);
 }

 void putdoor(int side,int door)
 {
   putdoor(currscr,side,door);
 }
 void putdoor(int scr,int side,int door);
 void dowarp(int type);
 void dowarp(int ring,int index);
} Map;




void zmap::clearscr(int scr)
{
 byte *di=((byte*)screens)+(scr*sizeof(mapscr));
 for(unsigned i=0; i<sizeof(mapscr); i++)
   *(di++) = 0;

 for(int i=0; i<4; i++)
   screens[scr].secret[i] = i;
 for(int i=0; i<16; i++)
   screens[scr].secret[i+4] = ((i>>2)<<4) + (i&3) + 12;

 screens[scr].valid=mVERSION;
}


void zmap::savescreen() {
 FILE *f=fopen("def.map","wb");
 if(!f)
  return;
  if(fwrite(screens+currscr,sizeof(mapscr),1,f)<1) {
   fclose(f);
   return;
   }
 fclose(f);
 return;
}


bool read_old_mapfile(mapscr *scr, FILE *f)
{
   byte *di = (byte*)scr;
   for(unsigned i=0; i<sizeof(mapscr); i++, di++)
     *di = 0;

   if(fread(scr, 1, 48, f) < 48)
     return false;
   if(fread(((byte*)scr)+88, 1, 352, f) < 352)
     return false;

   scr->flags  = ((oldmapscr*)scr)->__flags;
   scr->flags2 = ((oldmapscr*)scr)->__flags2;
   scr->catchall  = ((oldmapscr*)scr)->__catchall;
   scr->secret[0] =
   scr->secret[1] =
   scr->secret[2] =
   scr->secret[3] = ((oldmapscr*)scr)->__secret;
   ((oldmapscr*)scr)->__flags =
   ((oldmapscr*)scr)->__flags2 =
   ((oldmapscr*)scr)->__secret =
   ((oldmapscr*)scr)->__catchall = 0;

   return true;
}


static char *loaderror[] = {"OK","File not found","Incomplete data",
  "Invalid version","Invalid file"};

int zmap::load(char *path)
{
// int size=file_size(path);

 FILE *f=fopen(path,"rb");
 if(!f)
  return 1;

 clearmap();

 short version;
 byte build;
 //get the version
 if(fread(&version,1,sizeof(short),f) != sizeof(short)) {
   goto file_error;
 }
 //get the build
 if(fread(&build,1,sizeof(byte),f) != sizeof(byte)) {
   goto file_error;
 }

 for(int i=0; i<MAPSCRS; i++)
 {
  if(fread(screens+i,sizeof(mapscr),1,f)<1)
    goto file_error;
 }

 fclose(f);
 if(!(screens[0].valid&mVERSION)) {
   clearmap();
   return 3;
   }
 setCurrScr(0);
 return 0;

 file_error:
 fclose(f);
 clearmap();
 return 2;
}


int zmap::save(char *path)
{
 FILE *f=fopen(path,"wb");
 if(!f)
  return 1;
 short version=ZELDA_VERSION;
 byte  build=VERSION_BUILD;
 if(fwrite(&version,1,sizeof(short),f) != sizeof(short)) {
   fclose(f);
   return 3;
 }
 if(fwrite(&build,1,sizeof(byte),f) != sizeof(byte)) {
   fclose(f);
   return 3;
 }
 for(int i=0; i<MAPSCRS; i++) {
  if(fwrite(screens+i,sizeof(mapscr),1,f)<1) {
   fclose(f);
   return 2;
   }
  }
 fclose(f);
 return 0;
}


void put_combo(BITMAP *dest,int x,int y,word cmbdat,int flags,int page,int sflag)
{

  newcombo c = combobuf[(cmbdat&0xFF)+(page<<8)];

  if(c.tile==0) {
    rectfill(dest,x,y,x+15,y+15,0);
    rectfill(dest,x+3,y+3,x+12,y+12,vc(4));
    return;
    }

  putcombo(dest,x,y,cmbdat,page);

  for(int i=0; i<4; i++) {
    int tx=((i&2)<<2)+x;
    int ty=((i&1)<<3)+y;
    if((flags&cWALK) && (c.walk&(1<<i)))
      rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
    }

//  if((flags&cFLAGS)&&(cmbdat&0xF800))
  if((flags&cFLAGS)&&(sflag))
  {
//    rectfill(dest,x,y,x+15,y+15,vc(cmbdat>>10+1));
    rectfill(dest,x,y,x+15,y+15,vc(sflag));
    text_mode(-1);
    textprintf(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),"%d",sflag);
  }
  if(flags&cCSET)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    text_mode(inv?vc(15):vc(0));
    textprintf(dest,sfont,x+9,y+9,inv?vc(0):vc(15),"%d",(cmbdat>>8)&7);
  }
  else if(flags&cCTYPE)
  {
    bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
    text_mode(inv?vc(15):vc(0));
    textprintf(dest,sfont,x+1,y+9,inv?vc(0):vc(15),"%d",c.type);
  }
}


void over_combo(BITMAP *dest,int x,int y,word cmbdat,int flags,int page)
{
  newcombo c = combobuf[(cmbdat&0xFF)+(page<<8)];

  if(c.tile)
    overtile16(dest,c.tile,x,y,(cmbdat>>8)&7,c.flip);
}


void zmap::put_vert_door(BITMAP *dest,int tmppos,int x,int y,int cs)
{
  put_combo(dest,x,y,(tcmb(tmppos)&0xFF)+(cs<<8),0,CPage,0);
  put_combo(dest,x+16,y,(tcmb(tmppos+1)&0xFF)+(cs<<8),0,CPage,0);
}

void zmap::over_vert_door(BITMAP *dest,int tmppos,int x,int y,int cs)
{
  over_combo(dest,x,y,(tcmb(tmppos)&0xFF)+(cs<<8),0,CPage);
  over_combo(dest,x+16,y,(tcmb(tmppos+1)&0xFF)+(cs<<8),0,CPage);
}


void zmap::put_horiz_door(BITMAP *dest,int tmppos,int x,int y,int cs)
{
  put_combo(dest,x,y,(tcmb(tmppos)&0xFF)+(cs<<8),0,CPage,0);
  put_combo(dest,x,y+16,(tcmb(tmppos+16)&0xFF)+(cs<<8),0,CPage,0);
  put_combo(dest,x,y+32,(tcmb(tmppos+32)&0xFF)+(cs<<8),0,CPage,0);
}



inline word cmb(int index,int cset)
{ return (index&0xFF)+((cset&7)<<8); }


void zmap::check_alignments(BITMAP* dest,int x,int y,int scr=-1) {
  static int alignment_arrow_timer=0;
  alignment_arrow_timer++;
  if (alignment_arrow_timer>31) {
    if (scr<0) {
      scr=currscr;
    }
    if ((scr<128)) { //do the misalignment arrows
      word cmbcheck1, cmbcheck2;
      newcombo combocheck1, combocheck2;
//      word cmbcheck3;
//      newcombo combocheck3;
      if (scr>15) {
        for(int i=1; i<15; i++) //check the top row (except the corners)
        {
          cmbcheck1 = screens[scr].data[i];
          cmbcheck2 = screens[scr-16].data[i+160];
          combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
          combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr-16].cpage<<8)];
          if (((combocheck1.walk&5)*2)!=(combocheck2.walk&10)) {
            masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,0*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
          }
        }
      }

      if (scr<112) {
        for(int i=161; i<175; i++) //check the bottom row (except the corners)
        {
          cmbcheck1 = screens[scr].data[i];
          cmbcheck2 = screens[scr+16].data[i-160];
          combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
          combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr+16].cpage<<8)];
          if ((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) {
            masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,1*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
          }
        }
      }

      if (scr%16) {
        for(int i=16; i<160; i+=16) //check the left side (except the corners)
        {
          cmbcheck1 = screens[scr].data[i];
          cmbcheck2 = screens[scr-1].data[i+15];
          combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
          combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr-1].cpage<<8)];
          if (((combocheck1.walk&3)*4)!=(combocheck2.walk&12)) {
           masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,2*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
          }
        }
      }

      if ((scr%16)<15) {
        for(int i=31; i<175; i+=16) //check the right side (except the corners)
        {
          cmbcheck1 = screens[scr].data[i];
          cmbcheck2 = screens[scr+1].data[i-15];
          combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
          combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr+1].cpage<<8)];
          if ((combocheck1.walk&12)!=((combocheck2.walk&3)*4)) {
            masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,3*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
          }
        }
      }

/*

      //Check the 4 corners
      int i=0;  //top left tile
      cmbcheck1 = screens[scr].data[i];
      combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
      cmbcheck2 = screens[scr-16].data[i+160]; //one screen up
      cmbcheck3 = screens[scr-1].data[i+15]; //one screen left
      combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr-16].cpage<<8)];
      combocheck3 = combobuf[(cmbcheck3&0xFF)+(screens[scr-1].cpage<<8)];

      if ((scr>15)&&(scr%16)) { //not top row or left side of map
        if ((((combocheck1.walk&5)*2)!=(combocheck2.walk&10))&& //top
            (((combocheck1.walk&3)*4)!=(combocheck3.walk&12))) { //left
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,4*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if (((combocheck1.walk&5)*2)!=(combocheck2.walk&10)) { //top
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,0*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if (((combocheck1.walk&3)*4)!=(combocheck3.walk&12)) { //left
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,2*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if (scr>15) { //not top row (left side)
        if (((combocheck1.walk&5)*2)!=(combocheck2.walk&10)) { //top
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,0*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if (scr%16){ //not left side (top row)
        if (((combocheck1.walk&3)*4)!=(combocheck3.walk&12)) { //left
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,2*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      }

      i=15;  //top right tile
      cmbcheck1 = screens[scr].data[i];
      combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
      cmbcheck2 = screens[scr-16].data[i+160]; //one screen up
      cmbcheck3 = screens[scr+1].data[i-15]; //one screen right
      combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr-16].cpage<<8)];
      combocheck3 = combobuf[(cmbcheck3&0xFF)+(screens[scr+1].cpage<<8)];
      if ((scr>15)&&((scr%16)<15)) { //not top row or right side
        if ((((combocheck1.walk&5)*2)!=(combocheck2.walk&10)) &&
            ((combocheck1.walk&12)!=((combocheck3.walk&3)*4))) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,5*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if (((combocheck1.walk&5)*2)!=(combocheck2.walk&10)) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,0*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if ((combocheck1.walk&12)!=((combocheck3.walk&3)*4)) {
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,3*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if (scr>15) { //not top row (right side)
        if (((combocheck1.walk&5)*2)!=(combocheck2.walk&10)) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,0*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if ((scr%16)<15){ //not right side (top row)
        if ((combocheck1.walk&12)!=((combocheck3.walk&3)*4)) {
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,3*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      }

      i=160;  //bottom left tile
      cmbcheck1 = screens[scr].data[i];
      combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
      cmbcheck2 = screens[scr+16].data[i-160]; //one screen down
      cmbcheck3 = screens[scr-1].data[i+15]; //one screen left
      combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr+16].cpage<<8)];
      combocheck3 = combobuf[(cmbcheck3&0xFF)+(screens[scr-1].cpage<<8)];

      if ((scr<112)&&(scr%16)) { //not top row or left side
        if (((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) &&
            (((combocheck1.walk&3)*4)!=(combocheck3.walk&12))) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,6*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if ((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,1*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if (((combocheck1.walk&3)*4)!=(combocheck3.walk&12)) {
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,2*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if (scr>15) { //not top row (left side)
        if ((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,1*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if (scr%16){ //not left side (top row)
        if (((combocheck1.walk&3)*4)!=(combocheck3.walk&12)) {
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,2*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      }

      i=175;  //bottom right tile
      cmbcheck1 = screens[scr].data[i];
      combocheck1 = combobuf[(cmbcheck1&0xFF)+(screens[scr].cpage<<8)];
      cmbcheck2 = screens[scr+16].data[i-160]; //one screen down
      cmbcheck3 = screens[scr+1].data[i-15]; //one screen right
      combocheck2 = combobuf[(cmbcheck2&0xFF)+(screens[scr+16].cpage<<8)];
      combocheck3 = combobuf[(cmbcheck3&0xFF)+(screens[scr+1].cpage<<8)];
      if ((scr<112)&&((scr%16)<15)) { //not top row or right side
        if (((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) &&
            ((combocheck1.walk&12)!=((combocheck3.walk&3)*4))) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,7*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if ((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,1*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        } else if ((combocheck1.walk&12)!=((combocheck3.walk&3)*4)) {
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,3*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if (scr>15) { //not top row (right side)
        if ((combocheck1.walk&10)!=((combocheck2.walk&5)*2)) {
          masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,1*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      } else if ((scr%16)<15){ //not right side (top row)
        if ((combocheck1.walk&12)!=((combocheck3.walk&3)*4)) {
         masked_blit((BITMAP*)data[BMP_ARROWS].dat,dest,3*17+1,1,((i&15)<<4)+x,(i&0xF0)+y,16,16);
        }
      }

*/

    }

    alignment_arrow_timer++;
    if (alignment_arrow_timer>63) {
      alignment_arrow_timer=0;
    }
  }
}

void zmap::draw(BITMAP* dest,int x,int y,int flags,int scr=-1)
{
 if(scr<0)
   scr=currscr;

  mapscr* layer=Scr(scr);
  int layermap, layerscreen;
  layermap=layer->layermap[CurrentLayer-1]-1;
  if (layermap<0) {
    CurrentLayer=0;
  }

 if(!(screens[scr].valid&mVALID)) {
  rectfill(dest,x,y,x+255,y+175,dvc(1));
  return;
  }

 int dark = screens[scr].flags&4;

 if (LayerMaskInt[0]!=0) {
   for(int i=0; i<176; i++)
   {
     word cmbdat = screens[scr].data[i];
     int cmbflag = screens[scr].sflag[i];
     put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,flags|dark,CPage,cmbflag);
   }
 } else {
   rectfill(dest,x,y,x+255,y+175,0);
 }

 int cs=2;

 switch(screens[scr].door[up]) {
   case dLOCKED:      put_vert_door(dest,39,x+112,y+16,cs); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_vert_door(dest,55,x+112,y+16,cs); break;
   case dBOMB:        put_vert_door(dest,41,x+112,y+16,cs); over_vert_door(dest,57,x+112,y+32,cs); break;
   case dWALK:        put_combo(dest,x+120,y+16,(tcmb(34)&0xFF)+(cs<<8),flags,CPage,0); break;
   }
 switch(screens[scr].door[down]) {
   case dLOCKED:      put_vert_door(dest,135,x+112,y+144,cs); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_vert_door(dest,119,x+112,y+144,cs); break;
   case dBOMB:        put_vert_door(dest,137,x+112,y+144,cs); over_vert_door(dest,121,x+112,y+128,cs); break;
   case dWALK:        put_combo(dest,x+120,y+144,(tcmb(34)&0xFF)+(cs<<8),flags,CPage,0); break;
   }
 switch(screens[scr].door[left]) {
   case dLOCKED:      put_horiz_door(dest,68,x+16,y+64,cs); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_horiz_door(dest,69,x+16,y+64,cs); break;
   case dBOMB:        put_combo(dest,x+16,y+80,(tcmb(114)&255)+(cs<<8),0,CPage,0);
                      over_combo(dest,x+32,y+80,(tcmb(115)&255)+(cs<<8),0,CPage); break;
   case dWALK:        put_combo(dest,x+16,y+80,(tcmb(34)&0xFF)+(cs<<8),flags,CPage,0); break;
   }
 switch(screens[scr].door[right]) {
   case dLOCKED:      put_horiz_door(dest,75,x+224,y+64,cs); break;
   case d1WAYSHUTTER:
   case dSHUTTER:     put_horiz_door(dest,74,x+224,y+64,cs); break;
   case dBOMB:        put_combo(dest,x+224,y+80,(tcmb(125)&255)+(cs<<8),0,CPage,0);
                      over_combo(dest,x+208,y+80,(tcmb(124)&255)+(cs<<8),0,CPage); break;
   case dWALK:        put_combo(dest,x+224,y+80,(tcmb(34)&0xFF)+(cs<<8),flags,CPage,0); break;
   }

 if(!(flags&cDEBUG))
 {
   for(int j=168; j<176; j++)
    for(int i=0; i<256; i++)
      if(((i^j)&1)==0)
        putpixel(dest,x+i,y+j,vc(0));
 }

 if((dark) && !(flags&cNODARK))
 {
   for(int j=0; j<80; j++)
    for(int i=0; i<80-j; i++)
      if(((i^j)&1)==0)
        putpixel(dest,x+i,y+j,vc(0));
 }

 if((screens[scr].item) && !(flags&cNOITEM))
 {
   frame=0;
   putitem(dest,screens[scr].itemx+x,screens[scr].itemy+y,screens[scr].item);
 }

  for (int k=0; k<4; k++) {
    if(LayerMaskInt[k+1]!=0) {
      layermap=layer->layermap[k]-1;
      if (layermap>-1) {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for (int i=0; i<176; i++) {
          overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cpage);
        }
      }
    }
  }

  for (int i=0; i<176; i++) {
    int ct1=screens[currscr].data[i];
    int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
    int ct3=combobuf[ct2].type;
    if (ct3==cOVERHEAD) {
      overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,screens[currscr].data[i],screens[currscr].cpage);
    }
  }

  for (int k=4; k<6; k++) {
    if(LayerMaskInt[k+1]!=0) {
      layermap=layer->layermap[k]-1;
      if (layermap>-1) {
        layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
        for (int i=0; i<176; i++) {
          overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cpage);
        }
      }
    }
  }


  if (ShowMisalignments) {
    check_alignments(dest,x,y,scr=-1);
/*
    int drawmap, drawscr;
    if (CurrentLayer==0) {
      layerscreen=-1;
    } else {
      drawmap=Map.CurrScr()->layermap[CurrentLayer-1]-1;
      drawscr=Map.CurrScr()->layerscreen[CurrentLayer-1];
      layerscreen=drawmap*MAPSCRS+drawscr;
    }
    check_alignments(dest,x,y,layerscreen);
*/
  }




}




void zmap::draw_template(BITMAP* dest,int x,int y)
{
  for(int i=0; i<176; i++)
  {
    word cmbdat = screens[TEMPLATE].data[i];
    int cmbflag = screens[TEMPLATE].sflag[i];
    put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,0,CPage,cmbflag);
  }
}

void zmap::draw_secret(BITMAP *dest, int pos)
{
  word cmbdat = screens[TEMPLATE].data[pos];
  int cmbflag = screens[TEMPLATE].sflag[pos];
  put_combo(dest,0,0,cmbdat,0,CPage,cmbflag);
}


void zmap::scroll(int dir)
{
 if(currmap<MAXMAPS) {
   switch(dir) {
   case up:    if(currscr>15)  setCurrScr(currscr-16); break;
   case down:  if(currscr<MAPSCRS-16) setCurrScr(currscr+16); break;
   case left:  if(currscr&15)  setCurrScr(currscr-1);  break;
   case right: if((currscr&15)<15 && currscr<MAPSCRS-1) setCurrScr(currscr+1); break;
   }
 }
}



void zmap::putdoor(int scr,int side,int door)
{
 screens[scr].door[side]=door;
 word *di=screens[scr].data;
 switch(side) {
 case up:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK: di[7]=cmb(tcmb(7),2);
               di[8]=cmb(tcmb(8),2);
               di[23]=cmb(tcmb(23),2);
               di[24]=cmb(tcmb(24),2); break;
   default:
               di[7]=cmb(tcmb(37),2);
               di[8]=cmb(tcmb(38),2);
               di[23]=cmb(tcmb(53),2);
               di[24]=cmb(tcmb(54),2); break;
   } break;
 case down:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK:
               di[151]=cmb(tcmb(151),2);
               di[152]=cmb(tcmb(152),2);
               di[167]=cmb(tcmb(167),2);
               di[168]=cmb(tcmb(168),2); break;
   default:
               di[151]=cmb(tcmb(117),2);
               di[152]=cmb(tcmb(118),2);
               di[167]=cmb(tcmb(133),2);
               di[168]=cmb(tcmb(134),2); break;
   } break;
 case left:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK:
               di[64]=cmb(tcmb(64),2); di[65]=cmb(tcmb(65),2);
               di[80]=cmb(tcmb(80),2); di[81]=cmb(tcmb(81),2);
               di[96]=cmb(tcmb(96),2); di[97]=cmb(tcmb(97),2); break;
   default:
               di[64]=cmb(tcmb(66),2); di[65]=cmb(tcmb(67),2);
               di[80]=cmb(tcmb(82),2); di[81]=cmb(tcmb(83),2);
               di[96]=cmb(tcmb(98),2); di[97]=cmb(tcmb(99),2); break;
   } break;
 case right:
   switch(door) {
   case dWALL:
   case dBOMB:
   case dWALK:
               di[78]=cmb(tcmb(78),2);   di[79]=cmb(tcmb(79),2);
               di[94]=cmb(tcmb(94),2);   di[95]=cmb(tcmb(95),2);
               di[110]=cmb(tcmb(110),2); di[111]=cmb(tcmb(111),2); break;
   default:
               di[78]=cmb(tcmb(76),2);   di[79]=cmb(tcmb(77),2);
               di[94]=cmb(tcmb(92),2);   di[95]=cmb(tcmb(93),2);
               di[110]=cmb(tcmb(108),2); di[111]=cmb(tcmb(109),2); break;
   } break;
 }
}


void zmap::Ugo()
{
 for (int x=0; x<128; x++)
 undomap[x]=screens[x];
 can_undo=true;
}

void zmap::Uhuilai()
{
 if(can_undo)
  for (int x=0; x<128; x++)
   swap(screens[x],undomap[x]);
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
  if(!(screens[currscr].valid&mVALID)) {
    screens[currscr].valid |= mVALID;
    screens[currscr].color = copymapscr.color;
    }
  for(int i=0; i<176; i++) {
    screens[currscr].data[i] = copymapscr.data[i];
    screens[currscr].sflag[i] = copymapscr.sflag[i];
  }
  if(isDungeon(currscr))
    for(int i=0; i<4; i++)
      putdoor(currscr,i,screens[currscr].door[i]);
  saved=false;
  }
}

void zmap::PasteAll()
{
 if(can_paste) {
  Ugo();
  screens[currscr]=copymapscr;
  saved=false;
  }
}

void zmap::PasteToAll()
{
 if(can_paste) {
  Ugo();
  for(int x=0; x<128; x++) {
      if(!(screens[x].valid&mVALID)) {
        screens[x].valid |= mVALID;
        screens[x].color = copymapscr.color;
        }
      for(int i=0; i<176; i++) {
        screens[x].data[i] = copymapscr.data[i];
        screens[x].sflag[i] = copymapscr.sflag[i];
      }
      if(isDungeon(x))
        for(int i=0; i<4; i++)
          putdoor(currscr,i,screens[x].door[i]);
  }
  saved=false;
  }
}


void zmap::PasteAllToAll()
{
 if(can_paste) {
  Ugo();
  for (int x=0; x<128; x++){
      screens[x]=copymapscr;
      saved=false;
  }
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
    case wtCAVE:
    case wtNOWARP:
      break;
    default:
      setCurrMap(DMaps[dmap].map);
      setCurrScr(scr+DMaps[dmap].xoff);
      break;
   }
 }
 else if(type==1)
 {
   int dmap=screens[currscr].warpdmap2;
   int scr=screens[currscr].warpscr2;
   switch(screens[currscr].warptype2) {
    case wtCAVE:
    case wtNOWARP:
      break;
    default:
      setCurrMap(DMaps[dmap].map);
      setCurrScr(scr+DMaps[dmap].xoff);
      break;
   }
 }
 else
 {
   int dmap=misc.wind[type-2].dmap;
   int scr=misc.wind[type-2].scr;
   setCurrMap(DMaps[dmap].map);
   setCurrScr(scr+DMaps[dmap].xoff);
 }
}


void zmap::dowarp(int ring,int index)
{
  int dmap=misc.warp[ring].dmap[index];
  int scr=misc.warp[ring].scr[index];
  setCurrMap(DMaps[dmap].map);
  setCurrScr(scr+DMaps[dmap].xoff);
}



/******************************/
/******** ZQuest stuff ********/
/******************************/

word msg_count=0;

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

 short version=ZELDA_VERSION;
 byte build=VERSION_BUILD;
 if(fwrite(&version,1,sizeof(short),f)!=sizeof(short)) {
   goto error;
 }
 if(fwrite(&build,1,sizeof(byte),f)!=sizeof(byte)) {
   goto error;
 }

 if(fwrite(&msg_count,1,sizeof(word),f)!=sizeof(word)) {
   goto error;
  }
 if(fwrite(MsgStrings,1,sizeof(MsgStr)*msg_count,f)!=sizeof(MsgStr)*msg_count) {
   goto error;
  }
 fclose(f);
 return true;

 error:
  fclose(f);
  return false;
 
}


int load_msgstrs(char *path)
{
 FILE *f=fopen(path,"rb");
 if(!f)
  return 1;

 short version;
 byte build;
 //get the version
 if(fread(&version,1,sizeof(short),f) != sizeof(short)) {
  fclose(f);
  return 2;
 }
 //get the build
 if(fread(&build,1,sizeof(byte),f) != sizeof(byte)) {
  fclose(f);
  return 2;
 }

 if(fread(&msg_count,1,sizeof(word),f)!=sizeof(word)) {
  fclose(f);
  return 2;
  }
 if(fread(MsgStrings,1,sizeof(MsgStr)*msg_count,f)!=sizeof(MsgStr)*msg_count) {
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

 short version=ZELDA_VERSION;
 byte build=VERSION_BUILD;
 if(fwrite(&version,1,sizeof(short),f)!=sizeof(short)) {
   goto error;
 }
 if(fwrite(&build,1,sizeof(byte),f)!=sizeof(byte)) {
   goto error;
 }

 if(fwrite(colordata,1,newpsTOTAL,f)<newpsTOTAL) {
   goto error;
 }
 if(fwrite(misc.cycles,1,sizeof(palcycle)*256*3,f)!=sizeof(palcycle)*256*3) {
   goto error;
 }
 if(fwrite(palnames,1,MAXLEVELS*PALNAMESIZE,f)!=(MAXLEVELS*PALNAMESIZE)) {
   goto error;
 }


 fclose(f);
 return true;

 error:
   fclose(f);
   return false;
 
}


int load_pals(char *path)
{
 FILE *f=fopen(path,"rb");
 if(!f)
   return false;

 short version;
 byte build;
 //get the version
 if(fread(&version,1,sizeof(short),f) != sizeof(short)) {
   goto error;
 }
 //get the build
 if(fread(&build,1,sizeof(byte),f) != sizeof(byte)) {
   goto error;
 }

 if(fread(colordata,1,newpsTOTAL,f)<newpsTOTAL) {
   goto error;
 }
 if(fread(misc.cycles,1,sizeof(palcycle)*256*3,f)!=sizeof(palcycle)*256*3) {
   goto error;
 }
 if(fread(palnames,1,MAXLEVELS*PALNAMESIZE,f)!=(MAXLEVELS*PALNAMESIZE)) {
   goto error;
 }

 fclose(f);
 loadlvlpal(Color);
 return true;

 error:
   fclose(f);
   return false;
}



bool save_dmaps(char *path)
{
 FILE *f=fopen(path,"wb");
 if(!f)
   return false;

 short version=ZELDA_VERSION;
 byte build=VERSION_BUILD;
 if(fwrite(&version,1,sizeof(short),f)!=sizeof(short)) {
   goto error;
 }
 if(fwrite(&build,1,sizeof(byte),f)!=sizeof(byte)) {
   goto error;
 }

 if(fwrite(DMaps,1,sizeof(dmap)*MAXDMAPS,f)!=sizeof(dmap)*MAXDMAPS) {
   goto error;
 }
 fclose(f);
 return true;

 error:
   fclose(f);
   return false;
}


bool load_dmaps(char *path)
{
 FILE *f=fopen(path,"rb");
 if(!f)
   return false;

 short version;
 byte build;
 //get the version
 if(fread(&version,1,sizeof(short),f) != sizeof(short)) {
   goto error;
 }
 //get the build
 if(fread(&build,1,sizeof(byte),f) != sizeof(byte)) {
   goto error;
 }

 if (fread(DMaps,1,sizeof(dmap)*MAXDMAPS,f)!=sizeof(dmap)*MAXDMAPS) {
   goto error;
 }

 fclose(f);
 return true;

 error:
   fclose(f);
   return false;
}

char combo_header[16] = "ZQuest combos  ";
char combo_header2[12]= "Enhanced   ";

//  if (strncmp((char*)data[0].dat,sig_str,36))
//    Z_error("Incompatible version of zelda.dat.\nPlease upgrade to %s",VerStr(DATA_VERSION));



bool save_combos(char *path)
{
  word combos_used=count_combos();
  FILE *f = fopen(path,"wb");
  if(!f) return false;

  short version=ZELDA_VERSION;
  byte build=VERSION_BUILD;

  reset_combo_animations();

  if(fwrite(&version,1,sizeof(short),f)!=sizeof(short)) {
    goto error;
  }
  if(fwrite(&build,1,sizeof(byte),f)!=sizeof(byte)) {
    goto error;
  }

  if(fwrite(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
    goto error;
  }
  if(fwrite(combobuf,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used) {
    goto error;
  }

  fclose(f);
  return true;

  error:
    fclose(f);
    return false;
  
}

bool load_combos(char *path)
{
  word combos_used;
  FILE *f = fopen(path,"rb");
  if(!f) return false;

  short version;
  byte build;

  if(fread(&version,1,sizeof(short),f)!=sizeof(short)) {
    goto error;
  }
  if(fread(&build,1,sizeof(byte),f)!=sizeof(byte)) {
    goto error;
  }

  if(fread(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
    goto error;
  }
  if(fread(combobuf,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used) {
    goto error;
  }

  fclose(f);
  setup_combo_animations();
  return true;

  error:
    fclose(f);
    return false;
  
}


/*
bool load_combos(char *path)
{
  clear_combos();

  bool noversion=true;
  bool oldformat = true;
  word combos_used;
  oldcombo *buf = (oldcombo*)malloc(sizeof(oldcombo)*MAXCOMBOS);
  if(!buf) return false;

  for(int i=0; i<MAXCOMBOS*2; i++)
    ((int*)buf)[i] = 0;

  FILE *f = fopen(path,"rb");
  if(!f) goto error;

  short version;
  byte build;
  //get the version
  if(fread(&version,1,sizeof(short),f) != sizeof(short)) {
    goto error;
  }
  //get the build
  if(fread(&build,1,sizeof(byte),f) != sizeof(byte)) {
    goto error;
  }

  if(fread(&combos_used,1,sizeof(word),f) != sizeof(word))
    goto error;
  if(fread(combobuf,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used)
    goto error;
  noversion=false;
  oldformat=false;

  if (noversion) {
    for(int tmpcounter=0; tmpcounter<MAXCOMBOS; tmpcounter++)
      clear_combo(tmpcounter);

    for (int tmpcounter=0; tmpcounter<2048; tmpcounter++) {
      combobuf[tmpcounter].tile=buf[tmpcounter].tile;
      combobuf[tmpcounter].flip=buf[tmpcounter].flip;
      combobuf[tmpcounter].walk=buf[tmpcounter].walk;
      combobuf[tmpcounter].type=buf[tmpcounter].type;
      combobuf[tmpcounter].csets=buf[tmpcounter].csets;
    }
  }

  if(oldformat)
    pack_combos();

  fclose(f);
  free(buf);
  return true;

  error:
  fclose(f);
  free(buf);
  return false;
}
*/

void reset_tiles()
{
  init_tiles();
  usetiles=false;
}


bool save_tiles(char *path)
{
    word tiles_used=count_tiles();
    FILE *f = fopen(path,"wb");

    short version=ZELDA_VERSION;
    byte  build=VERSION_BUILD;
    reset_combo_animations();
    if(fwrite(&version,1,sizeof(short),f) != sizeof(short))
      goto error;
    if(fwrite(&build,1,sizeof(byte),f) != sizeof(byte))
      goto error;
    if(fwrite(&tiles_used,1,sizeof(word),f)!=sizeof(word))
      goto error;
    if(fwrite(tilebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)
       !=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)))
      goto error;
    fclose(f);
    return true;

    error:
      fclose(f);
      reset_tiles();
      return false;
    

}


bool load_tiles(char *path, int starttile)
{
    FILE *f = fopen(path,"rb");
    short version;
    byte build;

    //get the version
    if(fread(&version,1,sizeof(short),f) != sizeof(short)) {
      goto error;
    }
    //get the build
    if(fread(&build,1,sizeof(byte),f) != sizeof(byte)) {
      goto error;
    }

    word tiles_used;
    if(fread(&tiles_used,1,sizeof(word),f)!=sizeof(word))
      goto error;

//    byte *tb=(byte*)tilebuf+128;
    if(fread(tilebuf+(starttile*128),1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)
       !=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)))
      goto error;
    fclose(f);
    return true;

    error:
      fclose(f);
      reset_tiles();
      return false;
    

  return true;
}




void setMapCount(int c)
{
  bound(c,1,MAXMAPS);
  header.map_count=c;
  int currmap=Map.getCurrMap();
  Map.setCurrMap(bound(currmap,0,c-1)); // for bound checking
}



void reset_mapstyles()
{
  short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  //skip past combo section
  byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
  tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
  tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
  tmpptr6=tmpptr6+(MAXLEVELS*PALNAMESIZE); //skip past palette names
  tmpptr6=tmpptr6+(sizeof(itemdata)*MAXITEMS); //skip past item info
  tmpptr6=tmpptr6+(sizeof(wpndata)*MAXITEMS); //skip past weapon info
  memcpy(misc.triforce, tmpptr6, sizeof(byte)*8); //read triforce info
  tmpptr6=tmpptr6+(sizeof(byte)*8); //skip past triforce info
  memcpy(misc.icons, tmpptr6, sizeof(word)*4); //read game icon info
  tmpptr6=tmpptr6+(sizeof(word)*4); //skip past game icon info
  memcpy(&misc.colors, tmpptr6, sizeof(zcolors)); //read misc color info
}

void reset_items()
{
  short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  //skip past combo section
  byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
  tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
  tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
  tmpptr6=tmpptr6+(MAXLEVELS*PALNAMESIZE); //skip past palette names
  memcpy(itemsbuf, tmpptr6, (sizeof(itemdata)*MAXITEMS)); //read item info
}


void reset_wpns()
{
  short *tmpptr=(short*)zgpdata[DAT_NESZGP].dat+1;//skip version
  byte *tmpptr2=(byte*)tmpptr+1;//skip build
  word *tilestoskip=(word*)tmpptr2; //check number of tiles
  byte *tmpptr3=(byte*)(tilestoskip+1); //skip past number of tiles
  //skip past tile section
  byte *tmpptr4=tmpptr3+((dword)(*tilestoskip)*(dword)SINGLE_TILE_SIZE);
  word *combostoread=(word*)tmpptr4; //check number of combos
  byte *tmpptr5=(byte*)(combostoread+1); //skip past number of combos
  //skip past combo section
  byte *tmpptr6=tmpptr5+((*combostoread)*sizeof(newcombo));
  tmpptr6=tmpptr6+newpsTOTAL; //skip past palette info
  tmpptr6=tmpptr6+(sizeof(palcycle)*256*3); //skip past cycling info
  tmpptr6=tmpptr6+(MAXLEVELS*PALNAMESIZE); //skip past palette names
  tmpptr6=tmpptr6+(sizeof(itemdata)*MAXITEMS); //skip past item info
  memcpy(wpnsbuf, tmpptr6, (sizeof(wpndata)*MAXWPNS)); //read weapon info
}


int init_quest()
{
 memset(&header, 0, sizeof(zquestheader));
 memset(&zcheats, 0, sizeof(ZCHEATS));

 strcpy(header.id_str,QH_IDSTR);
 header.zelda_version = ZELDA_VERSION;
 header.internal = INTERNAL_VERSION;
 header.build=VERSION_BUILD;

 for(int i=0; i<ZQ_MAXDATA; i++)
   header.data_flags[i]=0;

 setMapCount(1);
 Map.clearall();
 init_msgstrs();
 header.str_count=1;

 if(data)
 {
  init_combos();
  reset_tiles();
 }


 for(int i=0; i<MAXDMAPS; i++)
   reset_dmap(i);
 for(int i=0; i<MAXMIDIS; i++)
   reset_midi(customMIDIs+i);

 byte *di = (byte*)&misc;
 for(unsigned i=0; i<sizeof(miscQdata); i++)
   *(di++)=0;

 init_colordata();
// init_palnames();
 reset_items();
 reset_wpns();

/* //handled by reset_mapstyles
 for(int i=0; i<8; i++)
   misc.triforce[i]=i+1;
*/

 reset_mapstyles();

 refresh_pal();

 memset(&zinit, 0, sizeof(zinitdata));
 zinit.shield=1;
 zinit.hc=3;
 zinit.start_heart=3;
 zinit.cont_heart=3;
 zinit.max_bombs=8;
 zinit.swordhearts[0]=0;
 zinit.swordhearts[1]=5;
 zinit.swordhearts[2]=12;
 zinit.swordhearts[3]=21;

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
 { d_ctext_proc,      160,  88,  152,   8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Requires Authorization" },
 { d_text_proc,       64,   104,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "File name:" },
// 3 (filename)
 { d_text_proc,       148,  104, 128,   8,    vc(11),  vc(1),  0,       0,          24,            0,       NULL },
 { d_text_proc,       64,   112,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Password:" },
// 5 (password)
 { d_edit_proc,       140,  112, 120,   8,    vc(12),  vc(1),  0,       0,          24,            0,       NULL },
 { d_button_proc,     90,   136,  60,   16,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
 { d_button_proc,     170,  136,  60,   16,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
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

 if(ret!=qe_OK)
   init_quest();
 else if(!quest_access(filename))
 {
   init_quest();
   ret=qe_pwd;
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

// wrapper to reinitialize everything on an error
int load_quest2(char *filename)
{
 int ret=loadquest(filename,&header,&misc,customMIDIs);

 if(ret!=qe_OK)
   init_quest();
 else
 {
   msg_count = header.str_count;
   usetiles = header.data_flags[0];
   setMapCount(header.map_count); // force a bound check
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

int mputw(int w, FILE *f)
{
   int b1, b2;

   b1 = (w & 0xFF00) >> 8;
   b2 = w & 0x00FF;

   if (putc(b1,f)==b1)
      if (putc(b2,f)==b2)
	 return w;

   return EOF;
}

long mputl(long l, FILE *f)
{
   int b1, b2, b3, b4;

   b1 = (int)((l & 0xFF000000L) >> 24);
   b2 = (int)((l & 0x00FF0000L) >> 16);
   b3 = (int)((l & 0x0000FF00L) >> 8);
   b4 = (int)l & 0x00FF;

   if (putc(b1,f)==b1)
      if (putc(b2,f)==b2)
	 if (putc(b3,f)==b3)
	    if (putc(b4,f)==b4)
	       return l;

   return EOF;
}

bool write_unencoded_midi(MIDI *m,FILE *f)
{
   int c;

   mputw(m->divisions,f);

   for(c=0; c<MIDI_TRACKS; c++)
   {
      mputl(m->track[c].len,f);

      if(m->track[c].len > 0)
      {
         if(int(fwrite(m->track[c].data,1,m->track[c].len,f))
            !=m->track[c].len)
            return false;
      }
   }
   return true;
}

int mgetw(FILE *f)
{
   int b1, b2;

   if ((b1 = getc(f)) != EOF)
      if ((b2 = getc(f)) != EOF)
	 return ((b1 << 8) | b2);

   return EOF;
}


long mgetl(FILE *f)
{
   int b1, b2, b3, b4;

   if ((b1 = getc(f)) != EOF)
      if ((b2 = getc(f)) != EOF)
	 if ((b3 = getc(f)) != EOF)
	    if ((b4 = getc(f)) != EOF)
	       return (((long)b1 << 24) | ((long)b2 << 16) |
                       ((long)b3 << 8) | (long)b4);

   return EOF;
}

/*
static void *read_unencoded_block(FILE *f, int size, int alloc_size)
{
   alert("reb1",NULL,NULL,"OK",NULL,13,27);
   void *p;

   alert("reb2",NULL,NULL,"OK",NULL,13,27);
   p = malloc(MAX(size, alloc_size));
   alert("reb3",NULL,NULL,"OK",NULL,13,27);
   if (!p)
      return NULL;

//   fread(p, size, f);

//   if (pack_ferror(f)) {
   alert("reb4",NULL,NULL,"OK",NULL,13,27);
   if (int(fread(&p,1,size,f)) != size) {
   alert("reb4b",NULL,NULL,"OK",NULL,13,27);
      free(p);
   alert("reb4c",NULL,NULL,"OK",NULL,13,27);
      return NULL;
   }

   alert("reb5",NULL,NULL,"OK",NULL,13,27);
   return p;
}
*/

static MIDI *read_unencoded_midi(FILE *f)
{
   MIDI *m;
   int c;

   m = (MIDI*)malloc(sizeof(MIDI));
   if (!m)
     return NULL;

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = 0;
      m->track[c].data = NULL;
   }

   m->divisions = mgetw(f);

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = mgetl(f);
      if (m->track[c].len > 0) {
	 m->track[c].data = (byte*)malloc(m->track[c].len);
         fread(m->track[c].data,1,m->track[c].len,f);
	 if (!m->track[c].data) {
	    destroy_midi(m);
	    return NULL;
	 }
      }
   }

   LOCK_DATA(m, sizeof(MIDI));
   for (c=0; c<MIDI_TRACKS; c++)
      if (m->track[c].data)
	 LOCK_DATA(m->track[c].data, m->track[c].len);

   return m;
}



static int save_unencoded_quest(char *filename)
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 reset_combo_animations();
 strcpy(header.id_str,QH_IDSTR);
 header.zelda_version = ZELDA_VERSION;
 header.internal = INTERNAL_VERSION;
 header.str_count = msg_count;
 header.data_flags[ZQ_TILES] = usetiles;
 header.data_flags[ZQ_CHEATS] = 1;
 header.build=VERSION_BUILD;
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
 if(!pfwrite(itemsbuf,sizeof(itemdata)*MAXITEMS,f)) {
   pack_fclose(f);
   return 6;
   }
 if(!pfwrite(wpnsbuf,sizeof(wpndata)*MAXWPNS,f)) {
   pack_fclose(f);
   return 6;
   }

 for(int i=0; i<header.map_count; i++) {
   if(!pfwrite(TheMaps+(i*MAPSCRS),sizeof(mapscr)*MAPSCRS,f)) {
     pack_fclose(f);
     return 6+i;
     }
   }

 word combos_used=count_combos();

 if(!pfwrite(&combos_used,sizeof(word),f)) {
   pack_fclose(f);
   return 16;
   }
 if(!pfwrite(combobuf,sizeof(newcombo)*combos_used,f)) {
   pack_fclose(f);
   return 16;
   }
 if(!pfwrite(colordata,newpsTOTAL,f)) {
   pack_fclose(f);
   return 17;
   }
 if(!pfwrite(palnames,MAXLEVELS*PALNAMESIZE,f)) {
   pack_fclose(f);
   return 17;
   }

 if(header.data_flags[0]) {
   word tiles_used=count_tiles();
   if(!pfwrite(&tiles_used,sizeof(word),f)) {
     pack_fclose(f);
     return 18;
   }
   if(!pfwrite(tilebuf,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)) {
     pack_fclose(f);
     return 18;
   }
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
 if(!pfwrite(&zcheats,sizeof(ZCHEATS),f)) {
   pack_fclose(f);
   return 60;
   }

 zinit.lastmap=Map.getCurrMap();
 zinit.lastscreen=Map.getCurrScr();
 if(!pfwrite(&zinit,sizeof(zinitdata),f)) {
   pack_fclose(f);
   return 61;
   }


 pack_fclose(f);
#endif
 return 0;
}



int save_quest(char *filename)
{
  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  int ret = save_unencoded_quest(tmpfilename);
  if(ret == 0)
  {
    ret = encode_file_007(tmpfilename, filename,
      ((INTERNAL_VERSION + rand()) & 0xffff) + 0x413F0000, ENC_STR);
    if(ret)
      ret += 100;
  }

  delete_file(tmpfilename);
  return ret;
}

int save_quest2(char *filename)
{
  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  int ret = save_unencoded_quest(tmpfilename);
  if(ret == 0)
  {
    ret = encode_file_007(tmpfilename, filename,
      ((INTERNAL_VERSION + rand()) & 0xffff) + 0x413F0000, ENC_STR);
    if(ret)
      ret += 100;
  }

  delete_file(tmpfilename);
  return ret;
}


bool load_zgp(char *path)
{
  word tiles_used;
  word combos_used;
  clear_tiles();
  clear_combos();
  //open the file
  FILE *f=fopen(path,"rb");
  if(!f)
   return false;

  short version;
  byte build;

  //read the version and make sure it worked
  if(fread(&version,1,sizeof(short),f) != sizeof(short))
    goto error;

  //read the build and make sure it worked
  if(fread(&build,1,sizeof(byte),f) != sizeof(byte))
    goto error;

  //read the tile info and make sure it worked
  if(fread(&tiles_used,1,sizeof(word),f)!=sizeof(word))
    goto error;
  if(fread(tilebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)
     !=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)))
    goto error;

  //read the combo info and make sure it worked
  if(fread(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
    goto error;
  }
  if(fread(combobuf,1,sizeof(newcombo)*combos_used,f)
     !=sizeof(newcombo)*combos_used) {
    goto error;
  }

  //read the palette info and make sure it worked
  if(fread(colordata,1,newpsTOTAL,f)<newpsTOTAL) {
    goto error;
  }
  if(fread(misc.cycles,1,sizeof(palcycle)*256*3,f)!=sizeof(palcycle)*256*3) {
    goto error;
  }
  if(fread(palnames,1,MAXLEVELS*PALNAMESIZE,f)!=(MAXLEVELS*PALNAMESIZE)) {
    goto error;
  }
  loadlvlpal(Color);

  //read the sprite info and make sure it worked
  if(fread(itemsbuf,1,sizeof(itemdata)*MAXITEMS,f)!=sizeof(itemdata)*MAXITEMS) {
    goto error;
  }
  if(fread(wpnsbuf,1,sizeof(wpndata)*MAXITEMS,f)!=sizeof(wpndata)*MAXITEMS) {
    goto error;
  }

  //read the triforce pieces info and make sure it worked
  if(fread(misc.triforce,1,sizeof(byte)*8,f)!=sizeof(byte)*8) {
    goto error;
  }

  //read the game icons info and make sure it worked
  if(fread(misc.icons,1,sizeof(word)*4,f)!=sizeof(word)*4) {
    goto error;
  }

  //read the misc colors info and map styles info and make sure it worked
  if(fread(&misc.colors,1,sizeof(zcolors),f)!=sizeof(zcolors)) {
    goto error;
  }

  //read the template screens and make sure it worked
  byte num_maps;
  if(fread(&num_maps,1,sizeof(byte),f)!=sizeof(byte)) {
    goto error;
  }
  if (header.map_count<num_maps) {
    setMapCount(num_maps);
  }
  if(fread(TheMaps+128,1,sizeof(mapscr)*4,f)!=sizeof(mapscr)*4) {
    goto error;
  }
  if (num_maps>1) { //dungeon templates
    if(fread(TheMaps+132+128,1,sizeof(mapscr)*4,f)!=sizeof(mapscr)*4) {
      goto error;
    }
  }

  setup_combo_animations();

  //yay!  it worked!  close the file and say everything was ok.
  fclose(f);
  return true;

  error:
    fclose(f);
    return false;
}



bool save_zgp(char *path)
{
  reset_combo_animations();

  //open the file
  FILE *f=fopen(path,"wb");
  if(!f)
   return false;

  short version=ZELDA_VERSION;
  byte  build=VERSION_BUILD;

  //write the version and make sure it worked
  if(fwrite(&version,1,sizeof(short),f) != sizeof(short))
    goto error;

  //write the build and make sure it worked
  if(fwrite(&build,1,sizeof(byte),f) != sizeof(byte))
    goto error;

  //write the tile info and make sure it worked
  word tiles_used;
  tiles_used=count_tiles();
  if(fwrite(&tiles_used,1,sizeof(word),f)!=sizeof(word))
    goto error;
  if(fwrite(tilebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)
     !=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)))
    goto error;

  //write the combo info and make sure it worked
  word combos_used;
  combos_used=count_combos();
  if(fwrite(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
    goto error;
  }
  if(fwrite(combobuf,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used) {
    goto error;
  }

  //write the palette info and make sure it worked
  if(fwrite(colordata,1,newpsTOTAL,f)<newpsTOTAL) {
    goto error;
  }
  if(fwrite(misc.cycles,1,sizeof(palcycle)*256*3,f)!=sizeof(palcycle)*256*3) {
    goto error;
  }
  if(fwrite(palnames,1,MAXLEVELS*PALNAMESIZE,f)!=(MAXLEVELS*PALNAMESIZE)) {
    goto error;
  }

  //write the sprite info and make sure it worked
  if(fwrite(itemsbuf,1,sizeof(itemdata)*MAXITEMS,f)!=sizeof(itemdata)*MAXITEMS) {
    goto error;
  }
  if(fwrite(wpnsbuf,1,sizeof(wpndata)*MAXITEMS,f)!=sizeof(wpndata)*MAXITEMS) {
    goto error;
  }

  //write the triforce pieces info and make sure it worked
  if(fwrite(misc.triforce,1,sizeof(byte)*8,f)!=sizeof(byte)*8) {
    goto error;
  }

  //write the game icons info and make sure it worked
  if(fwrite(misc.icons,1,sizeof(word)*4,f)!=sizeof(word)*4) {
    goto error;
  }

  //write the misc colors info and map styles info and make sure it worked
  if(fwrite(&misc.colors,1,sizeof(zcolors),f)!=sizeof(zcolors)) {
    goto error;
  }

  //write the template screens and make sure it worked
  byte num_maps;
  num_maps=(((header.map_count)<2)?1:2);
  if(fwrite(&num_maps,1,sizeof(byte),f)!=sizeof(byte)) {
    goto error;
  }
  if(fwrite(TheMaps+128,1,sizeof(mapscr)*4,f)!=sizeof(mapscr)*4) {
    goto error;
  }
  if (num_maps>1) { //dungeon templates
    if(fwrite(TheMaps+132+128,1,sizeof(mapscr)*4,f)!=sizeof(mapscr)*4) {
      goto error;
    }
  }

  //yay!  it worked!  close the file and say everything was ok.
  fclose(f);
  return true;

  error:
    fclose(f);
    return false;

}

bool save_qsu(char *path)
{
 reset_combo_animations();
 strcpy(header.id_str,QH_IDSTR);
 char tmppwd[30];
 strcpy(tmppwd,header.password);
 short tmppwdkey;
 for (int i=0; i<30; i++) {
   header.password[i]=0;
 }
 tmppwdkey=header.pwdkey;
 header.pwdkey=0;
 header.zelda_version = ZELDA_VERSION;
 header.internal = INTERNAL_VERSION;
 header.str_count = msg_count;
 header.data_flags[ZQ_TILES] = usetiles;
 header.data_flags[ZQ_CHEATS] = 1;
 header.build=VERSION_BUILD;

 word combos_used=count_combos();
 word tiles_used=count_tiles();

 for(int i=0; i<MAXMIDIS; i++)
   set_bit(header.data_flags+ZQ_MIDIS,i,int(customMIDIs[i].midi!=NULL));

 FILE *f = fopen(path,"wb");
 if(!f)
   goto error;
 if(fwrite(&header,1,sizeof(zquestheader),f)!=sizeof(zquestheader)) {
   goto error;
   }
 if(fwrite(MsgStrings,1,sizeof(MsgStr)*MAXMSGS,f)!=sizeof(MsgStr)*MAXMSGS) {
   goto error;
   }
 if(fwrite(DMaps,1,sizeof(dmap)*MAXDMAPS,f)!=sizeof(dmap)*MAXDMAPS) {
   goto error;
   }
 if(fwrite(&misc,1,sizeof(miscQdata),f)!=sizeof(miscQdata)) {
   goto error;
   }
 if(fwrite(itemsbuf,1,sizeof(itemdata)*MAXITEMS,f)!=sizeof(itemdata)*MAXITEMS) {
   goto error;
   }
 if(fwrite(wpnsbuf,1,sizeof(wpndata)*MAXWPNS,f)!=sizeof(wpndata)*MAXWPNS) {
   goto error;
   }

 for(int i=0; i<header.map_count; i++) {
   if(fwrite(TheMaps+(i*MAPSCRS),1,sizeof(mapscr)*MAPSCRS,f)!=sizeof(mapscr)*MAPSCRS) {
     goto error;
     }
   }


 if(fwrite(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
   goto error;
   }
 if(fwrite(combobuf,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used) {
   goto error;
   }
 if(fwrite(colordata,1,newpsTOTAL,f)!=newpsTOTAL) {
   goto error;
   }
 if(fwrite(palnames,1,MAXLEVELS*PALNAMESIZE,f)!=(MAXLEVELS*PALNAMESIZE)) {
   goto error;
 }

 if(header.data_flags[0]) {
   if(fwrite(&tiles_used,1,sizeof(word),f)!=sizeof(word)) {
   goto error;
   }
   if(fwrite(tilebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)
      !=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used))) {
   goto error;
   }
 }

 for(int i=0; i<MAXMIDIS; i++)
 {
   if(get_bit(header.data_flags+ZQ_MIDIS,i))
   {
     if((fwrite(customMIDIs+i,1,sizeof(music),f)!=sizeof(music)) ||
        !write_unencoded_midi(customMIDIs[i].midi,f) )
     {
       goto error;
     }

   }
 }

 if(fwrite(&zcheats,1,sizeof(ZCHEATS),f)!=sizeof(ZCHEATS)) {
   goto error;
   }

 zinit.lastmap=Map.getCurrMap();
 zinit.lastscreen=Map.getCurrScr();
 if(fwrite(&zinit,1,sizeof(zinitdata),f)!=sizeof(zinitdata)) {
   goto error;
   }

  fclose(f);
  strcpy(header.password, tmppwd);
  header.pwdkey=tmppwdkey;
  return true;


  error:
    fclose(f);
    return false;
}


bool load_qsu(char *path)
{
 word combos_used=count_combos();
 word tiles_used=count_tiles();
 int i;

 for(int i=0; i<MAXMIDIS; i++)
   set_bit(header.data_flags+ZQ_MIDIS,i,int(customMIDIs[i].midi!=NULL));

 FILE *f = fopen(path,"rb");
 if(!f)
   goto error;
 if(fread(&header,1,sizeof(zquestheader),f)!=sizeof(zquestheader)) {
   goto error;
   }
 if(fread(MsgStrings,1,sizeof(MsgStr)*MAXMSGS,f)!=sizeof(MsgStr)*MAXMSGS) {
   goto error;
   }
 if(fread(DMaps,1,sizeof(dmap)*MAXDMAPS,f)!=sizeof(dmap)*MAXDMAPS) {
   goto error;
   }
 if(fread(&misc,1,sizeof(miscQdata),f)!=sizeof(miscQdata)) {
   goto error;
   }
 if(fread(itemsbuf,1,sizeof(itemdata)*MAXITEMS,f)!=sizeof(itemdata)*MAXITEMS) {
   goto error;
   }
 if(fread(wpnsbuf,1,sizeof(wpndata)*MAXWPNS,f)!=sizeof(wpndata)*MAXWPNS) {
   goto error;
   }

 for(int i=0; i<header.map_count; i++) {
   if(fread(TheMaps+(i*MAPSCRS),1,sizeof(mapscr)*MAPSCRS,f)!=sizeof(mapscr)*MAPSCRS) {
     goto error;
     }
   }


 if(fread(&combos_used,1,sizeof(word),f)!=sizeof(word)) {
   goto error;
   }
 if(fread(combobuf,1,sizeof(newcombo)*combos_used,f)!=sizeof(newcombo)*combos_used) {
   goto error;
   }
 if(fread(colordata,1,newpsTOTAL,f)!=newpsTOTAL) {
   goto error;
   }
 if(fread(palnames,1,MAXLEVELS*PALNAMESIZE,f)!=(MAXLEVELS*PALNAMESIZE)) {
   goto error;
 }
 if(header.data_flags[0]) {
   if(fread(&tiles_used,1,sizeof(word),f)!=sizeof(word)) {
   goto error;
   }
   if(fread(tilebuf,1,dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used)),f)
      !=dword(dword(SINGLE_TILE_SIZE)*dword(tiles_used))) {
   goto error;
   }
 }

 for(i=0; i<MAXMIDIS; i++)
 {
   if(get_bit(header.data_flags+ZQ_MIDIS,i))
   {
     if((fread(customMIDIs+i,1,sizeof(music),f)!=sizeof(music)) ||
     !(customMIDIs[i].midi=read_unencoded_midi(f)) ) {
       goto error;
     }

   }
 }

 if(fread(&zcheats,1,sizeof(ZCHEATS),f)!=sizeof(ZCHEATS)) {
   goto error;
   }

 zinit.lastmap=Map.getCurrMap();
 zinit.lastscreen=Map.getCurrScr();
 if(fread(&zinit,1,sizeof(zinitdata),f)!=sizeof(zinitdata)) {
   goto error;
   }

 msg_count = header.str_count;
 usetiles = header.data_flags[ZQ_TILES];
 setup_combo_animations();
 loadlvlpal(Color);

  fclose(f);
  return true;


  error:
    fclose(f);
    return false;
}



