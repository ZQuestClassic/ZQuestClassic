#include "ffc_editor.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "zquest.h"
#include "zq_class.h"
#include "alert.h"
#include "zc_list_data.h"

#define SWAP_MAX 2147483647
#define SWAP_MIN (-2147483647-1)

extern script_data *ffscripts[NUMSCRIPTFFC];
extern int32_t Combo, CSet;
static int32_t tCSet;
static bool edited = false;
bool call_ffc_dialog(int32_t ffcombo, mapscr* scr)
{
	if(!scr)
		scr = Map.CurrScr();
	tCSet = CSet;
	edited = false;
	FFCDialog(scr,ffcombo).show();
	//if(edited) CSet = tCSet;
	return edited;
}
bool call_ffc_dialog(int32_t ffcombo, ffdata const& init, mapscr* scr)
{
	if(!scr)
		scr = Map.CurrScr();
	tCSet = CSet;
	edited = false;
	FFCDialog(scr,ffcombo,init).show();
	if(edited) CSet = tCSet;
	return edited;
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
	fwid = fhei = 15;
}
void ffdata::load(mapscr const* scr, int32_t ind)
{
	if(unsigned(ind)>MAXFFCS-1) return;
	x = scr->ffcs[ind].x.getZLong();
	y = scr->ffcs[ind].y.getZLong();
	dx = scr->ffcs[ind].vx.getZLong();
	dy = scr->ffcs[ind].vy.getZLong();
	ax = scr->ffcs[ind].ax.getZLong();
	ay = scr->ffcs[ind].ay.getZLong();
	data = scr->ffcs[ind].getData();
	cset = scr->ffcs[ind].cset;
	delay = scr->ffcs[ind].delay;
	flags = scr->ffcs[ind].flags;
	link = scr->ffcs[ind].link;
	twid = scr->ffTileWidth(ind)-1;
	thei = scr->ffTileHeight(ind)-1;
	fwid = scr->ffEffectWidth(ind)-1;
	fhei = scr->ffEffectHeight(ind)-1;
	script = scr->ffcs[ind].script;
	for(auto q = 0; q < 2; ++q)
		inita[q] = scr->ffcs[ind].inita[q];
	for(auto q = 0; q < 8; ++q)
		initd[q] = scr->ffcs[ind].initd[q];
}
void ffdata::save(mapscr* scr, int32_t ind)
{
	if(unsigned(ind)>MAXFFCS-1) return;
	scr->ffcs[ind].x = zslongToFix(x);
	scr->ffcs[ind].y = zslongToFix(y);
	scr->ffcs[ind].vx = zslongToFix(dx);
	scr->ffcs[ind].vy = zslongToFix(dy);
	scr->ffcs[ind].ax = zslongToFix(ax);
	scr->ffcs[ind].ay = zslongToFix(ay);
	scr->ffcs[ind].setData(data);
	scr->ffcs[ind].cset = cset;
	scr->ffcs[ind].delay = delay;
	scr->ffcs[ind].flags = flags;
	scr->ffcs[ind].link = link;
	scr->ffTileWidth(ind, twid+1);
	scr->ffTileHeight(ind, thei+1);
	scr->ffEffectWidth(ind, fwid+1);
	scr->ffEffectHeight(ind, fhei+1);
	scr->ffcs[ind].script = script;
	for(auto q = 0; q < 2; ++q)
		scr->ffcs[ind].inita[q] = inita[q];
	for(auto q = 0; q < 8; ++q)
		scr->ffcs[ind].initd[q] = initd[q];
	scr->ffcs[ind].updateSolid();
}

ffdata& ffdata::operator=(ffdata const& other)
{
	x = other.x;
	y = other.y;
	dx = other.dx;
	dy = other.dy;
	ax = other.ax;
	ay = other.ay;
	data = other.data;
	cset = other.cset;
	delay = other.delay;
	flags = other.flags;
	link = other.link;
	twid = other.twid;
	thei = other.thei;
	fwid = other.fwid;
	fhei = other.fhei;
	script = other.script;
	for(auto q = 0; q < 8; ++q)
		initd[q] = other.initd[q];
	for(auto q = 0; q < 2; ++q)
		inita[q] = other.inita[q];
	return *this;
}

FFCDialog::FFCDialog(mapscr* scr, int32_t ffind) :
	thescr(scr), ffind(ffind),
	list_link(GUI::ListData::numbers(true, 1, MAXFFCS)),
	list_ffcscript(GUI::ZCListData::ffc_script())
{
	ffc.load(scr, ffind);
}

FFCDialog::FFCDialog(mapscr* scr, int32_t ffind, ffdata const& init) :
	FFCDialog(scr, ffind)
{
	ffc = init;
}

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
	low = lb, high = hb, val = int32_t(0)+mem, \
	leftPadding = 0_px, fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		mem = (int16_t)val; \
	})
	
