// Wraps <allegro.h> with some temporary redefines to avert conflicts.

#ifndef ALLEGRO_H

// Grab GLIBC constants used below.
#  ifdef ALLEGRO_UNIX
#    include <features.h>
#  endif

// As far as I can tell, this glibc version introduces fadd etc. that
// conflict with some allegro 4 functions that we don't use.
#  define ZCALLEGRO_RENAME_FIXEDMATH (__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 28)

#  if ZCALLEGRO_RENAME_FIXEDMATH
#    define fadd alfadd
#    define fsub alfsub
#    define fmul alfmul
#    define fdiv alfdiv
#  endif

#  include <allegro.h>

#  if ZCALLEGRO_RENAME_FIXEDMATH
#    undef fadd
#    undef fsub
#    undef fmul
#    undef fdiv
#  endif

#endif
