#ifndef BASE_HANDLES_H_
#define BASE_HANDLES_H_

#include "base/combo.h"
#include "base/compiler.h"
#include "base/cpos_info.h"
#include "base/mapscr.h"
#include <stdint.h>
#include <variant>

class ffcdata;

struct screen_handle_t
{
	mapscr* base_scr;
	mapscr* scr;
	int32_t screen;
	// 0 = base screen, 1 = layer 1, etc. Up to 6.
	int32_t layer;
};

struct rpos_handle_t
{
	mapscr* scr;
	int32_t screen;
	// 0 = base screen, 1 = layer 1, etc. Up to 6.
	int32_t layer;
	rpos_t rpos = rpos_t::None;
	// rpos % 176
	int32_t pos;

	operator bool () const
	{
		return scr;
	}

	mapscr* base_scr() const;

	newcombo& combo() const;
	cpos_info& info() const;

	ZC_FORCE_INLINE int32_t data() const
	{
		return scr->data[pos];
	}

	void set_data(int32_t value) const;
	void modify_data(int32_t delta) const;
	void increment_data() const;
	void decrement_data() const;

	int32_t cset() const;
	void set_cset(int32_t value) const;

	uint8_t sflag() const;
	void set_sflag(uint8_t value) const;

	uint8_t cflag() const;
	uint8_t ctype() const;
};

struct ffc_handle_t
{
	mapscr* scr;
	uint8_t screen;
	ffc_id_t id;
	// The index into mapscr.ffcs
	uint8_t i;
	ffcdata* ffc;

	operator bool () const
	{
		return scr;
	}

	const newcombo& combo() const;
	cpos_info& info() const;

	int32_t data() const;
	void set_data(int32_t value) const;
	void modify_data(int32_t delta) const;
	void increment_data() const;
	void decrement_data() const;

	int32_t cset() const;
	void set_cset(int32_t cset) const;

	uint8_t cflag() const;
	uint8_t ctype() const;
};

// TODO: this may be a tad overengineered for its current usecases. Consider replacing
// with something simpler if later no use cases really take advantage of it.
struct combined_handle_t : std::variant<rpos_handle_t, ffc_handle_t>
{
	using std::variant<rpos_handle_t, ffc_handle_t>::variant;

	bool is_rpos() const;
	bool is_ffc() const;
	mapscr* base_scr() const;
	const newcombo& combo() const;
	int data() const;
	int id() const;
};

#endif
