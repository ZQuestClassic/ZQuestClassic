/*
  pal.cc
  Jeremy Craner, 1999
  Palette code for zelda.cc
*/

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

RGB mixRGB(int r1,int g1,int b1,int r2,int g2,int b2,int ratio)
{
  RGB x;
  x.r = ( r1*(64-ratio) + r2*ratio ) >> 6;
  x.g = ( g1*(64-ratio) + g2*ratio ) >> 6;
  x.b = ( b1*(64-ratio) + b2*ratio ) >> 6;
  return x;
}

inline RGB NESpal(int i)
{
  return _RGB(nes_pal+i*3);
}


void copy_pal(RGB *src,RGB *dest)
{
  for(int i=0; i<256; i++)
    dest[i]=src[i];
}


void loadfullpal()
{
  for(int i=0; i<240; i++)
    RAMpal[i]=_RGB(colordata+i*3);
  for(int i=240; i<255; i++)
    RAMpal[i]=((RGB*)data[PAL_GUI].dat)[i];
  refreshpal=true;
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

  RAMpal[CSET(6)+2]=NESpal((level==3)?0x37:0x27);
  refreshpal=true;
}


void loadpalset(int cset,int dataset)
{
  byte *si = colordata + CSET(dataset)*3;
  for(int i=0; i<16; i++) {
    RAMpal[CSET(cset)+i] = _RGB(si);
    si+=3;
    }
  refreshpal=true;
}


void ringcolor()
{
 int rc;
 switch(game.ring) {
  case 0: rc=0x29; break;
  case 1: rc=0x32; break;
  case 2: rc=0x16; break;
  }
 RAMpal[CSET(6)+1]=NESpal(rc);
 refreshpal=true;
}



void loadfadepal(int dataset)
{
  byte *si = colordata + CSET(dataset)*3;

  for(int i=0; i<pdFADE*16; i++) {
    RAMpal[CSET(2)+i] = _RGB(si);
    si+=3;
    }

  refreshpal=true;
}



void fade(int level,bool blackall,bool fromblack)
{
 int cx = fromblack ? 30 : 0;
 for(int i=0; i<=30; i++) {
  switch(cx) {
  case 0:  loadlvlpal(level); break;
  case 10: loadfadepal(level*pdLEVEL+poFADE1); break;
  case 20: loadfadepal(level*pdLEVEL+poFADE2); break;
  case 30:
    if(blackall) {
      for(int i=0; i<pdFADE*16; i++)
        RAMpal[CSET(2)+i]=black_palette[0];
      refreshpal=true;
      }
    else
      loadfadepal(level*pdLEVEL+poFADE3);
    break;
  }
  putsubscr(framebuf,0,0);
  advanceframe();
  fromblack ? cx-- : cx++;
 }
}



void lighting(int funct)
// 1 = use candle
// 2 = enter room
// 3 = leave room
{
 switch(funct) {
  case 1:
   if(darkroom) {
     fade(DMaps[currdmap].color,false,true);
     darkroom=false;
     }
   break;
  case 2:
   if(tmpscr[0].flags&fDARK && !darkroom) {
     fade(DMaps[currdmap].color,false,false);
     darkroom=true;
     }
   break;
  case 3:
   if(!(tmpscr[0].flags&fDARK) && darkroom) {
     fade(DMaps[currdmap].color,false,true);
     darkroom=false;
     }
   break;
  }
}


byte drycolors[11] = {0x12,0x11,0x22,0x21,0x31,0x32,0x33,0x35,0x34,0x36,0x37};

void dryuplake()
{
 if(whistleclk<0 || whistleclk>=88)
   return;
 if((++whistleclk)&7)
   return;
 if(whistleclk<88)
 {
   RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
   refreshpal=true;
 }
 else
 {
   if(hiddenstair(0,true))
     sfx(WAV_SECRET);
 }
}


void rehydratelake()
{
 if(whistleclk==-1)
   return;
 whistleclk&=0xF8;
 do {
   whistleclk-=8;
   RAMpal[CSET(3)+3] = NESpal(drycolors[whistleclk>>3]);
   refreshpal=true;
   advanceframe();
   if(((whistleclk>>3)&3) == 1)
     for(int i=0; i<4 && !Quit; i++)
       advanceframe();
   } while(whistleclk!=0 && !Quit);
 whistleclk=-1;
}



/* end of pal.cc */
