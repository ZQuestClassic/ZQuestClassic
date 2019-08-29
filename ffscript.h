#ifndef _FFSCRIPT_H_
#define _FFSCRIPT_H_
#include "zdefs.h"

// Defines for script flags
#define TRUEFLAG	1
#define MOREFLAG	2
#define	TRIGGERED	4

#define fflong(x,y,z)	(((x[(y)][(z)])<<24)+((x[(y)][(z)+1])<<16)+((x[(y)][(z)+2])<<8)+(x[(y)][(z)+3]))
#define ffword(x,y,z)	(((x[(y)][(z)])<<8)+(x[(y)][(z)+1]))

// Defines for the ASM operations
#define SETV	0
#define SETR	1
#define ADDR	2
#define ADDV	3
#define SUBR	4
#define SUBV	5
#define MULTR	6
#define MULTV	7
#define DIVR	8
#define DIVV	9
#define WAITFRAME	0xA
#define GOTO	0xB
#define CHECKTRIG	0xC
#define WARP	0xD
#define COMPARER	0xE
#define COMPAREV	0xF
#define GOTOTRUE	0x10
#define GOTOFALSE	0x11
#define GOTOLESS	0x12
#define GOTOMORE	0x13
#define LOAD1	0x14
#define LOAD2	0x15
#define SETA1	0x16
#define SETA2	0x17
#define QUIT	0x18

#define D0	0
#define D1	1
#define D2	2
#define D3	3
#define D4	4
#define D5	5
#define D6	6
#define D7	7
#define A1	8
#define A2	9
#define DATA	0xA
#define FCSET	0xB
#define	DELAY	0xC
#define FX	0xD
#define FY	0xE
#define XD	0xF
#define YD	0x10
#define XD2	0x11
#define YD2	0x12
#define FLAG	0x13
#define WIDTH	0x14
#define HEIGHT	0x15
#define LINK	0x16
#define COMBOD(n)	((0x17)+((n)*3))
#define COMBOC(n)	((0x18)+((n)*3))
#define COMBOF(n)	((0x19)+((n)*3))
#define LX	0x230
#define LY	0x231

void do_set(int script, word *pc, int i, bool v);
void do_add(int script, word *pc, int i, bool v);
void do_sub(int script, word *pc, int i, bool v);
void do_mult(int script, word *pc, int i, bool v);
void do_div(int script, word *pc, int i, bool v);
void do_comp(int script, word *pc, int i, bool v);
void do_load(int script, word *pc, int i, int a);
void do_seta(int script, word *pc, int i, int a);
int run_script(int script, int i);
int ffscript_engine();
#endif