#define SWAPFIELDB(str, mem, lb, hb, offs) \
Label(text = str, hAlign = 1.0), \
TextField( \
	type = GUI::TextField::type::SWAP_BYTE, \
	low = lb, high = hb, val = mem+offs, \
	leftPadding = 0_px, fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		mem = byte(val)-offs; \
	})

#define CHECKB(str, fl, inf) \
Row( \
	padding = 0_px, hAlign = 0.0, \
	Button(forceFitH = true, text = "?", \
		disabled = !inf[0], padding = 0_px, \
		fitParent = true, onPressFunc = [&]() \
		{ \
			InfoDialog("Flag Info",inf).show(); \
		}), \
	Checkbox( \
		hAlign = 0.0, \
		checked = (ffc.flags & fl), \
		text = str, \
		onToggleFunc = [&](bool state) \
		{ \
			SETFLAG(ffc.flags,fl,state); \
		} \
	) \
)
//}

std::shared_ptr<GUI::Widget> FFCDialog::FFC_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_initd[index]).show();
			}),
		tf_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = ffc.initd[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				ffc.initd[index] = val;
			})
	);
}

void FFCDialog::refreshScript()
{
	std::string label[8], help[8];
	for(auto q = 0; q < 8; ++q)
	{
		label[q] = "InitD["+std::to_string(q)+"]";
	}
	if(ffc.script)
	{
		zasm_meta const& meta = ffscripts[ffc.script]->meta;
		for(size_t q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				label[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				help[q] = meta.initd_help[q];
		}
		
		for(auto q = 0; q < 8; ++q)
		{
			if(unsigned(meta.initd_type[q]) < nswapMAX)
				tf_initd[q]->setSwapType(meta.initd_type[q]);
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
		{
			tf_initd[q]->setSwapType(nswapDEC);
		}
	}
	for(auto q = 0; q < 8; ++q)
	{
		l_initds[q]->setText(label[q]);
		h_initd[q] = help[q];
		ib_initds[q]->setDisabled(help[q].empty());
	}
}

static size_t ffctab = 0;
std::shared_ptr<GUI::Widget> FFCDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	window = Window(
		title = "FFC Editor ("+std::to_string(ffind+1)+")",
		info = "Edit an FFC, setting up its' combo, flags, and script data.\n"
			"Hotkeys: -/+: Change CSet",
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
						// showvals = false,
						onSelectFunc = [&](int32_t cmb, int32_t c)
						{
							ffc.data = cmb;
							ffc.cset = c;
							tCSet = c;
						}
					),
					// DummyWidget(),
					SWAPFIELD("X Pos:", ffc.x, SWAP_MIN, SWAP_MAX),
					// DummyWidget(),
					// DummyWidget(),
					SWAPFIELD("Y Pos:", ffc.y, SWAP_MIN, SWAP_MAX),
					SWAPFIELDB("Combo W:", ffc.fwid, 1, 64, 1),
					SWAPFIELD("X Speed:", ffc.dx, SWAP_MIN, SWAP_MAX),
					SWAPFIELDB("Combo H:", ffc.fhei, 1, 64, 1),
					SWAPFIELD("Y Speed:", ffc.dy, SWAP_MIN, SWAP_MAX),
					SWAPFIELDB("Tile W:", ffc.twid, 1, 4, 1),
					SWAPFIELD("X Accel:", ffc.ax, SWAP_MIN, SWAP_MAX),
					SWAPFIELDB("Tile H:", ffc.thei, 1, 4, 1),
					SWAPFIELD("Y Accel:", ffc.ay, SWAP_MIN, SWAP_MAX),
					SWAPFIELDS("A. Delay:", ffc.delay, 0, 9999)
				)),
				TabRef(name = "Flags", Columns<12>(
					//Standard Fl 1:
					Label(text = "Standard Flags", colSpan = 2),
					CHECKB("Draw Over", ffOVERLAY, "The FFC draws on a higher draw layer than it would otherwise if"
						" enabled."),
					CHECKB("Translucent", ffTRANS, "The FFC draws transparently"),
					CHECKB("Is a Changer (Invisible, Ethereal)", ffCHANGER, "If enabled, this FFC is a changer."
						" Changers are both invisible (not drawn) and ethereal (have no effect)."
						" When a moving FFC collides with a changer, the changer will change some properties of"
						" the colliding FFC. This can be used to make FFCs that move in patterns and such."),
					CHECKB("Run Script at Screen Init", ffPRELOAD, "The script attached to this FFC runs during the"
						" screen's initialization, before scrolling to the screen begins. This is useful for scripts"
						" that want to, for instance, modify combos on the screen and have those changes visible"
						" as you scroll onto the screen."),
					CHECKB("Only Visible to Lens", ffLENSVIS, "If this is enabled, the FFC will not be drawn when"
						" the lens is not active. Checking 'Invisible to Lens' as well will make the FFC entirely"
						" invisible."),
					CHECKB("Script Restarts when Carried Over", ffSCRIPTRESET, "If enabled, and 'Carry Over'"
						" is enabled, then when the FFC is carried over to a new screen, the script starts over."),
					CHECKB("Active during Item Holdup", ffIGNOREHOLDUP, "If disabled, the FFC will pause all activity"
						" during item holdup animations. Does not pause draws."),
					//CH fl 1:
					Label(text = "Changer-Specific Flags", colSpan = 2),
					CHECKB("Swap w/ Next FFC", ffSWAPNEXT, "If enabled, FFCs that collide with this changer will:\n"
						"Swap its' combo, cset, delay, link, movement, effect width/height, and flags with the"
						" next combo in the list after it."),
					CHECKB("Become Next Combo", ffCHANGENEXT, "If enabled, FFCs that collide with this changer will:\n"
						"Change its' combo to the next combo in the combo list."),
					CHECKB("Become This Combo+CSet", ffCHANGETHIS, "If enabled, FFCs that collide with this changer will:\n"
						"Change its' combo and cset to the combo and cset of this changer."),
					//Standard fl 2:
					CHECKB("Ethereal", ffETHEREAL, "If enabled, the FFC has no 'effect', and is not checked"
						" for any combo type related effects."),
					CHECKB("Ignore Changers", ffIGNORECHANGER, "If enabled, this FFC will ignore collisions with"
						" Changer FFCs."),
					CHECKB("Solid (Experimental)", ffSOLID, "If enabled, SHOULD make the FFC behave as solid."
						"\nNOTE: This is not fully implemented, and may not work at all."),
					CHECKB("Imprecise Collision", ffIMPRECISIONCHANGER, "Collision with changer FFCs is not"
						"subpixel-specific if enabled."),
					CHECKB("Invisible to Lens", ffLENSINVIS, "If this is enabled, the FFC will not be drawn when"
						" the lens is active. Checking 'Only Visible to Lens' as well will make the FFC entirely"
						" invisible."),
					CHECKB("Carry-Over", ffCARRYOVER, "If enabled, the FFC will 'Carry Over' to other screens."
						" Upon loading a new screen, the FFC of the new screen with the same index as this FFC"
						" will not be loaded, as this FFC will remain loaded in that slot instead. This FFC"
						" will carry over until either a script toggles this flag off for it, or a screen with"
						" the screen flag 'No FFC Carryover' is set is reached."),
					CHECKB("Stationary", ffSTATIONARY, "If enabled, the FFC will not move, even if it has"
						" movement values given to it."),
					//CH fl 2:
					CHECKB("Swap w/ Prev FFC", ffSWAPPREV, "If enabled, FFCs that collide with this changer will:\n"
						"Swap its' combo, cset, delay, link, movement, effect width/height, and flags with the"
						" previous combo in the list before it. Overrides 'Swap w/ Next FFC'."),
					CHECKB("Become Prev Combo", ffCHANGEPREV, "If enabled, FFCs that collide with this changer will:\n"
						"Change its' combo to the previous combo in the combo list."),
					DummyWidget()
				)),
				TabRef(name = "Script", Row(
					Column(
						FFC_INITD(0),
						FFC_INITD(1),
						FFC_INITD(2),
						FFC_INITD(3),
						FFC_INITD(4),
						FFC_INITD(5),
						FFC_INITD(6),
						FFC_INITD(7)
					),
					Column(
						padding = 0_px, fitParent = true,
						Rows<2>(vAlign = 0.0,
							SCRIPT_LIST_PROC("Action Script:", list_ffcscript, ffc.script, refreshScript)
						),
						Rows<2>(hAlign = 1.0,
							Label(text = "A1:"),
							TextField(
								val = ffc.inita[0],
								type = GUI::TextField::type::INT_DECIMAL,
								high = 32,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.inita[0] = val;
								}
							),
							Label(text = "A2:"),
							TextField(
								val = ffc.inita[1],
								type = GUI::TextField::type::INT_DECIMAL,
								high = 32,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.inita[1] = val;
								}
							)
						)
					)
				))
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
	refreshScript();
	return window;
}

bool FFCDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	bool m = false;
	switch(msg.message)
	{
		case message::OK:
			if(ffc.script && !ffc.data)
			{
				AlertDialog("Inactive FFC","FFCs that use Combo 0 cannot run scripts! Continue?",
					[&](bool ret,bool)
					{
						m = ret;
					}).show();
				if(!m) return false; //cancelled
			}
			ffc.save(thescr, ffind);
			saved = false;
			edited = true;
			[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
		
		case message::MINUSCS:
			m = true;
			[[fallthrough]];
		case message::PLUSCS:
			if(ffctab) break;
			tCSet = (tCSet+(m?11:1))%12;
			cmbsw->setCSet(tCSet);
			ffc.cset = tCSet;
			break;
	}
	return false;
}

