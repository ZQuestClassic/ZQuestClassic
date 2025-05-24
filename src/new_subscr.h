#ifndef NEW_SUBSCR_H_
#define NEW_SUBSCR_H_

#include "base/ints.h"
#include "base/zc_alleg.h"
#include "base/general.h"
#include "tiles.h"
#include <string>
#include <vector>

struct SubscrPage;
struct PACKFILE;

#define NEG_OR_MASK(v,mask) (v < 0 ? v : (v&mask))
struct SubscrTransition;
struct ZCSubscreen;

extern SubscrTransition subscr_pg_transition;
extern int subscr_item_clk, subscr_pg_clk;
extern byte subscr_pg_from, subscr_pg_to;
extern bool subscr_itemless, subscr_pg_animating;

void subscrpg_clear_animation();
bool subscrpg_animate(byte from, byte to, SubscrTransition const& transition, ZCSubscreen& parent);

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
	int32_t get_int_cset() const;
	void set_int_cset(int32_t val);
	int32_t get_int_color() const;
	void set_int_color(int32_t val);
	void load_old(subscreen_object const& old, int indx);
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
	static int32_t get_cset(byte c1, int16_t c2);
	static int32_t get_color(byte c1, int16_t c2);
};

struct SubscrMTInfo
{
	int32_t mt_tile;
	byte cset, mt_crn;
	
	int32_t tile() const;
	byte crn() const;
	int32_t getTileCrn() const;
	void setTileCrn(int32_t tile, byte crn);
	void setTileCrn(int32_t tilecrn);
	void setTile(int32_t tile);
	void setCrn(byte crn);
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};

struct SubscrSelectorTileInfo
{
	word sw, sh;
	int32_t tile;
	byte cset, frames, speed, delay;
	
	void clear();
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};
#define SUBSCR_SELECTOR_NUMTILEINFO 2
struct SubscrSelectorInfo
{
	int16_t x,y,w,h;
	SubscrSelectorTileInfo tileinfo[2];
	
	void clear();
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
};

#define SUBSCR_TRANSITION_MAXARG 4
#define SUBSCR_TRANS_NOHIDESELECTOR  0x0001
#define SUBSCR_TRANS_NUMFLAGS 1
struct SubscrTransition
{
	byte type;
	byte tr_sfx;
	word flags;
	int32_t arg[SUBSCR_TRANSITION_MAXARG];
	
	void clear();
	bool draw(BITMAP* dest, BITMAP* p1, BITMAP* p2, int dx, int dy);
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
	static byte num_args(byte ty);
	static int32_t argScale(byte ty, byte ind);
};

enum //Transition types
{	//args in [], arg types int-based or listed in {}
	sstrINSTANT, //Occurs instantly, []
	sstrSLIDE, //Slide out, [dir4, spd{zfix}]
	sstrPIXEL, //Pixellate between, [flags(inv), duration, xofs, yofs]
	sstrMAX
};
#define TR_PIXELATE_INVERT 0x01

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

enum //new subscreen object types
{
	widgNULL, widgFRAME, widgTEXT, widgLINE, widgRECT,
	widgTIME, widgMMETER, widgLMETER, widgBTNITM, widgCOUNTER,
	widgOLDCTR, widgMMAPTITLE, widgMMAP, widgLMAP, widgBGCOLOR,
	widgITEMSLOT, widgMCGUFF_FRAME, widgMCGUFF, widgTILEBLOCK, widgMINITILE,
	widgSELECTOR, widgLGAUGE, widgMGAUGE, widgTEXTBOX, widgSELECTEDTEXT,
	widgMISCGAUGE, widgBTNCOUNTER, widgCOUNTERPERCBAR,
	widgMAX
};
extern const std::string subwidg_internal_names[widgMAX];
enum { sstACTIVE, sstPASSIVE, sstOVERLAY, sstMAX };
extern const std::string subscr_names[sstMAX];
extern const std::string subscr_infos[sstMAX];

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
#define NUM_SYS_COLORS 16
#define SUB_COLOR_TRANSPARENT (-34)
#define MIN_SUBSCR_COLOR (-ssctMAX-NUM_SYS_COLORS-1) //== SUB_COLOR_TRANSPARENT at the moment
#define MAX_SUBSCR_COLOR 255
enum // special csets
{
	sscsTRIFORCECSET, sscsTRIFRAMECSET, sscsOVERWORLDMAPCSET, sscsDUNGEONMAPCSET,
	sscsBLUEFRAMECSET, sscsHCPIECESCSET, sscsSSVINECSET, sscsMAX
};
enum // special tiles
{
	ssmstSSVINETILE, ssmstMAGICMETER, ssmstMAX
};
int old_ssc_to_new_ctr(int ssc);
enum // custom negative counters
{
	sscMIN = -18,
	sscBTNCTRA_1, sscBTNCTRB_1, sscBTNCTRX_1, sscBTNCTRY_1,
	sscBTNCTRA_0, sscBTNCTRB_0, sscBTNCTRX_0, sscBTNCTRY_0,
	sscGENKEYMAGIC, sscGENKEYNOMAGIC, sscLEVKEYMAGIC, sscLEVKEYNOMAGIC,
	sscANYKEYMAGIC, sscANYKEYNOMAGIC, sscMAXHP, sscMAXMP, sscNONE = -1
};
enum //subscreen text alignment
{
	sstaLEFT, sstaCENTER, sstaRIGHT, sstaMAX
};

