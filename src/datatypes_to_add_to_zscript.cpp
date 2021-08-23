	static const int TYPE_DROPSET = 26;
	static const int TYPE_PONDS = 27;
	static const int TYPE_WARPRING = 28;
	static const int TYPE_DOORSET = 29;
	static const int TYPE_ZUICOLOURS = 30;
	static const int TYPE_RGBDATA = 31;
	static const int TYPE_PALETTE = 32;
	static const int TYPE_TUNES = 33;
	static const int TYPE_PALCYCLE = 34;
	static const int TYPE_GAMEDATA = 35;
	static const int TYPE_CHEATS = 36;  

	REFDROPS
	REFPONDS
	REFWARPRINGS
	REFDOORS
	REFUICOLOURS
	REFRGB
	REFPALETTE
	REFTUNES
	REFPALCYCLE
	REFGAMEDATA
	REFCHEATS
	
	
	case  TYPE_DROPSET: return "dropdata->";
	case  TYPE_PONDS: return "ponddata->";
	case  TYPE_WARPRING: return "warpring->";
	case  TYPE_DOORSET: return "doorset->";
	case  TYPE_ZUICOLOURS: return "misccolors->";
	case  TYPE_RGBDATA: return "rgbdata->";
	case  TYPE_PALETTE: return "palette->";
	case  TYPE_TUNES: return "musictrack->";
	case  TYPE_PALCYCLE: return "palcycle->";
	case  TYPE_GAMEDATA: return "gamedata->";
	case  TYPE_CHEATS: return "cheats->";
	
	{ "REFDROPS",         REFDROPS,            0,             0 },
	{ "REFPONDS",         REFPONDS,            0,             0 },
	{ "REFWARPRINGS",         REFWARPRINGS,            0,             0 },
	{ "REFDOORS",         REFDOORS,            0,             0 },
	{ "REFUICOLOURS",         REFUICOLOURS,            0,             0 },
	{ "REFRGB",         REFRGB,            0,             0 },
	{ "REFPALETTE",         REFPALETTE,            0,             0 },
	{ "REFTUNES",         REFTUNES,            0,             0 },
	{ "REFPALCYCLE",         REFPALCYCLE,            0,             0 },
	{ "REFGAMEDATA",         REFGAMEDATA,            0,             0 },
	{ "REFCHEATS",         REFCHEATS,            0,             0 },
	
	
	
	
	dword dropsetref, pondref, warpringref, doorsref, 
	zcoloursref, rgbref, paletteref, palcycleref, 
	tunesref;
	dword gamedataref, cheatsref; 
//shop names

//item_drop_sets[MAXITEMDROPSETS];
struct item_drop_object
{
	char name[64];
	word item[10];
	word chance[11]; //0=none
};

struct pondtype
{
	byte olditem[3];
	byte newitem[3];
	byte fairytile;
	byte aframes;
	byte aspeed;
	word msg[15];
	byte foo[32];
	//72
};


struct miscQdata
{
	shoptype shop[256];
	//160 (160=10*16)
	infotype info[256];
	//384 (224=14*16)
	warpring warp[9];
	//528 (144=18*8)
	palcycle cycles[256][3];
	//2832 (2304=3*256*3)
	//2850 (18=2*2)
	byte     triforce[8];                                     // positions of triforce pieces on subscreen
	//2858 (8)
	zcolors  colors;
	//3154 (296)
	word     icons[4];
	//3162 (8=2*4)
	//pondtype pond[16];
	//4314 (1152=72*16)
	word endstring;
	//  byte dummy;  // left over from a word
	//word expansion[98];
	//4512
};

struct warpring
{
	char name[32];
	word dmap[9];
	byte scr[9];
	byte size;
	//18
};


struct ZCHEATS
{
	dword flags;
	char  codes[4][41];
	//168
};

