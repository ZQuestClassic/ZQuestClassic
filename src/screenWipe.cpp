#include "screenWipe.h"
#include "zc_alleg.h"
#include "zelda.h"

static int chooseWipeShape();
static void drawScreenWipe(BITMAP* dest, int x, int y, int a, int max_a);

enum { bos_circle=0, bos_oval, bos_triangle, bos_smas };

static int black_opening_count=0;
static int black_opening_x,black_opening_y;
static int black_opening_shape;

static const qword trianglelines[16]=
{
    0x0000000000000000ULL,
    0xFD00000000000000ULL,
    0xFDFD000000000000ULL,
    0xFDFDFD0000000000ULL,
    0xFDFDFDFD00000000ULL,
    0xFDFDFDFDFD000000ULL,
    0xFDFDFDFDFDFD0000ULL,
    0xFDFDFDFDFDFDFD00ULL,
    0xFDFDFDFDFDFDFDFDULL,
    0x00FDFDFDFDFDFDFDULL,
    0x0000FDFDFDFDFDFDULL,
    0x000000FDFDFDFDFDULL,
    0x00000000FDFDFDFDULL,
    0x0000000000FDFDFDULL,
    0x000000000000FDFDULL,
    0x00000000000000FDULL,
};

static word screen_triangles[28][32];

// the ULL suffixes are to prevent this warning:
// warning: integer constant is too large for "long" type
static const qword triangles[4][16][8]= //[direction][value][line]
{
    {
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFD00000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL,
            0xFD00000000000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFD000000000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFD0000000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFD00000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFD000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFD0000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFD00ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        }
    },
    {
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x00000000000000FDULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x000000000000FDFDULL,
            0x00000000000000FDULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL,
            0x00000000000000FDULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL,
            0x00000000000000FDULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL,
            0x00000000000000FDULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL,
            0x00000000000000FDULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL,
            0x00000000000000FDULL,
            0x0000000000000000ULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL,
            0x00000000000000FDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL,
            0x000000000000FDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x0000000000FDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x00000000FDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x000000FDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        }
    },
    {
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0xFD00000000000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL
        },
        {
            0x0000000000000000ULL,
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL
        },
        {
            0xFD00000000000000ULL,
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFD000000000000ULL,
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFD0000000000ULL,
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFD00000000ULL,
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFD000000ULL,
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFD0000ULL,
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFD00ULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        }
    },
    {
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x00000000000000FDULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x00000000000000FDULL,
            0x000000000000FDFDULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x00000000000000FDULL,
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x00000000000000FDULL,
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x00000000000000FDULL,
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL
        },
        {
            0x0000000000000000ULL,
            0x0000000000000000ULL,
            0x00000000000000FDULL,
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL
        },
        {
            0x0000000000000000ULL,
            0x00000000000000FDULL,
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL
        },
        {
            0x00000000000000FDULL,
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0x000000000000FDFDULL,
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0x0000000000FDFDFDULL,
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0x00000000FDFDFDFDULL,
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0x000000FDFDFDFDFDULL,
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0x0000FDFDFDFDFDFDULL,
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0x00FDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        },
        {
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL,
            0xFDFDFDFDFDFDFDFDULL
        }
    }
};


