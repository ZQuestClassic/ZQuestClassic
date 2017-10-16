/*
  aglogo.cc
  Armageddon games logo with fire field generator.
  Adapted from code written by Midnight and included in SPHINX C-- examples.
  (Tweaked by Jeremy Craner.)
*/

#include "allegro.h"
#include "zeldadat.h"

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short word;
#endif

extern DATAFILE* data;

void SetCols(RGB* pal)
{
byte firepal[256*3];
word DI,BX=0;

firepal[0]=0;
firepal[1]=0;
firepal[2]=0;
do {
    word AX = BX * 64 / 85;
    byte DL = AX;

    DI = BX+BX+BX;
    firepal[DI+3] = DL;
    firepal[DI+3+1] = 0;
    if(BX<75)
      firepal[DI+3+2] = BX<8 ? (BX<<1)+12 : 20;
    else firepal[DI+3+2] = BX-75;

    firepal[DI+3+85+85+85] = 63;
    firepal[DI+3+85+85+85+1] = DL;
    firepal[DI+3+85+85+85+2] = 0;

    firepal[DI+3+85+85+85+85+85+85] = 63;
    firepal[DI+3+85+85+85+85+85+85+1] = 63;
    firepal[DI+3+85+85+85+85+85+85+2] = DL;

    BX++;
    } while( BX < 85 );

firepal[255*3]=0;
firepal[255*3+1]=0;
firepal[255*3+2]=0;
DI=0;
for(int i=0; i<256; i++) {
  pal[i].r = firepal[DI++];
  pal[i].g = firepal[DI++];
  pal[i].b = firepal[DI++];
  }
}


void AddFire(BITMAP* firebuf,word N)
{
while(N) {
  int cx = (rand()%10+1)<<1;
  int x = rand()%314;
  byte color = (rand()&127) + 128;
  while(cx) {
    firebuf->line[200][x]=color;
    firebuf->line[202][x]=color;
    x++;
    cx--;
    }
  N--;
  }
}


void CopyAvg(BITMAP* fire)
{
for(int y=4; y<204; y+=2) {
  for(int x=2; x<336; x+=2) {
    byte* si=&(fire->line[y][x-2]);
    int AX = ( *si + *(si+2) + *(si+4) + fire->line[y+2][x] ) >> 2;
    AX = (AX + fire->line[y-2][x]) >> 1;
    if(AX>128)
      AX-=2;
    else if(AX>12)
      AX-=4;
    else if(AX>8)
      AX-=2;
    else if(AX>0)
      AX--;
    AX=(AX<<8)+AX;
    ((word*)fire->line[y-4])[x>>1]=AX;
    ((word*)fire->line[y-3])[x>>1]=AX;
    }
  }
}


int aglogo(BITMAP* frame)
{
BITMAP* firebuf=create_bitmap(340,206);
if(!firebuf)
  return 1;
if(set_gfx_mode(GFX_VGA,320,200,0,0))
  return 2;
PALETTE pal;
SetCols(pal);
PALETTE workpal;
set_palette(black_palette);
clear(frame);
clear(firebuf);

for(int f=0; f<128; f++) {
  AddFire(firebuf,(f>>3)+1);
  CopyAvg(firebuf);
  }

int fadecnt=0;
bool blackout=false;
play_sample((SAMPLE*)data[WAV_00_AGFIRE].dat,255,128,1000,true);
do {
  AddFire(firebuf,17);
  CopyAvg(firebuf);
  blit(firebuf,frame,8,0,0,0,320,200);
  draw_rle_sprite(frame,(RLE_SPRITE*)data[RLE_AGTEXT].dat,24,90);
  vsync();
  blit(frame,screen,0,0,0,0,320,200);
  poll_joystick();
  if((keypressed()||joy_b1||joy_b2)&&fadecnt==65) {
    blackout=true;
    if(keypressed() && (readkey()>>8)==KEY_F12)
      save_bitmap("agfire.pcx",frame,pal);
    }
  if(!blackout) {
   if(fadecnt<65) {
    fade_interpolate(black_palette,pal,workpal,fadecnt++,0,255);
    set_palette_range(workpal,0,255,false);
    }
   }
  else {
   fade_interpolate(black_palette,pal,workpal,fadecnt--,0,255);
   set_palette_range(workpal,0,255,false);
   }
  } while(fadecnt>0);
stop_sample((SAMPLE*)data[WAV_00_AGFIRE].dat);
clear_keybuf();
return 0;
}

