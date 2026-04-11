#include "zc/scripting/types/bitmap.h"

#include "base/check.h"
#include "base/general.h"
#include "components/zasm/defines.h"
#include "core/zdefs.h"
#include "zc/scripting/script_object.h"

#include <cstdint>

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

void scripting_log_error_with_context(std::string text);

UserDataContainer<user_bitmap, MAX_USER_BITMAPS> user_bitmaps = {script_object_type::bitmap, "bitmap"};

user_bitmap* checkBitmap(int32_t ref)
{
	return user_bitmaps.check(ref);
}

user_bitmap* checkBitmap(int32_t ref, bool req_valid, bool skipError)
{
	switch (ref - 10)
	{
		case rtSCREEN:
		case rtBMP0:
		case rtBMP1:
		case rtBMP2:
		case rtBMP3:
		case rtBMP4:
		case rtBMP5:
		case rtBMP6:
			zprint2("Internal error: 'checkBitmap()' recieved ref pointing to system bitmap!\n");
			zprint2("Please report this as a bug!\n");

			if(skipError) return NULL;

			scripting_log_error_with_context("Tried to reference a non-existent bitmap with UID = {}", ref);
			return NULL;

		default:
		{
			user_bitmap* b = user_bitmaps.check(ref, skipError);
			if (req_valid && (!b || !b->u_bmp))
			{
				if (skipError) return NULL;

				scripting_log_error_with_context("Tried to reference an invalid user bitmap with UID = {}.", ref);
				Z_scripterrlog("Did you forget to create the bitmap with `new bitmap()` or `->Create()`?.\n");
				return NULL;
			}
			return b;
		}
	}
}

int32_t bitmap_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case BITMAPHEIGHT:
		{
			if (auto bmp = user_bitmaps.check(GET_REF(bitmapref)); bmp && bmp->u_bmp)
			{
				ret = bmp->height * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case BITMAPWIDTH:
		{
			if (auto bmp = user_bitmaps.check(GET_REF(bitmapref)); bmp && bmp->u_bmp)
			{
				ret = bmp->width * 10000;
			}
			else
			{
				ret = -10000;
			}
			break;
		}
		case CREATEBITMAP:
		{
			ret=FFCore.do_create_bitmap();
			break;
		}

		default:
			NOTREACHED();
	}

	return ret;
}

void bitmap_set_register([[maybe_unused]] int32_t reg, [[maybe_unused]] int32_t value)
{
	NOTREACHED();
}
