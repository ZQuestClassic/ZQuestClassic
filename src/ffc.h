#ifndef _FFC_H_
#define _FFC_H_

#include "base/zdefs.h"
#include "base/cpos_info.h"
#include "base/flags.h"
#include "base/zfix.h"
#include "solidobject.h"

namespace flags {
enum ffc_flags : uint32_t
{
	ffc_none                = 0,
	ffc_overlay             = F(0),            
	ffc_trans               = F(1),              
	ffc_solid               = F(2),              
	ffc_carryover           = F(3),          
	ffc_stationary          = F(4),         
	ffc_changer             = F(5), //Is a changer
	ffc_preload             = F(6), //Script is run before screen appears.
	ffc_lensvis             = F(7), //Invisible, but not to the Lens of Truth.
	ffc_scriptreset         = F(8), //Script resets when carried over.
	ffc_ethereal            = F(9), //Does not occlude combo and flags on the screen
	ffc_ignoreholdup        = F(10), //Updated even while Hero is holding an item
	ffc_ignorechanger       = F(11), //Ignore changers
	ffc_imprecisionchanger  = F(12), //Ignore subpixel for changer collision
	ffc_lensinvis           = F(13), //Visible, but not to the Lens of Truth
	ffc_platform            = F(14), //Move the player with, if touching

	// Changer flags.
	ffc_changespeed         = F(26), //Change speed to this (default, not implemented yet)
	ffc_changethis          = F(27), //Change combo/cset to this
	ffc_changeprev          = F(28), //Decrease combo ID
	ffc_changenext          = F(29), //Increase combo ID
	ffc_swapprev            = F(30), //Swap speed with prev. FFC
	ffc_swapnext            = F(31), //Swap speed with next FFC
};
} // ends namespace

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
	ffc_flags flags;
	word delay;
	byte cset, link, txsz = 1, tysz = 1;
	word script;
	int32_t initd[INITIAL_D];
	int32_t inita[INITIAL_A];
	bool hooked;
	cpos_info info;
	word data;
	
	ffcdata() = default;
	virtual void solid_update(bool push = true) override;
	// Note: If you use this to clear a ffc during gameplay, you must also call zc_ffc_set(ffc, 0)
	void clear();
	
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay);
	
	virtual bool setSolid(bool set) override;
	virtual void updateSolid() override;
	void setLoaded(bool set);
	bool getLoaded() const;
	
	//Overload to do damage to Hero on pushing them
	virtual void doContactDamage(int32_t hdir) override;
private:
	bool loaded;
};

#endif
