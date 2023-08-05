#ifndef _FFC_H_
#define _FFC_H_

#include "base/zdefs.h"
#include "base/cpos_info.h"
#include "zfix.h"
#include "solidobject.h"

//x = ffx
//y = ffy
//vx = ffxdelta
//vy = ffydelta
//ax = ffxdelta2
//ay = ffydelta2
//flags  = ffflags
//data = ffdata
//delay = ffdelay
//cset = ffcset
//link = fflink
//script = ffscript
//ffwidth, ffheight?

class ffcdata : public solid_object
{
public:
	zfix ax, ay;
	dword flags;
	word delay;
	byte cset;
	word link;
	byte txsz = 1, tysz = 1;
	word script;
	int32_t initd[INITIAL_D];
	int32_t inita[INITIAL_A];
	uint8_t screen_index;
	bool hooked;
	bool recently_hit;
	int32_t changer_x = -1000, changer_y = -1000;
	int32_t prev_changer_x = -10000000, prev_changer_y = -10000000;
	cpos_info info;
	// TODO z3 !! upstream
	int32_t script_misc[16];
	bool script_wait_draw;
	
	ffcdata() = default;
	ffcdata(ffcdata const& other);
	virtual void solid_update(bool push = true) override;
	void changerCopy(ffcdata& other, int32_t i = -1, int32_t j = -1);
	void clear();
	
	void setData(word newdata);
	void incData(int32_t inc);
	word const& getData() const {return data;}
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay);
	
	virtual bool setSolid(bool set) override;
	virtual void updateSolid() override;
	void setLoaded(bool set);
	bool getLoaded() const;
	
	//Overload to do damage to Hero on pushing them
	virtual void doContactDamage(int32_t hdir) override;
private:
	word data;
	bool loaded;
};

//FF combo flags
#define ffOVERLAY            0x00000001
#define ffTRANS              0x00000002
#define ffSOLID              0x00000004
#define ffCARRYOVER          0x00000008
#define ffSTATIONARY         0x00000010
#define ffCHANGER            0x00000020 //Is a changer
#define ffPRELOAD            0x00000040 //Script is run before screen appears.
#define ffLENSVIS            0x00000080 //Invisible, but not to the Lens of Truth.
#define ffSCRIPTRESET        0x00000100 //Script resets when carried over.
#define ffETHEREAL           0x00000200 //Does not occlude combo and flags on the screen
#define ffIGNOREHOLDUP       0x00000400 //Updated even while Hero is holding an item
#define ffIGNORECHANGER      0x00000800 //Ignore changers
#define ffIMPRECISIONCHANGER 0x00001000 //Ignore changers
#define ffLENSINVIS          0x00002000 //Visible, but not to the Lens of Truth

//FF combo changer flags
#define ffSWAPNEXT           0x80000000 //Swap speed with next FFC
#define ffSWAPPREV           0x40000000 //Swap speed with prev. FFC
#define ffCHANGENEXT         0x20000000 //Increase combo ID
#define ffCHANGEPREV         0x10000000 //Decrease combo ID
#define ffCHANGETHIS         0x08000000 //Change combo/cset to this
#define ffCHANGESPEED        0x04000000 //Change speed to this (default, not implemented yet)

#endif