struct DoorComboSet
{
	char name[21];
	//byte padding;
	//22
	word doorcombo_u[9][4];                                   //[door type][door combo]
	byte doorcset_u[9][4];                                    //[door type][door combo]
	word doorcombo_d[9][4];                                   //[door type][door combo]
	byte doorcset_d[9][4];                                    //[door type][door combo]
	//238 (216)
	word doorcombo_l[9][6];                                   //[door type][door combo]
	byte doorcset_l[9][6];                                    //[door type][door combo]
	word doorcombo_r[9][6];                                   //[door type][door combo]
	byte doorcset_r[9][6];                                    //[door type][door combo]
	//562 (324)
	word bombdoorcombo_u[2];                                  //rubble
	byte bombdoorcset_u[2];                                   //rubble
	word bombdoorcombo_d[2];                                  //rubble
	byte bombdoorcset_d[2];                                   //rubble
	//574 (12)
	word bombdoorcombo_l[3];                                  //rubble
	byte bombdoorcset_l[3];                                   //rubble
	//byte padding;
	word bombdoorcombo_r[3];                                  //rubble
	byte bombdoorcset_r[3];                                   //rubble
	//byte padding;
	//594 (18)
	word walkthroughcombo[4];                                 //[n, s, e, w]
	byte walkthroughcset[4];                                  //[n, s, e, w]
	
	byte flags[2];
	byte expansion[30];
	//638 (44)
};

struct zcolors
{
	byte text, caption;
	byte overw_bg, dngn_bg;
	byte dngn_fg, cave_fg;
	byte bs_dk, bs_goal;
	byte compass_lt, compass_dk;
	//10
	byte subscr_bg, subscr_shadow, triframe_color;
	byte bmap_bg,bmap_fg;
	byte link_dot;
	//15
	byte triforce_cset;
	byte triframe_cset;
	byte overworld_map_cset;
	byte dungeon_map_cset;
	byte blueframe_cset;
	//20
	word triforce_tile;
	word triframe_tile;
	word overworld_map_tile;
	word dungeon_map_tile;
	word blueframe_tile;
	//30
	word HCpieces_tile;
	byte HCpieces_cset;
	byte msgtext;
	byte foo[6];
	//40
	byte foo2[256];
	//296 bytes
};

struct palcycle
{
	byte first,count,speed;
	//3
};


//Audio


class zctune
{

public:

	char title[36];
	//20
	long start;
	long loop_start;
	long loop_end;
	//32
	short loop;
	short volume;
	byte flags;
	// 37
	void *data;
	// 41
	
	byte format;
	
	zctune()
	{
		data = NULL;
		format = MFORMAT_MIDI;
		reset();
	}
	
	zctune(char _title[36], long _start, long _loop_start, long _loop_end, short _loop,short _volume, void *_data, byte _format)
		: start(_start), loop_start(_loop_start), loop_end(_loop_end), loop(_loop), volume(_volume), data(_data), format(_format)
	{
		//memcpy(title, _title, 20); //NOT SAFE for short strings
		strncpy(title, _title, 36);
	}
	
	void clear()
	{
		memset(title,0,36);
		start = loop_start = loop_end = 0;
		loop = volume = flags = 0;
		data = NULL;
	}
	
	void copyfrom(zctune z)
	{
		start = z.start;
		loop_start = z.loop_start;
		loop_end = z.loop_end;
		loop = z.loop;
		flags = z.flags;
		volume = z.volume;
		//memcpy(title, z.title,20); //NOT SAFE for short title strings
		strncpy(title, z.title, 36);
		data = z.data;
	}
	
	void reset()
	{
		title[0]=0;
		loop=1;
		volume=144;
		start=0;
		loop_start=-1;
		loop_end=-1;
		flags=0;
		
		if(data) switch(format)
			{
			case MFORMAT_MIDI:
				destroy_midi((MIDI*) data);
				break;
				
			default:
				break;
			}
			
		data = NULL;
	}
	
};

//Game->

struct gamedata
{
	//private:
	char  _name[9];
	byte  _quest;
	//10
	//word _life,_maxlife;
	//short _drupy;
	//word _rupies,_arrows,_maxarrows,
	word _deaths;
	//20
	//byte  _keys,_maxbombs,
	byte  /*_wlevel,*/_cheat;
	//24
	bool  item[MAXITEMS];
	byte  items_off[MAXITEMS];
	//280
	word _maxcounter[32];	// 0 - life, 1 - rupees, 2 - bombs, 3 - arrows, 4 - magic, 5 - keys, 6-super bombs
	word _counter[32];
	short _dcounter[32];
	
