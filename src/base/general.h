#ifndef _GENERAL_H_
#define _GENERAL_H_

#include "base/ints.h"
#include "base/zfix.h"

int32_t get_bit(byte const* bitstr,int32_t bit);
void set_bit(byte *bitstr,int32_t bit,bool val);
bool toggle_bit(byte *bitstr,int32_t bit);
int32_t get_bitl(int32_t bitstr,int32_t bit);
void set_bitl(int32_t bitstr,int32_t bit,bool val);

template <class T>
static constexpr inline T sign(T a)
{
    return T(a < 0 ? -1: 1);
}
template <class T>
static constexpr inline void zc_swap(T &a,T &b)
{
    T c = a;
    a = b;
    b = c;
}
int vbound(int val, int low, int high);
double vbound(double val, double low, double high);
zfix vbound(zfix val, zfix low, zfix high);

#define zc_max(a,b) (((a) < (b)) ? (b) : (a))
#define zc_min(a,b) (((a) < (b)) ? (a) : (b))
#define SETFLAG(v, fl, b)	if(b) v |= (fl); else v &= ~(fl)

int wrap(int x,int low,int high);

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

#define MAXMAPS                    255 // 4 times the old number
#define MAPSCRSNORMAL              128
#define MAPSCRS192b136             132
#define MAPSCRS                    136
#define MAXSCRSNORMAL              (MAXMAPS*MAPSCRSNORMAL)
#define MAXSCRS                    (MAXMAPS*MAPSCRS)

#define TEMPLATES                  8
#define TEMPLATE                   131
#define TEMPLATE2                  132

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
#define MAXFAVORITECOMBOS          (FAVORITECOMBO_PER_PAGE * FAVORITECOMBO_PAGES)

#define FAVORITECOMBO_PER_ROW      20
#define FAVORITECOMBO_PER_COLUMN   7
#define FAVORITECOMBO_PER_PAGE     140
#define FAVORITECOMBO_PAGES        9

#define OLDMAXCOMBOALIASES         256
#define MAX250COMBOALIASES         2048
#define MAXCOMBOALIASES            8192
#define MAXCOMBOPOOLS              8192
#define MAXAUTOCOMBOS              8192

#define PALNAMESIZE                17

#define NUM_SHOPS                  256
#define NUM_INFOS                  256
#define NUM_PAL_CYCLES             256
#define NUM_WARP_RINGS             9

enum controls //Args for 'getInput()'
{
	//control_state indeces
	btnUp, btnDown, btnLeft, btnRight, btnA, btnB, btnS, btnL, btnR, btnP, btnEx1, btnEx2, btnEx3, btnEx4, btnAxisUp, btnAxisDown, btnAxisLeft, btnAxisRight,
	//Other controls
	btnM, btnF12, btnF11, btnF5, btnQ, btnI,
	btnLast,
};

#define INT_BTN_A       0x01
#define INT_BTN_B       0x02
#define INT_BTN_L       0x04
#define INT_BTN_R       0x08
#define INT_BTN_EX1     0x10
#define INT_BTN_EX2     0x20
#define INT_BTN_EX3     0x40
#define INT_BTN_EX4     0x80
#define INT_BTN_X       INT_BTN_EX1
#define INT_BTN_Y       INT_BTN_EX2

enum // used for gamedata ITEMS
{
	// 0
	itype_sword, itype_brang, itype_arrow, itype_candle, itype_whistle,
	itype_bait, itype_letter, itype_potion, itype_wand, itype_ring,
	itype_wallet, itype_amulet, itype_shield, itype_bow, itype_raft,
	itype_ladder, itype_book, itype_magickey, itype_bracelet, itype_flippers,
	// 20
	itype_boots, itype_hookshot, itype_lens, itype_hammer, itype_divinefire,
	itype_divineescape, itype_divineprotection, itype_bomb, itype_sbomb, itype_clock,
	itype_key, itype_magiccontainer, itype_triforcepiece, itype_map, itype_compass,
	itype_bosskey, itype_quiver, itype_lkey, itype_cbyrna, itype_rupee,
	// 40
	itype_arrowammo, itype_fairy, itype_magic, itype_heart, itype_heartcontainer,
	itype_heartpiece, itype_killem, itype_bombammo, itype_bombbag, itype_rocs,
	itype_hoverboots, itype_spinscroll,itype_crossscroll, itype_quakescroll,itype_whispring,
	itype_chargering, itype_perilscroll, itype_wealthmedal,itype_heartring,itype_magicring,
	// 60
	itype_spinscroll2, itype_quakescroll2, itype_agony, itype_stompboots, itype_whimsicalring,
	itype_perilring, itype_misc,
	// 67
	itype_custom1, itype_custom2, itype_custom3, itype_custom4, itype_custom5,
	itype_custom6, itype_custom7, itype_custom8, itype_custom9, itype_custom10,
	itype_custom11, itype_custom12, itype_custom13, itype_custom14, itype_custom15,
	itype_custom16, itype_custom17, itype_custom18, itype_custom19, itype_custom20,
	// 87
	itype_bowandarrow, itype_letterpotion,
	itype_last, 
	itype_script1 = 256, //Scripted Weapons
	itype_script2, itype_script3, itype_script4, itype_script5, itype_script6, itype_script7, itype_script8, itype_script9, itype_script10,
	itype_icerod, itype_atkring, itype_lantern, itype_pearl, itype_bottle, itype_bottlefill, itype_bugnet,
	itype_mirror, itype_switchhook, itype_itmbundle, itype_progressive_itm, itype_note, itype_refill,
	itype_liftglove,
	/*
	itype_templast,
	itype_ether, itype_bombos, itype_quake, 
	itype_powder,
	itype_trowel,
	itype_instrument,
	itype_sword180,
	itype_sword_gb,
	itype_firerod,
	itype_scripted_001 = 400, 
	itype_scripted_002,
	itype_scripted_003,
	itype_scripted_004,
	itype_scripted_005,
	itype_scripted_006,
	itype_scripted_007,
	itype_scripted_008,
	itype_scripted_009,
	itype_scripted_010,
	*/

	
	itype_maxusable,
	itype_max=512
};

