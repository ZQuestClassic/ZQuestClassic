//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  matrix.c
//
//  Code for the "Matrix" screen saver.
//
//--------------------------------------------------------

#include "matrix.h"

// external FONTs
extern FONT *deffont, *mfont;


#define MAX_COLS      256
#define MAX_TRACERS   MAX_COLS * 2

#define BLACK         253
#define LIGHT_GREEN   250
#define MED_GREEN     245
#define DARK_GREEN    244

//#ifndef _MSC_VER
#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))
//#endif

typedef unsigned char byte;

typedef struct TRACER
{
  byte x, y;
  byte speed;
  byte cnt;
} TRACER;

typedef struct COLUMN
{
  short speed, cnt;
} COLUMN;

static BITMAP *linebmp = NULL;
static TRACER tracer[MAX_TRACERS];
static TRACER eraser[MAX_TRACERS];
static COLUMN column[MAX_COLS];
static byte   activecol[MAX_COLS];
static int cols, rows, maxtracers, _speed, _density;

static void InitMatrix();
static void AddTracer();
static void AddEraser(int col);
static void UpdateTracers();
static void UpdateErasers();
static void UpdateColumns();
static void DrawLetter(int x, int y, int color);
static void DrawEraser(int x, int y, int type);

void Matrix(int speed, int density, int mousedelay)
{
  // speed 0-6, density 0-6
  _density = zc_max( zc_min(density, 6), 0);
  _speed = zc_max( zc_min(speed, 6), 0);

  InitMatrix();

  for(;;)
  {
	vsync();
    AddTracer();
    AddEraser(-1);
    UpdateTracers();
    UpdateErasers();
    UpdateColumns();

    poll_joystick();
	load_control_state();

    int idle;
    if(mousedelay > 0)
    {
      mousedelay--;
      idle = input_idle(false);
    }
    else
    {
      idle = input_idle(true);
    }

    if(!idle)
      break;
  }

  if(linebmp != NULL)
    destroy_bitmap(linebmp);

  while(keypressed())
    readkey();

  clear_to_color(screen, BLACK);
}

static void InitMatrix()
{
  RGB c_blck = {0,0,0,0};
  RGB c_lgrn = {0,255,0,0};
  RGB c_mgrn = {0,170,0,0};
  RGB c_dgrn = {0,85,0,0};

  set_color(BLACK, &c_blck);
  set_color(LIGHT_GREEN, &c_lgrn);
  set_color(MED_GREEN, &c_mgrn);
  set_color(DARK_GREEN, &c_dgrn);

  cols = zc_max( zc_min(SCREEN_W / 8, MAX_COLS), 1);
  rows = zc_max( zc_min(SCREEN_H / 8, MAX_COLS), 1);

  linebmp = create_bitmap_ex(8,8, rows*8);

  maxtracers = ((_density + 2) * cols) / 4;

  for(int i=0; i<maxtracers; i++)
  {
    tracer[i].speed = 0;
    eraser[i].speed = 0;
  }

  for(int i=0; i<cols; i++)
  {
    column[i].speed = ( (rand() % (_speed + 8)) * (7 - _speed) ) + (7 - _speed) * 6;
    column[i].cnt = 0;
    activecol[i] = 0;
  }

  //  text_mode(BLACK);
  while(keypressed())
    readkey();
}

static void AddTracer()
{
  static int delay = 0;
  if(--delay <= 0)
  {
    delay = rand() % (32 - _density*3 - _speed*2);
    for(int i=0; i<maxtracers; i++)
    {
      if(tracer[i].speed == 0)
      {
        tracer[i].x = rand()%cols;
        tracer[i].y = 0;
        tracer[i].speed = (rand()&7) * (4-_speed/2) + 8 - _speed;
        tracer[i].cnt = 0;
        activecol[tracer[i].x] = 1;
        DrawLetter(tracer[i].x, tracer[i].y, LIGHT_GREEN);
        break;
      }
    }
  }
}

static void AddEraser(int col = -1)
{
  for(int i=0; i<maxtracers; i++)
  {
    if( eraser[i].speed == 0 && (col>=0 || (tracer[i].speed
                                            && tracer[i].y > 5 + (_density - _speed)/2
                                            && (rand() % ((7 + _density - _speed)*5 + 10)) == 0)) )
    {
      eraser[i].x = col>=0 ? col : tracer[i].x;
      eraser[i].y = 0;
      eraser[i].speed = col>=0 ? (rand()&7) * (4-_speed/2) + 8 - _speed : tracer[i].speed + (rand()&3);
      eraser[i].cnt = 0;
      activecol[eraser[i].x] = 0;
      DrawEraser(eraser[i].x, eraser[i].y, 1);
      break;
    }
  }
}

static void UpdateTracers()
{
  for(int i=0; i<maxtracers; i++)
  {
    if(tracer[i].speed)
    {
      if(++tracer[i].cnt >= tracer[i].speed)
      {
        DrawLetter(tracer[i].x, tracer[i].y, MED_GREEN);

        if(tracer[i].y >= rows-1)
          tracer[i].speed = 0;
        else
        {
          tracer[i].cnt = 0;
          DrawLetter(tracer[i].x, ++tracer[i].y, LIGHT_GREEN);
        }
      }
    }
  }
}

static void UpdateErasers()
{
  for(int i=0; i<maxtracers; i++)
  {
    if(eraser[i].speed)
    {
      if(++eraser[i].cnt >= eraser[i].speed)
      {
        DrawEraser(eraser[i].x, eraser[i].y, 0);

        if(eraser[i].y >= rows-1)
          eraser[i].speed = 0;
        else
        {
          eraser[i].cnt = 0;
          DrawEraser(eraser[i].x, ++eraser[i].y, 1);
        }
      }
    }
  }
}

static void DrawLetter(int x, int y, int color)
{
  int r = rand();
  int letter = r & 255;

  if(letter < 32)
    letter += (r>>10) % 224;

  FONT *fnt = (r&512) || ((letter&0xE0)==0x80) ? mfont : deffont;

  textprintf_ex(screen, fnt, x<<3, y<<3, color, BLACK, "%c", letter);
}

static void DrawEraser(int x, int y, int type)
{
  x <<= 3;
  y <<= 3;

  if(type == 0)
  {
    rectfill(screen, x, y, x+7, y+7, BLACK);
  }
  else
  {
    for(int i=0; i<8; i++)
      for(int j=0; j<8; j++)
      {
        int pix = getpixel(screen, x+i, y+j);
        if(pix == LIGHT_GREEN || pix == MED_GREEN)
          putpixel(screen, x+i, y+j, DARK_GREEN);
      }
  }
}

static void UpdateColumns()
{
  for(int i=0; i<cols; i++)
  {
    if(++column[i].cnt >= column[i].speed)
    {
      column[i].cnt = 0;

      for(int j=0; j<maxtracers; j++)
      {
        if(tracer[j].x == i)
          tracer[j].y++;
        if(eraser[j].x == i)
          eraser[j].y++;
      }

      blit(screen, linebmp, i*8, 0, 0, 0, 8, rows*8 - 8);
      blit(linebmp, screen, 0, 0, i*8, 8, 8, rows*8 - 8);

      if(activecol[i])
      {
        DrawLetter(i, 0, MED_GREEN);
        if(++activecol[i] >= rows/3)
        {
          if(rand()&15)
            AddEraser(i);
        }
      }
      else
      {
        DrawEraser(i, 0, 0);
      }
    }
  }
}
 
