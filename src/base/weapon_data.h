#ifndef WEAPON_DATA_H_
#define WEAPON_DATA_H_

#include "base/ints.h"
#include "base/zfix.h"
#include "base/headers.h"
#include "base/containers.h"
#include "base/flags.h"
#include <functional>

namespace flags {
enum wdata_flags : uint16_t
{
	wdata_none                = 0,
	wdata_glow_rad            = F(0),
	wdata_set_step            = F(1),
};
} // ends namespace

struct weapon_data
{
	wdata_flags flags;
	
	move_flags moveflags = move_none;
	
	weapon_flags wflags = WFLAG_NONE;
	
	byte burnsprs[WPNSPR_MAX];
	byte light_rads[WPNSPR_MAX];
	byte glow_shape;
	
	int32_t override_flags;
	int32_t hxofs, hyofs, hxsz, hysz, hzsz, xofs, yofs, tilew, tileh;
	
	zfix step;
	
	byte unblockable;
	
	int32_t timeout;
	
	byte imitate_weapon, default_defense;
	
	byte lift_level;
	byte lift_time = 16;
	zfix lift_height = 8;
	
	word script;
	int32_t initd[INITIAL_D];
	
	int16_t pierce_count = -1;
	
	void clear();
	bool is_blank() const;
	void clear_lifting();
	bool is_blank_lifting() const;
	bool operator==(const weapon_data&) const = default;
	
	static weapon_data def_data;
	static weapon_data def_lifting_data;
private:
	static weapon_data make_blank_lifting();
};
#endif

