#ifndef _NEW_SUBSCR_H_
#define _NEW_SUBSCR_H_

#include "base/ints.h"
#include "base/zc_alleg.h"
#include "base/general.h"
#include <string>
#include <vector>

struct SubscrPage;
struct PACKFILE;

//Old subscreen stuff
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
//

struct SubscrColorInfo
{
	byte type;
	int16_t color;
	int32_t get_cset() const;
	int32_t get_color() const;
	void load_old(subscreen_object const& old, int indx);
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
	static int32_t get_cset(byte c1, int16_t c2);
	static int32_t get_color(byte c1, int16_t c2);
};

struct SubscrMTInfo
{
	int32_t tilecrn;
	byte cset;
	
	int32_t tile() const;
	byte crn() const;
	void setTileCrn(int32_t tile, byte crn);
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};

enum //selection directions
{
	SEL_UP,
	SEL_LEFT,
	SEL_DOWN,
	SEL_RIGHT,
	SEL_VERIFY_LEFT,
	SEL_VERIFY_RIGHT
};

enum //old subscreen object types
{
	ssoNULL, ssoNONE,
	sso2X2FRAME, ssoTEXT, ssoLINE, ssoRECT, ssoBSTIME,
	ssoTIME, ssoSSTIME, ssoMAGICMETER, ssoLIFEMETER, ssoBUTTONITEM,
	ssoICON, ssoCOUNTER, ssoCOUNTERS, ssoMINIMAPTITLE, ssoMINIMAP,
	ssoLARGEMAP, ssoCLEAR, ssoCURRENTITEM, ssoITEM, ssoTRIFRAME, ssoMCGUFFIN,
	ssoTILEBLOCK, ssoMINITILE, ssoSELECTOR1, ssoSELECTOR2, ssoMAGICGAUGE,
	ssoLIFEGAUGE, ssoTEXTBOX, ssoCURRENTITEMTILE, ssoSELECTEDITEMTILE, 
	ssoCURRENTITEMTEXT, ssoCURRENTITEMNAME, ssoSELECTEDITEMNAME,
	ssoCURRENTITEMCLASSTEXT, ssoCURRENTITEMCLASSNAME, ssoSELECTEDITEMCLASSNAME,
	ssoMAX, ssoTEMPOLD
};

