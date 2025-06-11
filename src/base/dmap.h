#ifndef DMAP_H_
#define DMAP_H_

#include "base/general.h"
#include "base/ints.h"
#include <string>

struct dmap
{
	byte map;
	word level;
	char xoff;
	byte compass;
	word color;
	byte midi;
	byte cont;
	byte type;
	byte grid[8];
	char name[22];
	std::string title;
	char intro[74];
	// 0: before getting map
	// 1: after getting map
	int32_t minimap_tile[2];
	int32_t largemap_tile[2];
	byte minimap_cset[2];
	byte largemap_cset[2];
	char tmusic[57];
	byte tmusictrack;
	byte active_subscreen;
	byte passive_subscreen;
	byte overlay_subscreen;
	byte disableditems[MAXITEMS];
	int32_t flags;
	char sideview;
	word script;
	int32_t initD[8];
	char initD_label[8][65];
	word active_sub_script;
	word passive_sub_script;
	int32_t sub_initD[8];
	char sub_initD_label[8][65];
	word onmap_script;
	int32_t onmap_initD[8];
	char onmap_initD_label[8][65];
	int16_t mirrorDMap;
	int32_t tmusic_loop_start;
	int32_t tmusic_loop_end;
	int32_t tmusic_xfade_in;
	int32_t tmusic_xfade_out;
	uint32_t intro_string_id;

	void clear()
	{
		*this = dmap();
	}
	
	void validate_subscreens();
};

extern dmap *DMaps;
// DMap flags
#define dmfCAVES            0x000001
#define dmf3STAIR           0x000002
#define dmfWHIRLWIND        0x000004
#define dmfGUYCAVES         0x000008
#define dmfNOCOMPASS        0x000010
#define dmfWAVY             0x000020
#define dmfWHIRLWINDRET     0x000040
#define dmfALWAYSMSG        0x000080
#define dmfVIEWMAP          0x000100
#define dmfDMAPMAP          0x000200
#define dmfMINIMAPCOLORFIX  0x000400
#define dmfSCRIPT1          0x000800
#define dmfSCRIPT2          0x001000
#define dmfSCRIPT3          0x002000
#define dmfSCRIPT4          0x004000
#define dmfSCRIPT5          0x008000
#define dmfSIDEVIEW         0x010000
#define dmfLAYER3BG         0x020000
#define dmfLAYER2BG         0x040000
#define dmfNEWCELLARENEMIES 0x080000
#define dmfBUNNYIFNOPEARL   0x100000
#define dmfMIRRORCONTINUE   0x200000
#define dmfEXTENDEDVIEWPORT 0x400000
#define dmfZ3_RESERVERD_2   0x800000

// dmap types
enum { dmDNGN, dmOVERW, dmCAVE, dmBSOVERW, dmMAX };

// dmap type bit masks (?)
#define dmfCONTINUE      128
#define dmfTYPE          127

#endif

