#ifndef _ZC_FFC_H_
#define _ZC_FFC_H_

#include "ffc.h"

void zc_ffc_set(ffcdata& ffc, word data);
void zc_ffc_modify(ffcdata& ffc, word amount);
void zc_ffc_changer(ffcdata& ffc, ffcdata& other, int32_t i, int32_t j);

#endif
 