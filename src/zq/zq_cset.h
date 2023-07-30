//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------


int32_t onColors_Main();
int32_t onColors_Levels();
int32_t onColors_Sprites();
void center_zq_cset_dialogs();

extern void return_RAMpal_color(AL_CONST PALETTE pal, int32_t x, int32_t y, RGB *rgb);
extern RGB_MAP rgb_table;
extern COLOR_MAP imagepal_table;
extern PALETTE imagepal;

 
