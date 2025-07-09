#include "base/weapon_data.h"

void weapon_data::clear()
{
	*this = weapon_data();
}
bool weapon_data::is_blank() const
{
	return *this == weapon_data();
}
void weapon_data::clear_lifting()
{
	clear();
	moveflags |= move_obeys_grav|move_can_pitfall;
	wflags |= WFLAG_BREAK_WHEN_LANDING;
	flags |= wdata_glow_rad;
}
bool weapon_data::is_blank_lifting() const
{
	weapon_data w;
	w.clear_lifting();
	return *this == w;
}

weapon_data weapon_data::make_blank_lifting()
{
	weapon_data data;
	data.clear_lifting();
	return data;
}

weapon_data weapon_data::def_data;
weapon_data weapon_data::def_lifting_data = weapon_data::make_blank_lifting();