enum
{
	crNONE = -1,
	crLIFE, crMONEY, crBOMBS, crARROWS, crMAGIC,
	crKEYS, crSBOMBS, crCUSTOM1, crCUSTOM2, crCUSTOM3,
	crCUSTOM4, crCUSTOM5, crCUSTOM6, crCUSTOM7, crCUSTOM8,
	crCUSTOM9, crCUSTOM10, crCUSTOM11, crCUSTOM12, crCUSTOM13,
	crCUSTOM14, crCUSTOM15, crCUSTOM16, crCUSTOM17, crCUSTOM18,
	crCUSTOM19, crCUSTOM20, crCUSTOM21, crCUSTOM22, crCUSTOM23,
	crCUSTOM24, crCUSTOM25, crCUSTOM26, crCUSTOM27, crCUSTOM28,
	crCUSTOM29, crCUSTOM30, crCUSTOM31, crCUSTOM32, crCUSTOM33,
	crCUSTOM34, crCUSTOM35, crCUSTOM36, crCUSTOM37, crCUSTOM38,
	crCUSTOM39, crCUSTOM40, crCUSTOM41, crCUSTOM42, crCUSTOM43,
	crCUSTOM44, crCUSTOM45, crCUSTOM46, crCUSTOM47, crCUSTOM48,
	crCUSTOM49, crCUSTOM50, crCUSTOM51, crCUSTOM52, crCUSTOM53,
	crCUSTOM54, crCUSTOM55, crCUSTOM56, crCUSTOM57, crCUSTOM58,
	crCUSTOM59, crCUSTOM60, crCUSTOM61, crCUSTOM62, crCUSTOM63,
	crCUSTOM64, crCUSTOM65, crCUSTOM66, crCUSTOM67, crCUSTOM68,
	crCUSTOM69, crCUSTOM70, crCUSTOM71, crCUSTOM72, crCUSTOM73,
	crCUSTOM74, crCUSTOM75, crCUSTOM76, crCUSTOM77, crCUSTOM78,
	crCUSTOM79, crCUSTOM80, crCUSTOM81, crCUSTOM82, crCUSTOM83,
	crCUSTOM84, crCUSTOM85, crCUSTOM86, crCUSTOM87, crCUSTOM88,
	crCUSTOM89, crCUSTOM90, crCUSTOM91, crCUSTOM92, crCUSTOM93,
	crCUSTOM94, crCUSTOM95, crCUSTOM96, crCUSTOM97, crCUSTOM98,
	crCUSTOM99, crCUSTOM100, MAX_COUNTERS
};

enum generic_ind //game->generic[]
{
	genHCP, genMDRAINRATE, genCANSLASH, genWLEVEL,
	genHCP_PER_HC, genCONTHP, genCONTHP_IS_PERC, genHP_PER_HEART,
	genMP_PER_BLOCK, genHERO_DMG_MULT, genENE_DMG_MULT,
	genDITH_TYPE, genDITH_ARG, genDITH_PERC, genLIGHT_RAD,genTDARK_PERC,genDARK_COL,
	genWATER_GRAV, genSIDESWIM_UP, genSIDESWIM_SIDE, genSIDESWIM_DOWN, genSIDESWIM_JUMP,
	genBUNNY_LTM, genSWITCHSTYLE, genSPRITEFLICKERSPEED, genSPRITEFLICKERCOLOR,
	genSPRITEFLICKERTRANSP, genLAST,
	genMAX = 256
};

enum class ScriptType {
	None,
	Global,
	FFC,
	Screen,
	Player,
	Item,
	Lwpn,
	NPC,
	Subscreen,
	Ewpn,
	DMap,
	ItemSprite,
	ScriptedActiveSubscreen,
	ScriptedPassiveSubscreen,
	Combo,
	OnMap,
	Generic,
	GenericFrozen,
	EngineSubscreen,
	First = Global,
	Last = EngineSubscreen,
};

#endif

