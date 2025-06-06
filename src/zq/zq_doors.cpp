#include <cstring>
#include <vector>

#include "base/files.h"
#include "base/gui.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include "base/misctypes.h"
#include "zc/zc_sys.h"
#include "zq/zq_misc.h"
#include "tiles.h"
#include "zq/zq_class.h"
#include "zq/zq_tiles.h"
#include "zq/zquest.h"
#include "dialog/alert.h"
#include "zq/render.h"
#include "dialog/door_sel.h"

extern void restore_mouse();
extern zquestheader header;

extern int32_t onHelp();
extern int32_t jwin_pal[jcMAX];
extern bool saved;

word door_combo_set_count;

int32_t onDoors()
{
    call_doorseldialog();
    refresh(rMAP | rCOMBOS);
    return D_O_K;
}

DoorComboSet working_dcs;

void fix_dcs(int32_t dir)
{
    switch(dir)
    {
    case up:
    {
        //first combo copying
        for(int32_t i=0; i<dt_max-2; i++)
        {
            if(working_dcs.doorcombo_u[i+1][0]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_u[i+1][0]=working_dcs.doorcombo_u[i][0];
                }
                
                working_dcs.doorcset_u[i+1][0]=working_dcs.doorcset_u[i][0];
            }
            
            if(working_dcs.doorcombo_u[i+1][1]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_u[i+1][1]=working_dcs.doorcombo_u[i][1];
                }
                
                working_dcs.doorcset_u[i+1][1]=working_dcs.doorcset_u[i][1];
            }
        }
        
        if(working_dcs.doorcombo_u[dt_bomb][0]==0)   //copy door top
        {
            working_dcs.doorcombo_u[dt_bomb][0]=working_dcs.doorcombo_u[dt_wall][0];
            working_dcs.doorcset_u[dt_bomb][0]=working_dcs.doorcset_u[dt_wall][0];
        }
        
        if(working_dcs.doorcombo_u[dt_bomb][1]==0)   //copy door top
        {
            working_dcs.doorcombo_u[dt_bomb][1]=working_dcs.doorcombo_u[dt_wall][1];
            working_dcs.doorcset_u[dt_bomb][1]=working_dcs.doorcset_u[dt_wall][1];
        }
        
        //combo matching
        for(int32_t i=0; i<dt_max-1; i++)
        {
            if((working_dcs.doorcombo_u[i][0]!=0)&&
                    (working_dcs.doorcombo_u[i][1]==0))      //match top half
            {
                working_dcs.doorcombo_u[i][1]=working_dcs.doorcombo_u[i][0]+1;
                working_dcs.doorcset_u[i][1]=working_dcs.doorcset_u[i][0];
            }
            
            if((working_dcs.doorcombo_u[i][2]!=0)&&
                    (working_dcs.doorcombo_u[i][3]==0))      //match bottom half
            {
                working_dcs.doorcombo_u[i][3]=working_dcs.doorcombo_u[i][2]+1;
                working_dcs.doorcset_u[i][3]=working_dcs.doorcset_u[i][2];
            }
        }
        
        if(working_dcs.bombdoorcombo_u[0]!=0)        //match top half
        {
            if(working_dcs.bombdoorcombo_u[1]==0)
            {
                working_dcs.bombdoorcombo_u[1]=working_dcs.bombdoorcombo_u[0]+1;
                working_dcs.bombdoorcset_u[1]=working_dcs.bombdoorcset_u[0];
            }
        }
        
        //local cset fix
        for(int32_t i=0; i<2; i++)
        {
            if(working_dcs.bombdoorcombo_u[i]==0)
            {
                if(working_dcs.doorcombo_u[dt_bomb][0]!=0)
                {
                    working_dcs.bombdoorcset_u[i]=working_dcs.doorcset_u[dt_bomb][0];
                }
                else
                {
                    working_dcs.bombdoorcset_u[i]=working_dcs.doorcset_u[0][0];
                }
            }
        }
        
        for(int32_t i=0; i<dt_max-1; i++)
        {
            for(int32_t j=1; j<4; j++)
            {
                if(working_dcs.doorcombo_u[i][j]==0)     //fix cset for doors
                {
                    working_dcs.doorcset_u[i][j]=working_dcs.doorcset_u[i][0];
                }
            }
        }
    }
    break;
    
    case down:
    {
        //first combo copying
        for(int32_t i=0; i<dt_max-2; i++)
        {
            if(working_dcs.doorcombo_d[i+1][2]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_d[i+1][2]=working_dcs.doorcombo_d[i][2];
                }
                
                working_dcs.doorcset_d[i+1][2]=working_dcs.doorcset_d[i][2];
            }
            
            if(working_dcs.doorcombo_d[i+1][3]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_d[i+1][3]=working_dcs.doorcombo_d[i][3];
                }
                
                working_dcs.doorcset_d[i+1][3]=working_dcs.doorcset_d[i][3];
            }
        }
        
        if(working_dcs.doorcombo_d[dt_bomb][2]==0)   //copy door top
        {
            working_dcs.doorcombo_d[dt_bomb][2]=working_dcs.doorcombo_d[dt_wall][2];
            working_dcs.doorcset_d[dt_bomb][2]=working_dcs.doorcset_d[dt_wall][2];
        }
        
        if(working_dcs.doorcombo_d[dt_bomb][3]==0)   //copy door top
        {
            working_dcs.doorcombo_d[dt_bomb][3]=working_dcs.doorcombo_d[dt_wall][3];
            working_dcs.doorcset_d[dt_bomb][3]=working_dcs.doorcset_d[dt_wall][3];
        }
        
        //combo matching
        for(int32_t i=0; i<dt_max-1; i++)
        {
            if((working_dcs.doorcombo_d[i][0]!=0)&&
                    (working_dcs.doorcombo_d[i][1]==0))      //match top half
            {
                working_dcs.doorcombo_d[i][1]=working_dcs.doorcombo_d[i][0]+1;
                working_dcs.doorcset_d[i][1]=working_dcs.doorcset_d[i][0];
            }
            
            if((working_dcs.doorcombo_d[i][2]!=0)&&
                    (working_dcs.doorcombo_d[i][3]==0))      //match bottom half
            {
                working_dcs.doorcombo_d[i][3]=working_dcs.doorcombo_d[i][2]+1;
                working_dcs.doorcset_d[i][3]=working_dcs.doorcset_d[i][2];
            }
        }
        
        if(working_dcs.bombdoorcombo_d[0]!=0)        //match top half
        {
            if(working_dcs.bombdoorcombo_d[1]==0)
            {
                working_dcs.bombdoorcombo_d[1]=working_dcs.bombdoorcombo_d[0]+1;
                working_dcs.bombdoorcset_d[1]=working_dcs.bombdoorcset_d[0];
            }
        }
        
        //local cset fix
        for(int32_t i=0; i<2; i++)
        {
            if(working_dcs.bombdoorcombo_d[i]==0)
            {
                if(working_dcs.doorcombo_d[dt_bomb][2]!=0)
                {
                    working_dcs.bombdoorcset_d[i]=working_dcs.doorcset_d[dt_bomb][2];
                }
                else
                {
                    working_dcs.bombdoorcset_d[i]=working_dcs.doorcset_d[0][2];
                }
            }
        }
        
        for(int32_t i=0; i<dt_max-1; i++)
        {
            for(int32_t j=0; j<3; j++)
            
            {
                if(working_dcs.doorcombo_d[i][j]==0)     //fix cset for doors
                {
                    working_dcs.doorcset_d[i][j]=working_dcs.doorcset_d[i][2];
                }
            }
        }
    }
    break;
    
    case left:
    {
        //first combo copying
        for(int32_t i=0; i<dt_max-2; i++)
        {
            if(working_dcs.doorcombo_l[i+1][0]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_l[i+1][0]=working_dcs.doorcombo_l[i][0];
                }
                
                working_dcs.doorcset_l[i+1][0]=working_dcs.doorcset_l[i][0];
            }
            
            if(working_dcs.doorcombo_l[i+1][2]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_l[i+1][2]=working_dcs.doorcombo_l[i][2];
                }
                
                working_dcs.doorcset_l[i+1][2]=working_dcs.doorcset_l[i][2];
            }
            
            if(working_dcs.doorcombo_l[i+1][4]==0)     //copy door top
            {
                if(i<dt_max-3)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_l[i+1][4]=working_dcs.doorcombo_l[i][4];
                }
                
                working_dcs.doorcset_l[i+1][4]=working_dcs.doorcset_l[i][4];
            }
        }
        
        if(working_dcs.doorcombo_l[dt_bomb][0]==0)   //copy door top
        {
            working_dcs.doorcombo_l[dt_bomb][0]=working_dcs.doorcombo_l[dt_wall][0];
            working_dcs.doorcset_l[dt_bomb][0]=working_dcs.doorcset_l[dt_wall][0];
        }
        
        if(working_dcs.doorcombo_l[dt_bomb][1]==0)   //copy door top
        {
            working_dcs.doorcombo_l[dt_bomb][1]=working_dcs.doorcombo_l[dt_wall][1];
            working_dcs.doorcset_l[dt_bomb][1]=working_dcs.doorcset_l[dt_wall][1];
        }
        
        if(working_dcs.doorcombo_l[dt_bomb][2]==0)   //copy door top
        {
            working_dcs.doorcombo_l[dt_bomb][2]=working_dcs.doorcombo_l[dt_wall][2];
            working_dcs.doorcset_l[dt_bomb][2]=working_dcs.doorcset_l[dt_wall][2];
        }
        
        if(working_dcs.doorcombo_l[dt_bomb][4]==0)   //copy door top
        {
            working_dcs.doorcombo_l[dt_bomb][4]=working_dcs.doorcombo_l[dt_wall][4];
            working_dcs.doorcset_l[dt_bomb][4]=working_dcs.doorcset_l[dt_wall][4];
        }
        
        if(working_dcs.doorcombo_l[dt_bomb][5]==0)   //copy door top
        {
            working_dcs.doorcombo_l[dt_bomb][5]=working_dcs.doorcombo_l[dt_wall][5];
            working_dcs.doorcset_l[dt_bomb][5]=working_dcs.doorcset_l[dt_wall][5];
        }
        
        //combo matching
        for(int32_t i=0; i<dt_max-1; i++)
        {
            if((working_dcs.doorcombo_l[i][0]!=0)&&
                    (working_dcs.doorcombo_l[i][2]==0))      //match left half - a
            {
                working_dcs.doorcombo_l[i][2]=working_dcs.doorcombo_l[i][0]+4;
                working_dcs.doorcset_l[i][2]=working_dcs.doorcset_l[i][0];
            }
            
            if((working_dcs.doorcombo_l[i][2]!=0)&&
                    (working_dcs.doorcombo_l[i][4]==0))      //match left half - b
            {
                working_dcs.doorcombo_l[i][4]=working_dcs.doorcombo_l[i][2]+4;
                working_dcs.doorcset_l[i][4]=working_dcs.doorcset_l[i][2];
            }
            
            if((working_dcs.doorcombo_l[i][1]!=0)&&
                    (working_dcs.doorcombo_l[i][3]==0))      //match right half - a
            {
                working_dcs.doorcombo_l[i][3]=working_dcs.doorcombo_l[i][1]+4;
                working_dcs.doorcset_l[i][3]=working_dcs.doorcset_l[i][1];
            }
            
            if((working_dcs.doorcombo_l[i][3]!=0)&&
                    (working_dcs.doorcombo_l[i][5]==0))      //match right half - b
            {
                working_dcs.doorcombo_l[i][5]=working_dcs.doorcombo_l[i][3]+4;
                working_dcs.doorcset_l[i][5]=working_dcs.doorcset_l[i][3];
            }
        }
        
        if((working_dcs.bombdoorcombo_l[0]!=0)&&
                (working_dcs.bombdoorcombo_l[1]==0))       //match rubble - a
        {
            working_dcs.bombdoorcombo_l[1]=working_dcs.bombdoorcombo_l[0]+4;
            working_dcs.bombdoorcset_l[1]=working_dcs.bombdoorcset_l[0];
        }
        
        if((working_dcs.bombdoorcombo_l[0]!=0)&&
                (working_dcs.bombdoorcombo_l[1]!=0)&&
                (working_dcs.bombdoorcombo_l[2]==0))       //match rubble - b
        {
            working_dcs.bombdoorcombo_l[2]=working_dcs.bombdoorcombo_l[1]+4;
            working_dcs.bombdoorcset_l[2]=working_dcs.bombdoorcset_l[1];
        }
        
        //local cset fix
        for(int32_t i=0; i<3; i++)
        {
            if(working_dcs.bombdoorcombo_l[i]==0)
            {
                if(working_dcs.doorcombo_l[dt_bomb][0]!=0)
                {
                    working_dcs.bombdoorcset_l[i]=working_dcs.doorcset_l[dt_bomb][2];
                }
                else
                {
                    working_dcs.bombdoorcset_l[i]=working_dcs.doorcset_l[0][0];
                }
            }
        }
        
        for(int32_t i=0; i<dt_max-1; i++)
        {
            for(int32_t j=1; j<6; j++)
            {
                if(working_dcs.doorcombo_l[i][j]==0)     //fix cset for doors
                {
                    working_dcs.doorcset_l[i][j]=working_dcs.doorcset_l[i][0];
                }
            }
        }
    }
    break;
    
    case right:
    {
        //first combo copying
        for(int32_t i=0; i<dt_max-2; i++)
        {
            if(working_dcs.doorcombo_r[i+1][1]==0)     //copy door top
            {
            
            
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_r[i+1][1]=working_dcs.doorcombo_r[i][1];
                }
                
                working_dcs.doorcset_r[i+1][1]=working_dcs.doorcset_r[i][1];
            }
            
            if(working_dcs.doorcombo_r[i+1][3]==0)     //copy door top
            {
                if(i<dt_max-4)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_r[i+1][3]=working_dcs.doorcombo_r[i][3];
                }
                
                working_dcs.doorcset_r[i+1][3]=working_dcs.doorcset_r[i][3];
            }
            
            if(working_dcs.doorcombo_r[i+1][5]==0)     //copy door top
            {
                if(i<dt_max-3)                                    //don't copy combo to bomb door, just cset
                {
                    working_dcs.doorcombo_r[i+1][5]=working_dcs.doorcombo_r[i][5];
                }
                
                working_dcs.doorcset_r[i+1][5]=working_dcs.doorcset_r[i][5];
            }
        }
        
        if(working_dcs.doorcombo_r[dt_bomb][0]==0)   //copy wall top
        {
            working_dcs.doorcombo_r[dt_bomb][0]=working_dcs.doorcombo_r[dt_wall][0];
            working_dcs.doorcset_r[dt_bomb][0]=working_dcs.doorcset_r[dt_wall][0];
        }
        
        if(working_dcs.doorcombo_r[dt_bomb][1]==0)   //copy wall top
        {
            working_dcs.doorcombo_r[dt_bomb][1]=working_dcs.doorcombo_r[dt_wall][1];
            working_dcs.doorcset_r[dt_bomb][1]=working_dcs.doorcset_r[dt_wall][1];
        }
        
        if(working_dcs.doorcombo_r[dt_bomb][3]==0)   //copy wall top
        {
            working_dcs.doorcombo_r[dt_bomb][3]=working_dcs.doorcombo_r[dt_wall][3];
            working_dcs.doorcset_r[dt_bomb][3]=working_dcs.doorcset_r[dt_wall][3];
        }
        
        if(working_dcs.doorcombo_r[dt_bomb][4]==0)   //copy wall top
        {
            working_dcs.doorcombo_r[dt_bomb][4]=working_dcs.doorcombo_r[dt_wall][4];
            working_dcs.doorcset_r[dt_bomb][4]=working_dcs.doorcset_r[dt_wall][4];
        }
        
        if(working_dcs.doorcombo_r[dt_bomb][5]==0)   //copy wall top
        {
            working_dcs.doorcombo_r[dt_bomb][5]=working_dcs.doorcombo_r[dt_wall][5];
            working_dcs.doorcset_r[dt_bomb][5]=working_dcs.doorcset_r[dt_wall][5];
        }
        
        
        //combo matching
        for(int32_t i=0; i<dt_max-1; i++)
        {
            if((working_dcs.doorcombo_r[i][0]!=0)&&
                    (working_dcs.doorcombo_r[i][2]==0))      //match left half - a
            {
                working_dcs.doorcombo_r[i][2]=working_dcs.doorcombo_r[i][0]+4;
                working_dcs.doorcset_r[i][2]=working_dcs.doorcset_r[i][0];
            }
            
            if((working_dcs.doorcombo_r[i][2]!=0)&&
                    (working_dcs.doorcombo_r[i][4]==0))      //match left half - b
            {
                working_dcs.doorcombo_r[i][4]=working_dcs.doorcombo_r[i][2]+4;
                working_dcs.doorcset_r[i][4]=working_dcs.doorcset_r[i][2];
            }
            
            if((working_dcs.doorcombo_r[i][1]!=0)&&
                    (working_dcs.doorcombo_r[i][3]==0))      //match right half - a
            {
                working_dcs.doorcombo_r[i][3]=working_dcs.doorcombo_r[i][1]+4;
                working_dcs.doorcset_r[i][3]=working_dcs.doorcset_r[i][1];
            }
            
            if((working_dcs.doorcombo_r[i][3]!=0)&&
                    (working_dcs.doorcombo_r[i][5]==0))      //match right half - b
            {
                working_dcs.doorcombo_r[i][5]=working_dcs.doorcombo_r[i][3]+4;
                working_dcs.doorcset_r[i][5]=working_dcs.doorcset_r[i][3];
            }
        }
        
        if((working_dcs.bombdoorcombo_r[0]!=0)&&
                (working_dcs.bombdoorcombo_r[1]==0))       //match rubble - a
        {
            working_dcs.bombdoorcombo_r[1]=working_dcs.bombdoorcombo_r[0]+4;
            working_dcs.bombdoorcset_r[1]=working_dcs.bombdoorcset_r[0];
        }
        
        if((working_dcs.bombdoorcombo_r[0]!=0)&&
                (working_dcs.bombdoorcombo_r[1]!=0)&&
                (working_dcs.bombdoorcombo_r[2]==0))       //match rubble - b
        {
            working_dcs.bombdoorcombo_r[2]=working_dcs.bombdoorcombo_r[1]+4;
            working_dcs.bombdoorcset_r[2]=working_dcs.bombdoorcset_r[1];
        }
        
        //local cset fix
        for(int32_t i=0; i<3; i++)
        {
            if(working_dcs.bombdoorcombo_r[i]==0)
            {
                if(working_dcs.doorcombo_r[dt_bomb][5]!=0)
                {
                    working_dcs.bombdoorcset_r[i]=working_dcs.doorcset_r[dt_bomb][5];
                }
                else
                {
                    working_dcs.bombdoorcset_r[i]=working_dcs.doorcset_r[0][5];
                }
            }
        }
        
        for(int32_t i=0; i<dt_max-1; i++)
        {
            for(int32_t j=0; j<6; j++)
            {
                if(working_dcs.doorcombo_r[i][j]==0)     //fix cset for doors
                {
                    working_dcs.doorcset_r[i][j]=working_dcs.doorcset_r[i][5];
                }
            }
        }
    }
    break;
    }
}