enum //PGGOTO modes
{
	PGGOTO_NONE,
	PGGOTO_NEXT,
	PGGOTO_PREV,
	PGGOTO_TRG,
	PGGOTO_MAX
};

//when to display an element
#define sspUP                         0x01
#define sspDOWN                       0x02
#define sspSCROLLING                  0x04
#define sspNOMSGSTR                   0x08
#define sspNUM 4

#define SUBSCRFLAG_SELECTABLE         0x00000001
#define SUBSCRFLAG_PGGOTO_NOWRAP      0x00000002
#define SUBSCRFLAG_SELOVERRIDE        0x00000004
#define SUBSCRFLAG_GEN_COUNT 3

#define SUBSCRFLAG_SPEC_01            0x00000001
#define SUBSCRFLAG_SPEC_02            0x00000002
#define SUBSCRFLAG_SPEC_03            0x00000004
#define SUBSCRFLAG_SPEC_04            0x00000008
#define SUBSCRFLAG_SPEC_05            0x00000010
#define SUBSCRFLAG_SPEC_06            0x00000020
#define SUBSCRFLAG_SPEC_07            0x00000040
#define SUBSCRFLAG_SPEC_08            0x00000080
#define SUBSCRFLAG_SPEC_09            0x00000100
#define SUBSCRFLAG_SPEC_10            0x00000200
#define SUBSCRFLAG_SPEC_11            0x00000400
#define SUBSCRFLAG_SPEC_12            0x00000800
#define SUBSCRFLAG_SPEC_13            0x00001000
#define SUBSCRFLAG_SPEC_14            0x00002000
#define SUBSCRFLAG_SPEC_15            0x00004000
#define SUBSCRFLAG_SPEC_16            0x00008000
#define SUBSCRFLAG_SPEC_17            0x00010000
#define SUBSCRFLAG_SPEC_18            0x00020000
#define SUBSCRFLAG_SPEC_19            0x00040000
#define SUBSCRFLAG_SPEC_20            0x00080000
#define SUBSCRFLAG_SPEC_21            0x00100000
#define SUBSCRFLAG_SPEC_22            0x00200000
#define SUBSCRFLAG_SPEC_23            0x00400000
#define SUBSCRFLAG_SPEC_24            0x00800000
#define SUBSCRFLAG_SPEC_25            0x01000000
#define SUBSCRFLAG_SPEC_26            0x02000000
#define SUBSCRFLAG_SPEC_27            0x04000000
#define SUBSCRFLAG_SPEC_28            0x08000000
#define SUBSCRFLAG_SPEC_29            0x10000000
#define SUBSCRFLAG_SPEC_30            0x20000000
#define SUBSCRFLAG_SPEC_31            0x40000000
#define SUBSCRFLAG_SPEC_32            0x80000000

#define SUBSCRCOMPAT_FONT_RAND        0x01
struct SubscrWidget
{
	byte posflags;
	int16_t x;
	int16_t y;
	word w;
	word h;
	dword flags, genflags;
	
	std::string label;
	
	//if SUBSCRFLAG_SELECTABLE...
	
	//Selector position, and directionals
	int32_t pos;
	int32_t pos_up, pos_down, pos_left, pos_right;
	
	//Selector draw overrides
	SubscrSelectorInfo selector_override;
	
	std::string override_text; //Override the SW_SelectedText text
	