//Misc constants
enum //text styles
{
	sstsNORMAL, sstsSHADOW, sstsSHADOWU, sstsOUTLINE8, sstsOUTLINEPLUS,
	sstsOUTLINEX, sstsSHADOWED, sstsSHADOWEDU, sstsOUTLINED8, sstsOUTLINEDPLUS,
	sstsOUTLINEDX, sstsMAX
};
enum //subscreen fonts
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
enum // subscreen color types
{
	ssctSYSTEM=0xFE,
	ssctMISC=0xFF
};
enum // special colors
{
	ssctTEXT, ssctCAPTION, ssctOVERWBG, ssctDNGNBG, ssctDNGNFG,
	ssctCAVEFG, ssctBSDK, ssctBSGOAL, ssctCOMPASSLT, ssctCOMPASSDK,
	ssctSUBSCRBG, ssctSUBSCRSHADOW, ssctTRIFRAMECOLOR, ssctBMAPBG, ssctBMAPFG,
	ssctHERODOT, ssctMSGTEXT, ssctMAX
};
enum // special csets
{
	sscsTRIFORCECSET, sscsTRIFRAMECSET, sscsOVERWORLDMAPCSET, sscsDUNGEONMAPCSET,
	sscsBLUEFRAMECSET, sscsHCPIECESCSET, sscsSSVINECSET, sscsMAX
};
enum // special tiles
{
	ssmstSSVINETILE, ssmstMAGICMETER, ssmstMAX
};
enum // counter objects
{
	sscRUPEES, sscBOMBS, sscSBOMBS, sscARROWS,
	sscGENKEYMAGIC, sscGENKEYNOMAGIC, sscLEVKEYMAGIC, sscLEVKEYNOMAGIC,
	sscANYKEYMAGIC, sscANYKEYNOMAGIC, sscSCRIPT1, sscSCRIPT2,
	sscSCRIPT3, sscSCRIPT4, sscSCRIPT5, sscSCRIPT6,
	sscSCRIPT7, sscSCRIPT8, sscSCRIPT9, sscSCRIPT10,
	sscSCRIPT11, sscSCRIPT12, sscSCRIPT13, sscSCRIPT14,
	sscSCRIPT15, sscSCRIPT16, sscSCRIPT17, sscSCRIPT18,
	sscSCRIPT19, sscSCRIPT20, sscSCRIPT21, sscSCRIPT22,
	sscSCRIPT23, sscSCRIPT24, sscSCRIPT25, sscLIFE, sscMAGIC, sscMAXHP, sscMAXMP,
	sscSCRIPT26, sscSCRIPT27, sscSCRIPT28, sscSCRIPT29, sscSCRIPT30,
	sscSCRIPT31, sscSCRIPT32, sscSCRIPT33, sscSCRIPT34, sscSCRIPT35,
	sscSCRIPT36, sscSCRIPT37, sscSCRIPT38, sscSCRIPT39, sscSCRIPT40,
	sscSCRIPT41, sscSCRIPT42, sscSCRIPT43, sscSCRIPT44, sscSCRIPT45,
	sscSCRIPT46, sscSCRIPT47, sscSCRIPT48, sscSCRIPT49, sscSCRIPT50,
	sscSCRIPT51, sscSCRIPT52, sscSCRIPT53, sscSCRIPT54, sscSCRIPT55,
	sscSCRIPT56, sscSCRIPT57, sscSCRIPT58, sscSCRIPT59, sscSCRIPT60,
	sscSCRIPT61, sscSCRIPT62, sscSCRIPT63, sscSCRIPT64, sscSCRIPT65,
	sscSCRIPT66, sscSCRIPT67, sscSCRIPT68, sscSCRIPT69, sscSCRIPT70,
	sscSCRIPT71, sscSCRIPT72, sscSCRIPT73, sscSCRIPT74, sscSCRIPT75,
	sscSCRIPT76, sscSCRIPT77, sscSCRIPT78, sscSCRIPT79, sscSCRIPT80,
	sscSCRIPT81, sscSCRIPT82, sscSCRIPT83, sscSCRIPT84, sscSCRIPT85,
	sscSCRIPT86, sscSCRIPT87, sscSCRIPT88, sscSCRIPT89, sscSCRIPT90,
	sscSCRIPT91, sscSCRIPT92, sscSCRIPT93, sscSCRIPT94, sscSCRIPT95,
	sscSCRIPT96, sscSCRIPT97, sscSCRIPT98, sscSCRIPT99, sscSCRIPT100,
	sscMAX
};
enum //subscreen text alignment
{
	sstaLEFT, sstaCENTER, sstaRIGHT
};

#define SUBSCRFLAG_SELECTABLE  0x00000001

#define SUBSCRFLAG_GENERAL     0x0000FFFF

#define SUBSCRFLAG_SPEC_01     0x00010000
#define SUBSCRFLAG_SPEC_02     0x00020000
#define SUBSCRFLAG_SPEC_03     0x00040000
#define SUBSCRFLAG_SPEC_04     0x00080000
#define SUBSCRFLAG_SPEC_05     0x00100000
#define SUBSCRFLAG_SPEC_06     0x00200000
#define SUBSCRFLAG_SPEC_07     0x00400000
#define SUBSCRFLAG_SPEC_08     0x00800000
#define SUBSCRFLAG_SPEC_09     0x01000000
#define SUBSCRFLAG_SPEC_10     0x02000000
#define SUBSCRFLAG_SPEC_11     0x04000000
#define SUBSCRFLAG_SPEC_12     0x08000000
#define SUBSCRFLAG_SPEC_13     0x10000000
#define SUBSCRFLAG_SPEC_14     0x20000000
#define SUBSCRFLAG_SPEC_15     0x40000000
#define SUBSCRFLAG_SPEC_16     0x80000000
#define SUBSCRFLAG_SPECIFIC    0xFFFF0000
struct SubscrWidget
{
	byte posflags;
	int16_t x;
	int16_t y;
	word w;
	word h;
	dword flags;
	