void auto_generate_dcs(int32_t dir)
{
    DoorComboSet & d = working_dcs;
    
    switch(dir)
    {
    case up:
    {
        for(int32_t i(1); i < 9; ++i)
            for(int32_t j(0); j < 4; ++j)
            {
                d.doorcombo_u[i][j] = d.doorcombo_u[0][j];
                d.doorcset_u[i][j] = d.doorcset_u[0][j];
            }
    }
    break;
    
    case down:
    {
        for(int32_t i(1); i < 9; ++i)
            for(int32_t j(0); j < 4; ++j)
            {
                d.doorcombo_d[i][j] = d.doorcombo_d[0][j];
                d.doorcset_d[i][j] = d.doorcset_d[0][j];
            }
    }
    break;
    
    case left:
    {
        for(int32_t i(1); i < 9; ++i)
            for(int32_t j(0); j < 6; ++j)
            {
                d.doorcombo_l[i][j] = d.doorcombo_l[0][j];
                d.doorcset_l[i][j] = d.doorcset_l[0][j];
            }
    }
    break;
    
    case right:
    {
        for(int32_t i(1); i < 9; ++i)
            for(int32_t j(0); j < 6; ++j)
            {
                d.doorcombo_r[i][j] = d.doorcombo_r[0][j];
                d.doorcset_r[i][j] = d.doorcset_r[0][j];
            }
    }
    break;
    }
}

