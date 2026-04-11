#ifndef ZC_SCRIPTING_MAPDATA_H_
#define ZC_SCRIPTING_MAPDATA_H_

#include "core/handles.h"
#include "core/mapscr.h"
#include "ffc.h"
#include <cstdint>

enum class mapdata_type
{
	None,
	CanonicalScreen,
	TemporaryCurrentScreen,
	TemporaryCurrentRegion,
	TemporaryScrollingScreen,
	TemporaryScrollingRegion,
};

struct mapdata
{
	mapdata_type type;
	mapscr* base_scr;
	mapscr* scr;
	int screen;
	int layer;

	bool temporary() const
	{
		return type != mapdata_type::None && type != mapdata_type::CanonicalScreen;
	}

	bool canonical() const
	{
		return type == mapdata_type::CanonicalScreen;
	}

	bool current() const
	{
		return type == mapdata_type::TemporaryCurrentRegion || type == mapdata_type::TemporaryCurrentScreen;
	}

	bool scrolling() const
	{
		return type == mapdata_type::TemporaryScrollingRegion || type == mapdata_type::TemporaryScrollingScreen;
	}

	int max_pos();
	rpos_handle_t resolve_pos(int pos);
	ffc_handle_t resolve_ffc_handle(int index);
	ffcdata* resolve_ffc(int index);
};

mapdata decode_mapdata_ref(int ref);
int create_mapdata_temp_ref(mapdata_type type, int screen, int layer);

int32_t mapdata_get_register(int32_t reg);
void mapdata_set_register(int32_t reg, int32_t value);

#endif
