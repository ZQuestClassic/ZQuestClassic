#ifndef _DMAP_H_
#define _DMAP_H_

#include "base/sizes.h"
#include "base/ints.h"

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
	//8
	byte grid[8];
	//16
	char name[21];
	char title[21];
	char intro[73];
	//byte padding;
	//132
	int32_t minimap_1_tile;                                      //before getting map
	byte minimap_1_cset;                                      //cset for minimap 1
	//byte padding;
	int32_t minimap_2_tile;                                      //after getting map
	byte minimap_2_cset;                                      //cset for minimap 2
	//byte padding;
	//140
	int32_t largemap_1_tile;                                     //large map
	byte largemap_1_cset;                                     //cset for large
	//byte padding;
	int32_t largemap_2_tile;                                     //large map
	byte largemap_2_cset;                                     //cset for large
	char tmusic[56];
	byte tmusictrack;
	byte active_subscreen;
	byte passive_subscreen;
	// int32_t emusic;
	//byte padding;
	//204
	byte disableditems[MAXITEMS];
	// 460
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
#define dmfZ3_RESERVERD_1   0x400000
#define dmfZ3_RESERVERD_2   0x800000

// dmap types
enum { dmDNGN, dmOVERW, dmCAVE, dmBSOVERW, dmMAX };

// dmap type bit masks (?)
#define dmfCONTINUE      128
#define dmfTYPE          127

#endif