static int32_t door_top_list[] =
{
    // dialog control number
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, -1
};

static int32_t door_bottom_list[] =
{
    // dialog control number
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, -1
};

static int32_t door_left_list[] =
{
    // dialog control number
    132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, -1
};

static int32_t door_right_list[] =
{
    // dialog control number
    213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, -1
};

static TABPANEL door_tabs[] =
{
    // (text)
    { (char *)"Top",     D_SELECTED,  door_top_list,    0, NULL },
    { (char *)"Bottom",  0,           door_bottom_list, 0, NULL },
    { (char *)"Left",    0,           door_left_list,   0, NULL },
    { (char *)"Right",   0,           door_right_list,  0, NULL },
    { NULL,              0,           NULL,             0, NULL }
};

int32_t dcs_auto_button_proc(int32_t msg, DIALOG *d, int32_t c);

static DIALOG doorcomboset_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
    //  { d_dummy_proc,      0,    0,      0,    0,    0,       0,      0,       0,          0,             0,       0, NULL, NULL },
    { jwin_win_proc,     32-8,   3,   256+16,  235,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Door Combos", NULL,
		(void*)"LClick: Choose combo (from clicked combo)"
		"\nCtrl + LClick: Choose combo (from last open combo page)"
		"\nRClick: Clear to Combo 0, CSet 0"
		"\nAlt + LClick: Place selected combo + cset"
		"\nShift + Alt + LClick: Place selected cset"
		},
    { jwin_button_proc,     60,   213,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Auto", NULL, NULL },
    { jwin_button_proc,     130,  213,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     200,  213,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { d_dummy_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
    //5
    { jwin_text_proc,      56,  34-4,   48,  8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Name:", NULL, NULL },
    { jwin_edit_proc,      104-18,  30-4,   168+18,  16,    vc(12),  vc(1),  0,       0,          20,             0,      NULL , NULL, NULL },
    { jwin_tab_proc,        36-8,   50-6,  248+16,  154,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],      0,      0,          0,             0, (void *) door_tabs, NULL, (void *)doorcomboset_dlg },
    //8
    { jwin_ctext_proc,      64,   64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Wall", NULL, NULL },
    { jwin_ctext_proc,      112,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Locked", NULL, NULL },
    { jwin_ctext_proc,      160,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      208,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Bomb", NULL, NULL },
    
    { jwin_ctext_proc,      64,   136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      112,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Unlocked", NULL, NULL },
    { jwin_ctext_proc,      160,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      160,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      208,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      208,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Walk-", NULL, NULL },
    { jwin_ctext_proc,      256,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Through", NULL, NULL },
    
    //21
    { jwin_frame_proc,   46,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   94,    80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   142,   80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   190,   80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   94,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   142,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   190,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   46,    80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   238,   80,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   246,   168,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    
    //31
    //Passage
    { d_combo_proc,      48,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      48,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Locked
    { d_combo_proc,      96,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      96,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Shuttered
    { d_combo_proc,     144,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     144,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Boss
    { d_combo_proc,     192,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     192,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Unlocked
    { d_combo_proc,      96,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      96,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Shuttered
    { d_combo_proc,     144,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     144,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Boss
    { d_combo_proc,     192,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     192,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Wall
    { d_combo_proc,      48,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      48,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bombed
    { d_combo_proc,     240,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     256,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     240,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     256,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bomb Rubble
    { d_combo_proc,     240,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     256,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Walkthrough
    { d_combo_proc,     248,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    
    
    //70
    { jwin_ctext_proc,      64,   64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Wall", NULL, NULL },
    { jwin_ctext_proc,      112,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Locked", NULL, NULL },
    { jwin_ctext_proc,      160,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      208,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Bomb", NULL, NULL },
    
    { jwin_ctext_proc,      64,   136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      112,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Unlocked", NULL, NULL },
    { jwin_ctext_proc,      160,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      160,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      208,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      208,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Walk-", NULL, NULL },
    { jwin_ctext_proc,      256,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Through", NULL, NULL },
    
    { jwin_frame_proc,   46,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   94,    96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   142,   96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   190,   96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   94,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   142,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   190,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   46,    96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   238,   80,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   246,   152,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    
    //93
    //Passage
    { d_combo_proc,      48,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      48,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Locked
    { d_combo_proc,      96,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      96,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Shuttered
    
    { d_combo_proc,     144,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     144,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Boss
    { d_combo_proc,     192,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     192,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Unlocked
    { d_combo_proc,      96,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      96,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     112,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Shuttered
    { d_combo_proc,     144,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     144,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     160,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Boss
    { d_combo_proc,     192,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     192,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     208,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Wall
    { d_combo_proc,      48,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      48,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      64,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bombed
    { d_combo_proc,     240,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     256,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     240,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     256,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    //Bomb Rubble
    { d_combo_proc,     240,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     256,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Walkthrough
    { d_combo_proc,     248,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    
    
    
    //132
    { jwin_ctext_proc,      56,   64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Wall", NULL, NULL },
    { jwin_ctext_proc,      104,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Locked", NULL, NULL },
    { jwin_ctext_proc,      152,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      200,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Bombed", NULL, NULL },
    
    { jwin_ctext_proc,      56,   136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      104,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Unlocked", NULL, NULL },
    { jwin_ctext_proc,      152,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      152,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      200,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      200,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Walk-", NULL, NULL },
    { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Through", NULL, NULL },
    
    { jwin_frame_proc,   38,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   86,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   134,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   182,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   86,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   134,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   182,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   38,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   230,   72,      52,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   246,   160,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    
    //155
    //Passage
    { d_combo_proc,      40,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Locked
    { d_combo_proc,      88,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Shuttered
    { d_combo_proc,     136,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Boss
    { d_combo_proc,     184,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Unlocked
    { d_combo_proc,      88,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Shuttered
    { d_combo_proc,     136,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Boss
    { d_combo_proc,     184,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Wall
    { d_combo_proc,      40,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bombed
    { d_combo_proc,     232,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     248,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     232,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     248,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     232,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     248,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bomb Rubble
    { d_combo_proc,     264,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     264,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     264,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Walkthrough
    { d_combo_proc,     248,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    
    
    //213
    { jwin_ctext_proc,      56,   64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Wall", NULL, NULL },
    { jwin_ctext_proc,      104,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Locked", NULL, NULL },
    { jwin_ctext_proc,      152,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      200,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Bombed", NULL, NULL },
    
    { jwin_ctext_proc,      56,   136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      104,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Unlocked", NULL, NULL },
    { jwin_ctext_proc,      152,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      152,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Shuttered", NULL, NULL },
    { jwin_ctext_proc,      200,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Open", NULL, NULL },
    { jwin_ctext_proc,      200,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Boss", NULL, NULL },
    { jwin_ctext_proc,      256,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Walk-", NULL, NULL },
    { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Through", NULL, NULL },
    
    { jwin_frame_proc,   38,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   86,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   134,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   182,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   86,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   134,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   182,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   38,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   230,   72,      52,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { jwin_frame_proc,   246,   160,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    
    //235
    //Passage
    { d_combo_proc,      40,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Locked
    { d_combo_proc,      88,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    { d_combo_proc,     104,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Shuttered
    { d_combo_proc,     136,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Boss
    { d_combo_proc,     184,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Unlocked
    { d_combo_proc,      88,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      88,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     104,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Open Shuttered
    { d_combo_proc,     136,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     136,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     152,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    
    //Open Boss
    { d_combo_proc,     184,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     184,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     200,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Wall
    { d_combo_proc,      40,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      40,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,      56,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bombed
    { d_combo_proc,     248,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     264,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     248,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     264,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     248,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     264,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Bomb Rubble
    { d_combo_proc,     232,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     232,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    { d_combo_proc,     232,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    //Walkthrough
    { d_combo_proc,     248,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL, NULL, NULL },
    
    { jwin_check_proc,  184,  202, 112,  9,    vc(14),  vc(1),  0,       0,          1,             0, (void *) "Overlay Walkthrough", NULL, NULL },
    
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


void fill_dcs_dlg()
{
    //north doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<4; y++)
        {
            doorcomboset_dlg[(x*4)+y+31].d1 = working_dcs.doorcombo_u[x][y];
            doorcomboset_dlg[(x*4)+y+31].fg = working_dcs.doorcset_u[x][y];
        }
    }
    
    for(int32_t x=0; x<2; x++)
    {
        doorcomboset_dlg[x+67].d1 = working_dcs.bombdoorcombo_u[x];
        doorcomboset_dlg[x+67].fg = working_dcs.bombdoorcset_u[x];
    }
    
    doorcomboset_dlg[69].d1 = working_dcs.walkthroughcombo[0];
    doorcomboset_dlg[69].fg = working_dcs.walkthroughcset[0];
    
    
    //south doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<4; y++)
        {
            doorcomboset_dlg[(x*4)+y+93].d1 = working_dcs.doorcombo_d[x][y];
            doorcomboset_dlg[(x*4)+y+93].fg = working_dcs.doorcset_d[x][y];
        }
    }
    
    for(int32_t x=0; x<2; x++)
    {
        doorcomboset_dlg[x+129].d1 = working_dcs.bombdoorcombo_d[x];
        doorcomboset_dlg[x+129].fg = working_dcs.bombdoorcset_d[x];
    }
    
    doorcomboset_dlg[131].d1 = working_dcs.walkthroughcombo[1];
    doorcomboset_dlg[131].fg = working_dcs.walkthroughcset[1];
    
    
    //east doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<6; y++)
        {
            doorcomboset_dlg[(x*6)+y+155].d1 = working_dcs.doorcombo_l[x][y];
            doorcomboset_dlg[(x*6)+y+155].fg = working_dcs.doorcset_l[x][y];
        }
    }
    
    for(int32_t x=0; x<3; x++)
    {
        doorcomboset_dlg[x+209].d1 = working_dcs.bombdoorcombo_l[x];
        doorcomboset_dlg[x+209].fg = working_dcs.bombdoorcset_l[x];
    }
    
    doorcomboset_dlg[212].d1 = working_dcs.walkthroughcombo[2];
    doorcomboset_dlg[212].fg = working_dcs.walkthroughcset[2];
    
    
    //west doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<6; y++)
        {
            doorcomboset_dlg[(x*6)+y+236].d1 = working_dcs.doorcombo_r[x][y];
            doorcomboset_dlg[(x*6)+y+236].fg = working_dcs.doorcset_r[x][y];
        }
    }
    
    
    for(int32_t x=0; x<3; x++)
    {
        doorcomboset_dlg[x+290].d1 = working_dcs.bombdoorcombo_r[x];
        doorcomboset_dlg[x+290].fg = working_dcs.bombdoorcset_r[x];
    }
    
    doorcomboset_dlg[293].d1 = working_dcs.walkthroughcombo[3];
    doorcomboset_dlg[293].fg = working_dcs.walkthroughcset[3];
}


void extract_dcs_dlg()
{
    //north doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<4; y++)
        {
            working_dcs.doorcombo_u[x][y] = doorcomboset_dlg[(x*4)+y+31].d1;
            working_dcs.doorcset_u[x][y] = doorcomboset_dlg[(x*4)+y+31].fg;
        }
    }
    
    for(int32_t x=0; x<2; x++)
    {
        working_dcs.bombdoorcombo_u[x] = doorcomboset_dlg[x+67].d1;
        working_dcs.bombdoorcset_u[x] = doorcomboset_dlg[x+67].fg;
    }
    
    working_dcs.walkthroughcombo[0] = doorcomboset_dlg[69].d1;
    working_dcs.walkthroughcset[0] = doorcomboset_dlg[69].fg;
    
    
    //south doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<4; y++)
        {
            working_dcs.doorcombo_d[x][y] = doorcomboset_dlg[(x*4)+y+93].d1;
            working_dcs.doorcset_d[x][y] = doorcomboset_dlg[(x*4)+y+93].fg;
        }
    }
    
    for(int32_t x=0; x<2; x++)
    {
        working_dcs.bombdoorcombo_d[x] = doorcomboset_dlg[x+129].d1;
        working_dcs.bombdoorcset_d[x] = doorcomboset_dlg[x+129].fg;
    }
    
    working_dcs.walkthroughcombo[1] = doorcomboset_dlg[131].d1;
    working_dcs.walkthroughcset[1] = doorcomboset_dlg[131].fg;
    
    
    //east doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<6; y++)
        {
            working_dcs.doorcombo_l[x][y] = doorcomboset_dlg[(x*6)+y+155].d1;
            working_dcs.doorcset_l[x][y] = doorcomboset_dlg[(x*6)+y+155].fg;
        }
    }
    
    for(int32_t x=0; x<3; x++)
    {
        working_dcs.bombdoorcombo_l[x] = doorcomboset_dlg[x+209].d1;
        working_dcs.bombdoorcset_l[x] = doorcomboset_dlg[x+209].fg;
    }
    
    working_dcs.walkthroughcombo[2] = doorcomboset_dlg[212].d1;
    working_dcs.walkthroughcset[2] = doorcomboset_dlg[212].fg;
    
    
    //west doors
    for(int32_t x=0; x<dt_max-1; x++)
    {
        for(int32_t y=0; y<6; y++)
        {
            working_dcs.doorcombo_r[x][y] = doorcomboset_dlg[(x*6)+y+236].d1;
            working_dcs.doorcset_r[x][y] = doorcomboset_dlg[(x*6)+y+236].fg;
        }
    }
    
    for(int32_t x=0; x<3; x++)
    {
        working_dcs.bombdoorcombo_r[x] = doorcomboset_dlg[x+290].d1;
        working_dcs.bombdoorcset_r[x] = doorcomboset_dlg[x+290].fg;
    }
    
    working_dcs.walkthroughcombo[3] = doorcomboset_dlg[293].d1;
    working_dcs.walkthroughcset[3] = doorcomboset_dlg[293].fg;
}

int32_t edit_dcs(int32_t index)
{
    char door_combo_set_name[21];
    working_dcs=DoorComboSets[index];
    sprintf(door_combo_set_name,"%s",DoorComboSetNames[index].c_str());
    doorcomboset_dlg[0].dp2 = get_zc_font(font_lfont);
    doorcomboset_dlg[6].dp = door_combo_set_name;
    
    fill_dcs_dlg();
    
    doorcomboset_dlg[294].flags = get_bit(working_dcs.flags,df_walktrans) ? D_SELECTED : 0;
    
    large_dialog(doorcomboset_dlg, 2.0);
    
    int32_t ret;
    
    do
    {
        ret = do_zqdialog(doorcomboset_dlg,4);
        
        if(ret==1)
        {
            extract_dcs_dlg();
            int32_t selected = -1;
            
            for(int32_t i=0; door_tabs[i].text; i++)
            {
                if(door_tabs[i].flags&D_SELECTED)
                {
                    selected = i;
                    break;
                }
            }
            
            // shortcut: (Shift + Auto), will *mostly* automate a dcs by duplicating the first door.
            // this isn't perfect, but it an indespensible tool for users putting together tilesets.
            if(key[KEY_RSHIFT] || key[KEY_LSHIFT])
            {
                auto_generate_dcs(selected);
            }
            
            fix_dcs(selected);
            fill_dcs_dlg();
            object_message(doorcomboset_dlg+7,MSG_DRAW,0);
        }
    }
    while(ret==1);
    
    if(ret==2)
    {
		DoorComboSetNames[index] = door_combo_set_name;
        extract_dcs_dlg();
        set_bit(working_dcs.flags,df_walktrans,doorcomboset_dlg[294].flags);
        DoorComboSets[index]=working_dcs;
        
        if(index==door_combo_set_count)
        {
            door_combo_set_count++;
        }
        
        saved=false;
    }
    
    return D_O_K;
}

void editdoorcomboset(int32_t index)
{
    reset_combo_animations();
    reset_combo_animations2();
    edit_dcs(index);
    setup_combo_animations();
    setup_combo_animations2();
    return;
}

const char *doorcombosetlist(int32_t index, int32_t *list_size)
{
    if(index>=0)
    {
        if(index>=door_combo_set_count)
            index=door_combo_set_count-1;
            
        return DoorComboSetNames[index].c_str();
    }
    
    *list_size=door_combo_set_count;
    return NULL;
}

int32_t doorcombosetlist_del();
int32_t copydcs();
int32_t pastedcs();
int32_t replacedcs();

static ListData doorcombosetlist_dlg_list(doorcombosetlist, &font);

static DIALOG doorcombosetlist_dlg[] =
{
    /* (dialog proc)        (x)   (y)   (w)     (h)     (fg)                 (bg)                  (key)    (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,        52,   40,   216+1,  150,    vc(14),              vc(1),                0,       D_EXIT,     0,             0, (void *) "Edit Door Combo Set", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_list_proc,       68,   65,   185,    96,     jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],   0,       D_EXIT,     0,             0, (void *) &doorcombosetlist_dlg_list, NULL, NULL },
    { jwin_button_proc,     90,   165,  61,     21,     vc(14),              vc(1),                13,      D_EXIT,     0,             0, (void *) "Edit", NULL, NULL },
    { jwin_button_proc,     170,  165,  61,     21,     vc(14),              vc(1),                27,      D_EXIT,     0,             0, (void *) "Done", NULL, NULL },
    { d_dummy_proc,         0,    0,    0,      0,      0,                   0,                    0,       0,          -1,            0,       NULL, NULL, NULL },
    { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    0,       0,          KEY_DEL,       0, (void *) doorcombosetlist_del, NULL, NULL },
    { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    'c',     0,          0,             0, (void *) copydcs, NULL, NULL },
    { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    'v',     0,          0,             0, (void *) pastedcs, NULL, NULL },
    { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    'r',     0,          0,             0, (void *) replacedcs, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static int32_t copiedDoor;

void doorlist_rclick_func(int32_t index, int32_t x, int32_t y)
{
    if(index < 0) // Clicked (none)?
        return;
	
    NewMenu rcmenu
	{
		{ "Save", [&]()
			{
				if(!prompt_for_new_file_compat("Save Doorset(.zdoors)", "zdoors", NULL,datapath,false))
					return;
				PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
				if(!f) return;
				int32_t ret = writeonezdoorset(f,index);
				pack_fclose(f);
				char tmpbuf[512]={0};
				char name[256] = {0};
				if ( ret )
				{
					extract_name(temppath,name,FILENAMEALL);
					sprintf(tmpbuf,"Saved %s",name);
					jwin_alert("Success!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
				}
				else
				{
					extract_name(temppath,name,FILENAMEALL);
					sprintf(tmpbuf,"Failed to save %s",name);
					jwin_alert("Error!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
				}
			} },
		{ "Load", [&]()
			{
				if(!prompt_for_existing_file_compat("Load Item(.zdoors)", "zdoors", NULL,datapath,false))
					return;
				char name[256] = {0};
				extract_name(filepath,name,FILENAMEALL);
				PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
				if(!f) return;
				int32_t ret = readonezdoorset(f,index);
							
				if (!ret)
				{
					al_trace("Could not read from .zdoors packfile %s\n", temppath);
					jwin_alert("ZDOOR File: Error","Could not load the specified doorset.",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
				}
				else if ( ret == 1 )
				{
					jwin_alert("ZDOORS File: Success!","Loaded the source doorsets!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
					saved=false;
				}
				else if ( ret == 2 )
				{
					jwin_alert("ZDOORS File: Issue:","Targets exceed doorset count!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
					saved=false;
				}
				pack_fclose(f);
				doorcombosetlist_dlg[2].flags|=D_DIRTY; //Causes the dialogie list to refresh, updating the item name.
				saved=false;
			} },
	};
	rcmenu.pop(x, y);
}

void reset_doorcomboset(int32_t index);
void fix_door_combo_set(word &door_combo_set, byte index);

int32_t doorcombosetlist_del()
{
    int32_t d=doorcombosetlist_dlg[2].d1;
    
    if((d>0 || door_combo_set_count>2) && d<door_combo_set_count-1)
    {
        if(jwin_alert("Confirm Delete","Delete this door combo set?",DoorComboSetNames[d].c_str(),NULL,"Yes","No",'y',27,get_zc_font(font_lfont))==1)
        {
            saved=false;
            
            for(int32_t i=d; i<MAXDOORCOMBOSETS-1; i++)
            {
                DoorComboSets[i]=DoorComboSets[i+1];
            }
            
            reset_doorcomboset(MAXDOORCOMBOSETS-1);
            --door_combo_set_count;
            int32_t sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;
            
            for(int32_t s=0; s<sc; s++)
            {
                fix_door_combo_set(TheMaps[s].door_combo_set, d);
            }
        }
        
        return D_CLOSE;
    }
    
    return D_O_K;
}

int32_t copydcs()
{
    doorcombosetlist_dlg[5].d1=doorcombosetlist_dlg[2].d1;
    return D_O_K;
}

int32_t pastedcs()
{
    if(doorcombosetlist_dlg[5].d1==-1)
    {
        return D_O_K;
    }
    
    DoorComboSets[doorcombosetlist_dlg[2].d1]=DoorComboSets[doorcombosetlist_dlg[5].d1];
    doorcombosetlist_dlg[5].d1=-1;
    return D_CLOSE;
}

int32_t replacedcs()
{
    if(doorcombosetlist_dlg[5].d1==-1)
    {
        return D_O_K;
        
    }
    
    DoorComboSets[doorcombosetlist_dlg[2].d1]=DoorComboSets[doorcombosetlist_dlg[5].d1];
    int32_t sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;
    
    for(int32_t s=0; s<sc; s++)
    {
        if(TheMaps[s].door_combo_set==doorcombosetlist_dlg[5].d1)
        {
            TheMaps[s].door_combo_set=doorcombosetlist_dlg[2].d1;
        }
    }
    
    doorcombosetlist_dlg[5].d1=-1;
    return D_O_K;
}

void fix_door_combo_set(word &door_combo_set, byte index)
{
    if(door_combo_set == index)
        door_combo_set = 0;
    else if(door_combo_set > index)
        --door_combo_set;
}


void reset_doorcomboset(int32_t index)
{
    bound(index,0,MAXDOORCOMBOSETS-1);
    memset(&DoorComboSets[index], 0, sizeof(DoorComboSet));
}

void init_doorcombosets()
{
    for(int32_t i=0; i<MAXDOORCOMBOSETS; i++)
        reset_doorcomboset(i);
        
    door_combo_set_count=0;
}

int32_t onDoorCombos()
{
    go();
    int32_t index=0;
    doorcombosetlist_dlg[0].dp2=get_zc_font(font_lfont);
    doorcombosetlist_dlg[2].dp3 = (void *)&doorlist_rclick_func;
    doorcombosetlist_dlg[2].flags|=(D_USER<<1);
    
    while(index!=-1)
    {
        bool hasroom=false;
        
        if(door_combo_set_count<MAXDOORCOMBOSETS)
        {
            hasroom=true;
            DoorComboSetNames[door_combo_set_count++] = "<New Door Combo Set>";
        }
        
        large_dialog(doorcombosetlist_dlg,1.5);
            
        int32_t ret=do_zqdialog(doorcombosetlist_dlg,2);
        index=doorcombosetlist_dlg[2].d1;
        
        int32_t doedit=false;
        
        switch(ret)
        {
        case 2:
        case 3:
            doedit=true;
            break;
            
        case 0:
        case 4:
            index=-1;
            break;
        }
        
        if(hasroom)
			DoorComboSetNames[--door_combo_set_count].clear();
            
        if(doedit)
        {
            editdoorcomboset(index);
            refresh(rMENU);
        }
    }
    
    comeback();
    return D_O_K;
}

void center_zq_door_dialogs()
{
    jwin_center_dialog(doorcomboset_dlg);
    jwin_center_dialog(doorcombosetlist_dlg);
}
