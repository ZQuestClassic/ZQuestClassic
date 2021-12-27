#include "ffc_editor.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"

extern int32_t Combo, CSet;
static int32_t tCSet;
static bool edited = false;
void call_ffc_dialog(mapscr* scr, int32_t ffcombo)
{
	tCSet = CSet;
	edited = false;
	FFCDialog(scr,ffcombo).show();
	if(edited) CSet = tCSet;
}

ffdata::ffdata(mapscr const* scr, int32_t ind)
{
	clear();
	load(scr, ind);
}

ffdata::ffdata()
{
	clear();
}

void ffdata::clear()
{
	memset(this, 0, sizeof(ffdata));
}
void ffdata::load(mapscr const* scr, int32_t ind)
{
	if(unsigned(ind)>31) return;
	x = scr->ffx[ind];
	y = scr->ffy[ind];
	dx = scr->ffxdelta[ind];
	dy = scr->ffydelta[ind];
	ax = scr->ffxdelta2[ind];
	ay = scr->ffydelta2[ind];
	data = scr->ffdata[ind];
	cset = scr->ffcset[ind];
	delay = scr->ffdelay[ind];
	flags = scr->ffflags[ind];
	link = scr->fflink[ind];
	twid = scr->ffTileWidth(ind);
	thei = scr->ffTileHeight(ind);
	fwid = scr->ffEffectWidth(ind);
	fhei = scr->ffEffectHeight(ind);
	script = scr->ffscript[ind];
	for(auto q = 0; q < 2; ++q)
		inita[q] = scr->inita[ind][q];
	for(auto q = 0; q < 8; ++q)
		initd[q] = scr->initd[ind][q];
}
void ffdata::save(mapscr* scr, int32_t ind)
{
	if(unsigned(ind)>31) return;
	scr->ffx[ind] = x;
	scr->ffy[ind] = y;
	scr->ffxdelta[ind] = dx;
	scr->ffydelta[ind] = dy;
	scr->ffxdelta2[ind] = ax;
	scr->ffydelta2[ind] = ay;
	scr->ffdata[ind] = data;
	scr->ffcset[ind] = cset;
	scr->ffdelay[ind] = delay;
	scr->ffflags[ind] = flags;
	scr->fflink[ind] = link;
	scr->ffTileWidth(ind, twid);
	scr->ffTileHeight(ind, thei);
	scr->ffEffectWidth(ind, fwid);
	scr->ffEffectHeight(ind, fhei);
	scr->ffscript[ind] = script;
	for(auto q = 0; q < 2; ++q)
		scr->inita[ind][q] = inita[q];
	for(auto q = 0; q < 8; ++q)
		scr->initd[ind][q] = initd[q];
}

FFCDialog::FFCDialog(mapscr* scr, int32_t ffind) :
	thescr(scr), ffind(ffind), ffc(scr, ffind),
	list_link(GUI::ListData::numbers(true, 1, 32))
{}

//{ Macros
#define SWAPFIELD(str, mem, lb, hb) \
Label(text = str, hAlign = 1.0), \
TextField( \
	type = GUI::TextField::type::SWAP_ZSINT, \
	low = lb, high = hb, val = mem, \
	leftPadding = 0_px, fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		mem = val; \
	})
	
#define SWAPFIELDS(str, mem, lb, hb) \
Label(text = str, hAlign = 1.0), \
TextField( \
	type = GUI::TextField::type::SWAP_SSHORT, \
	val = int32_t(mem), low = lb, high = hb, \
	leftPadding = 0_px, fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		mem = (int16_t)val; \
	})
	
#define SWAPFIELDB(str, mem, lb, hb) \
Label(text = str, hAlign = 1.0), \
TextField( \
	type = GUI::TextField::type::SWAP_BYTE, \
	val = int32_t(mem), low = lb, high = hb, \
	leftPadding = 0_px, fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		mem = (byte)val; \
	})
//}

static size_t ffctab = 0;
std::shared_ptr<GUI::Widget> FFCDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	return Window(
		title = "FFC Editor",
		onEnter = message::OK,
		onClose = message::CANCEL,
		shortcuts={
			PlusPad=message::PLUSCS,
			Equals=message::PLUSCS,
			MinusPad=message::MINUSCS,
			Minus=message::MINUSCS
		},
		Column(
			TabPanel(
				ptr = &ffctab,
				TabRef(name = "Data", Rows<4>(
					Label(text = "Link to:", hAlign = 1.0),
					DropDownList(data = list_link,
						fitParent = true,
						selectedValue = ffc.link,
						onSelectFunc = [&](int32_t val)
						{
							ffc.link = (byte)val;
						}
					),
					cmbsw = SelComboSwatch(
						combo = ffc.data,
						cset = ffc.cset,
						rowSpan = 2, colSpan = 2,
						showvals = false,
						onSelectFunc = [&](int32_t cmb, int32_t c)
						{
							ffc.data = cmb;
							ffc.cset = c;
							tCSet = c;
						}
					),
					SWAPFIELD("X Pos:", ffc.x, -320000, 2880000),
					SWAPFIELD("Y Pos:", ffc.y, -320000, 2880000),
					SWAPFIELDB("Combo W:", ffc.fwid, 1, 64),
					SWAPFIELD("X Speed:", ffc.dx, -1280000, 1280000),
					SWAPFIELDB("Combo H:", ffc.fhei, 1, 64),
					SWAPFIELD("Y Speed:", ffc.dy, -1280000, 1280000),
					SWAPFIELDB("Tile W:", ffc.twid, 1, 4),
					SWAPFIELD("X Accel:", ffc.ax, -1280000, 1280000),
					SWAPFIELDB("Tile H:", ffc.thei, 1, 4),
					SWAPFIELD("Y Accel:", ffc.ay, -1280000, 1280000),
					SWAPFIELDS("A. Delay:", ffc.delay, 0, 9999)
				)),
				TabRef(name = "Flags", DummyWidget()),
				TabRef(name = "Script", DummyWidget())
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
}

bool FFCDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			ffc.save(thescr, ffind);
			saved = false;
			edited = true;
			[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
		
		case message::MINUSCS:
			if(ffctab) break;
			tCSet = (tCSet+11)%12;
			cmbsw->setCSet(tCSet);
			break;
		case message::PLUSCS:
			if(ffctab) break;
			tCSet = (tCSet+1)%12;
			cmbsw->setCSet(tCSet);
			break;
	}
	return false;
}

