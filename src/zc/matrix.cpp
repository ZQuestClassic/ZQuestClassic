#include "base/ints.h"
#include "base/fonts.h"
#include "zc/matrix.h"
#include "zc/render.h"
FONT* get_zc_font(int index);

#define MAX_COLS      256
#define MAX_TRACERS   MAX_COLS * 2

#define BLACK         253
#define LIGHT_GREEN   250
#define MED_GREEN     245
#define DARK_GREEN    244

typedef struct zcMatrixTRACER
{
    byte x, y;
    byte speed;
    byte cnt;
} zcMatrixTRACER;

typedef struct zcMatrixCOLUMN
{
    int16_t speed, cnt;
} zcMatrixCOLUMN;

static RenderTreeItem rti_matrix("matrix");
static BITMAP *target_bitmap = NULL;
static BITMAP *linebmp = NULL;
static zcMatrixTRACER tracer[MAX_TRACERS];
static zcMatrixTRACER eraser[MAX_TRACERS];
static zcMatrixCOLUMN column[MAX_COLS];
static byte   activecol[MAX_COLS];
static int32_t cols, rows, maxtracers, _speed, _density;

static void InitMatrix();
static void AddTracer();
static void AddEraser(int32_t col);
static void UpdateTracers();
static void UpdateErasers();
static void UpdateColumns();
static void DrawLetter(int32_t x, int32_t y, int32_t color);
static void DrawEraser(int32_t x, int32_t y, int32_t type);

extern void zc_throttle_fps();

void Matrix(int32_t speed, int32_t density, int32_t mousedelay)
{
    rti_matrix.visible = true;
    rti_root.add_child_before(&rti_matrix, &rti_menu);

    int w = al_get_bitmap_width(rti_game.bitmap);
    int h = al_get_bitmap_height(rti_game.bitmap);
    target_bitmap = create_bitmap_ex(8, w, h);
    clear_bitmap(target_bitmap);
    
    if (rti_matrix.bitmap)
        al_destroy_bitmap(rti_matrix.bitmap);
    rti_matrix.bitmap = al_create_bitmap(w, h);
    rti_matrix.a4_bitmap = target_bitmap;

    // speed 0-6, density 0-6
    _density = zc_max(zc_min(density, 6), 0);
    _speed = zc_max(zc_min(speed, 6), 0);
    
    InitMatrix();
    input_idle(true);
    
    for(;;)
    {
        //vsync();
        zc_throttle_fps();

        AddTracer();
        AddEraser(-1);
        UpdateTracers();
        UpdateErasers();
        UpdateColumns();

        auto [game_pos_x, game_pos_y] = rti_game.pos();
        rti_matrix.set_transform({
            .x = game_pos_x,
            .y = game_pos_y,
            .xscale = rti_game.get_transform().xscale,
            .yscale = rti_game.get_transform().yscale,
        });

        update_hw_screen();
        
        poll_joystick();
        
        int32_t idle;
        
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

    rti_root.remove_child(&rti_matrix);
    destroy_bitmap(target_bitmap);
    
    if(linebmp != NULL)
        destroy_bitmap(linebmp);
        
    clear_keybuf();
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
    
    cols = zc_max(zc_min(target_bitmap->w / 8, MAX_COLS), 1);
    rows = zc_max(zc_min(target_bitmap->h / 8, MAX_COLS), 1);
    
    linebmp = create_bitmap_ex(8,8, rows*8);
    
    maxtracers = ((_density + 2) * cols) / 4;
    
    for(int32_t i=0; i<maxtracers; i++)
    {
        tracer[i].speed = 0;
        eraser[i].speed = 0;
    }
    
    for(int32_t i=0; i<cols; i++)
    {
        column[i].speed = ((zc_oldrand() % (_speed + 8)) * (7 - _speed)) + (7 - _speed) * 6;
        column[i].cnt = 0;
        activecol[i] = 0;
    }
    
    clear_keybuf();
}

static void AddTracer()
{
    static int32_t delay = 0;
    
    if(--delay <= 0)
    {
        delay = zc_oldrand() % (32 - _density*3 - _speed*2);
        
        for(int32_t i=0; i<maxtracers; i++)
        {
            if(tracer[i].speed == 0)
            {
                tracer[i].x = zc_oldrand()%cols;
                tracer[i].y = 0;
                tracer[i].speed = (zc_oldrand()&7) * (4-_speed/2) + 8 - _speed;
                tracer[i].cnt = 0;
                activecol[tracer[i].x] = 1;
                DrawLetter(tracer[i].x, tracer[i].y, LIGHT_GREEN);
                break;
            }
        }
    }
}

static void AddEraser(int32_t col = -1)
{
    for(int32_t i=0; i<maxtracers; i++)
    {
        if(eraser[i].speed == 0 && (col>=0 || (tracer[i].speed
                                               && tracer[i].y > 5 + (_density - _speed)/2
                                               && (zc_oldrand() % ((7 + _density - _speed)*5 + 10)) == 0)))
        {
            eraser[i].x = col>=0 ? col : tracer[i].x;
            eraser[i].y = 0;
            eraser[i].speed = col>=0 ? (zc_oldrand()&7) * (4-_speed/2) + 8 - _speed : tracer[i].speed + (zc_oldrand()&3);
            eraser[i].cnt = 0;
            activecol[eraser[i].x] = 0;
            DrawEraser(eraser[i].x, eraser[i].y, 1);
            break;
        }
    }
}

static void UpdateTracers()
{
    for(int32_t i=0; i<maxtracers; i++)
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
    for(int32_t i=0; i<maxtracers; i++)
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

static void DrawLetter(int32_t x, int32_t y, int32_t color)
{
    int32_t r = zc_oldrand();
    int32_t letter = r & 255;
    
    if(letter < 32)
        letter += (r>>10) % 224;
        
    FONT *fnt = get_zc_font((r&512) || ((letter&0xE0)==0x80) ? font_mfont : font_deffont);
    
    textprintf_ex(target_bitmap, fnt, x<<3, y<<3, color, BLACK, "%c", letter);
}

static void DrawEraser(int32_t x, int32_t y, int32_t type)
{
    x <<= 3;
    y <<= 3;
    
    if(type == 0)
    {
        rectfill(target_bitmap, x, y, x+7, y+7, BLACK);
    }
    else
    {
        for(int32_t i=0; i<8; i++)
            for(int32_t j=0; j<8; j++)
            {
                int32_t pix = getpixel(target_bitmap, x+i, y+j);
                
                if(pix == LIGHT_GREEN || pix == MED_GREEN)
                    putpixel(target_bitmap, x+i, y+j, DARK_GREEN);
            }
    }
}

static void UpdateColumns()
{
    for(int32_t i=0; i<cols; i++)
    {
        if(++column[i].cnt >= column[i].speed)
        {
            column[i].cnt = 0;
            
            for(int32_t j=0; j<maxtracers; j++)
            {
                if(tracer[j].x == i)
                {
                    tracer[j].y++;
                    if(tracer[j].y >= rows)
                        tracer[j].speed = 0;
                }
                    
                if(eraser[j].x == i)
                {
                    eraser[j].y++;
                    if(eraser[j].y >= rows)
                        eraser[j].speed = 0;
                }
            }
            
            blit(target_bitmap, linebmp, i*8, 0, 0, 0, 8, rows*8 - 8);
            blit(linebmp, target_bitmap, 0, 0, i*8, 8, 8, rows*8 - 8);
            
            if(activecol[i])
            {
                DrawLetter(i, 0, MED_GREEN);
                
                if(++activecol[i] >= rows/3)
                {
                    if(zc_oldrand()&15)
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
