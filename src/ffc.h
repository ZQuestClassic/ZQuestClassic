#ifndef FFC_H_
#define FFC_H_

#include "base/zdefs.h"
#include "base/cpos_info.h"
#include "base/flags.h"
#include "base/zfix.h"
#include "sprite.h"

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
	ffc_platform            = F(14), //Move the Hero with, if touching

	// Changer flags.
	ffc_changespeed         = F(26), //Change speed to this (default, not implemented yet)
	ffc_changethis          = F(27), //Change combo/cset to this
	ffc_changeprev          = F(28), //Decrease combo ID
	ffc_changenext          = F(29), //Increase combo ID
	ffc_swapprev            = F(30), //Swap speed with prev. FFC
	ffc_swapnext            = F(31), //Swap speed with next FFC
};
} // ends namespace

// A unique identifier for an ffc in the current region.
// Equal to: (region screen index offset) * 128 + (index in mapscr ffcs)
// For non-scrolling regions, or for the top-left screen in a region, this is equal to the index.
typedef uint16_t ffc_id_t;

class ffcdata : public sprite
{
public:
	uint16_t index;
	zfix ax, ay;
	ffc_flags flags;
	word delay;
	byte cset;
	ffc_id_t link;
	word script;
	int32_t initd[INITIAL_D];
	bool hooked;
	bool recently_hit;
	int32_t changer_x = -1000, changer_y = -1000;
	int32_t prev_changer_x = -10000000, prev_changer_y = -10000000;
	cpos_info info;
	bool script_wait_draw;
	word data;
	int8_t layer = 1;
	
	ffcdata() = default;
	virtual void solid_update(bool push = true) override;
	// Note: If you use this to clear a ffc during gameplay, you must also call zc_ffc_set(ffc, 0)
	void clear();

	void draw(BITMAP* dest) override;
	void draw_ffc(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay);

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