void beginScreenWipeOut(int x, int y)
{
    black_opening_shape=chooseWipeShape();
    
    int w=256, h=224;
    int blockrows=28, blockcolumns=32;
    int xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
    
    for(int blockrow=0; blockrow<blockrows; ++blockrow)  //30
    {
        for(int blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn)  //40
        {
            screen_triangles[blockrow][blockcolumn]=zc_max(abs(int(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
        }
    }
    
    black_opening_count = 66;
    black_opening_x = x;
    black_opening_y = y;
    lensclk = 0;
}

void beginScreenWipeIn(int x, int y)
{
    black_opening_shape=chooseWipeShape();
    
    int w=256, h=224;
    int blockrows=28, blockcolumns=32;
    int xoffset=(x-(w/2))/8, yoffset=(y-(h/2))/8;
    
    for(int blockrow=0; blockrow<blockrows; ++blockrow)  //30
    {
        for(int blockcolumn=0; blockcolumn<blockcolumns; ++blockcolumn)  //40
        {
            screen_triangles[blockrow][blockcolumn]=zc_max(abs(int(double(blockcolumns-1)/2-blockcolumn+xoffset)),abs(int(double(blockrows-1)/2-blockrow+yoffset)))|0x0100|((blockrow-yoffset<blockrows/2)?0:0x8000)|((blockcolumn-xoffset<blockcolumns/2)?0x4000:0);
        }
    }
    
    black_opening_count = -66;
    black_opening_x = x;
    black_opening_y = y;
    lensclk = 0;
}

void updateScreenWipe(bool advance)
{
    if(black_opening_count<0) //shape is opening up
    {
        drawScreenWipe(framebuf,black_opening_x,black_opening_y,(66+black_opening_count),66);
        if(advance)
            black_opening_count++;
    }
    else if(black_opening_count>0) //shape is closing
    {
        drawScreenWipe(framebuf,black_opening_x,black_opening_y,black_opening_count,66);
        
        if(advance)
            black_opening_count--;
    }
}

bool screenWipeIsActive()
{
    return black_opening_count!=0;
}

void abortScreenWipe()
{
    black_opening_count=0;
}

static int chooseWipeShape()
{
    // First, count how many bits are set
    int numBits=0;
    int bitCounter;
    
    for(int i=0; i<4; i++)
    {
        if(COOLSCROLL&(1<<i))
            numBits++;
    }
    
    // Shouldn't happen...
    if(numBits==0)
        return bos_circle;
        
    // Pick one of the set bits
    bitCounter=zc_rand()%numBits+1;
    
    for(int i=0; i<4; i++)
    {
        // If this bit is set, decrement the bit counter. If the counter's 0,
        // i is the selected wipe type (from the bos_ enum).
        if((COOLSCROLL&(1<<i))!=0)
        {
            bitCounter--;
            if(bitCounter==0)
                return i;
        }
    }
    
    // This should be unreachable, but the compiler doesn't know that.
    return bos_circle;
}

static void drawOvalWipe(BITMAP* dest, int x, int y, int a, int max_a)
{
    int w=256, h=224;
    double new_w=(w/2)+abs(w/2-x);
    double new_h=(h/2)+abs(h/2-y);
    double b=sqrt(((new_w*new_w)/4)+(new_h*new_h));
    ellipsefill(tmp_scr,x,y,int(2*a*b/max_a)/8*8,int(a*b/max_a)/8*8,0);
}

static void drawTriangleWipe(BITMAP* dest, int x, int y, int a, int max_a)
{
    int w=256, h=224;
    double new_w=(w/2)+abs(w/2-x);
    double new_h=(h/2)+abs(h/2-y);
    double r=a*(new_w*sqrt((double)3)+new_h)/max_a;
    double P2= (PI/2);
    double P23=(2*PI/3);
    double P43=(4*PI/3);
    double Pa= (-4*PI*a/(3*max_a));
    double angle=P2+Pa;
    double a0=angle;
    double a2=angle+P23;
    double a4=angle+P43;
    triangle(tmp_scr, x+int(cos(a0)*r), y-int(sin(a0)*r),
             x+int(cos(a2)*r), y-int(sin(a2)*r),
             x+int(cos(a4)*r), y-int(sin(a4)*r),
             0);
}

static void drawSMASWipe(BITMAP* dest, int x, int y, int a, int max_a)
{
    int w=256, h=224;
    int distance=zc_max(abs(w/2-x),abs(h/2-y))/8;
    
    for(int blockrow=0; blockrow<28; ++blockrow)  //30
    {
        for(int linerow=0; linerow<8; ++linerow)
        {
            qword *triangleline=(qword*)(tmp_scr->line[(blockrow*8+linerow)]);
            
            for(int blockcolumn=0; blockcolumn<32; ++blockcolumn)  //40
            {
                *triangleline=triangles[(screen_triangles[blockrow][blockcolumn]&0xC000)>>14]
                              [zc_min(zc_max((((31+distance)*(max_a-a)/max_a)+((screen_triangles[blockrow][blockcolumn]&0x0FFF)-0x0100)-(15+distance)),0),15)]
                              [linerow];
                ++triangleline;
            }
        }
    }
}

static void drawCircleWipe(BITMAP* dest, int x, int y, int a, int max_a)
{
    int w=256, h=224;
    double new_w=(w/2)+abs(w/2-x);
    double new_h=(h/2)+abs(h/2-y);
    int r=int(sqrt((new_w*new_w)+(new_h*new_h))*a/max_a);
    circlefill(tmp_scr,x,y,r,0);
}

static void drawScreenWipe(BITMAP* dest, int x, int y, int a, int max_a)
{
    clear_to_color(tmp_scr, BLACK);
    int w=256, h=224;
    
    switch(black_opening_shape)
    {
    case bos_oval:
        drawOvalWipe(dest, x, y, a, max_a);
        break;
    
    case bos_triangle:
        drawTriangleWipe(dest, x, y, a, max_a);
        break;
    
    case bos_smas:
        drawSMASWipe(dest, x, y, a, max_a);
        break;
    
    case bos_circle:
    default:
        drawCircleWipe(dest, x, y, a, max_a);
        break;
    }
    
    masked_blit(tmp_scr, dest, 0, 0, 0, 0, 320, 240);
}