	//Script to run on pressing a button ..plus info
	word generic_script;
	int32_t generic_initd[8];
	byte gen_script_btns;
	
	//Page transition on pressing a button ..plus info
	byte pg_btns, pg_mode, pg_targ;
	SubscrTransition pg_trans;
	
	SubscrWidget() = default;
	SubscrWidget(byte ty);
	SubscrWidget(subscreen_object const& old);
	virtual ~SubscrWidget() = default;
	
	virtual bool load_old(subscreen_object const& old);
	virtual int16_t getX() const; //Returns x in pixels
	virtual int16_t getY() const; //Returns y in pixels
	virtual word getW() const; //Returns width in pixels
	virtual word getH() const; //Returns height in pixels
	virtual byte getType() const;
	virtual int32_t getItemVal() const;
	virtual int32_t getDisplayItem() const;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const;
	virtual bool visible(byte pos, bool showtime) const;
	virtual SubscrWidget* clone() const;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false);
	virtual int32_t write(PACKFILE *f) const;
	
	void check_btns(byte btnflgs, ZCSubscreen& parent) const;
	std::string getTypeName() const;
	
	void replay_rand_compat(byte pos) const;
	
	SubscrPage const* getParentPage() const;
	ZCSubscreen const* getParentSub() const;
protected:
	byte type;
	byte compat_flags;
	SubscrPage const* parentPage;
	
	virtual int32_t read(PACKFILE *f, word s_version);
	
	friend struct SubscrPage;
public:
	static SubscrWidget* fromOld(subscreen_object const& old);
	static SubscrWidget* readWidg(PACKFILE* f, word s_version);
	static SubscrWidget* newType(byte type);
	static byte numFlags(byte type);
	byte numFlags();
};

#define SUBSCR_2X2FR_TRANSP           SUBSCRFLAG_SPEC_01
#define SUBSCR_2X2FR_OVERLAY          SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_2X2FR          2
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

#define SUBSCR_NUMFLAG_TEXT           0
struct SW_Text : public SubscrWidget
{
	int32_t fontid;
	std::string text;
	byte align, shadtype;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg;
	
	SW_Text() = default;
	SW_Text(subscreen_object const& old);
	
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

#define SUBSCR_LINE_TRANSP            SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_LINE           1
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

#define SUBSCR_RECT_TRANSP            SUBSCRFLAG_SPEC_01
#define SUBSCR_RECT_FILLED            SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_RECT           2
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

#define SUBSCR_TIME_ALTSTR            SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_TIME           1
struct SW_Time : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg;
	
