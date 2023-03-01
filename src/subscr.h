//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#ifndef _SUBSCR_H_
#define _SUBSCR_H_

#include "sprite.h"
#include "items.h"
#include <stdio.h>

#define ssflagSHOWGRID  1
#define ssflagSHOWINVIS 2


extern bool show_subscreen_dmap_dots;
extern bool show_subscreen_numbers;
extern bool show_subscreen_items;
extern bool show_subscreen_life;

void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int32_t x, int32_t y, int32_t textstyle, int32_t alignment, int32_t color, int32_t shadow, int32_t bg);
void frame2x2(BITMAP *dest,miscQdata *misc,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h,int32_t flip,bool overlay,bool trans);
void frame2x2_a5(miscQdata *misc,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h,int32_t /*flip*/,bool overlay,bool trans);
void drawgrid(BITMAP *dest,int32_t x,int32_t y,int32_t c1,int32_t c2);
void draw_block(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h);
void drawdmap(BITMAP *dest, miscQdata *misc, int32_t x, int32_t y, bool showmap, int32_t showhero, int32_t showcompass, int32_t herocolor, int32_t lccolor, int32_t dccolor);
void lifemeter(BITMAP *dest,int32_t x,int32_t y,int32_t tile,bool bs_style);
void magicmeter(BITMAP *dest,int32_t x,int32_t y);
void buttonitem(BITMAP *dest, int32_t button, int32_t x, int32_t y);
void putxnum(BITMAP *dest,int32_t x,int32_t y,int32_t num,FONT *tempfont,int32_t color,int32_t shadowcolor,int32_t bgcolor,int32_t textstyle,bool usex,int32_t digits,bool infinite,char idigit);
void defaultcounters(BITMAP *dest, int32_t x, int32_t y, FONT *tempfont, int32_t color, int32_t shadowcolor, int32_t bgcolor, bool usex, int32_t textstyle, int32_t digits, char idigit);
void counter(BITMAP *dest, int32_t x, int32_t y, FONT *tempfont, int32_t color, int32_t shadowcolor, int32_t bgcolor, int32_t alignment, int32_t textstyle, int32_t digits, char idigit, bool showzero, int32_t itemtype1, int32_t itemtype2, int32_t itemtype3, int32_t infiniteitem, bool onlyselected);
void minimaptitle(BITMAP *dest, int32_t x, int32_t y, FONT *tempfont, int32_t color, int32_t shadowcolor, int32_t bgcolor, int32_t alignment, int32_t textstyle);
void animate_selectors();
void delete_selectors();

INLINE void putdot(BITMAP *dest,int32_t x,int32_t y,int32_t c)
{
    rectfill(dest,x,y,x+2,y+2,c);
}

// subscreen default types
enum { ssdtOLD, ssdtNEWSUBSCR, ssdtREV2, ssdtBSZELDA, ssdtBSZELDAMODIFIED, ssdtBSZELDAENHANCED, ssdtBSZELDACOMPLETE, ssdtZ3, ssdtMAX };

enum { sstACTIVE, sstPASSIVE, sstMAX };

enum { sssFULLPUSH, sssFULLSLIDEDOWN, sssMAX };

// subscreen object types
enum { ssoNULL, ssoNONE, sso2X2FRAME, ssoTEXT,
       ssoLINE, ssoRECT, ssoBSTIME, ssoTIME, ssoSSTIME, ssoMAGICMETER, ssoLIFEMETER, ssoBUTTONITEM, ssoICON, ssoCOUNTER,
       ssoCOUNTERS, ssoMINIMAPTITLE, ssoMINIMAP, ssoLARGEMAP, ssoCLEAR, ssoCURRENTITEM, ssoITEM, ssoTRIFRAME, ssoTRIFORCE, ssoTILEBLOCK, ssoMINITILE, ssoSELECTOR1, ssoSELECTOR2,
       ssoMAGICGAUGE, ssoLIFEGAUGE, ssoTEXTBOX, ssoCURRENTITEMTILE, ssoSELECTEDITEMTILE, ssoCURRENTITEMTEXT, ssoCURRENTITEMNAME, ssoSELECTEDITEMNAME, ssoCURRENTITEMCLASSTEXT,
       ssoCURRENTITEMCLASSNAME, ssoSELECTEDITEMCLASSNAME, ssoMAX
     };
