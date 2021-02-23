GIF Loader/Saver Readme
by Paul Bartrum.
Small modifications/adaptation by Grzegorz Adam Hankiewicz


DESCRIPTION/FEATURES

Two routines for loading and saving GIF images.
They use exactly the same format as the built-in image loaders and
savers.

* Loads GIF87a and GIF89a images.
* Supports loading interlaced images.
* Saves GIF87a images.


REQUIREMENTS

DJGPP and Allegro 3.0 or later.


IMPORTANT

I've done a little research on finding out the Unisys and IBM patent
restrictions on the LZW compression format (which is used in both loading
and saving) and it seems the FSF (www.fsf.org) analysis of the wording of the
patent implies:

A license is only required if you write a for-profit program that PRODUCES
GIF images.

There is no license needed to write a program that DISPLAYS a GIF image.


BUGS/NON-FEATURES


* The save_gif() routine uses 2Mb of memory for a massive speed increase
  (about 10x faster).  Of course the memory is freed when it finishes saving,
  so it shouldn't be a problem unless your program reserves heaps of memory
  then calls save_gif().

* The save_gif() routine only saves GIF 87a images because there's no point
  in saving 89a images - it doesn't use any of the new features.


INSTALLATION

Copy the gif.zip file one dir up of Allegro's. Then unzip preserving
directory structure (-d switch with Pkunzip). This will create the
allegro/addons/gif/* files.

Now enter allegro/addons/gif and type 'make' to compile the library. Once
this is done, you will end up with the library inside the lib directory, and
a test example program inside the test directory.

You can now type 'make install'. This will copy the library and header files
to your DJGPP's subdirectories. Once you have done this, you can easily use
the GIF addon from your program adding these lines _after_ Allegro's
include:

#include <stdio.h>
#include <allegro.h>
#include <load_gif.h> /* Include gif loading routine */
#include <save_gif.h> /* Include gif saving routine */

Somewhere inside your program and _after_ allegro_init() you will have to
register the .gif extension if you want Allegro's load/save_bitmap generic
routines to use the gif functions. Example:

	/* init allegro */
	allegro_init();
	install_keyboard();

	/* tell allegro of the gif routines */
	register_bitmap_file_type("GIF", load_gif, save_gif);

Then, when you link your program exe file, add -lgif before -lalleg.
Example:

gcc -o foo.exe src/foo.c src/bar.c -lgif -lalleg


To uninstall the library go to the allegro/addons/gif dir and type 'make
uninstall'. This will remove the installed *.a *.h files. Now you can safely
delete the addons/gif directory.

CONTACT

If you need help, find a bug, or have some advice to offer, contact me at:
    pbartrum@geocities.com
My webpage is http://www.geocities.com/SiliconValley/Bay/8849/


COPYRIGHT

I'll trust you not to rip me or my cat off.
