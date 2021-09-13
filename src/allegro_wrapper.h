// Wraps <allegro.h> with some temporary redefines to avert conflicts.

#ifndef ALLEGRO_H

// Grab GLIBC constants used below.
#  ifdef ALLEGRO_UNIX
#    include <features.h>
#  endif

#define ALLEGRO_NO_FIX_ALIASES
#  include <allegro.h>

#endif