//ssoCURRENTITEM shows what item of that type you currently have.  if the itemtype is sword, it will show what sword you have
//ssoBUTTONITEM shows what item is currently assigned to the A or B button
//ssoITEMTILE shows a requested item
//ssoCURRENTITEMTILE shows a tile (or animation) if you have the item requested.  if itemtype is set to wooden boomerang and you have a wooden boomerang, it will show the tile you choose
//ssoSELECTEDITEMTILE shows a tile (or animation) if the selection cursor is on the item requested.
//ssoCURRENTITEMTEXT shows the requested text if you have the item requested.  if itemtype is set to wooden boomerang and you have a wooden boomerang, it will show the text you choose, "Curved stick", for instance.
//ssoCURRENTITEMNAME shows the name of an item if you have the item requested.  if itemtype is set to wooden boomerang and you have a wooden boomerang, it will show the name of the item "Wooden Boomerang"
//ssoSELECTEDITEMNAME shows the name of of the item that the selection cursor is on.
//ssoCURRENTITEMCLASSTEXT shows the requested text if you have the item requested.  if itemtype is set to boomerang and you have a wooden boomerang, it will show the text you choose, "Curved stick", for instance.
//ssoCURRENTITEMCLASSNAME shows the name of an item if you have the item requested.  if itemtype is set to boomerang and you have a wooden boomerang, it will show the name of the item "Wooden Boomerang"
//ssoSELECTEDITEMCLASSNAME shows the name of of the item class that the selection cursor is on.

//text styles
enum { sstsNORMAL, sstsSHADOW, sstsSHADOWU, sstsOUTLINE8, sstsOUTLINEPLUS, sstsOUTLINEX, sstsSHADOWED, sstsSHADOWEDU, sstsOUTLINED8, sstsOUTLINEDPLUS, sstsOUTLINEDX, sstsMAX };

//subscreen fonts
enum
{ 
	ssfZELDA, ssfSS1, ssfSS2, ssfSS3, ssfSS4, ssfZTIME, ssfSMALL, ssfSMALLPROP, ssfZ3SMALL,
	ssfGBLA, ssfZ3, ssfGORON, ssfZORAN, ssfHYLIAN1, ssfHYLIAN2, ssfHYLIAN3, ssfHYLIAN4,
	ssfPROP, ssfGBORACLE, ssfGBORACLEP, ssfDSPHANTOM, ssfDSPHANTOMP, ssfAT800, ssfACORN,
	ssADOS, ssfALLEG, ssfAPL2, ssfAPL280, ssfAPL2GS, ssfAQUA, ssfAT400, ssfC64, ssfC64HR,
	ssfCGA, ssfCOCO, ssfCOCO2, ssfCOUPE, ssfCPC, ssfFANTASY, ssfFDSKANA, ssfFDSLIKE,
	ssfFDSROM, ssfFF, ssfFUTHARK, ssfGAIA, ssfHIRA, ssfJP, ssfKONG, ssfMANA, ssfML, ssfMOT,
	ssfMSX0, ssfMSX1, ssfPET, ssfPSTART, ssfSATURN, ssfSCIFI, ssfSHERW, ssfSINQL, ssfSPEC,
	ssfSPECLG, ssfTI99, ssfTRS, ssfZ2, ssfZX, ssfLISA,
	ssfMAX
};

// subscreen color types
enum { ssctSYSTEM=0xFE, ssctMISC=0xFF };

// special colors
enum { ssctTEXT, ssctCAPTION, ssctOVERWBG, ssctDNGNBG, ssctDNGNFG, ssctCAVEFG, ssctBSDK, ssctBSGOAL, ssctCOMPASSLT, ssctCOMPASSDK, ssctSUBSCRBG, ssctSUBSCRSHADOW,
       ssctTRIFRAMECOLOR, ssctBMAPBG, ssctBMAPFG, ssctHERODOT, ssctMSGTEXT, ssctMAX
     };

// special csets
enum { sscsTRIFORCECSET, sscsTRIFRAMECSET, sscsOVERWORLDMAPCSET, sscsDUNGEONMAPCSET, sscsBLUEFRAMECSET, sscsHCPIECESCSET, sscsSSVINECSET, sscsMAX };

// special tiles
enum { ssmstSSVINETILE, ssmstMAGICMETER, ssmstMAX };


