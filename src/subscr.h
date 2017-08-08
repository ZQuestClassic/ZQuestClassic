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
#include "quest/Quest.h"
#include <stdio.h>
#include <sstream>

#define ssflagSHOWGRID  1
#define ssflagSHOWINVIS 2


extern bool show_subscreen_dmap_dots;
extern bool show_subscreen_numbers;
extern bool show_subscreen_items;
extern bool show_subscreen_life;
extern gamedata *game;
extern Quest *curQuest;

void frame2x2(BITMAP *dest,miscQdata *misc,int x,int y,int tile,int cset,int w,int h,int flip,bool overlay,bool trans);
void drawgrid(BITMAP *dest,int x,int y,int c1,int c2);
void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h);
void drawdmap(BITMAP *dest, miscQdata *misc, int x, int y, bool showmap, int showlink, int showcompass, int linkcolor, int lccolor, int dccolor);
void lifemeter(BITMAP *dest,int x,int y,int tile,bool bs_style);
void magicmeter(BITMAP *dest,int x,int y);
void buttonitem(BITMAP *dest, int button, int x, int y);
void putxnum(BITMAP *dest,int x,int y,int num,FONT *tempfont,int color,int shadowcolor,int bgcolor,int textstyle,bool usex,int digits,bool infinite,char idigit);
void defaultcounters(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, bool usex, int textstyle, int digits, char idigit);
void counter(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, int alignment, int textstyle, int digits, char idigit, bool showzero, int itemtype1, int itemtype2, int itemtype3, const ItemDefinitionRef &infiniteitem, bool onlyselected);
void minimaptitle(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, int alignment, int textstyle);
void animate_selectors();
void delete_selectors();
FONT *ss_font(int fontnum);
char *time_str_short2(dword time);
char *time_str_med(dword time);

INLINE void putdot(BITMAP *dest,int x,int y,int c)
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
enum { ssfZELDA, ssfSS1, ssfSS2, ssfSS3, ssfSS4, ssfZTIME, ssfSMALL, ssfSMALLPROP, ssfZ3SMALL, ssfGBLA, ssfZ3,
       ssfGORON, ssfZORAN, ssfHYLIAN1, ssfHYLIAN2, ssfHYLIAN3, ssfHYLIAN4, ssfPROP, ssfGBORACLE, ssfGBORACLEP,
       ssfDSPHANTOM, ssfDSPHANTOMP, ssfMAX
     };

// subscreen color types
enum { ssctSYSTEM=0xFE, ssctMISC=0xFF };

// special colors
enum { ssctTEXT, ssctCAPTION, ssctOVERWBG, ssctDNGNBG, ssctDNGNFG, ssctCAVEFG, ssctBSDK, ssctBSGOAL, ssctCOMPASSLT, ssctCOMPASSDK, ssctSUBSCRBG, ssctSUBSCRSHADOW,
       ssctTRIFRAMECOLOR, ssctBMAPBG, ssctBMAPFG, ssctLINKDOT, ssctMSGTEXT, ssctMAX
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
       sscSCRIPT23, sscSCRIPT24, sscSCRIPT25, sscMAX
     };


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
    int i;
};

extern sso_struct bisso[ssoMAX];


class subscreen_group;

class subscreen_object
{
public:
    subscreen_object(byte type, byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3)
        : type(type), pos(pos), x(x), y(y), w(w), h(h), colortype1(colortype1), color1(color1), colortype2(colortype2), color2(color2), colortype3(colortype3), color3(color3)
    {}
    subscreen_object(byte type, PACKFILE *f, int &status);

    virtual ~subscreen_object() {}

    byte  type;
    byte  pos;
    word  x;
    word  y;
    word  w;
    word  h;
    byte  colortype1;
    short color1;
    byte  colortype2;
    short color2;
    byte  colortype3;
    short color3;

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime) = 0;
    virtual int get_alignment() { return sstaLEFT; }
    virtual int sso_x() { return x; }
    virtual int sso_y() { return y; }
    virtual int sso_w() { return w; }
    virtual int sso_h() { return h; }

    bool serialize(PACKFILE *f);
    virtual bool serializeExtraData(PACKFILE *f) = 0;
    
    // copies only some of the properties (i.e., not position) depending on the type of subscreen object
    virtual void copyFromExtra(const subscreen_object &other) {}
    void copyFrom(const subscreen_object &other)
    {
        if (type != other.type)
            return;
        pos&=~(sspUP|sspDOWN|sspSCROLLING);
        pos|= other.pos&(sspUP|sspDOWN|sspSCROLLING);
        copyFromExtra(other);
    }

    virtual std::string toStringExtra() {
        return std::string();
    }

    std::string toString()
    {
        std::stringstream ss;

        ss << "Type:  " << (int)type << std::endl
            << "Position:  " << (int)pos << std::endl
            << "X:  " << x << std::endl
            << "Y:  " << y << std::endl
            << "W:  " << w << std::endl
            << "H:  " << h << std::endl
            << "Color Type 1:  " << (int)colortype1 << std::endl
            << "Color 1:  " << color1 << std::endl
            << "Color Type 2:  " << (int)colortype2 << std::endl
            << "Color 2:  " << color2 << std::endl
            << "Color Type 3:  " << (int)colortype3 << std::endl
            << "Color 3:  " << color3 << std::endl
            << toStringExtra();
        return ss.str();
    }

    virtual subscreen_object *clone() = 0;


