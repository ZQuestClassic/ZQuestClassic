#ifndef _COMBO_H_
#define _COMBO_H_

#include "base/ints.h"
#include "zfix.h"
#include <string>
#include <vector>
#include <functional>

#define NUM_COMBO_ATTRIBUTES       4
#define NUM_COMBO_ATTRIBYTES       8
#define NUM_COMBO_ATTRISHORTS      8

///user flags
#define cflag1                     0x00001
#define cflag2                     0x00002
#define cflag3                     0x00004
#define cflag4                     0x00008
#define cflag5                     0x00010
#define cflag6                     0x00020
#define cflag7                     0x00040
#define cflag8                     0x00080
#define cflag9                     0x00100
#define cflag10                    0x00200
#define cflag11                    0x00400
#define cflag12                    0x00800
#define cflag13                    0x01000
#define cflag14                    0x02000
#define cflag15                    0x04000
#define cflag16                    0x08000

struct newcombo
{
    int32_t tile;
    byte flip;
    byte walk = 0xF0;
    byte type;
    byte csets;
    byte frames;
    byte speed;
    word nextcombo;
    byte nextcset;
    byte flag;
    byte skipanim;
    word nexttimer;
    byte skipanimy;
    byte animflags;
    byte expansion[6];
	int32_t attributes[NUM_COMBO_ATTRIBUTES]; // combodata->Attributes[] and Screen->GetComboAttribute(pos, indx) / SetComboAttribute(pos, indx)
	int32_t usrflags; // combodata->Flags and Screen->ComboFlags[pos]
	int16_t genflags; // general flags
	int32_t triggerflags[6];
	int32_t triggerlevel;
	byte triggerbtn;
	byte triggeritem;
	byte trigtimer;
	byte trigsfx;
	int32_t trigchange;
	word trigprox;
	byte trigctr;
	int32_t trigctramnt;
	byte triglbeam;
	int8_t trigcschange;
	int16_t spawnitem;
	int16_t spawnenemy;
	int8_t exstate = -1;
	int32_t spawnip;
	byte trigcopycat;
	byte trigcooldown;
	byte trig_lstate, trig_gstate;
	int32_t trig_statetime;
	word trig_genscr;
	byte trig_group;
	word trig_group_val;
	byte liftflags;
	byte liftlvl;
	byte liftsfx;
	byte liftitm;
	byte liftgfx;
	word liftcmb, liftundercmb;
	byte liftcs, liftundercs;
	byte liftsprite;
	byte liftdmg;
	int16_t liftbreaksprite = -1;
	byte liftbreaksfx;
	byte lifthei = 8;
	byte lifttime = 16;
	byte lift_parent_item;
	word prompt_cid;
	byte prompt_cs;
	int16_t prompt_x = 12;
	int16_t prompt_y = -8;
	std::string label;
	byte attribytes[NUM_COMBO_ATTRIBYTES];
	int16_t attrishorts[NUM_COMBO_ATTRISHORTS];
	word script;
	int32_t initd[2];
	int32_t o_tile;
	byte cur_frame;
	byte aclk;
	byte speed_mult = 1;
	byte speed_div = 1;
	zfix speed_add;
	byte sfx_appear, sfx_disappear, sfx_loop, sfx_walking, sfx_standing;
	byte spr_appear, spr_disappear, spr_walking, spr_standing;
	
	void set_tile(int32_t newtile);
	void clear();
	bool is_blank(bool ignoreEff = false);
	
	int each_tile(std::function<bool(int32_t)> proc) const;
};

#define AF_FRESH          0x01
#define AF_CYCLE          0x02
#define AF_CYCLENOCSET    0x04
#define AF_TRANSPARENT    0x08

#define LF_LIFTABLE       0x01
#define LF_DROPSET        0x02
#define LF_DROPONLIFT     0x04
#define LF_SPECIALITEM    0x08
#define LF_NOUCSET        0x10
#define LF_NOWPNCMBCSET   0x20
#define LF_BREAKONSOLID   0x40

extern std::vector<newcombo> combobuf;

#endif

