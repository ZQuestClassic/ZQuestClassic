#cmakedefine ALLEGRO_LEGACY_BCC32
#cmakedefine ALLEGRO_LEGACY_BEOS
#cmakedefine ALLEGRO_LEGACY_DJGPP
#cmakedefine ALLEGRO_LEGACY_DMC
#cmakedefine ALLEGRO_LEGACY_HAIKU
#cmakedefine ALLEGRO_LEGACY_MACOSX
#cmakedefine ALLEGRO_LEGACY_MINGW32
#cmakedefine ALLEGRO_LEGACY_MPW
#cmakedefine ALLEGRO_LEGACY_MSVC
#cmakedefine ALLEGRO_LEGACY_PSP
#cmakedefine ALLEGRO_LEGACY_QNX
#cmakedefine ALLEGRO_LEGACY_UNIX
#cmakedefine ALLEGRO_LEGACY_WATCOM
#cmakedefine WANT_ALLEGRO_5

#ifdef WANT_ALLEGRO_5
    #define ALLEGRO_LEGACY_A5
#endif

/* These are always defined now. */
#define ALLEGRO_LEGACY_NO_ASM
#define ALLEGRO_LEGACY_USE_C