	char  version[9];
	char  title[65];
	//354
	byte  _hasplayed;
	//byte  padding;
	//356
	dword _time;
	//360
	byte  _timevalid;
	byte  lvlitems[MAXLEVELS];
	byte  lvlkeys[MAXLEVELS];
	//byte  _HCpieces;
	byte  _continue_scrn;
	word  _continue_dmap;
	//620
	/*word  _maxmagic, _magic;
	short _dmagic;*/
	//byte  _magicdrainrate;
	//byte  _canslash;                                           //Link slashes instead of stabs.
	byte _generic[256];	// Generic gamedata. 0 - Heart pieces, 1- magicdrainrate, 2-canslash, 3-wlevel,
	// 4- HC Pieces per container  5- Continue hearts  6- Continue percent (1=yes)
	//byte  padding[2];
	//636
	byte  visited[MAXDMAPS];
	//892 (256)
	byte  bmaps[MAXDMAPS*64];                                 // the dungeon progress maps
	//17276 (16384)
	word  maps[MAXMAPS2*MAPSCRSNORMAL];                       // info on map changes, items taken, etc.
	//82556 (65280)
	byte  guys[MAXMAPS2*MAPSCRSNORMAL];                       // guy counts (though dungeon guys are reset on entry)
	//115196 (32640)
	char  qstpath[2048];
	byte  icon[128];
	byte  pal[48];
	long  screen_d[MAXDMAPS*MAPSCRSNORMAL][8];                // script-controlled screen variables
	long  global_d[256];                                      // script-controlled global variables
	std::vector< ZCArray <long> > globalRAM;
	
	byte awpn, bwpn;											// Currently selected weapon slots
	
	bool isclearing; // The gamedata is being cleared
	//115456 (260)
	
	// member functions
	// public:
	gamedata()
	{
		Clear();
	}
	
	~gamedata()
	{}
	
	void Clear();
	void Copy(const gamedata& g);
	
	gamedata &operator = (const gamedata& data)
	{
		this->Copy(data);
		this->globalRAM=data.globalRAM;
		return *this;
	}
	
	char *get_name();
	void set_name(char *n);
	
	byte get_quest();
	void set_quest(byte q);
	void change_quest(short q);
	
	word get_counter(byte c);
	void set_counter(word change, byte c);
	void change_counter(short change, byte c);
	
	word get_maxcounter(byte c);
	void set_maxcounter(word change, byte c);
	void change_maxcounter(short change, byte c);
	
	short get_dcounter(byte c);
	void set_dcounter(short change, byte c);
	void change_dcounter(short change, byte c);
	
	word get_life();
	void set_life(word l);
	void change_life(short l);
	
	word get_maxlife();
	void set_maxlife(word m);
	void change_maxlife(short m);
	
	short get_drupy();
	void set_drupy(short d);
	void change_drupy(short d);
	
	word get_rupies();
	word get_spendable_rupies();
	void set_rupies(word r);
	void change_rupies(short r);
	
	word get_maxarrows();
	void set_maxarrows(word a);
	void change_maxarrows(short a);
	
	word get_arrows();
	void set_arrows(word a);
	void change_arrows(short a);
	
	word get_deaths();
	void set_deaths(word d);
	void change_deaths(short d);
	
	word get_keys();
	void set_keys(word k);
	void change_keys(short k);
	
	word get_bombs();
	void set_bombs(word k);
	void change_bombs(short k);
	
	word get_maxbombs();
	void set_maxbombs(word b, bool setSuperBombs=true);
	void change_maxbombs(short b);
	
	word get_sbombs();
	void set_sbombs(word k);
	void change_sbombs(short k);
	
	word get_wlevel();
	void set_wlevel(word l);
	void change_wlevel(short l);
	
