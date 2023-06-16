#include "base/handles.h"
#include "zc/maps.h"
#include "ffc.h"

int32_t rpos_handle_t::pos() const
{
	int32_t pos = static_cast<int32_t>(rpos)%176;
	return pos;
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

int32_t ffc_handle_t::data() const
{
	return ffc->getData();
}

void ffc_handle_t::set_data(int32_t value) const
{
	ffc->setData(value);
}

void ffc_handle_t::set_cset(int32_t cset) const
{
	ffc->cset = cset;
}

void ffc_handle_t::increment_data() const
{
	ffc->setData(ffc->getData());
}
