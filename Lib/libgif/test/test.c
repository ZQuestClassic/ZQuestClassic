/* GIF loader/saver test program
 * By Paul Bartrum
 */

#include <stdio.h>
#include <allegro.h>
#include "../src/load_gif.h"	/* In your program should be #include <load_gif.h> */
#include "../src/save_gif.h"	/* In your program should be #include <save_gif.h> */

int main(int argc, char **argv)
{
	BITMAP *bmp;
	PALETTE pal;

	/* init allegro */
	allegro_init();
	install_keyboard();

	/* tell allegro of the gif routines */
	register_bitmap_file_type("GIF", load_gif, save_gif);

	/* if there are arguments, load the image */
	if(argc > 1)
	{
		bmp = load_bitmap(argv[1], pal);
		if(bmp == NULL) {
			printf("Couldn't load image %s\n", argv[1]);
			exit(0);
		}

		/* set the proper graphics mode and palette */
		if(bmp->w < 320 && bmp->h < 200)
			set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);
		else if(bmp->w < 640 && bmp->h < 480)
			set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0);
		else
			set_gfx_mode(GFX_AUTODETECT, 800, 600, 0, 0);

		set_palette(pal);

		/* blit image onto screen */
		blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);

		/* wait for keypress then free up bitmap and quit */
		readkey();
		destroy_bitmap(bmp);
		return 1;
	}

	printf("Loading image test.pcx\n");

	/* load test image */
	bmp = load_bitmap("test.pcx", pal);
	if(bmp == NULL) {
		printf("Couldn't load image test.pcx\n");
		exit(0);
	}

	printf("Saving test.pcx as a gif (test.gif)\n");

	/* save as a gif */
	save_bitmap("test.gif", bmp, pal);

	printf("Loading test.gif\n");

	/* load the gif */
	bmp = load_bitmap("test.gif", pal);
	if(bmp == NULL) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);		/* must be in text mode to use printf() */
		printf("Couldn't load test.gif\n");
		exit(0);
	}

	printf("Press a key to display...\n");

	/* wait for keypress */
	readkey();

	/* set the display and palette */
	set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);
	set_palette(pal);

	/* copy the bitmap just loaded onto the screen */
	blit(bmp, screen, 0, 0, 0, 0, bmp->w, bmp->h);

	/* output text "Press a key to continue" */
	textout(screen, font, "Press a",  220,  80, makecol(0, 0, 0));
	textout(screen, font, "key to",   220, 100, makecol(0, 0, 0));
	textout(screen, font, "continue", 220, 120, makecol(0, 0, 0));

	/* wait for keypress */
	readkey();

	/* shut everything down */
	destroy_bitmap(bmp);

	/* that's it! */
   return 0;
}