private:
    //not implemented
    void operator=(const subscreen_object &other);
};

class subscreen_object_2x2frame : public subscreen_object
{
public:
    subscreen_object_2x2frame(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t tile, uint32_t flip, bool overlay, bool transparent)
        : subscreen_object(sso2X2FRAME, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), flip(flip), overlay(overlay), transparent(transparent)
    {}
    subscreen_object_2x2frame(PACKFILE *f, int &status);

    uint32_t tile;  // was: d1
    uint32_t flip;  // was: d2
    bool overlay;   // was: d3
    bool transparent;   // was: d4

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "Flip:  " << flip << std::endl
            << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 8*w; }
    virtual int sso_h() { return 8*h; }

    virtual subscreen_object *clone() { return new subscreen_object_2x2frame(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_2x2frame &othero = (const subscreen_object_2x2frame &)other;
        tile = othero.tile;
        flip = othero.flip;
        overlay = othero.overlay;
        transparent = othero.transparent;
        color1 = othero.color1;
        colortype1 = othero.colortype1;
    }
};

class subscreen_object_currentitem : public subscreen_object
{
public:
    subscreen_object_currentitem(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t itemfamily, bool visible, int32_t posselect, uint32_t upselect, uint32_t downselect, uint32_t leftselect, uint32_t rightselect, ItemDefinitionRef itemref)
        : subscreen_object(ssoCURRENTITEM, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), itemfamily(itemfamily), visible(visible), posselect(posselect), upselect(upselect), downselect(downselect), leftselect(leftselect), rightselect(rightselect), itemref(itemref)
    {}
    subscreen_object_currentitem(PACKFILE *f, int &status);

    uint32_t itemfamily;  // was: d1
    bool visible; // was: d2
    int32_t posselect; // was: d3
    uint32_t upselect; // was: d4
    uint32_t downselect; // was: d5
    uint32_t leftselect; // was: d6
    uint32_t rightselect; // was: d7
    ItemDefinitionRef itemref; // was: d8

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Item Family:  " << itemfamily << std::endl
            << "Visible:  " << visible << std::endl
            << "Select Position:  " << posselect << std::endl
            << "Up:  " << upselect << std::endl
            << "Down:  " << downselect << std::endl
            << "Left:  " << leftselect << std::endl
            << "Right:  " << rightselect << std::endl
            << "Item Override:  ";
        if (!curQuest->isValid(itemref))
            ss << "(None)";
        else
            ss << curQuest->getItemDefinition(itemref).name;
        ss << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 16; }
    virtual int sso_h() { return 16; }

    virtual subscreen_object *clone() { return new subscreen_object_currentitem(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_currentitem &othero = (const subscreen_object_currentitem &)other;
        visible = othero.visible;
    }
};

class subscreen_object_selector1 : public subscreen_object
{
public:
    subscreen_object_selector1(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t tile, uint32_t flip, bool overlay, bool transparent, bool large)
        : subscreen_object(ssoSELECTOR1, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), flip(flip), overlay(overlay), transparent(transparent), large(large)
    {}
    subscreen_object_selector1(PACKFILE *f, int &status);

    uint32_t tile; // was: d1; not used
    uint32_t flip; // was: d2; not used
    bool overlay; // was: d3
    bool transparent; // was: d4
    bool large; // was: d5

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "Flip:  " << flip << std::endl
            << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl
            << "Large:  " << large << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_h() { return large ? 32 : 16; }
    virtual int sso_w() { return large ? 32 : 16; }

    virtual subscreen_object *clone() { return new subscreen_object_selector1(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_selector1 &othero = (const subscreen_object_selector1 &)other;
        tile = othero.tile;
        flip = othero.flip;
        overlay = othero.overlay;
        transparent = othero.transparent;
        large = othero.large;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
    }
};

// Why are there two of these, instead of just having a variable distinguish the two? -DD
class subscreen_object_selector2 : public subscreen_object
{
public:
    subscreen_object_selector2(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t tile, uint32_t flip, bool overlay, bool transparent, bool large)
        : subscreen_object(ssoSELECTOR2, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), flip(flip), overlay(overlay), transparent(transparent), large(large)
    {}
    subscreen_object_selector2(PACKFILE *f, int &status);

    uint32_t tile; // was: d1; not used
    uint32_t flip; // was: d2; not used
    bool overlay; // was: d3
    bool transparent; // was: d4
    bool large; // was: d5

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "Flip:  " << flip << std::endl
            << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl
            << "Large:  " << large << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_h() { return large ? 32 : 16; }
    virtual int sso_w() { return large ? 32 : 16; }

