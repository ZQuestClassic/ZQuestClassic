#ifndef _ZC_FFC_H_
#define _ZC_FFC_H_

#include "base/ints.h"

class ffcdata;

void zc_ffc_set(ffcdata& ffc, word data);
void zc_ffc_update(ffcdata& ffc, word data);
void zc_ffc_modify(ffcdata& ffc, int32_t amount);
void zc_ffc_changer(ffcdata& ffc, ffcdata& other, int32_t i, int32_t j);

#endif