// counter objects
enum { sscRUPEES, sscBOMBS, sscSBOMBS, sscARROWS,
       sscGENKEYMAGIC, sscGENKEYNOMAGIC, sscLEVKEYMAGIC, sscLEVKEYNOMAGIC,
       sscANYKEYMAGIC, sscANYKEYNOMAGIC, sscSCRIPT1, sscSCRIPT2,
       sscSCRIPT3, sscSCRIPT4, sscSCRIPT5, sscSCRIPT6,
       sscSCRIPT7, sscSCRIPT8, sscSCRIPT9, sscSCRIPT10,
       sscSCRIPT11, sscSCRIPT12, sscSCRIPT13, sscSCRIPT14,
       sscSCRIPT15, sscSCRIPT16, sscSCRIPT17, sscSCRIPT18,
       sscSCRIPT19, sscSCRIPT20, sscSCRIPT21, sscSCRIPT22,
       sscSCRIPT23, sscSCRIPT24, sscSCRIPT25, sscLIFE, sscMAGIC, sscMAXHP, sscMAXMP, sscMAX
     };
int32_t scounter_to_ctr(int32_t ssc);


//subscreen items
/*enum { ssiBOMB, ssiSWORD, ssiSHIELD, ssiCANDLE, ssiLETTER, ssiPOTION, ssiLETTERPOTION, ssiBOW, ssiARROW, ssiBOWANDARROW, ssiBAIT, ssiRING, ssiBRACELET, ssiMAP,
       ssiCOMPASS, ssiBOSSKEY, ssiMAGICKEY, ssiBRANG, ssiWAND, ssiRAFT, ssiLADDER, ssiWHISTLE, ssiBOOK, ssiWALLET, ssiSBOMB, ssiHCPIECE, ssiAMULET, ssiFLIPPERS,
       ssiHOOKSHOT, ssiLENS, ssiHAMMER, ssiBOOTS, ssiDINSFIRE, ssiFARORESWIND, ssiNAYRUSLOVE, ssiQUIVER, ssiBOMBBAG, ssiCBYRNA, ssiROCS, ssiHOVERBOOTS,
       ssiSPINSCROLL, ssiCROSSSCROLL, ssiQUAKESCROLL, ssiWHISPRING, ssiCHARGERING, ssiPERILSCROLL, ssiWEALTHMEDAL, ssiHEARTRING, ssiMAGICRING, ssiSPINSCROLL2,
       ssiQUAKESCROLL2, ssiAGONY, ssiSTOMPBOOTS, ssiWHIMSICALRING, ssiPERILRING, ssiMAX };*/

//subscreen text alignment
enum { sstaLEFT, sstaCENTER, sstaRIGHT };

//when to display an element
#define sspUP 1
#define sspDOWN 2
#define sspSCROLLING 4

struct sso_struct
{
    char *s;
    int32_t i;
};

extern sso_struct bisso[ssoMAX];


struct subscreen_object
{
    byte  type;
    byte  pos;
    int16_t  x;
    int16_t  y;
    word  w;
    word  h;
    byte  colortype1;
    int16_t color1;
    byte  colortype2;
    int16_t color2;
    byte  colortype3;
    int16_t color3;
    int32_t   d1;
    int32_t   d2;
    int32_t   d3; //pos
    int32_t   d4; //up
    int32_t   d5; //down
    int32_t   d6; //left
    int32_t   d7; //right
    int32_t   d8;
    int32_t   d9;
    int32_t   d10;
    byte  frames;
    byte  speed;
    byte  delay;
    word  frame;
    void  *dp1;
};

struct subscreen_group
{
    byte             ss_type;
    char             name[64];
    subscreen_object objects[MAXSUBSCREENITEMS];
};



/****  Subscr items code  ****/
extern subscreen_object default_subscreen_active[ssdtMAX][2][66];
extern subscreen_object default_subscreen_passive[ssdtMAX][2][21];
//extern subscreen_object z3_active[200];
//extern subscreen_object z3_passive[200];
extern subscreen_object z3_active_a[80];
extern subscreen_object z3_passive_a[66];
extern subscreen_object z3_active_ab[82];
extern subscreen_object z3_passive_ab[75];
extern subscreen_group custom_subscreen[MAXCUSTOMSUBSCREENS];
extern subscreen_group *current_subscreen_active;
extern subscreen_group *current_subscreen_passive;

