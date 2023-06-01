#ifndef _HANDLES_H_
#define _HANDLES_H_

#include <stdint.h>

enum class rpos_t : int32_t {
	NONE = -1,
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
	mapscr* screen;
	int32_t screen_index;
	// 0 = base screen, 1 = layer 1, etc. Up to 6.
	int32_t layer;
	rpos_t rpos;

	int32_t pos() const;

	int32_t data() const;
	void set_data(int32_t value) const;
	void increment_data() const;

	int32_t cset() const;
	void set_cset(int32_t value) const;
};

struct ffc_handle_t
{
	mapscr* screen;
	int32_t screen_index;
	int32_t i;
	ffcdata* ffc;

	int32_t data() const;
	void set_data(int32_t value) const;
	void increment_data() const;
};

#endif