    virtual subscreen_object *clone() { return new subscreen_object_selector2(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_selector2 &othero = (const subscreen_object_selector2 &)other;
        tile = othero.tile;
        flip = othero.flip;
        overlay = othero.overlay;
        transparent = othero.transparent;
        large = othero.large;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
    }
};

class subscreen_object_counter : public subscreen_object
{
public:
    subscreen_object_counter(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle, uint32_t digits, char infdigit, uint32_t flags, uint32_t countertype1, uint32_t countertype2, uint32_t countertype3, ItemDefinitionRef itemref)
        : subscreen_object(ssoCOUNTER, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle), digits(digits), infdigit(infdigit), flags(flags), countertype1(countertype1), countertype2(countertype2), countertype3(countertype3), itemref(itemref)
    {}
    subscreen_object_counter(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    uint32_t digits; // was: d4
    char infdigit; // was: d5
    uint32_t flags; // was: d6
    uint32_t countertype1; // was: d7
    uint32_t countertype2; // was: d8
    uint32_t countertype3; // was: d9
    ItemDefinitionRef itemref; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Digits:  " << digits << std::endl
            << "Infinite Digit:  " << infdigit << std::endl
            << "Flags: " << flags << std::endl
            << "Counter Type 1:  " << countertype1 << std::endl
            << "Counter Type 2:  " << countertype2 << std::endl
            << "Counter Type 3:  " << countertype3 << std::endl
            << "Item:  ";
        if (!curQuest->isValid(itemref))
            ss << "(None)";
        else
            ss << curQuest->getItemDefinition(itemref).name;
        ss << std::endl;
        return ss.str();
    }
    

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int get_alignment() { return alignment; }
    virtual int sso_x() 
    {
        int ret = x;
        switch(textstyle)
        {
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_y()
    {
        int ret = y;
        switch(textstyle)
        {
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_h()
    {
        int ret = text_height(ss_font(fontnum));;
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual int sso_w()
    {
        int ret = text_length(ss_font(fontnum), "0")*digits;
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual subscreen_object *clone() { return new subscreen_object_counter(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_counter &othero = (const subscreen_object_counter &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        digits = othero.digits;
        infdigit = othero.infdigit;
        flags = othero.flags;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_line : public subscreen_object
{
public:
    subscreen_object_line(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        bool overlay, bool transparent)
        : subscreen_object(ssoLINE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), overlay(overlay), transparent(transparent)
    {}
    subscreen_object_line(PACKFILE *f, int &status);

    bool overlay; // was d3 (or d1? something's buggy)
    bool transparent; // was d4 (or d2?)

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    

    virtual subscreen_object *clone() { return new subscreen_object_line(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_line &othero = (const subscreen_object_line &)other;
        overlay = othero.overlay;
        transparent = othero.transparent;
        w = othero.w;
        h = othero.h;
        colortype1 = other.colortype1;
        color1 = other.color1;
    }
};

class subscreen_object_buttonitem : public subscreen_object
{
public:
    subscreen_object_buttonitem(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t button, bool transparent)
        : subscreen_object(ssoBUTTONITEM, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), button(button), transparent(transparent)
    {}
    subscreen_object_buttonitem(PACKFILE *f, int &status);

    uint32_t button; // was: d1
    bool transparent; // was: d2

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int sso_w() { return 16; }
    virtual int sso_h() { return 16; }

    virtual subscreen_object *clone() { return new subscreen_object_buttonitem(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_buttonitem &othero = (const subscreen_object_buttonitem &)other;
        transparent = othero.transparent;
    }
};

class subscreen_object_selecteditemname : public subscreen_object
{
public:
    subscreen_object_selecteditemname(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle, bool wword, uint32_t tabsize)
        : subscreen_object(ssoSELECTEDITEMNAME, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle), wword(wword), tabsize(tabsize)
    {}
    subscreen_object_selecteditemname(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    bool wword; // was: d4
    uint32_t tabsize; // was: d5

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Word Wrap:  " << wword << std::endl
            << "Tab Size:  " << tabsize << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);        

    virtual subscreen_object *clone() { return new subscreen_object_selecteditemname(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_selecteditemname &othero = (const subscreen_object_selecteditemname &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        wword = othero.wword;
        tabsize = othero.tabsize;
        w = othero.w;
        h = othero.h;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_text : public subscreen_object
{
public:
    subscreen_object_text(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle, std::string text)
        : subscreen_object(ssoTEXT, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle), text(text)
    {}
    subscreen_object_text(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    std::string text; // was: dp1

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Text:  " << text << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int get_alignment() { return alignment; }
    virtual int sso_x() 
    {
        int ret = x;
        switch(textstyle)
        {
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_y()
    {
        int ret = y;
        switch(textstyle)
        {
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_h()
    {
        int ret = text_height(ss_font(fontnum));;
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual int sso_w()
    {
        int ret = text_length(ss_font(fontnum), text.c_str());
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual subscreen_object *clone() { return new subscreen_object_text(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_text &othero = (const subscreen_object_text &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_rect : public subscreen_object
{
public:
    subscreen_object_rect(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        bool filled, bool transparent)
        : subscreen_object(ssoRECT, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), filled(filled), transparent(transparent)
    {}
    subscreen_object_rect(PACKFILE *f, int &status);

    bool filled;        // was: d1
    bool transparent;   // was: d2

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Filled:  " << filled << std::endl
            << "Transparent:  " << transparent << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);

    virtual subscreen_object *clone() { return new subscreen_object_rect(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_rect &othero = (const subscreen_object_rect &)other;
        filled = othero.filled;
        transparent = othero.transparent;
        w = othero.w;
        h = othero.h;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
    }
};

class subscreen_object_triforce : public subscreen_object
{
public:
    subscreen_object_triforce(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t tile, uint32_t flip, bool overlay, bool transparent, uint32_t triforcenum)
        : subscreen_object(ssoTRIFORCE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), flip(flip), overlay(overlay), transparent(transparent), triforcenum(triforcenum)
    {}
    subscreen_object_triforce(PACKFILE *f, int &status);

    uint32_t tile;  // was: d1
    uint32_t flip;  // was: d2
    bool overlay;   // was: d3
    bool transparent;   // was: d4
    uint32_t triforcenum;   // was: d5

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "Flip:  " << flip << std::endl
            << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl
            << "Triforce Number:  " << triforcenum << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 16; }
    virtual int sso_h() { return 16; }

    virtual subscreen_object *clone() { return new subscreen_object_triforce(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_triforce &othero = (const subscreen_object_triforce &)other;
        tile = othero.tile;
        flip = othero.flip;
        overlay = othero.overlay;
        transparent = othero.transparent;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
    }
};

class subscreen_object_tileblock : public subscreen_object
{
public:
    subscreen_object_tileblock(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t tile, uint32_t flip, bool overlay, bool transparent)
        : subscreen_object(ssoTILEBLOCK, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), flip(flip), overlay(overlay), transparent(transparent)
    {}
    subscreen_object_tileblock(PACKFILE *f, int &status);

    uint32_t tile;  // was: d1
    uint32_t flip;  // was: d2
    bool overlay;   // was: d3
    bool transparent;   // was: d4

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "Flip:  " << flip << std::endl
            << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 16*w; }
    virtual int sso_h() { return 16*h; }

    virtual subscreen_object *clone() { return new subscreen_object_tileblock(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_tileblock &othero = (const subscreen_object_tileblock &)other;
        tile = othero.tile;
        flip = othero.flip;
        overlay = othero.overlay;
        transparent = othero.transparent;
        w = othero.w;
        h = othero.h;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
    }
};

class subscreen_object_clear : public subscreen_object
{
public:
    subscreen_object_clear(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3
        )
        : subscreen_object(ssoCLEAR, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3)
    {}
    subscreen_object_clear(PACKFILE *f, int &status);

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 5; }
    virtual int sso_h() { return 5; }

    virtual subscreen_object *clone() { return new subscreen_object_clear(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_clear &othero = (const subscreen_object_clear &)other;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
    }
};

class subscreen_object_minitile : public subscreen_object
{
public:
    subscreen_object_minitile(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        int32_t tile, uint32_t specialtile, uint32_t tileoffset, uint32_t flip, bool overlay, bool transparent)
        : subscreen_object(ssoMINITILE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), specialtile(specialtile), tileoffset(tileoffset), flip(flip), overlay(overlay), transparent(transparent)
    {}
    subscreen_object_minitile(PACKFILE *f, int &status);

    int32_t tile; // was: d1
    uint32_t specialtile; // was: d2
    uint32_t tileoffset; // was: d3
    uint32_t flip; // was: d4
    bool overlay; // was: d5
    bool transparent; // was: d6

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "Special Tile:  " << specialtile << std::endl
            << "Tile Offset:  " << tileoffset << std::endl
            << "Flip:  " << flip << std::endl
            << "Overlay:  " << overlay << std::endl
            << "Transparent:  " << transparent << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 8; }
    virtual int sso_h() { return 8; }

    virtual subscreen_object *clone() { return new subscreen_object_minitile(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_minitile &othero = (const subscreen_object_minitile &)other;
        tile = othero.tile;
        specialtile = othero.specialtile;
        tileoffset = othero.tileoffset;
        flip = othero.flip;
        overlay = othero.overlay;
        transparent = othero.transparent;
        w = othero.w;
        h = othero.h;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
    }
};

class subscreen_object_magicgauge : public subscreen_object
{
public:
    subscreen_object_magicgauge(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t containernum, uint32_t notlast_tile, uint32_t last_tile, uint32_t cap_tile, uint32_t aftercap_tile, uint32_t frames, uint32_t speed, uint32_t delay, uint32_t showflag, uint32_t flags)
        : subscreen_object(ssoMAGICGAUGE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), containernum(containernum), notlast_tile(notlast_tile), last_tile(last_tile), cap_tile(cap_tile), aftercap_tile(aftercap_tile), frames(frames), speed(speed), delay(delay), showflag(showflag), flags(flags)
    {}
    subscreen_object_magicgauge(PACKFILE *f, int &status);

    uint32_t containernum; // was: d1
    uint32_t notlast_tile; // was: d2
    uint32_t last_tile; // was: d3
    uint32_t cap_tile; // was: d4
    uint32_t aftercap_tile; // was: d5
    uint32_t frames; // was: d6 (not used)
    uint32_t speed; // was: d7 (not used)
    uint32_t delay; // was: d8 (not used)
    uint32_t showflag; // was: d9
    uint32_t flags; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Container Number:  " << containernum << std::endl
            << "Inside Tile:  " << notlast_tile << std::endl
            << "Last Tile:  " << last_tile << std::endl
            << "Cap Tile:  " << cap_tile << std::endl
            << "After Cap Tile:  " << aftercap_tile << std::endl
            << "Frames:  " << frames << std::endl
            << "Speed:  " << speed << std::endl
            << "Delay:  " << delay << std::endl
            << "Show Flag:  " << showflag << std::endl
            << "Flags:  " << flags << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 8; }
    virtual int sso_h() { return 8; }

    virtual subscreen_object *clone() { return new subscreen_object_magicgauge(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_magicgauge &othero = (const subscreen_object_magicgauge &)other;
        notlast_tile = othero.notlast_tile;
        last_tile = othero.last_tile;
        cap_tile = othero.cap_tile;
        aftercap_tile = othero.aftercap_tile;
        frames = othero.frames;
        speed = othero.speed;
        delay = othero.delay;
        showflag = othero.showflag;
        flags = othero.flags;
        w = othero.w;
        h = othero.h;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
    }
};

class subscreen_object_lifegauge : public subscreen_object
{
public:
    subscreen_object_lifegauge(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t containernum, uint32_t notlast_tile, uint32_t last_tile, uint32_t cap_tile, uint32_t aftercap_tile, uint32_t frames, uint32_t speed, uint32_t delay, uint32_t showflag, uint32_t flags)
        : subscreen_object(ssoLIFEGAUGE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), containernum(containernum), notlast_tile(notlast_tile), last_tile(last_tile), cap_tile(cap_tile), aftercap_tile(aftercap_tile), frames(frames), speed(speed), delay(delay), showflag(showflag), flags(flags)
    {}
    subscreen_object_lifegauge(PACKFILE *f, int &status);

    uint32_t containernum; // was: d1
    uint32_t notlast_tile; // was: d2
    uint32_t last_tile; // was: d3
    uint32_t cap_tile; // was: d4
    uint32_t aftercap_tile; // was: d5
    uint32_t frames; // was: d6 (not used)
    uint32_t speed; // was: d7 (not used)
    uint32_t delay; // was: d8 (not used)
    uint32_t showflag; // was: d9
    uint32_t flags; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Container Number:  " << containernum << std::endl
            << "Inside Tile:  " << notlast_tile << std::endl
            << "Last Tile:  " << last_tile << std::endl
            << "Cap Tile:  " << cap_tile << std::endl
            << "After Cap Tile:  " << aftercap_tile << std::endl
            << "Frames:  " << frames << std::endl
            << "Speed:  " << speed << std::endl
            << "Delay:  " << delay << std::endl
            << "Show Flag:  " << showflag << std::endl
            << "Flags:  " << flags << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);
    virtual int sso_w() { return 8; }
    virtual int sso_h() { return 8; }

    virtual subscreen_object *clone() { return new subscreen_object_lifegauge(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_lifegauge &othero = (const subscreen_object_lifegauge &)other;
        notlast_tile = othero.notlast_tile;
        last_tile = othero.last_tile;
        cap_tile = othero.cap_tile;
        aftercap_tile = othero.aftercap_tile;
        frames = othero.frames;
        speed = othero.speed;
        delay = othero.delay;
        showflag = othero.showflag;
        flags = othero.flags;
        w = othero.w;
        h = othero.h;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
    }
};

class subscreen_object_none : public subscreen_object
{
public:
    subscreen_object_none(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3)        
        : subscreen_object(ssoNONE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3)
    {}

    subscreen_object_none(PACKFILE *f, int &status);

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int sso_w() { return 5; }
    virtual int sso_h() { return 5; }

    virtual subscreen_object *clone() { return new subscreen_object_none(*this); }
};

class subscreen_object_triframe : public subscreen_object
{
public:
    subscreen_object_triframe(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t triframe_tile, uint32_t triframe_cset, uint32_t triforce_tile, uint32_t triforce_cset, bool showframe, bool showpieces, bool largepieces)        
        : subscreen_object(ssoTRIFRAME, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), triframe_tile(triframe_tile), triframe_cset(triframe_cset), triforce_tile(triforce_tile), triforce_cset(triforce_cset), showframe(showframe), showpieces(showpieces), largepieces(largepieces)
    {}

    subscreen_object_triframe(PACKFILE *f, int &status);

    uint32_t triframe_tile; // was: d1
    uint32_t triframe_cset; // was: d2
    uint32_t triforce_tile; // was: d3
    uint32_t triforce_cset; // was: d4
    bool showframe; // was: d5
    bool showpieces; // was: d6
    bool largepieces; // was: d7

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Frame Tile:  " << triframe_tile << std::endl
            << "Frame CSet:  " << triframe_cset << std::endl
            << "Triforce Tile:  " << triforce_tile << std::endl
            << "Triforce CSet:  " << triforce_cset << std::endl
            << "Show Frame:  " << showframe << std::endl
            << "Show Pieces:  " << showpieces << std::endl
            << "Large Pieces:  " << largepieces << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int sso_w() { return 16 * (largepieces ? 7 : 6); }
    virtual int sso_h() { return 16 * (largepieces ? 7 : 3); }

    virtual subscreen_object *clone() { return new subscreen_object_triframe(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_triframe &othero = (const subscreen_object_triframe &)other;
        triframe_tile = othero.triframe_tile;
        triframe_cset = othero.triframe_cset;
        triforce_tile = othero.triforce_tile;
        triforce_cset = othero.triforce_cset;
        showframe = othero.showframe;
        showpieces = othero.showpieces;
        largepieces = othero.largepieces;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
    }
};

class subscreen_object_largemap : public subscreen_object
{
public:
    subscreen_object_largemap(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        bool showmap, bool showlink, bool showrooms, bool large)
        : subscreen_object(ssoLARGEMAP, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), showmap(showmap), showlink(showlink), showrooms(showrooms), large(large)
    {}

    subscreen_object_largemap(PACKFILE *f, int &status);

    bool showmap; // was: d1
    bool showlink; // was: d2
    bool showrooms; // was: d3
    bool large; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Show Map:  " << showmap << std::endl
            << "Show Link:  " << showlink << std::endl
            << "Show Rooms:  " << showrooms << std::endl
            << "Large:  " << large << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int sso_w() { return 16 * (large ? 9 : 7); }
    virtual int sso_h() { return 80; }

    virtual subscreen_object *clone() { return new subscreen_object_largemap(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_largemap &othero = (const subscreen_object_largemap &)other;
        showmap = othero.showmap;
        showlink = othero.showlink;
        showrooms = othero.showrooms;
        large = othero.large;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
    }
};

class subscreen_object_bstime : public subscreen_object
{
public:
    subscreen_object_bstime(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle)
        : subscreen_object(ssoBSTIME, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle)
    {}

    subscreen_object_bstime(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    bool showlink; // was: d2
    bool showrooms; // was: d3
    bool large; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Show Link:  " << showlink << std::endl
            << "Show Rooms:  " << showrooms << std::endl
            << "Large:  " << large << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);   

    virtual int sso_y() {
        int ret = y;
        switch(textstyle)
        {
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_x()
    {
        int ret = x;
        switch(textstyle)
        {
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_h()
    {
        int ret = text_height(ss_font(fontnum));
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual int sso_w()
    {
        int ret = text_length(ss_font(fontnum), time_str_short2(game->get_time()));
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual subscreen_object *clone() { return new subscreen_object_bstime(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_bstime &othero = (const subscreen_object_bstime &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_minimap : public subscreen_object
{
public:
    subscreen_object_minimap(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        bool showmap, bool showlink, bool showcompass)
        : subscreen_object(ssoMINIMAP, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), showmap(showmap), showlink(showlink), showcompass(showcompass)
    {}

    subscreen_object_minimap(PACKFILE *f, int &status);

    bool showmap; // was: d1
    bool showlink; // was: d2
    bool showcompass; // was: d3

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Show Map:  " << showmap << std::endl
            << "Show Link:  " << showlink << std::endl
            << "Show Compass:  " << showcompass << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int sso_w() { return 80; }
    virtual int sso_h() { return 48; }

    virtual subscreen_object *clone() { return new subscreen_object_minimap(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_minimap &othero = (const subscreen_object_minimap &)other;
        showmap = othero.showmap;
        showlink = othero.showlink;
        showcompass = othero.showcompass;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_minimaptitle : public subscreen_object
{
public:
    subscreen_object_minimaptitle(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle, bool needmap)
        : subscreen_object(ssoMINIMAPTITLE, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle), needmap(needmap)
    {}

    subscreen_object_minimaptitle(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    bool needmap; // was: d4

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Need Map:  " << needmap << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    
    virtual int get_alignment() { return alignment; }
    virtual int sso_w() { return 80; }
    virtual int sso_h() { return 16; }

    virtual subscreen_object *clone() { return new subscreen_object_minimaptitle(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_minimaptitle &othero = (const subscreen_object_minimaptitle &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        needmap = othero.needmap;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_counters : public subscreen_object
{
public:
    subscreen_object_counters(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, bool usex, uint32_t textstyle, uint32_t digits, char idigit)
        : subscreen_object(ssoCOUNTERS, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), usex(usex), textstyle(textstyle), digits(digits), idigit(idigit)
    {}

    subscreen_object_counters(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    bool usex; // was: d2
    uint32_t textstyle; // was: d3
    uint32_t digits; // was: d4
    char idigit; // was: d5

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Use X:  " << usex << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Digits:  " << digits << std::endl
            << "Infinite Digit:  " << idigit << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);    

    virtual int sso_y() {
        int ret = y;
        switch(textstyle)
        {
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_x() {
        int ret = x;
        switch(textstyle)
        {
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_w() { 
        int ret = 32;
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual int sso_h() {
        int ret = 32;
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual subscreen_object *clone() { return new subscreen_object_counters(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_counters &othero = (const subscreen_object_counters &)other;
        fontnum = othero.fontnum;
        usex = othero.usex;
        textstyle = othero.textstyle;
        digits = othero.digits;
        idigit = othero.idigit;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_sstime : public subscreen_object
{
public:
    subscreen_object_sstime(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle)
        : subscreen_object(ssoSSTIME, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle)
    {}

    subscreen_object_sstime(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    bool showlink; // was: d2
    bool showrooms; // was: d3
    bool large; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Show Link:  " << showlink << std::endl
            << "Show Rooms:  " << showrooms << std::endl
            << "Large:  " << large << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);   
    
    virtual int get_alignment() { return sstaRIGHT; }
    
    virtual int sso_y() {
        int ret = y;
        switch(textstyle)
        {
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_x()
    {
        int ret = x;
        switch(textstyle)
        {
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_h()
    {
        int ret = text_height(ss_font(fontnum));
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual int sso_w()
    {
        int ret = text_length(ss_font(fontnum), time_str_med(game->get_time()));
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual subscreen_object *clone() { return new subscreen_object_sstime(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_sstime &othero = (const subscreen_object_sstime &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_time : public subscreen_object
{
public:
    subscreen_object_time(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle)
        : subscreen_object(ssoTIME, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle)
    {}

    subscreen_object_time(PACKFILE *f, int &status);

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    bool showlink; // was: d2
    bool showrooms; // was: d3
    bool large; // was: d10

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Show Link:  " << showlink << std::endl
            << "Show Rooms:  " << showrooms << std::endl
            << "Large:  " << large << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);   

    virtual int get_alignment() { return sstaRIGHT; }

    virtual int sso_y() {
        int ret = y;
        switch(textstyle)
        {
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_x()
    {
        int ret = x;
        switch(textstyle)
        {
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret-=1;
            break;
        }
        return ret;
    }

    virtual int sso_h()
    {
        int ret = text_height(ss_font(fontnum));
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual int sso_w()
    {
        int ret = text_length(ss_font(fontnum), time_str_med(game->get_time()));
        switch(textstyle)
        {
        case sstsSHADOW:
        case sstsSHADOWU:
        case sstsOUTLINE8:
        case sstsOUTLINEPLUS:
        case sstsOUTLINEX:
        case sstsSHADOWED:
        case sstsSHADOWEDU:
        case sstsOUTLINED8:
        case sstsOUTLINEDPLUS:
        case sstsOUTLINEDX:
            ret+=1;
            break;
        }
        return ret;
    }

    virtual subscreen_object *clone() { return new subscreen_object_time(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_time &othero = (const subscreen_object_time &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
    }
};

class subscreen_object_lifemeter : public subscreen_object
{
public:
    subscreen_object_lifemeter(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t tile, bool bsstyle, bool threerows)
        : subscreen_object(ssoLIFEMETER, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), tile(tile), bsstyle(bsstyle), threerows(threerows)
    {}

    subscreen_object_lifemeter(PACKFILE *f, int &status);

    uint32_t tile; // was: d1
    bool bsstyle; // was: d2
    bool threerows; // was: d3

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Tile:  " << tile << std::endl
            << "BS Style:  " << bsstyle << std::endl
            << "Three Rows:  " << threerows << std::endl;
        return ss.str();
    }

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);   

    virtual int sso_y() {
        return y + (bsstyle ? 0 : (threerows ? 8 : 16));
    }

    virtual int sso_h() {
        return threerows ? 24 : 16;
    }

    virtual int sso_w() { return 64; }

    virtual subscreen_object *clone() { return new subscreen_object_lifemeter(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_lifemeter &othero = (const subscreen_object_lifemeter &)other;
        bsstyle = othero.bsstyle;
        threerows = othero.threerows;
    }
};

class subscreen_object_magicmeter : public subscreen_object
{
public:
    subscreen_object_magicmeter(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3)
        : subscreen_object(ssoMAGICMETER, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3)
    {}

    subscreen_object_magicmeter(PACKFILE *f, int &status);

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);   

    virtual int sso_x() {
        return x-10;
    }

    virtual int sso_h() { return 8; }

    virtual int sso_w() { return 82; }

    virtual subscreen_object *clone() { return new subscreen_object_magicmeter(*this); }    
};

class subscreen_object_textbox : public subscreen_object
{
public:
    subscreen_object_textbox(byte pos, word x, word y, word w, word h, byte colortype1, short color1, byte colortype2, short color2, byte colortype3, short color3,
        uint32_t fontnum, uint32_t alignment, uint32_t textstyle, bool wword, uint32_t tabsize, const std::string &thetext)
        : subscreen_object(ssoTEXTBOX, pos, x, y, w, h, colortype1, color1, colortype2, color2, colortype3, color3), fontnum(fontnum), alignment(alignment), textstyle(textstyle), wword(wword), tabsize(tabsize), thetext(thetext)
    {}

    uint32_t fontnum; // was: d1
    uint32_t alignment; // was: d2
    uint32_t textstyle; // was: d3
    bool wword; // was: d4
    uint32_t tabsize; // was: d5

    std::string thetext; // was: dp1

    virtual std::string toStringExtra()
    {
        std::stringstream ss;
        ss << "Font ID:  " << fontnum << std::endl
            << "Alignment:  " << alignment << std::endl
            << "Text Style:  " << textstyle << std::endl
            << "Word Wrap:  " << wword << std::endl
            << "Tab Size:  " << tabsize << std::endl
            << "Text:  " << thetext << std::endl;
        return ss.str();
    }

    subscreen_object_textbox(PACKFILE *f, int &status);

    virtual void show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime);
    virtual bool serializeExtraData(PACKFILE *f);       

    virtual subscreen_object *clone() { return new subscreen_object_textbox(*this); }
    virtual void copyFromExtra(const subscreen_object &other)
    {
        const subscreen_object_textbox &othero = (const subscreen_object_textbox &)other;
        fontnum = othero.fontnum;
        alignment = othero.alignment;
        textstyle = othero.textstyle;
        wword = othero.wword;
        tabsize = othero.tabsize;
        colortype1 = othero.colortype1;
        color1 = othero.color1;
        colortype2 = othero.colortype2;
        color2 = othero.color2;
        colortype3 = othero.colortype3;
        color3 = othero.color3;
        w = othero.w;
        h = othero.h;
    }
};

class subscreen_group
{
public:
    subscreen_group() : ss_type(ssoNONE) {}
    ~subscreen_group();
    subscreen_group(const subscreen_group &other);
    subscreen_group &operator=(const subscreen_group &other);

    byte                                ss_type;
    std::string                         ss_name;
    std::vector<subscreen_object *>     ss_objects;    
};



/****  Subscr items code  ****/
extern subscreen_group custom_subscreen[MAXCUSTOMSUBSCREENS];
extern subscreen_group *current_subscreen_active;
extern subscreen_group *current_subscreen_passive;

extern item *Bitem, *Aitem;
extern ItemDefinitionRef   Bid, Aid;
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
const int fringe[8] = { 6,2,4,7,6,8,7,5 };
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
int stripspaces(char *source, char *target, int stop);
void put_passive_subscr(BITMAP *dest,miscQdata *misc,int x,int y,bool showtime,int pos2);
void puttriframe(BITMAP *dest, miscQdata *misc, int x, int y, int triframecolor, int numbercolor, int triframetile, int triframecset, int triforcetile, int triforcecset, bool showframe, bool showpieces, bool largepieces);
void puttriforce(BITMAP *dest, miscQdata *misc, int x, int y, int tile, int cset, int w, int h, int flip, bool overlay, bool trans, int trinum);
void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h);
void draw_block_flip(BITMAP *dest,int x,int y,int tile,int cset,int w,int h,int flip,bool overlay,bool trans);
void putBmap(BITMAP *dest, miscQdata *misc, int x, int y,bool showmap, bool showrooms, bool showlink, int roomcolor, int linkcolor, bool large);
void load_Sitems(miscQdata *misc);
void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int textstyle, int alignment, int color, int shadow, int bg);
void textprintf_styled_aligned_ex(BITMAP *bmp, const FONT *f, int x, int y, int textstyle, int alignment, int color, int shadow, int bg, const char *format, ...);
void update_subscreens(int dmap=-1);
void show_custom_subscreen(BITMAP *dest, miscQdata *misc, subscreen_group *css, int xofs, int yofs, bool showtime, int pos2);
FONT *ss_font(int fontnum);
void purge_blank_subscreen_objects(subscreen_group *tempss);
int subscreen_cset(miscQdata *misc,int c1, int c2);

void sso_bounding_box(BITMAP *bmp, subscreen_group *tempss, int index, int color);

const subscreen_group &get_default_subscreen_active(int style, int world);
const subscreen_group &get_default_subscreen_passive(int style, int magic);
const subscreen_group &get_z3_active_a();
const subscreen_group &get_z3_active_ab();
const subscreen_group &get_z3_passive_a();
const subscreen_group &get_z3_passive_ab();

#endif

/*** end of subscr.cc ***/

