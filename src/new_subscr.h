#ifndef _NEW_SUBSCR_H_
#define _NEW_SUBSCR_H_

#include "base/ints.h"
#include "base/zc_alleg.h"
#include "base/general.h"
#include <string>
#include <vector>

struct subscreen_object;

struct SubscrColorInfo
{
	byte type;
	int16_t color;
	int32_t get_cset() const;
	int32_t get_color() const;
	void load_old(subscreen_object const& old, int indx);
};

enum
{
	ssoNULL, ssoNONE,
	sso2X2FRAME, ssoTEXT, ssoLINE, ssoRECT, ssoBSTIME,
	ssoTIME, ssoSSTIME, ssoMAGICMETER, ssoLIFEMETER, ssoBUTTONITEM,
	ssoICON, ssoCOUNTER, ssoCOUNTERS, ssoMINIMAPTITLE, ssoMINIMAP,
	ssoLARGEMAP, ssoCLEAR, ssoCURRENTITEM, ssoITEM, ssoTRIFRAME, ssoTRIFORCE,
	ssoTILEBLOCK, ssoMINITILE, ssoSELECTOR1, ssoSELECTOR2, ssoMAGICGAUGE,
	ssoLIFEGAUGE, ssoTEXTBOX, ssoCURRENTITEMTILE, ssoSELECTEDITEMTILE, 
	ssoCURRENTITEMTEXT, ssoCURRENTITEMNAME, ssoSELECTEDITEMNAME,
	ssoCURRENTITEMCLASSTEXT, ssoCURRENTITEMCLASSNAME, ssoSELECTEDITEMCLASSNAME,
	ssoMAX
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
	SubscrWidget(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old);
	virtual int16_t getX() const; //Returns x in pixels
	virtual int16_t getY() const; //Returns y in pixels
	virtual word getW() const; //Returns width in pixels
	virtual word getH() const; //Returns height in pixels
	virtual int16_t getXOffs() const; //Returns any special x-offset
	virtual int16_t getYOffs() const; //Returns any special y-offset
	virtual byte getType() const;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const;
	virtual bool visible(byte pos, bool showtime) const;
	
	static SubscrWidget fromOld(subscreen_object const& old);
protected:
	byte type;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
};

#define SUBSCR_LINE_TRANSP     SUBSCRFLAG_SPEC_01
struct SW_Line : public SubscrWidget
{
	SubscrColorInfo c_line;
	
	SW_Line() = default;
	SW_Line(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
};

struct SW_Time : public SubscrWidget
{
	int32_t fontid;
	byte align, shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	
	SW_Time() = default;
	SW_Time(subscreen_object const& old);
	
	virtual bool load_old(subscreen_object const& old) override;
	virtual int16_t getX() const override; //Returns x in pixels
	virtual int16_t getY() const override; //Returns y in pixels
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual int16_t getXOffs() const override; //Returns any special x-offset
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
	virtual bool visible(byte pos, bool showtime) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
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
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const override;
};


struct SubscrPage
{
	std::vector<SubscrWidget> contents;
	
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
};
struct ZCSubscreen
{
	std::vector<SubscrPage> pages;
	size_t curpage;
	
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
};

#endif

