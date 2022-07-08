#include "ffc_editor.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "../zq_class.h"
#include "alert.h"
 
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
	twid = scr->ffTileWidth(ind)-1;
	thei = scr->ffTileHeight(ind)-1;
	fwid = scr->ffEffectWidth(ind)-1;
	fhei = scr->ffEffectHeight(ind)-1;
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
	scr->ffTileWidth(ind, twid+1);
	scr->ffTileHeight(ind, thei+1);
	scr->ffEffectWidth(ind, fwid+1);
	scr->ffEffectHeight(ind, fhei+1);
	scr->ffscript[ind] = script;
	for(auto q = 0; q < 2; ++q)
		scr->inita[ind][q] = inita[q];
	for(auto q = 0; q < 8; ++q)
		scr->initd[ind][q] = initd[q];
	SETFLAG(scr->numff,(1<<ind),data!=0);
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
	list_link(GUI::ListData::numbers(true, 1, 32)),
	list_ffcscript(GUI::ListData::ffc_script())
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

static size_t ffctab = 0;
std::shared_ptr<GUI::Widget> FFCDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	return Window(
		title = "FFC Editor ("+std::to_string(ffind+1)+")",
		info = "Edit an FFC, setting up its' combo, flags, and script data.\n"
			"Hotkeys: -/+: Change CSet",
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
						// showvals = false,
						onSelectFunc = [&](int32_t cmb, int32_t c)
						{
							ffc.data = cmb;
							ffc.cset = c;
							tCSet = c;
						}
					),
					// DummyWidget(),
					SWAPFIELD("X Pos:", ffc.x, -320000, 2880000),
					// DummyWidget(),
					// DummyWidget(),
					SWAPFIELD("Y Pos:", ffc.y, -320000, 2880000),
					SWAPFIELDB("Combo W:", ffc.fwid, 1, 64, 1),
					SWAPFIELD("X Speed:", ffc.dx, -1280000, 1280000),
					SWAPFIELDB("Combo H:", ffc.fhei, 1, 64, 1),
					SWAPFIELD("Y Speed:", ffc.dy, -1280000, 1280000),
					SWAPFIELDB("Tile W:", ffc.twid, 1, 4, 1),
					SWAPFIELD("X Accel:", ffc.ax, -1280000, 1280000),
					SWAPFIELDB("Tile H:", ffc.thei, 1, 4, 1),
					SWAPFIELD("Y Accel:", ffc.ay, -1280000, 1280000),
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
						INITD_ROW2(0, ffc.initd),
						INITD_ROW2(1, ffc.initd),
						INITD_ROW2(2, ffc.initd),
						INITD_ROW2(3, ffc.initd),
						INITD_ROW2(4, ffc.initd),
						INITD_ROW2(5, ffc.initd),
						INITD_ROW2(6, ffc.initd),
						INITD_ROW2(7, ffc.initd)
					),
					Column(
						padding = 0_px, fitParent = true,
						Rows<2>(vAlign = 0.0,
							SCRIPT_LIST("Action Script:", list_ffcscript, ffc.script)
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
					[&](bool ret)
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

