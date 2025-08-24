#ifndef BASE_HANDLES_H_
#define BASE_HANDLES_H_

#include "base/combo.h"
#include "base/compiler.h"
#include "base/cpos_info.h"
#include "base/mapscr.h"
#include <stdint.h>
#include <variant>
#include <utility>

class ffcdata;

struct screen_handle_t
{
	// The 0th-layer screen associated with this screen.
	mapscr* base_scr;
	// Null if scr was not valid.
	mapscr* scr;
	int32_t screen;
	// 0 = base screen, 1 = layer 1, etc. Up to 6.
	int32_t layer;
};

// References to a set of 7 screens (one for each layer).
typedef std::array<screen_handle_t, 7> screen_handles_t;

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
	mapscr* get_mapscr() const;

	newcombo& combo() const;
	cpos_info& info() const;
	std::pair<int32_t, int32_t> xy() const;
	std::pair<int32_t, int32_t> center_xy() const;

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
	mapscr* get_mapscr() const;
	int32_t layer() const;

	const newcombo& combo() const;
	cpos_info& info() const;
	std::pair<int32_t, int32_t> xy() const;
	std::pair<int32_t, int32_t> center_xy() const;

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
	rpos_handle_t const& get_rpos() const;
	ffc_handle_t const& get_ffc() const;
	
	operator bool() const;
	mapscr* base_scr() const;
	mapscr* get_mapscr() const;
	int32_t layer() const;
	int32_t get_screen() const;
	int id() const;
	int local_id() const;
	
	const newcombo& combo() const;
	cpos_info& info() const;
	std::pair<int32_t, int32_t> xy() const;
	std::pair<int32_t, int32_t> center_xy() const;
	
	int data() const;
	void set_data(int32_t value) const;
	void modify_data(int32_t delta) const;
	void increment_data() const;
	void decrement_data() const;
	
	int32_t cset() const;
	void set_cset(int32_t cset) const;
	
	uint8_t sflag() const;
	void set_sflag(uint8_t value) const;
	
	uint8_t cflag() const;
	uint8_t ctype() const;
};

#endif
