//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#include "zc_alleg.h"
#include "zdefs.h"
#include "jwin.h"
#include "zsys.h"
#include "zc_sys.h"
#include "zq_misc.h"
#include "tiles.h"
#include "zq_class.h"
#include <strings.h>
#include "zq_tiles.h"
#include "gui.h"

extern int d_dummy_proc(int msg,DIALOG *d,int c);
extern int d_combo_proc(int msg,DIALOG *d,int c);
extern int select_data(char *prompt,int index,char *(proc)(int,int*), FONT *title_font);
extern void refresh(int flags);
DoorComboSet *DoorComboSets;
extern void restore_mouse();
extern mapscr *TheMaps;
extern zquestheader header;
extern word map_count;


extern FONT *lfont;

extern int onHelp();
extern int jwin_pal[jcMAX];
extern bool saved;

word door_combo_set_count;



char *doors_string[8]={"Wall","Passage","Locked","Shutter","Bombable","Walk-through","1-Way Shutter","Boss"};

char *doorslist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>7)
      index=7;
    return doors_string[index];
  }
  *list_size=8;
  return NULL;
}

void edit_door(int side)
{
  int index=Map.CurrScr()->door[side];
  switch(index)
  {
    case 0:
    case 1:
    case 2: break;
    case 4: index=3; break;


    case 6: index=4; break;
    case 8: index=5; break;
    case 14: index=6; break;
    case 10: index=7; break;
    default: index=0;
  }
  int ret=select_data("Select Door Type",index,doorslist,lfont);
  if(ret!=-1)
  {
    switch(ret)
    {
      case 0:
      case 1:
      case 2: index=ret; break;
      case 3: index=4; break;
      case 4: index=6; break;
      case 5: index=8; break;
      case 6: index=14; break;
      case 7: index=10; break;
      default: index=0;
    }
    saved=false;
    //   Map.Ugo();
    Map.putdoor(side,index);
    refresh(rMAP);
  }
}

char *doorcombosetlist(int index, int *list_size);