	SW_Time() = default;
	SW_Time(byte ty);
	SW_Time(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual bool visible(byte pos, bool showtime) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_NUMFLAG_MAGICMET       0
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

#define SUBSCR_LIFEMET_BOT            SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_LIFEMET        1
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

#define SUBSCR_BTNITM_TRANSP          SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_BTNITM         1
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

#define SUBSCR_COUNTER_SHOW0          SUBSCRFLAG_SPEC_01
#define SUBSCR_COUNTER_ONLYSEL        SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_COUNTER        2
struct SW_Counter : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg, c_text2 = {ssctMISC,ssctTEXT}, c_shadow2, c_bg2;
	int32_t ctrs[3] = {crNONE,crNONE,crNONE};
	byte mindigits, maxdigits = 3;
	int32_t infitm = -1;
	char infchar = 'A';
	
	SW_Counter() = default;
	SW_Counter(subscreen_object const& old);
	
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

#define SUBSCR_COUNTERS_USEX          SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_COUNTERS       1
struct SW_Counters : public SubscrWidget
{
	int32_t fontid;
	byte shadtype;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg;
	byte digits;
	int32_t infitm = -1;
	char infchar = 'A';
	
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

#define SUBSCR_BTNCOUNTER_SHOW0       SUBSCRFLAG_SPEC_01
#define SUBSCR_BTNCOUNTER_NOCOLLAPSE  SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_BTNCOUNTER     2
struct SW_BtnCounter : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg, c_text2 = {ssctMISC,ssctTEXT}, c_shadow2, c_bg2;
	byte mindigits, maxdigits = 3;
	char infchar = 'A';
	byte btn, costind;
	
	SW_BtnCounter() = default;
	
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

#define SUBSCR_MMAPTIT_REQMAP         SUBSCRFLAG_SPEC_01
#define SUBSCR_MMAPTIT_ONELINE        SUBSCRFLAG_SPEC_02
#define SUBSCR_MMAPTIT_WORDWRAP       SUBSCRFLAG_SPEC_03
#define SUBSCR_NUMFLAG_MMAPTIT        3
struct SW_MMapTitle : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype, tabsize = 4;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg;
	
	SW_MMapTitle() = default;
	SW_MMapTitle(subscreen_object const& old);
	
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
private:
	byte get_strs(char* line1, char* line2) const;
	void draw_old(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const;
	void draw_new(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const;
};

#define SUBSCR_MMAP_SHOWMAP           SUBSCRFLAG_SPEC_01
#define SUBSCR_MMAP_SHOWPLR           SUBSCRFLAG_SPEC_02
#define SUBSCR_MMAP_SHOWCMP           SUBSCRFLAG_SPEC_03
#define SUBSCR_NUMFLAG_MMAP           3
struct SW_MMap : public SubscrWidget
{
	SubscrColorInfo c_plr, c_cmp_blink, c_cmp_off;
	byte compass_litems = liTRIFORCE;
	
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

#define SUBSCR_LMAP_SHOWMAP           SUBSCRFLAG_SPEC_01
#define SUBSCR_LMAP_SHOWROOM          SUBSCRFLAG_SPEC_02
#define SUBSCR_LMAP_SHOWPLR           SUBSCRFLAG_SPEC_03
#define SUBSCR_LMAP_LARGE             SUBSCRFLAG_SPEC_04
#define SUBSCR_NUMFLAG_LMAP           4
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

#define SUBSCR_NUMFLAG_CLEAR          0
struct SW_Clear : public SubscrWidget
{
	SubscrColorInfo c_bg = {ssctMISC,ssctSUBSCRBG};
	
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

#define SUBSCR_CURITM_INVIS             SUBSCRFLAG_SPEC_01
#define SUBSCR_CURITM_NONEQP            SUBSCRFLAG_SPEC_02
#define SUBSCR_CURITM_IGNR_SP_SELTEXT   SUBSCRFLAG_SPEC_03
#define SUBSCR_CURITM_IGNR_SP_DISPLAY   SUBSCRFLAG_SPEC_04
#define SUBSCR_CURITM_NO_INTER_WO_ITEM  SUBSCRFLAG_SPEC_05
#define SUBSCR_CURITM_NO_INTER_WO_EQUIP SUBSCRFLAG_SPEC_06
#define SUBSCR_CURITM_NO_UNEQUIP        SUBSCRFLAG_SPEC_07
#define SUBSCR_NUMFLAG_CURITM           7
struct SW_ItemSlot : public SubscrWidget
{
	int32_t iclass, iid = -1;
	
	SW_ItemSlot() = default;
	SW_ItemSlot(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual int32_t getItemVal() const override;
	virtual int32_t getDisplayItem() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_TRIFR_SHOWFR             SUBSCRFLAG_SPEC_01
#define SUBSCR_TRIFR_SHOWPC             SUBSCRFLAG_SPEC_02
#define SUBSCR_TRIFR_LGPC               SUBSCRFLAG_SPEC_03
#define SUBSCR_NUMFLAG_TRIFR            3
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

#define SUBSCR_MCGUF_OVERLAY            SUBSCRFLAG_SPEC_01
#define SUBSCR_MCGUF_TRANSP             SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_MCGUF            2
struct SW_McGuffin : public SubscrWidget
{
	int32_t tile, number;
	byte flip;
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

#define SUBSCR_TILEBL_OVERLAY           SUBSCRFLAG_SPEC_01
#define SUBSCR_TILEBL_TRANSP            SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_TILEBL           2
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

#define SUBSCR_MINITL_OVERLAY           SUBSCRFLAG_SPEC_01
#define SUBSCR_MINITL_TRANSP            SUBSCRFLAG_SPEC_02
#define SUBSCR_NUMFLAG_MINITL           2
struct SW_MiniTile : public SubscrWidget
{
	int32_t tile, special_tile;
	byte crn, flip;
	SubscrColorInfo cs;
	
	SW_MiniTile() = default;
	SW_MiniTile(subscreen_object const& old);
	
	int32_t get_tile() const;
	int32_t get_int_tile() const;
	void set_int_tile(int32_t val);
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

#define SUBSCR_SELECTOR_TRANSP          SUBSCRFLAG_SPEC_01
#define SUBSCR_SELECTOR_LARGE           SUBSCRFLAG_SPEC_02
#define SUBSCR_SELECTOR_USEB            SUBSCRFLAG_SPEC_03
#define SUBSCR_NUMFLAG_SELECTOR         3
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

#define SUBSCR_GAUGE_MOD1              SUBSCRFLAG_SPEC_01
#define SUBSCR_GAUGE_MOD2              SUBSCRFLAG_SPEC_02
#define SUBSCR_GAUGE_MOD3              SUBSCRFLAG_SPEC_03
#define SUBSCR_GAUGE_MOD4              SUBSCRFLAG_SPEC_04
#define SUBSCR_GAUGE_UNQLAST           SUBSCRFLAG_SPEC_05
#define SUBSCR_GAUGE_FULLTILE          SUBSCRFLAG_SPEC_06
#define SUBSCR_GAUGE_ANIM_UNDER        SUBSCRFLAG_SPEC_07
#define SUBSCR_GAUGE_ANIM_OVER         SUBSCRFLAG_SPEC_08
#define SUBSCR_GAUGE_ANIM_PERCENT      SUBSCRFLAG_SPEC_09
#define SUBSCR_GAUGE_ANIM_SKIP         SUBSCRFLAG_SPEC_10
#define SUBSCR_GAUGE_INFITM_REQ        SUBSCRFLAG_SPEC_11
#define SUBSCR_GAUGE_INFITM_BAN        SUBSCRFLAG_SPEC_12
#define SUBSCR_GAUGE_GRID_RTOL         SUBSCRFLAG_SPEC_13
#define SUBSCR_GAUGE_GRID_TTOB         SUBSCRFLAG_SPEC_14
#define SUBSCR_GAUGE_GRID_COLUMN1ST    SUBSCRFLAG_SPEC_15
#define SUBSCR_GAUGE_GRID_SNAKE        SUBSCRFLAG_SPEC_16
#define SUBSCR_NUMFLAG_GAUGE           16
struct SW_GaugePiece : public SubscrWidget
{
	SubscrMTInfo mts[4];
	word frames = 1, speed = 1, delay, container;
	byte gauge_wid, gauge_hei, unit_per_frame;
	int8_t hspace, vspace;
	int16_t grid_xoff, grid_yoff;
	word anim_val;
	int16_t inf_item = -1;
	
	SW_GaugePiece() = default;
	virtual ~SW_GaugePiece() = default;
	
	virtual word get_ctr() const = 0;
	virtual word get_ctr_max() const = 0;
	virtual word get_per_container() const = 0;
	virtual bool infinite() const;
	
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
	virtual void draw_piece(BITMAP* dest, int dx, int dy, int container, int anim_offs) const;
};

#define SUBSCR_NUMFLAG_LGAUGE          SUBSCR_NUMFLAG_GAUGE
struct SW_LifeGaugePiece : public SW_GaugePiece
{
	SW_LifeGaugePiece() = default;
	SW_LifeGaugePiece(subscreen_object const& old);

	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual word get_ctr() const override;
	virtual word get_ctr_max() const override;
	virtual word get_per_container() const override;
	virtual bool infinite() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_NUMFLAG_MGAUGE          SUBSCR_NUMFLAG_GAUGE
struct SW_MagicGaugePiece : public SW_GaugePiece
{
	int16_t showdrain = -1;
	
	SW_MagicGaugePiece() = default;
	SW_MagicGaugePiece(subscreen_object const& old);

	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual word get_ctr() const override;
	virtual word get_ctr_max() const override;
	virtual word get_per_container() const override;
	virtual bool infinite() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_NUMFLAG_MISCGAUGE       SUBSCR_NUMFLAG_GAUGE
struct SW_MiscGaugePiece : public SW_GaugePiece
{
	int16_t counter;
	word per_container = 1;
	
	SW_MiscGaugePiece() = default;

	virtual bool load_old(subscreen_object const& old) override {return false;};
	virtual byte getType() const override;
	virtual word get_ctr() const override;
	virtual word get_ctr_max() const override;
	virtual word get_per_container() const override;
	virtual bool infinite() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define SUBSCR_TEXTBOX_WORDWRAP        SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_TEXTBOX         1
struct SW_TextBox : public SubscrWidget
{
	int32_t fontid;
	std::string text;
	byte align, shadtype, tabsize = 4;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg;
	
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

#define SUBSCR_SELTEXT_WORDWRAP        SUBSCRFLAG_SPEC_01
#define SUBSCR_NUMFLAG_SELTEXT         1
struct SW_SelectedText : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype, tabsize = 4;
	SubscrColorInfo c_text = {ssctMISC,ssctTEXT}, c_shadow, c_bg;
	
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

#define SUBSCR_COUNTERPERCBAR_TRANSP   SUBSCRFLAG_SPEC_01
#define SUBSCR_COUNTERPERCBAR_VERTICAL SUBSCRFLAG_SPEC_02
#define SUBSCR_COUNTERPERCBAR_INVERT   SUBSCRFLAG_SPEC_03
#define SUBSCR_NUMFLAG_COUNTERPERCBAR  3
struct SW_CounterPercentBar : public SubscrWidget
{
	int16_t counter;
	SubscrColorInfo c_fill, c_bg;
	SW_CounterPercentBar() = default;
	SW_CounterPercentBar(byte ty);
	
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const override;
	virtual SubscrWidget* clone() const override;
	virtual bool copy_prop(SubscrWidget const* src, bool all = false) override;
	virtual int32_t write(PACKFILE *f) const override;
protected:
	virtual int32_t read(PACKFILE *f, word s_version) override;
};

#define MAX_SUBSCR_PAGES 255
struct SubscrPage
{
	byte cursor_pos;
	
	void move_cursor(int dir, bool item_only = false);
	int32_t movepos_legacy(int dir, word startp, word fp = 255, word fp2 = 255, word fp3 = 255, bool equip_only=true, bool item_only=true, bool stay_on_page = false);
	void move_legacy(int dir, bool equip_only=true, bool item_only=true);
	SubscrWidget* get_widg_pos(byte pos, bool item_only) const;
	SubscrWidget* get_sel_widg() const;
	int32_t get_item_pos(byte pos, bool item_only = true);
	int32_t get_sel_item(bool display = false);
	int32_t get_pos_of_item(int32_t itemid);
	int32_t find_label_index(std::string const& lbl) const;
	int32_t widget_index(SubscrWidget* widg) const;
	
	SubscrWidget* get_widget(int indx);
	void delete_widg(word ind);
	//add_widg?
	void swap_widg(word ind1, word ind2);
	void clear();
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
	void swap(SubscrPage& other);
	
	SubscrPage() = default;
	~SubscrPage();
	SubscrPage& operator=(SubscrPage const& other);
	SubscrPage(const SubscrPage& other);
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
	
	word getIndex() const;
	void setParent(ZCSubscreen const* newparent);
	ZCSubscreen const* getParent() const;
	
	void push_back(SubscrWidget* widg);
	size_t size() const;
	bool empty() const;
	SubscrWidget* at(size_t ind);
	SubscrWidget* const& operator[](size_t ind) const;

	std::vector<SubscrWidget*> contents;

private:
	word index;
	ZCSubscreen const* parent;
	
	void force_update();
	
	friend struct ZCSubscreen;
};
#define SUBFLAG_ACT_NOPAGEWRAP   0x00000001
#define SUBFLAG_ACT_OVERRIDESEL  0x00000002
struct ZCSubscreen
{
	std::vector<SubscrPage> pages;
	byte curpage, sub_type;
	std::string name;
	dword flags;
	
	word def_btns[4]={255,255,255,255};
	
	byte btn_left, btn_right;
	SubscrTransition trans_left, trans_right;
	
	SubscrSelectorInfo selector_setting;
	
	word script;
	int32_t initd[8];
	
	SubscrPage& cur_page();
	SubscrPage* get_page(byte ind);
	bool get_page_pos(int32_t itmid, word& pgpos);
	int32_t get_item_pos(word pgpos);
	void delete_page(byte ind);
	bool add_page(byte ind);
	void swap_pages(byte ind1, byte ind2);
	void clear();
	void copy_settings(const ZCSubscreen& src, bool all = false);
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
	void load_old(subscreen_group const& g);
	void load_old(subscreen_object const* arr);
	
	ZCSubscreen() = default;
	ZCSubscreen(ZCSubscreen const& other);
	ZCSubscreen& operator=(ZCSubscreen const& other);
	
	int32_t read(PACKFILE *f, word s_version);
	int32_t write(PACKFILE *f) const;
	
	void check_btns(byte btnflgs);
	void page_change(byte mode, byte targ, SubscrTransition const& trans, bool nowrap);
private:
	bool wrap_pg(int& pg, bool nowrap);
};
#endif

