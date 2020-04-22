
#define COLOURSETTINGS

//Graphics->Colours[296]

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

#define PALETTECYCLE
//Graphics->PaletteCycle[3]


struct palcycle
{
	byte first,count,speed;
	//3
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
