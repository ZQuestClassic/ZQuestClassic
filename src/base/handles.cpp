#include "base/handles.h"
#include "base/combo.h"
#include "base/general.h"
#include "base/mapscr.h"
#include "zc/combos.h"
#include "zc/maps.h"
#include "zc/zc_ffc.h"
#include "ffc.h"
#include <variant>

mapscr* rpos_handle_t::base_scr() const
{
	return layer == 0 ? scr : get_scr(scr->map, scr->screen);
}

mapscr* rpos_handle_t::get_mapscr() const
{
	return scr;
}

newcombo& rpos_handle_t::combo() const
{
	return combobuf[scr->data[pos]];
}

cpos_info& rpos_handle_t::info() const
{
	return cpos_get(*this);
}

std::pair<int32_t, int32_t> rpos_handle_t::xy() const
{
	return COMBOXY_REGION(rpos);
}

std::pair<int32_t, int32_t> rpos_handle_t::center_xy() const
{
	auto [cx, cy] = xy();
	return {cx + 8, cy + 8};
}

void rpos_handle_t::set_data(int32_t value) const
{
	scr->data[pos] = value;
}

void rpos_handle_t::modify_data(int32_t delta) const
{
	scr->data[pos] = BOUND_COMBO(scr->data[pos] + delta);
}

void rpos_handle_t::increment_data() const
{
	if (scr->data[pos] + 1 <= MAXCOMBOS)
		scr->data[pos] += 1;
}

void rpos_handle_t::decrement_data() const
{
	if (scr->data[pos] > 0)
		scr->data[pos] -= 1;
}

int32_t rpos_handle_t::cset() const
{
	return scr->cset[pos];
}

void rpos_handle_t::set_cset(int32_t value) const
{
	scr->cset[pos] = value;
}

uint8_t rpos_handle_t::sflag() const
{
	return scr->sflag[pos];
}

void rpos_handle_t::set_sflag(uint8_t value) const
{
	scr->sflag[pos] = value;
}

uint8_t rpos_handle_t::cflag() const
{
	return combo_caches::flag.minis[scr->data[pos]].flag;
}

uint8_t rpos_handle_t::ctype() const
{
	return combo_caches::type.minis[scr->data[pos]].type;
}

mapscr* ffc_handle_t::get_mapscr() const
{
	return scr;
}
int32_t ffc_handle_t::layer() const
{
	return 7;
}

const newcombo& ffc_handle_t::combo() const
{
	return combobuf[ffc->data];
}

cpos_info& ffc_handle_t::info() const
{
	return ffc->info;
}

std::pair<int32_t, int32_t> ffc_handle_t::xy() const
{
	return {ffc->x, ffc->y};
}

std::pair<int32_t, int32_t> ffc_handle_t::center_xy() const
{
	return {ffc->x + ffc->hit_width / 2, ffc->y + ffc->hit_height / 2};
}

int32_t ffc_handle_t::data() const
{
	return ffc->data;
}

void ffc_handle_t::set_data(int32_t value) const
{
	zc_ffc_set(*ffc, value);
}

void ffc_handle_t::modify_data(int32_t delta) const
{
	zc_ffc_set(*ffc, BOUND_COMBO(ffc->data + delta));
}

void ffc_handle_t::increment_data() const
{
	if (ffc->data + 1 <= MAXCOMBOS)
		zc_ffc_set(*ffc, ffc->data + 1);
}

void ffc_handle_t::decrement_data() const
{
	if (ffc->data > 0)
		zc_ffc_set(*ffc, ffc->data - 1);
}

int32_t ffc_handle_t::cset() const
{
	return ffc->cset;
}

void ffc_handle_t::set_cset(int32_t cset) const
{
	ffc->cset = cset;
}

uint8_t ffc_handle_t::cflag() const
{
	return combo_caches::flag.minis[ffc->data].flag;
}

uint8_t ffc_handle_t::ctype() const
{
	return combo_caches::type.minis[ffc->data].type;
}


#define IMPL_COMBINED_HANDLE_CALL_0(func) \
void combined_handle_t::func() const \
{ \
	if (std::holds_alternative<rpos_handle_t>(*this)) \
	{ \
		auto& rpos_handle = std::get<rpos_handle_t>(*this); \
		if (rpos_handle.rpos != rpos_t::None) \
			rpos_handle.func(); \
		return; \
	} \
	std::get<ffc_handle_t>(*this).func(); \
}
#define IMPL_COMBINED_HANDLE_CALL_1(func, ty1) \
void combined_handle_t::func(ty1 val1) const \
{ \
	if (std::holds_alternative<rpos_handle_t>(*this)) \
	{ \
		auto& rpos_handle = std::get<rpos_handle_t>(*this); \
		if (rpos_handle.rpos != rpos_t::None) \
			rpos_handle.func(val1); \
		return; \
	} \
	std::get<ffc_handle_t>(*this).func(val1); \
}