	//if SUBSCRFLAG_SELECTABLE...
	int32_t pos;
	int32_t pos_up;
	int32_t pos_down;
	int32_t pos_left;
	int32_t pos_right;
	std::string override_text;
	
	byte gen_script_btns;
	word generic_script;
	//!TODO Generic InitD[]?
	
	SubscrWidget() = default;
	SubscrWidget(byte ty);
	SubscrWidget(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old);
	virtual int16_t getX() const; //Returns x in pixels
	virtual int16_t getY() const; //Returns y in pixels
	virtual word getW() const; //Returns width in pixels
	virtual word getH() const; //Returns height in pixels
	virtual int16_t getXOffs() const; //Returns any special x-offset
	virtual int16_t getYOffs() const; //Returns any special y-offset
	virtual byte getType() const;
	virtual int32_t getItemVal() const;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const;
	virtual bool visible(byte pos, bool showtime) const;
	virtual SubscrWidget* clone() const;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false);
	virtual int32_t write(PACKFILE *f) const;
	
	static SubscrWidget* fromOld(subscreen_object const& old);
	static SubscrWidget* readWidg(PACKFILE* f, word s_version);
	static SubscrWidget* newType(byte type);
protected:
	byte type;
	
	virtual int32_t read(PACKFILE *f, word s_version);
};

#define SUBSCR_2X2FR_TRANSP    SUBSCRFLAG_SPEC_01
#define SUBSCR_2X2FR_OVERLAY   SUBSCRFLAG_SPEC_02
struct SW_2x2Frame : public SubscrWidget
{
	SubscrColorInfo cs;
	int32_t tile;
	