extern item *Bitem, *Aitem, *Yitem, *Xitem;
extern int32_t   Bid, Aid, Xid, Yid;
const byte tripiece[2][8][3] =
{
    //  112,112,0, 128,112,1, 96,128,0, 144,128,1,
    //  112,128,2, 112,128,1, 128,128,3, 128,128,0
    {
        //old style
        {32,8,0},
        {48,8,1},
        {16,24,0},
        {64,24,1},
        {32,24,2},
        {32,24,1},
        {48,24,3},
        {48,24,0}
    },
    {
        //bs style
        {32,8,0},
        {56,8,1},
        {8,56,0},
        {80,56,1},
        {32,56,2},
        {32,56,1},
        {56,56,3},
        {56,56,0}
    }
};
const byte bmap_original[2][16] =
{
    {0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0},
    {0,3,0,0,0,3,0,0,0,0,3,0,0,0,3,0}
};
const byte bmap_bs[2][12] =
{
    {0,0,1,0,0,1,0,0,1,0,1,0},
    {0,3,0,0,3,0,0,3,0,3,0,0}
};
const int32_t fringe[8] = { 6,2,4,7,6,8,7,5 };
const byte bmaptiles_old[8*5] =
{
    0, 1, 2, 3, 2, 3, 3, 4,
    20,21,22,23,22,23,23,24,
    20,21,22,23,22,23,23,24,
    20,21,22,23,22,23,23,24,
    40,41,42,43,42,43,43,44
};

const byte bmaptiles_original[5][8] =
{
    {0, 1, 2, 3, 2, 3, 3, 4},
    {20,21,22,23,22,23,23,24},
    {20,21,22,23,22,23,23,24},
    {20,21,22,23,22,23,23,24},
    {40,41,42,43,42,43,43,44}
};

const byte bmaptiles_bs[5][6] =
{
    {0, 1, 2, 3, 3, 4},
    {20,21,22,23,23,24},
    {20,21,22,23,23,24},
    {20,21,22,23,23,24},
    {40,41,42,43,43,44}
};

void reset_subscr_items();
void update_subscr_items();
void add_subscr_item(item *newItem);
int32_t stripspaces(char *source, char *target, int32_t stop);
void put_passive_subscr(BITMAP *dest,miscQdata *misc,int32_t x,int32_t y,bool showtime,int32_t pos2);
void puttriframe(BITMAP *dest, miscQdata *misc, int32_t x, int32_t y, int32_t triframecolor, int32_t numbercolor, int32_t triframetile, int32_t triframecset, int32_t triforcetile, int32_t triforcecset, bool showframe, bool showpieces, bool largepieces);
void puttriforce(BITMAP *dest, miscQdata *misc, int32_t x, int32_t y, int32_t tile, int32_t cset, int32_t w, int32_t h, int32_t flip, bool overlay, bool trans, int32_t trinum);
void draw_block(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h);
void draw_block_flip(BITMAP *dest,int32_t x,int32_t y,int32_t tile,int32_t cset,int32_t w,int32_t h,int32_t flip,bool overlay,bool trans);
void putBmap(BITMAP *dest, miscQdata *misc, int32_t x, int32_t y,bool showmap, bool showrooms, bool showhero, int32_t roomcolor, int32_t herocolor, bool large);
void load_Sitems(miscQdata *misc);
void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int32_t x, int32_t y, int32_t textstyle, int32_t alignment, int32_t color, int32_t shadow, int32_t bg);
void textprintf_styled_aligned_ex(BITMAP *bmp, const FONT *f, int32_t x, int32_t y, int32_t textstyle, int32_t alignment, int32_t color, int32_t shadow, int32_t bg, const char *format, ...);
void update_subscreens(int32_t dmap=-1);
void show_custom_subscreen(BITMAP *dest, miscQdata *misc, subscreen_group *css, int32_t xofs, int32_t yofs, bool showtime, int32_t pos2);
FONT *ss_font(int32_t fontnum);
int32_t to_real_font(int32_t ss_font);
int32_t to_ss_font(int32_t real_font);
int32_t ss_objects(subscreen_group *tempss);
void purge_blank_subscreen_objects(subscreen_group *tempss);
int32_t subscreen_cset(miscQdata *misc,int32_t c1, int32_t c2);

int32_t sso_x(subscreen_object *tempsso);
int32_t sso_y(subscreen_object *tempsso);
int32_t sso_h(subscreen_object *tempsso);
int32_t sso_w(subscreen_object *tempsso);
int32_t get_alignment(subscreen_object *tempsso);
void sso_bounding_box(BITMAP *bmp, subscreen_group *tempss, int32_t index, int32_t color);


bool findWeaponWithParent(int32_t id, int32_t type);
int32_t countWeaponWithParent(int32_t id, int32_t type);
#endif

/*** end of subscr.cc ***/

