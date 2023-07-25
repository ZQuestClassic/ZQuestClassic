#ifndef _NEW_SUBSCR_H_
#define _NEW_SUBSCR_H_

#include "base/ints.h"
#include <string>

struct SubscrColorInfo
{
	byte type;
	int16_t color;
	int32_t get_cset();
	int32_t get_color();
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
	
	virtual bool load_old(subscreen_object const& old);
	virtual word getW() const; //Returns width in pixels
	virtual word getH() const; //Returns height in pixels
	virtual byte getType() const;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const;
	virtual bool visible(byte pos, bool showtime) const;
};

#define SUBSCR_2X2FR_TRANSP    SUBSCRFLAG_SPEC_01
#define SUBSCR_2X2FR_OVERLAY   SUBSCRFLAG_SPEC_02
struct SW_2x2Frame : public SubscrWidget
{
	SubscrColorInfo cs;
	int32_t tile;
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const;
};

struct SW_Text : public SubscrWidget
{
	int32_t fontid;
	std::string text;
	byte align;
	byte shadtype;
	SubscrColorInfo c_text, c_shadow, c_bg;
	virtual bool load_old(subscreen_object const& old) override;
	virtual word getW() const override; //Returns width in pixels
	virtual word getH() const override; //Returns height in pixels
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const;
};

#define SUBSCR_LINE_TRANSP     SUBSCRFLAG_SPEC_01
struct SW_Line : public SubscrWidget
{
	SubscrColorInfo c_line;
	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const;
};

#define SUBSCR_RECT_TRANSP     SUBSCRFLAG_SPEC_01
#define SUBSCR_RECT_FILLED     SUBSCRFLAG_SPEC_02
struct SW_Rect : public SubscrWidget
{
	SubscrColorInfo c_fill, c_outline;
	virtual bool load_old(subscreen_object const& old) override;
	virtual byte getType() const override;
	virtual void draw(BITMAP* dest, int32_t xofs, int32_t yofs) const;
};




struct SubscrPage
{
	std::vector<SubscrWidget> contents;
	
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime);
};

#endif

