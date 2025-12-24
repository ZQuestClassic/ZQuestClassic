#include "base/zc_alleg.h"
#include "metadata/versionsig.h"

PACKFILE *pack_fopen_password(const char *filename, const char *mode, const char *password)
{
	packfile_password(password);
	PACKFILE *new_pf = pack_fopen(filename, mode);
	packfile_password("");
	return new_pf;
}

uint64_t file_size_ex_password(const char *filename, const char *password)
{
	packfile_password(password);
	uint64_t new_pf = file_size_ex(filename);
	packfile_password("");
	return new_pf;
}

bool alleg4_save_bitmap(BITMAP* source, int scale, const char* filename, AL_CONST RGB *pal)
{
	BITMAP* scaled = nullptr;
	if (scale != 1)
	{
		int w = source->w;
		int h = source->h;
		scaled = create_bitmap_ex(8, w*scale, h*scale);
		stretch_blit(source, scaled, 0, 0, w, h, 0, 0, w*scale, h*scale);
	}

	PALETTE default_pal;
	if (!pal)
		get_palette(default_pal);
	int result = save_bitmap(filename, scaled ? scaled : source, pal ? pal : default_pal);
	destroy_bitmap(scaled);
	return result == 0;
}

void alleg4_set_window_title(const char* title)
{
#if ZC_IS_PRERELEASE > 0
	char title2[200] = {0};
    strcpy(title2, title);
    strcat(title2, " [PRERELEASE]");
	set_window_title(title2);
#else
	set_window_title(title);
#endif
}