	byte get_cheat();
	void set_cheat(byte c);
	void change_cheat(short c);
	
	byte get_hasplayed();
	void set_hasplayed(byte p);
	void change_hasplayed(short p);
	
	dword get_time();
	void set_time(dword t);
	void change_time(long long t);
	
	byte get_timevalid();
	void set_timevalid(byte t);
	void change_timevalid(short t);
	
	byte get_HCpieces();
	void set_HCpieces(byte p);
	void change_HCpieces(short p);
	
	byte get_hcp_per_hc();
	void set_hcp_per_hc(byte val);
	
	byte get_cont_hearts();
	void set_cont_hearts(byte val);
	
	bool get_cont_percent();
	void set_cont_percent(bool ispercent);
	
	byte get_hp_per_heart();
	void set_hp_per_heart(byte val);
	
	byte get_mp_per_block();
	void set_mp_per_block(byte val);
	
	byte get_hero_dmgmult();
	void set_hero_dmgmult(byte val);
	
	byte get_ene_dmgmult();
	void set_ene_dmgmult(byte val);
	
	byte get_continue_scrn();
	void set_continue_scrn(byte s);
	void change_continue_scrn(short s);
	
	word get_continue_dmap();
	void set_continue_dmap(word d);
	void change_continue_dmap(short d);
	
	word get_maxmagic();
	void set_maxmagic(word m);
	void change_maxmagic(short m);
	
	word get_magic();
	void set_magic(word m);
	void change_magic(short m);
	
	short get_dmagic();
	void set_dmagic(short d);
	void change_dmagic(short d);
	
	byte get_magicdrainrate();
	void set_magicdrainrate(byte r);
	void change_magicdrainrate(short r);
	
	byte get_canslash();
	void set_canslash(byte s);
	void change_canslash(short s);
	
	short get_generic(byte c);
	void set_generic(byte change, byte c);
	void change_generic(short change, byte c);
	
	byte get_lkeys();
	
	void set_item(int id, bool value);
	void set_item_no_flush(int id, bool value);
	inline bool get_item(int id)
	{
		if ( ((unsigned)id) >= MAX_ITEMS ) return false;
		return item[id];
	}
	
};

//dropdata
class DropsetSymbols : public LibrarySymbols
{
public:
	static DropsetSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static DropsetSymbols singleton;
	DropsetSymbols();
};

//ponddata
class PondSymbols : public LibrarySymbols
{
public:
	static PondSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static PondSymbols singleton;
	PondSymbols();
};

//warpringdata
class WarpringSymbols : public LibrarySymbols
{
public:
	static WarpringSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static WarpringSymbols singleton;
	WarpringSymbols();
};

//doorsetdata
class DoorsetSymbols : public LibrarySymbols
{
public:
	static DoorsetSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static DoorsetSymbols singleton;
	DoorsetSymbols();
};

//misccolordata
class MiscColourSymbols : public LibrarySymbols
{
public:
	static MiscColourSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static MiscColourSymbols singleton;
	MiscColourSymbols();
};

//rgbdata
class RGBSymbols : public LibrarySymbols
{
public:
	static RGBSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static RGBSymbols singleton;
	RGBSymbols();
};

//palettedata
class PaletteSymbols : public LibrarySymbols
{
public:
	static PaletteSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static PaletteSymbols singleton;
	PaletteSymbols();
};

//tunesdata
class TunesSymbols : public LibrarySymbols
{
public:
	static TunesSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static TunesSymbols singleton;
	TunesSymbols();
};

//palcycledata
class PalCycleSymbols : public LibrarySymbols
{
public:
	static PalCycleSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static PalCycleSymbols singleton;
	PalCycleSymbols();
};

//gamedata
class GamedataSymbols : public LibrarySymbols
{
public:
	static GamedataSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static GamedataSymbols singleton;
	GamedataSymbols();
};

//cheatdata
class CheatsSymbols : public LibrarySymbols
{
public:
	static CheatsSymbols &getInst()
	{
		return singleton;
	}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
	static CheatsSymbols singleton;
	CheatsSymbols();
};
