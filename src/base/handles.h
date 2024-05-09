#ifndef _HANDLES_H_
#define _HANDLES_H_

#include "base/combo.h"
#include "base/cpos_info.h"
#include <stdint.h>
#include <variant>

enum class rpos_t : int32_t {
	None = -1,
};

struct mapscr;
class ffcdata;

struct screen_handle_t
{
	mapscr* base_screen;
	mapscr* screen;
	int32_t map;
	int32_t index;
	// 0 = base screen, 1 = layer 1, etc. Up to 6.
	int32_t layer;
};

struct rpos_handle_t
{
	// rpos_handle_t(mapscr* screen, int32_t screen_index, int32_t layer, rpos_t rpos): screen(screen), screen_index(screen_index), layer(layer), rpos(rpos)
	// {
	// 	pos = (int)rpos % 176;
	// }

	// rpos_handle_t(mapscr* screen, int32_t screen_index, int32_t layer, rpos_t rpos): screen(screen), screen_index(screen_index), layer(layer), rpos(rpos), pos(pos)
	// {
	// }

	mapscr* screen;
	int32_t screen_index;
	// 0 = base screen, 1 = layer 1, etc. Up to 6.
	int32_t layer;
	rpos_t rpos = rpos_t::None;
	// rpos % 176
	int32_t pos;

	newcombo& combo() const;
	cpos_info& info() const;

	int32_t data() const;
	void set_data(int32_t value) const;
	void increment_data() const;

	int32_t cset() const;
	void set_cset(int32_t value) const;

	uint8_t sflag() const;
	void set_sflag(uint8_t value) const;

	uint8_t cflag() const;
};

struct ffc_handle_t
{
	mapscr* screen;
	uint8_t screen_index;
	// A unique identifier for this ffc in this region.
	// Roughly = (region screen index offset) * 128 + i.
	// For non-regions, or for the top-left screen in a region, this is equal to i.
	uint16_t id;
	// The index into mapscr.ffcs
	uint8_t i;
	ffcdata* ffc;

	const newcombo& combo() const;
	cpos_info& info() const;

	int32_t data() const;
	void set_data(int32_t value) const;
	void increment_data() const;
	void decrement_data() const;

	void set_cset(int32_t cset) const;
};

// TODO: this may be a tad overengineered for its current usecases. Consider replacing
// with something simpler if later no use cases really take advantage of it.
struct combined_handle_t : std::variant<rpos_handle_t, ffc_handle_t>
{
	using std::variant<rpos_handle_t, ffc_handle_t>::variant;

	bool is_rpos() const;
	bool is_ffc() const;
	int data() const;
	int id() const;
};

#endif
