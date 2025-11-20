#ifndef ZC_PAL_TABLES_H_
#define ZC_PAL_TABLES_H_

#include "allegro/color.h"

extern PALETTE RAMpal;
extern RGB_MAP* rgb_table;
extern COLOR_MAP* trans_table;
extern COLOR_MAP* trans_table2;

extern void refresh_rgb_tables();

#endif
