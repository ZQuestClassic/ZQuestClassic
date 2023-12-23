#ifndef _MSGSTR_H_
#define _MSGSTR_H_

#include <string>
#include "base/ints.h"
#include "base/headers.h"
#include "base/containers.h"

#define MAX_SCC_ARG 65023

//SCCs
/* Note: Printable ASCII begins at 32 and ends at 126, inclusive. */
#define MSGC_COLOUR                1    // 2 args (cset,swatch)
#define MSGC_SPEED                 2    // 1 arg  (speed)
#define MSGC_GOTOIFGLOBAL          3    // 3 args (register, val, newtring)
#define MSGC_GOTOIFRAND            4    // 2 args (factor, newstring)
#define MSGC_GOTOIF                5    // 2 args (itemid, newstring)
#define MSGC_GOTOIFCTR             6    // 3 args (counter, val, newstring)
#define MSGC_GOTOIFCTRPC           7    // 3 args (counter, val, newstring)
#define MSGC_GOTOIFTRI             8    // 2 args (level, newstring)
#define MSGC_GOTOIFTRICOUNT        9    // 2 args (tricount, newstring)
#define MSGC_CTRUP                 10   // 2 args (counter, val)
#define MSGC_CTRDN                 11   // 2 args (counter, val)
#define MSGC_CTRSET                12   // 2 args (counter, val)
#define MSGC_CTRUPPC               13   // 2 args (counter, val)
#define MSGC_CTRDNPC               14   // 2 args (counter, val)
#define MSGC_CTRSETPC              15   // 2 args (counter, val)
#define MSGC_GIVEITEM              16   // 1 arg  (itemid)
#define MSGC_TAKEITEM              17   // 1 arg  (itemid)
#define MSGC_WARP                  18   // 6 args (dmap, screen, x, y, effect, sound
#define MSGC_SETSCREEND            19   // 4 args (dmap, screen, reg, value)
#define MSGC_SFX                   20   // 1 arg  (sfx)
#define MSGC_MIDI                  21   // 1 arg  (midi)
#define MSGC_NAME                  22   // 0 args
#define MSGC_GOTOIFCREEND          23   // 5 args (dmap, screen, reg, val, newstring)
#define MSGC_CHANGEPORTRAIT        24   // not implemented
#define MSGC_NEWLINE               25   // 0 args
#define MSGC_SHDCOLOR              26   // 2 args (cset,swatch)
#define MSGC_SHDTYPE               27   // 1 arg  (type)
#define MSGC_DRAWTILE              28   // 5 args (tile, cset, wid, hei, flip)
#define MSGC_ENDSTRING             29   // 0 args
#define MSGC_WAIT_ADVANCE          30   // 0 args
//31
//32
//33-127 are ascii chars, unusable
#define MSGC_SETUPMENU             128  // 5 args (tile, cset, wid, hei, flip)
#define MSGC_MENUCHOICE            129  // 5 args (pos, upos, dpos, lpos, rpos)
#define MSGC_RUNMENU               130  // 0 args
#define MSGC_GOTOMENUCHOICE        131  // 2 args (pos, newstring)
#define MSGC_TRIGSECRETS           132  // 1 arg (perm)
#define MSGC_SETSCREENSTATE        133  // 2 args (ind, state)
#define MSGC_SETSCREENSTATER       134  // 4 args (map, screen, ind, state)
#define MSGC_FONT                  135  // 1 arg (font)
#define MSGC_RUN_FRZ_GENSCR        136  // 2 args (script num, force_redraw)
#define MSGC_TRIG_CMB_COPYCAT      137  // 1 arg (copycat id)
//138+

#define MSG_NEW_SIZE 8192
#define MSGBUF_SIZE (MSG_NEW_SIZE*8)

#define STRINGFLAG_WRAP             0x01
#define STRINGFLAG_CONT             0x02
#define STRINGFLAG_CENTER           0x04
#define STRINGFLAG_RIGHT            0x08
#define STRINGFLAG_FULLTILE         0x10
#define STRINGFLAG_TRANS_BG         0x20
#define STRINGFLAG_TRANS_FG         0x40
enum
{
	STR_ADVP_TEXT,
	STR_ADVP_NEXTSTR,
	STR_ADVP_POSSZ,
	STR_ADVP_PORTRAIT,
	STR_ADVP_BACKGROUND,
	STR_ADVP_FONT,
	STR_ADVP_SFX,
	STR_ADVP_SPACING,
	STR_ADVP_MARGINS,
	STR_ADVP_SHADOW,
	STR_ADVP_LAYER,
	STR_ADVP_FLAGS,
	STR_ADVP_SZ
};

struct MsgStr
{
	std::string s;
	word nextstring;
	int32_t tile;
	byte cset;
	bool trans;
	byte font;
	int16_t x;
	int16_t y;
	uint16_t w;
	uint16_t h;
	byte sfx;
	word listpos;
	byte vspace;
	byte hspace;
	byte stringflags;
	int16_t margins[4];
	int32_t portrait_tile;
	byte portrait_cset;
	byte portrait_x;
	byte portrait_y;
	byte portrait_tw;
	byte portrait_th;
	byte shadow_type;
	byte shadow_color;
	byte drawlayer;
	
	// Copy everything except listpos
	MsgStr& operator=(MsgStr const& other);
	// Copy text data - just s and nextstring
	void copyText(MsgStr const& other);
	// Copy style data - everything except s, nextstring, and listpos
	void copyStyle(MsgStr const& other);
	void copyAll(MsgStr const& other);
	void advpaste(MsgStr const& other, bitstring const& pasteflags);
	void clear();
};

extern MsgStr* MsgStrings;

#endif

