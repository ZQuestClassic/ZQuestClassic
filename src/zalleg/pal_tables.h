#pragma once

#include "allegro/color.h"

extern PALETTE RAMpal;
extern RGB_MAP* rgb_table;
extern COLOR_MAP* trans_table;
extern COLOR_MAP* trans_table2;

void refresh_rgb_tables();