	SW_2x2Frame() = default;
	SW_2x2Frame(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

struct SW_Text : public SubscrWidget
{
	int32_t fontid;
	std::string text;
	byte align, shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	
	SW_Text() = default;
	SW_Text(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual int16_t getXOffs() const override; //Returns any special x-offset
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_LINE_TRANSP     SUBSCRFLAG_SPEC_01
struct SW_Line : public SubscrWidget
{
	SubscrColorInfo c_line;
	
	SW_Line() = default;
	SW_Line(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_RECT_TRANSP     SUBSCRFLAG_SPEC_01
#define SUBSCR_RECT_FILLED     SUBSCRFLAG_SPEC_02
struct SW_Rect : public SubscrWidget
{
	SubscrColorInfo c_fill, c_outline;
	
	SW_Rect() = default;
	SW_Rect(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

struct SW_Time : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	
	SW_Time() = default;
	SW_Time(byte ty);
	SW_Time(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual int16_t getXOffs() const override; //Returns any special x-offset
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual bool visible(byte pos, bool showtime) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

struct SW_MagicMeter : public SubscrWidget
{
	SW_MagicMeter() = default;
	SW_MagicMeter(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_LIFEMET_BOT     SUBSCRFLAG_SPEC_01
struct SW_LifeMeter : public SubscrWidget
{
	byte rows;
	
	SW_LifeMeter() = default;
	SW_LifeMeter(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_BTNITM_TRANSP   SUBSCRFLAG_SPEC_01
struct SW_ButtonItem : public SubscrWidget
{
	byte btn;
	SW_ButtonItem() = default;
	SW_ButtonItem(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_COUNTER_SHOW0   SUBSCRFLAG_SPEC_01
#define SUBSCR_COUNTER_ONLYSEL SUBSCRFLAG_SPEC_02
struct SW_Counter : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	int32_t ctrs[3];
	byte digits;
	int32_t infitm;
	char infchar;
	
	SW_Counter() = default;
	SW_Counter(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual int16_t getXOffs() const override; //Returns any special x-offset
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_COUNTERS_USEX   SUBSCRFLAG_SPEC_01
struct SW_Counters : public SubscrWidget
{
	int32_t fontid;
	byte shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	byte digits;
	int32_t infitm;
	char infchar;
	
	SW_Counters() = default;
	SW_Counters(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_MMAPTIT_REQMAP  SUBSCRFLAG_SPEC_01
struct SW_MMapTitle : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	
	SW_MMapTitle() = default;
	SW_MMapTitle(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual int16_t getXOffs() const override; //Returns any special x-offset
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_MMAP_SHOWMAP    SUBSCRFLAG_SPEC_01
#define SUBSCR_MMAP_SHOWPLR    SUBSCRFLAG_SPEC_02
#define SUBSCR_MMAP_SHOWCMP    SUBSCRFLAG_SPEC_03
struct SW_MMap : public SubscrWidget
{
	SubscrColorInfo c_plr, c_cmp_blink, c_cmp_off;
	
	SW_MMap() = default;
	SW_MMap(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_LMAP_SHOWMAP    SUBSCRFLAG_SPEC_01
#define SUBSCR_LMAP_SHOWROOM   SUBSCRFLAG_SPEC_02
#define SUBSCR_LMAP_SHOWPLR    SUBSCRFLAG_SPEC_03
#define SUBSCR_LMAP_LARGE      SUBSCRFLAG_SPEC_04
struct SW_LMap : public SubscrWidget
{
	SubscrColorInfo c_room, c_plr;
	
	SW_LMap() = default;
	SW_LMap(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

struct SW_Clear : public SubscrWidget
{
	SubscrColorInfo c_bg;
	
	SW_Clear() = default;
	SW_Clear(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_CURITM_INVIS    SUBSCRFLAG_SPEC_01
#define SUBSCR_CURITM_NONEQP   SUBSCRFLAG_SPEC_02
struct SW_CurrentItem : public SubscrWidget
{
	int32_t iclass, iid;
	
	SW_CurrentItem() = default;
	SW_CurrentItem(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual int32_t getItemVal() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_TRIFR_SHOWFR    SUBSCRFLAG_SPEC_01
#define SUBSCR_TRIFR_SHOWPC    SUBSCRFLAG_SPEC_02
#define SUBSCR_TRIFR_LGPC      SUBSCRFLAG_SPEC_03
struct SW_TriFrame : public SubscrWidget
{
	int32_t frame_tile, piece_tile;
	byte frame_cset, piece_cset;
	SubscrColorInfo c_outline, c_number;
	
	SW_TriFrame() = default;
	SW_TriFrame(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_MCGUF_OVERLAY   SUBSCRFLAG_SPEC_01
#define SUBSCR_MCGUF_TRANSP    SUBSCRFLAG_SPEC_02
struct SW_McGuffin : public SubscrWidget
{
	int32_t tile, number;
	byte cset;
	SubscrColorInfo cs;
	
	SW_McGuffin() = default;
	SW_McGuffin(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_TILEBL_OVERLAY  SUBSCRFLAG_SPEC_01
#define SUBSCR_TILEBL_TRANSP   SUBSCRFLAG_SPEC_02
struct SW_TileBlock : public SubscrWidget
{
	int32_t tile;
	byte flip;
	SubscrColorInfo cs;
	
	SW_TileBlock() = default;
	SW_TileBlock(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_MINITL_OVERLAY  SUBSCRFLAG_SPEC_01
#define SUBSCR_MINITL_TRANSP   SUBSCRFLAG_SPEC_02
struct SW_MiniTile : public SubscrWidget
{
	int32_t tile, special_tile;
	byte crn, flip;
	SubscrColorInfo cs;
	
	SW_MiniTile() = default;
	SW_MiniTile(subscreen_object const& old);
	
	int32_t get_tile() const;
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_SELECTOR_TRANSP   SUBSCRFLAG_SPEC_01
#define SUBSCR_SELECTOR_LARGE    SUBSCRFLAG_SPEC_02
#define SUBSCR_SELECTOR_USEB     SUBSCRFLAG_SPEC_03
struct SW_Selector : public SubscrWidget
{
	SW_Selector() = default;
	SW_Selector(byte ty);
	SW_Selector(subscreen_object const& old);

	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_LGAUGE_MOD1     SUBSCRFLAG_SPEC_01
#define SUBSCR_LGAUGE_MOD2     SUBSCRFLAG_SPEC_02
#define SUBSCR_LGAUGE_MOD3     SUBSCRFLAG_SPEC_03
#define SUBSCR_LGAUGE_MOD4     SUBSCRFLAG_SPEC_04
#define SUBSCR_LGAUGE_UNQLAST  SUBSCRFLAG_SPEC_05
struct SW_LifeGaugePiece : public SubscrWidget
{
	SubscrMTInfo mts[4];
	word frames, speed, delay, container;
	
	SW_LifeGaugePiece() = default;
	SW_LifeGaugePiece(subscreen_object const& old);

	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_MGAUGE_MOD1     SUBSCRFLAG_SPEC_01
#define SUBSCR_MGAUGE_MOD2     SUBSCRFLAG_SPEC_02
#define SUBSCR_MGAUGE_MOD3     SUBSCRFLAG_SPEC_03
#define SUBSCR_MGAUGE_MOD4     SUBSCRFLAG_SPEC_04
#define SUBSCR_MGAUGE_UNQLAST  SUBSCRFLAG_SPEC_05
struct SW_MagicGaugePiece : public SubscrWidget
{
	SubscrMTInfo mts[4];
	word frames, speed, delay, container;
	int16_t showdrain = -1;
	
	SW_MagicGaugePiece() = default;
	SW_MagicGaugePiece(subscreen_object const& old);

	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_TEXTBOX_WORDWRAP     SUBSCRFLAG_SPEC_01
struct SW_TextBox : public SubscrWidget
{
	int32_t fontid;
	std::string text;
	byte align, shadtype, tabsize = 4;
	SubscrColorInfo c_text, c_shadow, c_bg;
	
	SW_TextBox() = default;
	SW_TextBox(subscreen_object const& old);

	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_SELTEXT_WORDWRAP     SUBSCRFLAG_SPEC_01
struct SW_SelectedText : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype, tabsize = 4;
	SubscrColorInfo c_text = {ssctMISC,0}, c_shadow, c_bg;
	
	SW_SelectedText() = default;
	SW_SelectedText(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

struct SubscrPage
{
	std::vector<SubscrWidget*> contents;
	int32_t cursor_pos, init_cursor_pos;
	
	void move_cursor(int dir, bool item_only = false);
	int32_t move_legacy(int dir, int startp, int fp=-1, int fp2=-1, int fp3=-1, bool equip_only=true, bool item_only=true);
	SubscrWidget* get_widg_pos(int32_t pos, bool sel_only = true);
	SubscrWidget* get_sel_widg();
	int32_t get_item_pos(int32_t pos, bool sel_only = true);
	int32_t get_sel_item();
	int32_t get_pos_of_item(int32_t itemid);
	
	void clear();
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
	
	SubscrPage() = default;
	~SubscrPage();
	SubscrPage& operator=(SubscrPage const& other);
	SubscrPage(const SubscrPage& other);
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};
struct ZCSubscreen
{
	std::vector<SubscrPage> pages;
	byte curpage, sub_type;
	std::string name;
	
	SubscrPage& cur_page();
	SubscrPage* get_page(byte ind);
	bool get_page_pos(int32_t itmid, byte& pg, byte& pos);
	int32_t get_item_pos(byte pos, byte pg);
	void clear();
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
	void load_old(subscreen_group const& g);
	void load_old(subscreen_object const* arr);
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};

bool new_widget_type(int ty);
#endif

