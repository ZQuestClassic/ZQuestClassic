#pragma once

#include <user_object.h>
#include "allegro5/display.h"
#include "allegro/gfx.h"
#include "base/ints.h"
#include "base/zapp.h"

#include <cstdint>

//Old, 2.50 bitmap IDs
enum { rtSCREEN = -1, rtBMP0 = 0, rtBMP1,
	rtBMP2, rtBMP3, rtBMP4, rtBMP5, rtBMP6, firstUserGeneratedBitmap };

#define MAX_USER_BITMAPS 256

//User-generated / Script-Generated bitmap object
#define UBMPFLAG_FREEING               0x01
#define UBMPFLAG_CAN_DELETE            0x02

struct user_bitmap : public script_object_base
{
	BITMAP* u_bmp;
	int32_t width;
	int32_t height;
	byte flags;

	user_bitmap() = default;
	user_bitmap(const user_bitmap&) = delete;

	~user_bitmap()
	{
		if (!is_exiting())
			destroy_bitmap(u_bmp);
	}

	void destroy()
	{
		destroy_bitmap(u_bmp);
		width = 0;
		height = 0;
		flags = 0;
		u_bmp = NULL;
	}

	void free_obj()
	{
		flags |= UBMPFLAG_FREEING;
	}

	void mark_can_del()
	{
		flags |= UBMPFLAG_CAN_DELETE;
	}

	bool is_freeing()
	{
		return flags & UBMPFLAG_FREEING;
	}

	bool can_del()
	{
		return flags & UBMPFLAG_CAN_DELETE;
	}
};

//Holds all of the user-generated / script-generated bitmaps and their information.
//User bitmap lowest viable ID is 'rtBMP6+1' (firstUserGeneratedBitmap)
struct script_bitmaps
{
	void update();
	user_bitmap& get(int32_t id);
};

user_bitmap* checkBitmap(int32_t ref);
// The most a script-created bitmap can be in either direction. Keeps the byte count within an
// int, which create_bitmap_ex needs.
#define MAX_SCRIPT_BITMAP_SIZE 32767

// True if a script can make a bitmap this size. A height of 0 is allowed, but must be given to
// create_bitmap_ex as 1 (see create_script_bitmap).
inline bool is_valid_bitmap_size(int32_t w, int32_t h)
{
	return w >= 0 && h >= 0 && w <= MAX_SCRIPT_BITMAP_SIZE && h <= MAX_SCRIPT_BITMAP_SIZE;
}
// create_bitmap_ex for a size that is_valid_bitmap_size allows.
inline BITMAP* create_script_bitmap(int32_t w, int32_t h)
{
	return create_bitmap_ex(8, w, std::max(h, 1));
}
user_bitmap* checkBitmap(int32_t ref, bool req_valid, bool skipError = false);

int32_t bitmap_get_register(int32_t reg);
void bitmap_set_register(int32_t reg, int32_t value);