static DIALOG door_select_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     40,   32,   240,  144,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select Door" },
  { d_dummy_proc,      160,  40,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_button_proc,     149,  59,   21,   15,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
  { jwin_button_proc,     149,  105,  21,   15,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
  { jwin_button_proc,     129,  79,   15,   21,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
  { jwin_button_proc,     175,  79,   15,   21,   vc(14),  vc(9),  0,       D_EXIT,     0,             0,       (void *) " " },
  { jwin_button_proc,     86,  151,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "O&K" },
  { jwin_button_proc,     170,  151,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_text_proc,       64,  122,  120,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Door Combo Set:" },
  { jwin_droplist_proc,   64,  130,   192,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0,       (void *) doorcombosetlist },
  { NULL }
};

int onDoors()
{
  restore_mouse();
  door_select_dlg[0].dp2 = lfont;
  if(Map.getCurrScr()==TEMPLATE)
    return D_O_K;
  if(Map.getCurrMap()>=Map.getMapCount())
    return D_O_K;

  Map.Ugo();
  bool done=false;
  int ret=0;
  door_select_dlg[9].d1=Map.CurrScr()->door_combo_set;
  do
  {
    ret = zc_popup_dialog(door_select_dlg,-1);
    Map.CurrScr()->door_combo_set=door_select_dlg[9].d1;
    switch (ret)
    {
      case 2:
      case 3:
      case 4:
      case 5:
        edit_door(ret-2);
        break;
      case 6:
        for (int i=0; i>4; i++)
        {
          Map.putdoor(i,Map.CurrScr()->door[i]);
        }
        done=true;
        break;
      case 0:
      case 7:
        //why is this here?
/*
        for (int i=0; i>4; i++)
        {
          Map.putdoor(i,Map.CurrScr()->door[i]);
        }
*/
        Map.Uhuilai();
        done=true;
    }
  } while(!done);
  return D_O_K;
}

/************************************/
/**********  onDoorCombos  **********/
/************************************/

/*
  word doorcombo_u[7][4];   //[door type][door combo]
  byte doorcset_u[7][4];    //[door type][door combo]
  word doorcombo_d[7][4];   //[door type][door combo]
  byte doorcset_d[7][4];    //[door type][door combo]
  word doorcombo_l[7][6];   //[door type][door combo]
  byte doorcset_l[7][6];    //[door type][door combo]
  word doorcombo_r[7][6];   //[door type][door combo]
  byte doorcset_r[7][6];    //[door type][door combo]
  word bombdoorcombo_u[6];   //[door type][door combo]
  byte bombdoorcset_u[6];    //[door type][door combo]
word bombdoorcombo_d[6];   //[door type][door combo]
byte bombdoorcset_d[6];    //[door type][door combo]
word bombdoorcombo_l[9];   //[door type][door combo]
byte bombdoorcset_l[9];    //[door type][door combo]
word bombdoorcombo_r[9];   //[door type][door combo]
byte bombdoorcset_r[9];    //[door type][door combo]
word walkthroughcombo[4]; //[n, s, e, w]
byte walkthroughcset[4];  //[n, s, e, w]
*/

DoorComboSet working_dcs;

void fix_dcs(int dir)
{
  switch (dir)
  {
    case up:
    {
      //first combo copying
      for (int i=0; i<dt_max-2; i++)
      {
        if (working_dcs.doorcombo_u[i+1][0]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_u[i+1][0]=working_dcs.doorcombo_u[i][0];
          }
          working_dcs.doorcset_u[i+1][0]=working_dcs.doorcset_u[i][0];
        }
        if (working_dcs.doorcombo_u[i+1][1]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_u[i+1][1]=working_dcs.doorcombo_u[i][1];
          }
          working_dcs.doorcset_u[i+1][1]=working_dcs.doorcset_u[i][1];
        }
      }
      if (working_dcs.doorcombo_u[dt_bomb][0]==0)  //copy door top
      {
        working_dcs.doorcombo_u[dt_bomb][0]=working_dcs.doorcombo_u[dt_wall][0];
        working_dcs.doorcset_u[dt_bomb][0]=working_dcs.doorcset_u[dt_wall][0];
      }
      if (working_dcs.doorcombo_u[dt_bomb][1]==0)  //copy door top
      {
        working_dcs.doorcombo_u[dt_bomb][1]=working_dcs.doorcombo_u[dt_wall][1];
        working_dcs.doorcset_u[dt_bomb][1]=working_dcs.doorcset_u[dt_wall][1];
      }

      //combo matching
      for (int i=0; i<dt_max-1; i++)
      {
        if ((working_dcs.doorcombo_u[i][0]!=0)&&
          (working_dcs.doorcombo_u[i][1]==0))      //match top half
        {
          working_dcs.doorcombo_u[i][1]=working_dcs.doorcombo_u[i][0]+1;
          working_dcs.doorcset_u[i][1]=working_dcs.doorcset_u[i][0];
        }
        if ((working_dcs.doorcombo_u[i][2]!=0)&&
          (working_dcs.doorcombo_u[i][3]==0))      //match bottom half
        {
          working_dcs.doorcombo_u[i][3]=working_dcs.doorcombo_u[i][2]+1;
          working_dcs.doorcset_u[i][3]=working_dcs.doorcset_u[i][2];
        }
      }
      if (working_dcs.bombdoorcombo_u[0]!=0)       //match top half
      {
        if (working_dcs.bombdoorcombo_u[1]==0)
        {
          working_dcs.bombdoorcombo_u[1]=working_dcs.bombdoorcombo_u[0]+1;
          working_dcs.bombdoorcset_u[1]=working_dcs.bombdoorcset_u[0];
        }
      }
      //local cset fix
      for (int i=0; i<2; i++)
      {
        if (working_dcs.bombdoorcombo_u[i]==0)
        {
          if (working_dcs.doorcombo_u[dt_bomb][0]!=0)
          {
            working_dcs.bombdoorcset_u[i]=working_dcs.doorcset_u[dt_bomb][0];
          }
          else
          {
            working_dcs.bombdoorcset_u[i]=working_dcs.doorcset_u[0][0];
          }
        }
      }
      for (int i=0; i<dt_max-1; i++)
      {
        for (int j=1; j<4; j++)
        {
          if (working_dcs.doorcombo_u[i][j]==0)    //fix cset for doors
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
      for (int i=0; i<dt_max-2; i++)
      {
        if (working_dcs.doorcombo_d[i+1][2]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_d[i+1][2]=working_dcs.doorcombo_d[i][2];
          }
          working_dcs.doorcset_d[i+1][2]=working_dcs.doorcset_d[i][2];
        }
        if (working_dcs.doorcombo_d[i+1][3]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_d[i+1][3]=working_dcs.doorcombo_d[i][3];
          }
          working_dcs.doorcset_d[i+1][3]=working_dcs.doorcset_d[i][3];
        }
      }
      if (working_dcs.doorcombo_d[dt_bomb][2]==0)  //copy door top
      {
        working_dcs.doorcombo_d[dt_bomb][2]=working_dcs.doorcombo_d[dt_wall][2];
        working_dcs.doorcset_d[dt_bomb][2]=working_dcs.doorcset_d[dt_wall][2];
      }
      if (working_dcs.doorcombo_d[dt_bomb][3]==0)  //copy door top
      {
        working_dcs.doorcombo_d[dt_bomb][3]=working_dcs.doorcombo_d[dt_wall][3];
        working_dcs.doorcset_d[dt_bomb][3]=working_dcs.doorcset_d[dt_wall][3];
      }

      //combo matching
      for (int i=0; i<dt_max-1; i++)
      {
        if ((working_dcs.doorcombo_d[i][0]!=0)&&
          (working_dcs.doorcombo_d[i][1]==0))      //match top half
        {
          working_dcs.doorcombo_d[i][1]=working_dcs.doorcombo_d[i][0]+1;
          working_dcs.doorcset_d[i][1]=working_dcs.doorcset_d[i][0];
        }
        if ((working_dcs.doorcombo_d[i][2]!=0)&&
          (working_dcs.doorcombo_d[i][3]==0))      //match bottom half
        {
          working_dcs.doorcombo_d[i][3]=working_dcs.doorcombo_d[i][2]+1;
          working_dcs.doorcset_d[i][3]=working_dcs.doorcset_d[i][2];
        }
      }
      if (working_dcs.bombdoorcombo_d[0]!=0)       //match top half
      {
        if (working_dcs.bombdoorcombo_d[1]==0)
        {
          working_dcs.bombdoorcombo_d[1]=working_dcs.bombdoorcombo_d[0]+1;
          working_dcs.bombdoorcset_d[1]=working_dcs.bombdoorcset_d[0];
        }
      }

      //local cset fix
      for (int i=0; i<2; i++)
      {
        if (working_dcs.bombdoorcombo_d[i]==0)
        {
          if (working_dcs.doorcombo_d[dt_bomb][2]!=0)
          {
            working_dcs.bombdoorcset_d[i]=working_dcs.doorcset_d[dt_bomb][2];
          }
          else
          {
            working_dcs.bombdoorcset_d[i]=working_dcs.doorcset_d[0][2];
          }
        }
      }
      for (int i=0; i<dt_max-1; i++)
      {
        for (int j=0; j<3; j++)

        {
          if (working_dcs.doorcombo_d[i][j]==0)    //fix cset for doors
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
      for (int i=0; i<dt_max-2; i++)
      {
        if (working_dcs.doorcombo_l[i+1][0]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_l[i+1][0]=working_dcs.doorcombo_l[i][0];
          }
          working_dcs.doorcset_l[i+1][0]=working_dcs.doorcset_l[i][0];
        }
        if (working_dcs.doorcombo_l[i+1][2]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_l[i+1][2]=working_dcs.doorcombo_l[i][2];
          }
          working_dcs.doorcset_l[i+1][2]=working_dcs.doorcset_l[i][2];
        }
        if (working_dcs.doorcombo_l[i+1][4]==0)    //copy door top
        {
          if (i<dt_max-3)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_l[i+1][4]=working_dcs.doorcombo_l[i][4];
          }
          working_dcs.doorcset_l[i+1][4]=working_dcs.doorcset_l[i][4];
        }
      }

      if (working_dcs.doorcombo_l[dt_bomb][0]==0)  //copy door top
      {
        working_dcs.doorcombo_l[dt_bomb][0]=working_dcs.doorcombo_l[dt_wall][0];
        working_dcs.doorcset_l[dt_bomb][0]=working_dcs.doorcset_l[dt_wall][0];
      }
      if (working_dcs.doorcombo_l[dt_bomb][1]==0)  //copy door top
      {
        working_dcs.doorcombo_l[dt_bomb][1]=working_dcs.doorcombo_l[dt_wall][1];
        working_dcs.doorcset_l[dt_bomb][1]=working_dcs.doorcset_l[dt_wall][1];
      }
      if (working_dcs.doorcombo_l[dt_bomb][2]==0)  //copy door top
      {
        working_dcs.doorcombo_l[dt_bomb][2]=working_dcs.doorcombo_l[dt_wall][2];
        working_dcs.doorcset_l[dt_bomb][2]=working_dcs.doorcset_l[dt_wall][2];
      }
      if (working_dcs.doorcombo_l[dt_bomb][4]==0)  //copy door top
      {
        working_dcs.doorcombo_l[dt_bomb][4]=working_dcs.doorcombo_l[dt_wall][4];
        working_dcs.doorcset_l[dt_bomb][4]=working_dcs.doorcset_l[dt_wall][4];
      }
      if (working_dcs.doorcombo_l[dt_bomb][5]==0)  //copy door top
      {
        working_dcs.doorcombo_l[dt_bomb][5]=working_dcs.doorcombo_l[dt_wall][5];
        working_dcs.doorcset_l[dt_bomb][5]=working_dcs.doorcset_l[dt_wall][5];
      }

      //combo matching
      for (int i=0; i<dt_max-1; i++)
      {
        if ((working_dcs.doorcombo_l[i][0]!=0)&&
          (working_dcs.doorcombo_l[i][2]==0))      //match left half - a
        {
          working_dcs.doorcombo_l[i][2]=working_dcs.doorcombo_l[i][0]+4;
          working_dcs.doorcset_l[i][2]=working_dcs.doorcset_l[i][0];
        }
        if ((working_dcs.doorcombo_l[i][2]!=0)&&
          (working_dcs.doorcombo_l[i][4]==0))      //match left half - b
        {
          working_dcs.doorcombo_l[i][4]=working_dcs.doorcombo_l[i][2]+4;
          working_dcs.doorcset_l[i][4]=working_dcs.doorcset_l[i][2];
        }
        if ((working_dcs.doorcombo_l[i][1]!=0)&&
          (working_dcs.doorcombo_l[i][3]==0))      //match right half - a
        {
          working_dcs.doorcombo_l[i][3]=working_dcs.doorcombo_l[i][1]+4;
          working_dcs.doorcset_l[i][3]=working_dcs.doorcset_l[i][1];
        }
        if ((working_dcs.doorcombo_l[i][3]!=0)&&
          (working_dcs.doorcombo_l[i][5]==0))      //match right half - b
        {
          working_dcs.doorcombo_l[i][5]=working_dcs.doorcombo_l[i][3]+4;
          working_dcs.doorcset_l[i][5]=working_dcs.doorcset_l[i][3];
        }
      }

      if ((working_dcs.bombdoorcombo_l[0]!=0)&&
        (working_dcs.bombdoorcombo_l[1]==0))       //match rubble - a
      {
        working_dcs.bombdoorcombo_l[1]=working_dcs.bombdoorcombo_l[0]+4;
        working_dcs.bombdoorcset_l[1]=working_dcs.bombdoorcset_l[0];
      }
      if ((working_dcs.bombdoorcombo_l[0]!=0)&&
        (working_dcs.bombdoorcombo_l[1]!=0)&&
        (working_dcs.bombdoorcombo_l[2]==0))       //match rubble - b
      {
        working_dcs.bombdoorcombo_l[2]=working_dcs.bombdoorcombo_l[1]+4;
        working_dcs.bombdoorcset_l[2]=working_dcs.bombdoorcset_l[1];
      }

      //local cset fix
      for (int i=0; i<3; i++)
      {
        if (working_dcs.bombdoorcombo_l[i]==0)
        {
          if (working_dcs.doorcombo_l[dt_bomb][0]!=0)
          {
            working_dcs.bombdoorcset_l[i]=working_dcs.doorcset_l[dt_bomb][2];
          }
          else
          {
            working_dcs.bombdoorcset_l[i]=working_dcs.doorcset_l[0][0];
          }
        }
      }

      for (int i=0; i<dt_max-1; i++)
      {
        for (int j=1; j<6; j++)
        {
          if (working_dcs.doorcombo_l[i][j]==0)    //fix cset for doors
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
      for (int i=0; i<dt_max-2; i++)
      {
        if (working_dcs.doorcombo_r[i+1][1]==0)    //copy door top
        {


          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_r[i+1][1]=working_dcs.doorcombo_r[i][1];
          }
          working_dcs.doorcset_r[i+1][1]=working_dcs.doorcset_r[i][1];
        }
        if (working_dcs.doorcombo_r[i+1][3]==0)    //copy door top
        {
          if (i<dt_max-4)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_r[i+1][3]=working_dcs.doorcombo_r[i][3];
          }
          working_dcs.doorcset_r[i+1][3]=working_dcs.doorcset_r[i][3];
        }
        if (working_dcs.doorcombo_r[i+1][5]==0)    //copy door top
        {
          if (i<dt_max-3)                                   //don't copy combo to bomb door, just cset
          {
            working_dcs.doorcombo_r[i+1][5]=working_dcs.doorcombo_r[i][5];
          }
          working_dcs.doorcset_r[i+1][5]=working_dcs.doorcset_r[i][5];
        }
      }

      if (working_dcs.doorcombo_r[dt_bomb][0]==0)  //copy wall top
      {
        working_dcs.doorcombo_r[dt_bomb][0]=working_dcs.doorcombo_r[dt_wall][0];
        working_dcs.doorcset_r[dt_bomb][0]=working_dcs.doorcset_r[dt_wall][0];
      }
      if (working_dcs.doorcombo_r[dt_bomb][1]==0)  //copy wall top
      {
        working_dcs.doorcombo_r[dt_bomb][1]=working_dcs.doorcombo_r[dt_wall][1];
        working_dcs.doorcset_r[dt_bomb][1]=working_dcs.doorcset_r[dt_wall][1];
      }
      if (working_dcs.doorcombo_r[dt_bomb][3]==0)  //copy wall top
      {
        working_dcs.doorcombo_r[dt_bomb][3]=working_dcs.doorcombo_r[dt_wall][3];
        working_dcs.doorcset_r[dt_bomb][3]=working_dcs.doorcset_r[dt_wall][3];
      }
      if (working_dcs.doorcombo_r[dt_bomb][4]==0)  //copy wall top
      {
        working_dcs.doorcombo_r[dt_bomb][4]=working_dcs.doorcombo_r[dt_wall][4];
        working_dcs.doorcset_r[dt_bomb][4]=working_dcs.doorcset_r[dt_wall][4];
      }
      if (working_dcs.doorcombo_r[dt_bomb][5]==0)  //copy wall top
      {
        working_dcs.doorcombo_r[dt_bomb][5]=working_dcs.doorcombo_r[dt_wall][5];
        working_dcs.doorcset_r[dt_bomb][5]=working_dcs.doorcset_r[dt_wall][5];
      }


      //combo matching
      for (int i=0; i<dt_max-1; i++)
      {
        if ((working_dcs.doorcombo_r[i][0]!=0)&&
          (working_dcs.doorcombo_r[i][2]==0))      //match left half - a
        {
          working_dcs.doorcombo_r[i][2]=working_dcs.doorcombo_r[i][0]+4;
          working_dcs.doorcset_r[i][2]=working_dcs.doorcset_r[i][0];
        }
        if ((working_dcs.doorcombo_r[i][2]!=0)&&
          (working_dcs.doorcombo_r[i][4]==0))      //match left half - b
        {
          working_dcs.doorcombo_r[i][4]=working_dcs.doorcombo_r[i][2]+4;
          working_dcs.doorcset_r[i][4]=working_dcs.doorcset_r[i][2];
        }
        if ((working_dcs.doorcombo_r[i][1]!=0)&&
          (working_dcs.doorcombo_r[i][3]==0))      //match right half - a
        {
          working_dcs.doorcombo_r[i][3]=working_dcs.doorcombo_r[i][1]+4;
          working_dcs.doorcset_r[i][3]=working_dcs.doorcset_r[i][1];
        }
        if ((working_dcs.doorcombo_r[i][3]!=0)&&
          (working_dcs.doorcombo_r[i][5]==0))      //match right half - b
        {
          working_dcs.doorcombo_r[i][5]=working_dcs.doorcombo_r[i][3]+4;
          working_dcs.doorcset_r[i][5]=working_dcs.doorcset_r[i][3];
        }
      }

      if ((working_dcs.bombdoorcombo_r[0]!=0)&&
        (working_dcs.bombdoorcombo_r[1]==0))       //match rubble - a
      {
        working_dcs.bombdoorcombo_r[1]=working_dcs.bombdoorcombo_r[0]+4;
        working_dcs.bombdoorcset_r[1]=working_dcs.bombdoorcset_r[0];
      }
      if ((working_dcs.bombdoorcombo_r[0]!=0)&&
        (working_dcs.bombdoorcombo_r[1]!=0)&&
        (working_dcs.bombdoorcombo_r[2]==0))       //match rubble - b
      {
        working_dcs.bombdoorcombo_r[2]=working_dcs.bombdoorcombo_r[1]+4;
        working_dcs.bombdoorcset_r[2]=working_dcs.bombdoorcset_r[1];
      }

      //local cset fix
      for (int i=0; i<3; i++)
      {
        if (working_dcs.bombdoorcombo_r[i]==0)
        {
          if (working_dcs.doorcombo_r[dt_bomb][5]!=0)
          {
            working_dcs.bombdoorcset_r[i]=working_dcs.doorcset_r[dt_bomb][5];
          }
          else
          {
            working_dcs.bombdoorcset_r[i]=working_dcs.doorcset_r[0][5];
          }
        }
      }

      for (int i=0; i<dt_max-1; i++)
      {
        for (int j=0; j<6; j++)
        {
          if (working_dcs.doorcombo_r[i][j]==0)    //fix cset for doors
          {
            working_dcs.doorcset_r[i][j]=working_dcs.doorcset_r[i][5];
          }
        }
      }
    }
      break;
  }
}

