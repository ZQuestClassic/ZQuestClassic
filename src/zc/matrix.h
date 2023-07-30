//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  matrix.h
//
//  Code for the "Matrix" screen saver.
//
//--------------------------------------------------------

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "base/zc_alleg.h"
#include "zc/zc_sys.h"

extern int32_t  input_idle(bool checkmouse);

void Matrix(int32_t speed, int32_t density, int32_t mousedelay);
#endif
 
