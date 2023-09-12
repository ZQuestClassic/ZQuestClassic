#ifndef _GENERAL_H_
#define _GENERAL_H_

#include "base/ints.h"

int32_t get_bit(byte const* bitstr,int32_t bit);
void set_bit(byte *bitstr,int32_t bit,byte val);
bool toggle_bit(byte *bitstr,int32_t bit);
int32_t get_bitl(int32_t bitstr,int32_t bit);
void set_bitl(int32_t bitstr,int32_t bit,byte val);

#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))

//Script Counts
#define NUMSCRIPTFFC               512
#define NUMSCRIPTFFCOLD            256
#define NUMSCRIPTITEM              256
#define NUMSCRIPTGUYS              256
#define NUMSCRIPTWEAPONS           256
#define NUMSCRIPTGLOBAL            8
#define NUMSCRIPTGLOBAL255OLD      7
#define NUMSCRIPTGLOBAL253         4
#define NUMSCRIPTGLOBALOLD         3
#define NUMSCRIPTHEROOLD           3
#define NUMSCRIPTPLAYER            5
#define NUMSCRIPTSCREEN            256
#define NUMSCRIPTSDMAP             256
#define NUMSCRIPTSITEMSPRITE       256
#define NUMSCRIPTSCOMBODATA        512
#define NUMSCRIPTSGENERIC          512
#define NUMSCRIPTSSUBSCREEN        256

//Script-related
#define INITIAL_A                  2
#define INITIAL_D                  8
#define FFSCRIPT_MISC              32
#define BITS_SP                    10
#define MASK_SP                    ((1<<BITS_SP)-1)
#define MAX_SCRIPT_REGISTERS       (1<<BITS_SP)
#define MAX_SCRIPT_REGISTERS_250   256
#define MAX_PC                     dword(-1)

//Sizes
#define MAX_SIGNED_32              (2147483647)
#define MIN_SIGNED_32              (-2147483647-1)
#define MAX_DWORD                  dword(-1)
#define MIN_DWORD                  0

#define SINGLE_TILE_SIZE           128
#define TILES_PER_ROW              20
#define TILE_ROWS_PER_PAGE         13
#define TILES_PER_PAGE             (TILES_PER_ROW*TILE_ROWS_PER_PAGE)
#define MAXTILEROWS                (TILE_ROWS_PER_PAGE*TILE_PAGES)   //Last row that we can show when trying to grab tiles from  .QST file. -Z

#define TILEROW(tile)              ((tile)/TILES_PER_ROW)
#define TILECOL(tile)              ((tile)%TILES_PER_ROW)
#define TILEPAGE(tile)             ((tile)/TILES_PER_PAGE)

#define TILE_PAGES                 825
#define TILE_PAGES_ZC250           252 //2.50.x

#define OLDMAXTILES                (TILES_PER_PAGE*6)                // 1560 tiles
#define NEWMAXTILES                (TILES_PER_PAGE*TILE_PAGES)       // 214500 tiles
#define ZC250MAXTILES              (TILES_PER_PAGE*TILE_PAGES_ZC250) // 32760 tiles

#define NEWTILE_SIZE2              (NEWMAXTILES*SINGLE_TILE_SIZE)    // 27456000 bytes 
#define ZC250TILESIZE              (ZC250MAXTILES*SINGLE_TILE_SIZE)  // 4193280 bytes (new packed format, 6 pages)
#define OLDTILE_SIZE2              (OLDMAXTILES*SINGLE_TILE_SIZE)    // 199680 bytes (new packed format, 6 pages)

#define TILEBUF_SIZE               (320*480)                         // 153600 bytes (old unpacked format)

#define COMBOS_PER_ROW             20

#define WAV_COUNT                  256

#define MAXFFCS                    128
#define MAX_FFCID                  (region_scr_count * MAXFFCS - 1)
#define MAXSCREENS                 128
#define MAXCUSTOMMIDIS192b177      32   // uses bit string for midi flags, so 32 bytes
#define MAXCUSTOMMIDIS             252  // uses bit string for midi flags, so 32 bytes
#define MAXCUSTOMMIDIS_ZQ          (4+MAXCUSTOMMIDIS)
#define MIDIFLAGS_SIZE             ((MAXCUSTOMMIDIS+7)>>3)
#define MAXCUSTOMTUNES             252
//Midi offsets
//The offset from dmap/mapscr-> midi/screen_midi to currmidi
#define MIDIOFFSET_DMAP            (ZC_MIDI_COUNT-4)
#define MIDIOFFSET_MAPSCR          (ZC_MIDI_COUNT-4)
//The offset from currmidi to ZScript MIDI values
#define MIDIOFFSET_ZSCRIPT         (ZC_MIDI_COUNT-1)
//Use together as `(MIDIOFFSET_DMAP-MIDIOFFSET_ZSCRIPT)` to go from `dmap` directly to `zscript`


#define MAXMUSIC                   256 // uses bit string for music flags, so 32 bytes
#define MUSICFLAGS_SIZE            (MAXMUSIC>>3)

#define MAXMAPS2                   255 // 4 times the old number
#define MAPSCRSNORMAL              128
#define MAPSCRS192b136             132
#define MAPSCRS                    136
#define TEMPLATES                  8
#define TEMPLATE                   131
#define TEMPLATE2                  132

#define MAXQTS                     256
#define MAXMSGS                    65535
#define MAXDOORCOMBOSETS           256
#define MAXDMAPS                   512 //this and
#define MAXLEVELS                  512 //this should be the same number (was 32)
#define OLDMAXLEVELS               256
#define OLDMAXDMAPS                256
#define MAXITEMS                   256
#define MAXWPNS                    256
#define OLDBETAMAXGUYS             256 //max 2.5 guys through beta 20
#define MAXGUYS                    512
#define MAXITEMDROPSETS            256
#define COMBOS_PER_PAGE            256
#define COMBO_PAGES                255
#define MAXCOMBOS                  (COMBO_PAGES*COMBOS_PER_PAGE)
#define BOUND_COMBO(c)             vbound(c, 0, MAXCOMBOS)
#define MAXSUBSCREENITEMS          256
#define MAXCUSTOMSUBSCREENS        128
#define MAXNPCS                    512

#define MAXFAVORITECOMMANDS        64
#define MAXFAVORITECOMBOS          300
#define MAXFAVORITECOMBOALIASES    MAXFAVORITECOMBOS

#define FAVORITECOMBO_PER_ROW      30

#define OLDMAXCOMBOALIASES         256
#define MAX250COMBOALIASES         2048
#define MAXCOMBOALIASES            8192
#define MAXCOMBOPOOLS              8192

#define PALNAMESIZE                17

#define NUM_SHOPS                  256
#define NUM_INFOS                  256
#define NUM_PAL_CYCLES             256
#define NUM_WARP_RINGS             9

#endif