static int door_north_list[] =
{
  // dialog control number
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, -1
};

static int door_south_list[] =
{
  // dialog control number
  70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, -1
};

static int door_west_list[] =
{
  // dialog control number
  132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, -1
};

static int door_east_list[] =
{
  // dialog control number
  213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, -1
};

static TABPANEL door_tabs[] =
{
  // (text)
  { "North",  D_SELECTED,  door_north_list },
  { "South",  0,           door_south_list },
  { "West",   0,           door_west_list },
  { "East",   0,           door_east_list },
  { NULL }
};

int dcs_auto_button_proc(int msg, DIALOG *d, int c);

static DIALOG doorcomboset_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
//  { d_dummy_proc,      0,    0,      0,    0,    0,       0,      0,       0,          0,             0,       0 },
  { jwin_win_proc,     32-8,   3,   256+16,  235,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Door Combos" },
  { dcs_auto_button_proc,     60,   213,  61,   21,   vc(14),  vc(1),  13,      0,     0,             0,       (void *) "Auto" },
  { jwin_button_proc,     130,  213,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     200,  213,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0,       (void *) onHelp },
  //5
  { jwin_text_proc,      56,  34-4,   48,  8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Name:" },
  { jwin_edit_proc,      104-18,  30-4,   168+18,  16,    vc(12),  vc(1),  0,       0,          20,             0,      NULL  },
  { jwin_tab_proc,        36-8,   50-6,  248+16,  154,    jwin_pal[jcBOXFG],  jwin_pal[jcBOX],      0,      0,          0,             0,       (void *) door_tabs, NULL, (void *)doorcomboset_dlg },
  //8
  { jwin_ctext_proc,      64,   64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wall" },
  { jwin_ctext_proc,      112,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Locked" },
  { jwin_ctext_proc,      160,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      208,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bomb" },

  { jwin_ctext_proc,      64,   136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Passage" },
  { jwin_ctext_proc,      112,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Unlocked" },
  { jwin_ctext_proc,      160,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      160,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      208,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      208,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Walk-" },
  { jwin_ctext_proc,      256,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Through" },

  //21
  { jwin_frame_proc,   46,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   94,    80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   142,   80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   190,   80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   94,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   142,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   190,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   46,    80,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   238,   80,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   246,   168,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },

  //31
  //Passage
  { d_combo_proc,      48,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      48,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Locked
  { d_combo_proc,      96,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      96,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Shuttered
  { d_combo_proc,     144,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     144,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Boss
  { d_combo_proc,     192,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     192,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Unlocked
  { d_combo_proc,      96,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      96,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Shuttered
  { d_combo_proc,     144,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     144,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Boss
  { d_combo_proc,     192,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     192,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Wall
  { d_combo_proc,      48,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      48,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bombed
  { d_combo_proc,     240,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     256,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     240,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     256,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bomb Rubble
  { d_combo_proc,     240,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     256,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Walkthrough
  { d_combo_proc,     248,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },



  //70
  { jwin_ctext_proc,      64,   64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wall" },
  { jwin_ctext_proc,      112,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Locked" },
  { jwin_ctext_proc,      160,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      208,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  64+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bomb" },

  { jwin_ctext_proc,      64,   136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Passage" },
  { jwin_ctext_proc,      112,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Unlocked" },
  { jwin_ctext_proc,      160,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      160,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      208,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      208,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Walk-" },
  { jwin_ctext_proc,      256,  136+8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Through" },

  { jwin_frame_proc,   46,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   94,    96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   142,   96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   190,   96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   94,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   142,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   190,    152,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   46,    96,      36,   36,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   238,   80,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   246,   152,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },

  //93
  //Passage
  { d_combo_proc,      48,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      48,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Locked
  { d_combo_proc,      96,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      96,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Shuttered

  { d_combo_proc,     144,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     144,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Boss
  { d_combo_proc,     192,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     192,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Unlocked
  { d_combo_proc,      96,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      96,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     112,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Shuttered
  { d_combo_proc,     144,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     144,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     160,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Boss
  { d_combo_proc,     192,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     192,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     208,  160+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Wall
  { d_combo_proc,      48,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      48,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      64,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bombed
  { d_combo_proc,     240,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     256,   88+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     240,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     256,  104+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  //Bomb Rubble
  { d_combo_proc,     240,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     256,   72+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Walkthrough
  { d_combo_proc,     248,  144+8+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },




  //132
  { jwin_ctext_proc,      56,   64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wall" },
  { jwin_ctext_proc,      104,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Locked" },
  { jwin_ctext_proc,      152,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      200,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bombed" },

  { jwin_ctext_proc,      56,   136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Passage" },
  { jwin_ctext_proc,      104,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Unlocked" },
  { jwin_ctext_proc,      152,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      152,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      200,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      200,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Walk-" },
  { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Through" },

  { jwin_frame_proc,   38,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   86,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   134,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   182,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   86,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   134,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   182,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   38,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   230,   72,      52,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   246,   160,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },

  //155
  //Passage
  { d_combo_proc,      40,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Locked
  { d_combo_proc,      88,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Shuttered
  { d_combo_proc,     136,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Boss
  { d_combo_proc,     184,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Unlocked
  { d_combo_proc,      88,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Shuttered
  { d_combo_proc,     136,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Boss
  { d_combo_proc,     184,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Wall
  { d_combo_proc,      40,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bombed
  { d_combo_proc,     232,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     248,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     232,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     248,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     232,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     248,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bomb Rubble
  { d_combo_proc,     264,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     264,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     264,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Walkthrough
  { d_combo_proc,     248,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },



  //213
  { jwin_ctext_proc,      56,   64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Wall" },
  { jwin_ctext_proc,      104,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Locked" },
  { jwin_ctext_proc,      152,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      200,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  64,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Bombed" },

  { jwin_ctext_proc,      56,   136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Passage" },
  { jwin_ctext_proc,      104,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Unlocked" },
  { jwin_ctext_proc,      152,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      152,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Shuttered" },
  { jwin_ctext_proc,      200,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Open" },
  { jwin_ctext_proc,      200,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Boss" },
  { jwin_ctext_proc,      256,  136-8,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Walk-" },
  { jwin_ctext_proc,      256,  136,    0,   8,    vc(14),  vc(1),  0,       0,          0,             0,       (void *) "Through" },

  { jwin_frame_proc,   38,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   86,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   134,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   182,   72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   86,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   134,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   182,    144,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   38,    72,      36,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   230,   72,      52,   52,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },
  { jwin_frame_proc,   246,   160,      20,   20,   0,       0,      0,       0,          FR_DEEP,       0,       NULL },

  //235
  //Passage
  { d_combo_proc,      40,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Locked
  { d_combo_proc,      88,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  { d_combo_proc,     104,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Shuttered
  { d_combo_proc,     136,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Boss
  { d_combo_proc,     184,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Unlocked
  { d_combo_proc,      88,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      88,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     104,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Open Shuttered
  { d_combo_proc,     136,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     136,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     152,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },


  //Open Boss
  { d_combo_proc,     184,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,  144+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     184,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     200,  176+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Wall
  { d_combo_proc,      40,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      40,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,      56,   104+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bombed
  { d_combo_proc,     248,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     264,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     248,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     264,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     248,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     264,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Bomb Rubble
  { d_combo_proc,     232,   72+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     232,   88+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },
  { d_combo_proc,     232,   104+2,  16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  //Walkthrough
  { d_combo_proc,     248,  160+2,   16,   16,   2,       0,      0,       0,          0,             0,       NULL },

  { jwin_check_proc,  184,  202, 112,  9,    vc(14),  vc(1),  0,       0,          1,             0,       (void *) "Overlay Walkthrough" },

  { NULL }
};


void fill_dcs_dlg()
{
  //north doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<4; y++)
    {
      doorcomboset_dlg[(x*4)+y+31].d1 = working_dcs.doorcombo_u[x][y];
      doorcomboset_dlg[(x*4)+y+31].fg = working_dcs.doorcset_u[x][y];
    }
  }

  for(int x=0; x<2; x++)
  {
    doorcomboset_dlg[x+67].d1 = working_dcs.bombdoorcombo_u[x];
    doorcomboset_dlg[x+67].fg = working_dcs.bombdoorcset_u[x];
  }

  doorcomboset_dlg[69].d1 = working_dcs.walkthroughcombo[0];
  doorcomboset_dlg[69].fg = working_dcs.walkthroughcset[0];


  //south doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<4; y++)
    {
      doorcomboset_dlg[(x*4)+y+93].d1 = working_dcs.doorcombo_d[x][y];
      doorcomboset_dlg[(x*4)+y+93].fg = working_dcs.doorcset_d[x][y];
    }
  }

  for(int x=0; x<2; x++)
  {
    doorcomboset_dlg[x+129].d1 = working_dcs.bombdoorcombo_d[x];
    doorcomboset_dlg[x+129].fg = working_dcs.bombdoorcset_d[x];
  }

  doorcomboset_dlg[131].d1 = working_dcs.walkthroughcombo[1];
  doorcomboset_dlg[131].fg = working_dcs.walkthroughcset[1];


  //east doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<6; y++)
    {
      doorcomboset_dlg[(x*6)+y+155].d1 = working_dcs.doorcombo_l[x][y];
      doorcomboset_dlg[(x*6)+y+155].fg = working_dcs.doorcset_l[x][y];
    }
  }

  for(int x=0; x<3; x++)
  {
    doorcomboset_dlg[x+209].d1 = working_dcs.bombdoorcombo_l[x];
    doorcomboset_dlg[x+209].fg = working_dcs.bombdoorcset_l[x];
  }

  doorcomboset_dlg[212].d1 = working_dcs.walkthroughcombo[2];
  doorcomboset_dlg[212].fg = working_dcs.walkthroughcset[2];


  //west doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<6; y++)
    {
      doorcomboset_dlg[(x*6)+y+236].d1 = working_dcs.doorcombo_r[x][y];
      doorcomboset_dlg[(x*6)+y+236].fg = working_dcs.doorcset_r[x][y];
    }
  }


  for(int x=0; x<3; x++)
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
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<4; y++)
    {
      working_dcs.doorcombo_u[x][y] = doorcomboset_dlg[(x*4)+y+31].d1;
      working_dcs.doorcset_u[x][y] = doorcomboset_dlg[(x*4)+y+31].fg;
    }
  }

  for(int x=0; x<2; x++)
  {
    working_dcs.bombdoorcombo_u[x] = doorcomboset_dlg[x+67].d1;
    working_dcs.bombdoorcset_u[x] = doorcomboset_dlg[x+67].fg;
  }

  working_dcs.walkthroughcombo[0] = doorcomboset_dlg[69].d1;
  working_dcs.walkthroughcset[0] = doorcomboset_dlg[69].fg;


  //south doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<4; y++)
    {
      working_dcs.doorcombo_d[x][y] = doorcomboset_dlg[(x*4)+y+93].d1;
      working_dcs.doorcset_d[x][y] = doorcomboset_dlg[(x*4)+y+93].fg;
    }
  }

  for(int x=0; x<2; x++)
  {
    working_dcs.bombdoorcombo_d[x] = doorcomboset_dlg[x+129].d1;
    working_dcs.bombdoorcset_d[x] = doorcomboset_dlg[x+129].fg;
  }

  working_dcs.walkthroughcombo[1] = doorcomboset_dlg[131].d1;
  working_dcs.walkthroughcset[1] = doorcomboset_dlg[131].fg;


  //east doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<6; y++)
    {
      working_dcs.doorcombo_l[x][y] = doorcomboset_dlg[(x*6)+y+155].d1;
      working_dcs.doorcset_l[x][y] = doorcomboset_dlg[(x*6)+y+155].fg;
    }
  }

  for(int x=0; x<3; x++)
  {
    working_dcs.bombdoorcombo_l[x] = doorcomboset_dlg[x+209].d1;
    working_dcs.bombdoorcset_l[x] = doorcomboset_dlg[x+209].fg;
  }

  working_dcs.walkthroughcombo[2] = doorcomboset_dlg[212].d1;
  working_dcs.walkthroughcset[2] = doorcomboset_dlg[212].fg;


  //west doors
  for (int x=0; x<dt_max-1; x++)
  {
    for(int y=0; y<6; y++)
    {
      working_dcs.doorcombo_r[x][y] = doorcomboset_dlg[(x*6)+y+236].d1;
      working_dcs.doorcset_r[x][y] = doorcomboset_dlg[(x*6)+y+236].fg;
    }
  }

  for(int x=0; x<3; x++)
  {
    working_dcs.bombdoorcombo_r[x] = doorcomboset_dlg[x+290].d1;
    working_dcs.bombdoorcset_r[x] = doorcomboset_dlg[x+290].fg;
  }

  working_dcs.walkthroughcombo[3] = doorcomboset_dlg[293].d1;
  working_dcs.walkthroughcset[3] = doorcomboset_dlg[293].fg;
}


int dcs_auto_button_proc(int msg, DIALOG *d, int c)
{
  int down=0;
  int last_draw;

  switch (msg)
  {
    case MSG_DRAW:
      jwin_draw_text_button(screen, d->x, d->y, d->w, d->h, (char*)d->dp, d->flags, true);
      break;

    case MSG_WANTFOCUS:
      return D_WANTFOCUS;

    case MSG_KEY:
      /* close dialog? */
      extract_dcs_dlg();
      fix_dcs(doorcomboset_dlg[7].d1+1);
      fill_dcs_dlg();
      object_message(doorcomboset_dlg+7,MSG_DRAW,0);
      if (d->flags & D_EXIT)
      {
        return D_CLOSE;
      }
      break;

    case MSG_CLICK:
      last_draw = 0;

      /* track the mouse until it is released */
      while (gui_mouse_b())
      {
        down = mouse_in_rect(d->x, d->y, d->w, d->h);

        /* redraw? */
        if (last_draw != down)
        {
          if(down)
            d->flags |= D_SELECTED;
          else
            d->flags &= ~D_SELECTED;
          scare_mouse();
          object_message(d, MSG_DRAW, 0);
          unscare_mouse();
          last_draw = down;
        }

        /* let other objects continue to animate */
        broadcast_dialog_message(MSG_IDLE, 0);
      }

      /* redraw in normal state */
      if(down)
      {
        d->flags &= ~D_SELECTED;
        scare_mouse();
        object_message(d, MSG_DRAW, 0);
        extract_dcs_dlg();
        fix_dcs(doorcomboset_dlg[7].d1+1);
        fill_dcs_dlg();
        object_message(doorcomboset_dlg+7,MSG_DRAW,0);
        unscare_mouse();
      }

      /* should we close the dialog? */
      if(down && (d->flags & D_EXIT))
        return D_CLOSE;

      break;
    default:
      jwin_button_proc(msg, d, c);
      break;
  }

  return D_O_K;
}

int edit_dcs(int index)
{
  char door_combo_set_name[sizeof(DoorComboSets[0].name)];
  working_dcs=DoorComboSets[index];
  sprintf(door_combo_set_name,"%s",working_dcs.name);
  doorcomboset_dlg[0].dp2 = lfont;
  doorcomboset_dlg[6].dp = door_combo_set_name;

  fill_dcs_dlg();

  doorcomboset_dlg[294].flags = get_bit(working_dcs.flags,df_walktrans) ? D_SELECTED : 0;

  int ret = zc_popup_dialog(doorcomboset_dlg,4);

  if (ret==2)
  {
    sprintf(working_dcs.name, "%s", door_combo_set_name);
    extract_dcs_dlg();
    set_bit(working_dcs.flags,df_walktrans,doorcomboset_dlg[294].flags);
    DoorComboSets[index]=working_dcs;
    if (index==door_combo_set_count)
    {
      door_combo_set_count++;
    }
    saved=false;
  }
  return D_O_K;
}

void editdoorcomboset(int index)
{
  reset_combo_animations();
  DoorComboSet tempdcs=DoorComboSets[index];
  edit_dcs(index);
  setup_combo_animations();
  return;
}

char *doorcombosetlist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>=door_combo_set_count)
      index=door_combo_set_count-1;
    return DoorComboSets[index].name;
  }
  *list_size=door_combo_set_count;
  return NULL;
}

int doorcombosetlist_del();
int copydcs();
int pastedcs();
int replacedcs();

static DIALOG doorcombosetlist_dlg[] =
{
  /* (dialog proc)        (x)   (y)   (w)     (h)     (fg)                 (bg)                  (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,        52,   40,   216+1,  146+1,  vc(14),              vc(1),                0,       D_EXIT,     0,             0,       (void *) "Edit Door Combo Set" },
  { d_dummy_proc,         160,  47,   0,      8,      vc(15),              vc(1),                0,       0,          0,             0,       NULL },
  { jwin_list_proc,       68,   65,   185,    93,     jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],   0,       D_EXIT,     0,             0,       (void *) doorcombosetlist },
  { jwin_button_proc,     90,   162,  61,     21,     vc(14),              vc(1),                13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     170,  162,  61,     21,     vc(14),              vc(1),                27,      D_EXIT,     0,             0,       (void *) "Done" },
  { d_dummy_proc,         0,    0,    0,      0,      0,                   0,                    0,       0,          -1,            0,       NULL},
  { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    0,       0,          KEY_DEL,       0,       (void *) doorcombosetlist_del },
  { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    'c',     0,          0,             0,       (void *) copydcs },
  { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    'v',     0,          0,             0,       (void *) pastedcs },
  { d_keyboard_proc,      0,    0,    0,      0,      0,                   0,                    'r',     0,          0,             0,       (void *) replacedcs },
  { NULL }
};

void reset_doorcomboset(int index);
void fix_door_combo_set(word &door_combo_set, byte index);

int doorcombosetlist_del()
{
  char buf[25];
  int d=doorcombosetlist_dlg[2].d1;
  if((d>0 || door_combo_set_count>2) && d<door_combo_set_count-1)
  {
    strncpy(buf,DoorComboSets[d].name,sizeof(DoorComboSets[d].name));
    if(jwin_alert("Confirm Delete","Delete this door combo set?",buf,NULL,"Yes","No",'y',27,lfont)==1)
    {
      saved=false;
      for(int i=d; i<MAXDOORCOMBOSETS-1; i++)
      {
        DoorComboSets[i]=DoorComboSets[i+1];
      }
      reset_doorcomboset(MAXDOORCOMBOSETS-1);
      --door_combo_set_count;
      int sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;
      for(int s=0; s<sc; s++)
      {
        fix_door_combo_set(TheMaps[s].door_combo_set, d);
      }
    }
    return D_CLOSE;
  }

  return D_O_K;
}

int copydcs()
{
  doorcombosetlist_dlg[5].d1=doorcombosetlist_dlg[2].d1;
  return D_O_K;
}

int pastedcs()
{
  if (doorcombosetlist_dlg[5].d1==-1)
  {
    return D_O_K;
  }
  DoorComboSets[doorcombosetlist_dlg[2].d1]=DoorComboSets[doorcombosetlist_dlg[5].d1];
  doorcombosetlist_dlg[5].d1=-1;
  return D_CLOSE;
}

int replacedcs()
{
  if (doorcombosetlist_dlg[5].d1==-1)
  {
    return D_O_K;

  }
  DoorComboSets[doorcombosetlist_dlg[2].d1]=DoorComboSets[doorcombosetlist_dlg[5].d1];
  int sc = vbound(map_count,0,Map.getMapCount())*MAPSCRS;
  for(int s=0; s<sc; s++)
  {
    if (TheMaps[s].door_combo_set==doorcombosetlist_dlg[5].d1)
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


void reset_doorcomboset(int index)
{
  bound(index,0,MAXDOORCOMBOSETS-1);
  memset(&DoorComboSets[index], 0, sizeof(DoorComboSet));
}

void init_doorcombosets()
{
  for(int i=0; i<MAXDOORCOMBOSETS; i++)
    reset_doorcomboset(i);
  door_combo_set_count=0;
}

int onDoorCombos()
{
  go();
  int index=0;
  doorcombosetlist_dlg[0].dp2=lfont;
  while(index!=-1)
  {
    bool hasroom=false;
    if(door_combo_set_count<MAXDOORCOMBOSETS)
    {
      hasroom=true;
      strcpy(DoorComboSets[door_combo_set_count++].name,"<New Door Combo Set>");
    }

    int ret=zc_popup_dialog(doorcombosetlist_dlg,2);
    index=doorcombosetlist_dlg[2].d1;

    int doedit=false;

    switch(ret)
    {
      case 2:
      case 3: doedit=true; break;
      case 4: index=-1; break;
    }
    if(hasroom)
      memset(DoorComboSets[--door_combo_set_count].name,0,sizeof(DoorComboSets[0].name));

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
  jwin_center_dialog(door_select_dlg);
  jwin_center_dialog(doorcomboset_dlg);
  jwin_center_dialog(doorcombosetlist_dlg);
}