#define IMPL_COMBINED_HANDLE_GET(func, ty) \
ty combined_handle_t::func() const \
{ \
	if (std::holds_alternative<rpos_handle_t>(*this)) \
	{ \
		auto& rpos_handle = std::get<rpos_handle_t>(*this); \
		return rpos_handle.func(); \
	} \
	return std::get<ffc_handle_t>(*this).func(); \
}
#define IMPL_COMBINED_HANDLE_GET_DEF(func, ty, defret) \
ty combined_handle_t::func() const \
{ \
	if (std::holds_alternative<rpos_handle_t>(*this)) \
	{ \
		auto& rpos_handle = std::get<rpos_handle_t>(*this); \
		if (rpos_handle.rpos == rpos_t::None) \
			return defret; \
		return rpos_handle.func(); \
	} \
	return std::get<ffc_handle_t>(*this).func(); \
}

#define IMPL_COMBINED_HANDLE_SET(func, ty) IMPL_COMBINED_HANDLE_CALL_1(func, ty)

typedef std::pair<int32_t, int32_t> int_pair;

bool combined_handle_t::is_rpos() const
{
	return std::holds_alternative<rpos_handle_t>(*this);
}

bool combined_handle_t::is_ffc() const
{
	return !std::holds_alternative<rpos_handle_t>(*this);
}
rpos_handle_t const& combined_handle_t::get_rpos() const
{
	return std::get<rpos_handle_t>(*this);
}
ffc_handle_t const& combined_handle_t::get_ffc() const
{
	return std::get<ffc_handle_t>(*this);
}

combined_handle_t::operator bool() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
		return bool(std::get<rpos_handle_t>(*this));

	return bool(std::get<ffc_handle_t>(*this));
}
mapscr* combined_handle_t::base_scr() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
		return std::get<rpos_handle_t>(*this).base_scr();

	return std::get<ffc_handle_t>(*this).scr;
}
IMPL_COMBINED_HANDLE_GET(get_mapscr, mapscr*)

int32_t combined_handle_t::layer() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
	{
		auto& rpos_handle = std::get<rpos_handle_t>(*this);
		return rpos_handle.layer;
	}
	return std::get<ffc_handle_t>(*this).layer();
}

int32_t combined_handle_t::get_screen() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
		return std::get<rpos_handle_t>(*this).screen;

	return std::get<ffc_handle_t>(*this).screen;
}
uint8_t combined_handle_t::sflag() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
		return std::get<rpos_handle_t>(*this).sflag();

	return 0; // nothing for ffc_handle_t
}
void combined_handle_t::set_sflag(uint8_t value) const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
		return std::get<rpos_handle_t>(*this).set_sflag(value);

	return; // nothing for ffc_handle_t
}

IMPL_COMBINED_HANDLE_GET(combo, const newcombo&)
IMPL_COMBINED_HANDLE_GET_DEF(data, int32_t, 0)
IMPL_COMBINED_HANDLE_SET(set_data, int32_t)
IMPL_COMBINED_HANDLE_CALL_1(modify_data, int32_t)
IMPL_COMBINED_HANDLE_CALL_0(increment_data)
IMPL_COMBINED_HANDLE_CALL_0(decrement_data)
IMPL_COMBINED_HANDLE_GET_DEF(cset, int32_t, 0)
IMPL_COMBINED_HANDLE_SET(set_cset, int32_t)
IMPL_COMBINED_HANDLE_GET(cflag, uint8_t)
IMPL_COMBINED_HANDLE_GET(ctype, uint8_t)

int32_t combined_handle_t::id() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
	{
		auto& rpos_handle = std::get<rpos_handle_t>(*this);
		return (int32_t)rpos_handle.rpos;
	}

	return std::get<ffc_handle_t>(*this).id;
}
int32_t combined_handle_t::local_id() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
	{
		auto& rpos_handle = std::get<rpos_handle_t>(*this);
		return (int32_t)rpos_handle.pos;
	}

	return std::get<ffc_handle_t>(*this).i;
}

IMPL_COMBINED_HANDLE_GET(info, cpos_info&)
IMPL_COMBINED_HANDLE_GET(xy, int_pair)
IMPL_COMBINED_HANDLE_GET(center_xy, int_pair)
