#include "base/handles.h"
#include "zc/maps.h"
#include "zc/zc_ffc.h"
#include "ffc.h"
#include <variant>

int32_t rpos_handle_t::pos() const
{
	return (int32_t)rpos % 176;
}

int32_t rpos_handle_t::data() const
{
	return screen->data[pos()];
}

void rpos_handle_t::set_data(int32_t value) const
{
	screen->data[pos()] = value;
}

void rpos_handle_t::increment_data() const
{
	screen->data[pos()] += 1;
}

int32_t rpos_handle_t::cset() const
{
	return screen->cset[pos()];
}

void rpos_handle_t::set_cset(int32_t value) const
{
	screen->cset[pos()] = value;
}

uint8_t rpos_handle_t::sflag() const
{
	return screen->sflag[pos()];
}

void rpos_handle_t::set_sflag(uint8_t value) const
{
	screen->sflag[pos()] = value;
}

int32_t ffc_handle_t::data() const
{
	return ffc->data;
}

void ffc_handle_t::set_data(int32_t value) const
{
	zc_ffc_set(*ffc, value);
}

void ffc_handle_t::set_cset(int32_t cset) const
{
	ffc->cset = cset;
}

void ffc_handle_t::increment_data() const
{
	zc_ffc_modify(*ffc, 1);
}

void ffc_handle_t::decrement_data() const
{
	zc_ffc_modify(*ffc, -1);
}

bool combined_handle_t::is_rpos() const
{
	return std::holds_alternative<rpos_handle_t>(*this);
}

bool combined_handle_t::is_ffc() const
{
	return !std::holds_alternative<rpos_handle_t>(*this);
}

int32_t combined_handle_t::data() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
	{
		auto& rpos_handle = std::get<rpos_handle_t>(*this);
		if (rpos_handle.rpos == rpos_t::None)
			return 0;
		return rpos_handle.data();
	}

	return std::get<ffc_handle_t>(*this).data();
}

int32_t combined_handle_t::id() const
{
	if (std::holds_alternative<rpos_handle_t>(*this))
	{
		auto& rpos_handle = std::get<rpos_handle_t>(*this);
		return (int32_t)rpos_handle.rpos;
	}

	return std::get<ffc_handle_t>(*this).id;
}
