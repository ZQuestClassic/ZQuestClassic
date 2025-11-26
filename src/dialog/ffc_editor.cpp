#include "ffc_editor.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "alert.h"
#include "zc_list_data.h"

extern script_data *ffscripts[NUMSCRIPTFFC];
extern int32_t Combo, CSet;
static bool edited = false;
bool call_ffc_dialog(int32_t ffcombo, mapscr* scr, int screen)
{
	edited = false;
	FFCDialog(scr,screen,ffcombo).show();
	return edited;
}
bool call_ffc_dialog(int32_t ffcombo, ffdata const& init, mapscr* scr, int screen)
{
	edited = false;
	FFCDialog(scr,screen,ffcombo,init).show();
	return edited;
}

ffdata::ffdata(mapscr* scr, int32_t ind)
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
	layer = 1;
}
void ffdata::load(mapscr* scr, int32_t ind)
{
	if(unsigned(ind)>MAXFFCS-1) return;

	ffcdata& ffc = scr->getFFC(ind);
	x = ffc.x.getZLong();
	y = ffc.y.getZLong();
	dx = ffc.vx.getZLong();
	dy = ffc.vy.getZLong();
	ax = ffc.ax.getZLong();
	ay = ffc.ay.getZLong();
	data = ffc.data;
	cset = ffc.cset;
	delay = ffc.delay;
	flags = ffc.flags;
	link = ffc.link;
	layer = ffc.layer;
	twid = scr->ffTileWidth(ind)-1;
	thei = scr->ffTileHeight(ind)-1;
	fwid = scr->ffEffectWidth(ind)-1;
	fhei = scr->ffEffectHeight(ind)-1;
	script = ffc.script;
	for(auto q = 0; q < 8; ++q)
		initd[q] = ffc.initd[q];
}
void ffdata::save(mapscr* scr, int32_t screen, int32_t ind)
{
	if(unsigned(ind)>MAXFFCS-1) return;

	Map.DoSetFFCCommand(Map.getCurrMap(), screen, ind, {
		.x = zslongToFix(x),
		.y = zslongToFix(y),
		.vx = zslongToFix(dx),
		.vy = zslongToFix(dy),
		.ax = zslongToFix(ax),
		.ay = zslongToFix(ay),
		.data = data,
		.cset = cset,
		.delay = delay,
		.link = link,
		.script = script,
		.tw = (byte)(twid+1),
		.th = (byte)(thei+1),
		.ew = (byte)(fwid+1),
		.eh = (byte)(fhei+1),
		.flags = flags,
		.initd = initd,
		.layer = layer,
	});
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
	layer = other.layer;
	twid = other.twid;
	thei = other.thei;
	fwid = other.fwid;
	fhei = other.fhei;
	script = other.script;
	initd = other.initd;
	return *this;
}

FFCDialog::FFCDialog(mapscr* scr, int32_t screen, int32_t ffind) :
	thescr(scr), screen(screen), ffind(ffind),
	list_link(GUI::ListData::numbers(true, 1, MAXFFCS)),
	list_ffcscript(GUI::ZCListData::ffc_script())
{
	ffc.load(scr, ffind);
}

FFCDialog::FFCDialog(mapscr* scr, int32_t screen, int32_t ffind, ffdata const& init) :
	FFCDialog(scr, screen, ffind)
{
	ffc = init;
}

//{ Macros
#define SWAPFIELD(str, mem, lb, hb, info) \
Label(text = str, hAlign = 1.0), \
TextField( \
	type = GUI::TextField::type::SWAP_ZSINT, \
	low = lb, high = hb, val = mem, \
	leftPadding = 0_px, fitParent = true, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		mem = val; \
	}), \
INFOBTN(info)

#define CHECKB(str, fl, inf) \
INFOBTN_T(str, inf), \
Checkbox( \
	hAlign = 0.0, \
	checked = (ffc.flags & fl), \
	text = str, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(ffc.flags,fl,state); \
	} \
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
			type = GUI::TextField::type::SWAP_ZSINT,
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

void FFCDialog::refreshSize()
{
	GUI::DialogRef& ref = cmbsw->alDialog;
	if(!ref) return;
	if (prev_cache_x < 0)
	{
		prev_cache_x = ref->x;
		prev_cache_y = ref->y;
	}
	const auto max_sz = 4*32+4;
	ref->w = (ffc.twid+1)*32+4;
	ref->h = (ffc.thei+1)*32+4;
	ref->x = prev_cache_x + (max_sz - ref->w) / 2;
	ref->y = prev_cache_y + (max_sz - ref->h) / 2;
}
static size_t ffctabs[2];
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
				ptr = &ffctabs[0],
				TabRef(name = "Data", Row(
					Column(
						Rows<3>(
							SWAPFIELD("X Pos:", ffc.x, SWAP_MIN, SWAP_MAX, 
								"The number of pixels from the left of the screen the FFC begins at."
								"\nNote that if this value becomes < -32 or > 288, it will deactivate or"
								" wrap around if the Screen Flag 'FF Combos Wrap Around' is enabled"),
							SWAPFIELD("Y Pos:", ffc.y, SWAP_MIN, SWAP_MAX, 
								"The number of pixels from the top of the screen the FFC begins at."
								" Note that if this value becomes < -32 or > 208, it will deactivate or"
								" wrap around if the Screen Flag 'FF Combos Wrap Around' is enabled"),
							SWAPFIELD("X Speed:", ffc.dx, SWAP_MIN, SWAP_MAX, 
								"The initial velocity this FFC has parallel to the X-axis, in pixels per frame."
								"\nPositive numbers move the FFC right, while negative numbers moves"
								" the FFC move left. Fractions may also be used. For example, entering a X Speed of 1 makes"
								" the FFC move right 1 pixel every frame. Entering a X Speed of 0.5 makes the FFC move right"
								" 1 pixel per 2 frames"),
							SWAPFIELD("Y Speed:", ffc.dy, SWAP_MIN, SWAP_MAX, 
								"The initial velocity this FFC has parallel to the Y-axis, in pixels per frame."
								"\nPositive numbers move the FFC down, while negative numbers"
								" move up. Fractions may also be used. For example, entering a Y Speed of 1 makes"
								" the FFC move down 1 pixel every frame. Entering a Y Speed of 0.5 makes the FFC move down"
								" 1 pixel per 2 frames"),
							SWAPFIELD("X Accel:", ffc.ax, SWAP_MIN, SWAP_MAX,
								"The FFC's rightward acceleration."
								"\nA zero can be entered so that the FFC moves at a constant velocity,"
								" or a negative number can be entered so the FFC accelerates leftward."),
							SWAPFIELD("Y Accel:", ffc.ay, SWAP_MIN, SWAP_MAX, 
								"The FFC's downward acceleration.\n"
								"\nzero can be entered so that the FFC moves at a constant velocity,\n"
								" or a negative number can be entered so the FFC accelerates upward.")
						),
						Frame(hAlign = 0.0,
							cmb_container = Column(width = 128_px + 5_px + 1_em + text_length(GUI_DEF_FONT, "Combo: 99999"), height = 128_px + 4_px,
								cmbsw = SelComboSwatch(vAlign = 0.0, hAlign = 0.0, padding = 0_px,
									combo = ffc.data,
									cset = ffc.cset,
									// showvals = false,
									onSelectFunc = [&](int32_t cmb, int32_t c)
									{
										ffc.data = cmb;
										ffc.cset = c;
									}
								)
							)
						)
					),
					Column(vAlign = 0.0,
						Rows_Columns<3, 4>(hAlign = 0.0,
							Label(text = "Combo W:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 64, val = ffc.fwid+1,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.fwid = val-1;
								}),
							INFOBTN("The Combo W. of a FFC denotes the width of the field of effect"
								" that the FFC's Combo Type has in pixels. This also effects solidity and platforms!"),
							Label(text = "Combo H:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 64, val = ffc.fhei+1,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.fhei = val-1;
								}),
							INFOBTN("The Combo H. of a FFC denotes the height of the field of effect"
								" that the FFC's Combo Type has in pixels. This also effects solidity and platforms!"),
							
							Label(text = "Tile W:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 4, val = ffc.twid+1,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.twid = val-1;
									refreshSize();
								}),
							INFOBTN("How large the width of the FFC will be drawn."
								"\nIt will be drawn with tiles from the tile page, rather than the combo page."
								" If the FFC is larger than 1x1 tiles and the combo is animated,"
								" the combo needs to use 'A.SkipX' and 'A.SkipY' values to animate as desired."),
							Label(text = "Tile H:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 4, val = ffc.thei+1,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.thei = val-1;
									refreshSize();
								}),
							INFOBTN("How large the height of the FFC will be drawn."
								"\nIt will be drawn with tiles from the tile page, rather than the combo page."
								" If the FFC is larger than 1x1 tiles and the combo is animated,"
								" the combo needs to use 'A.SkipX' and 'A.SkipY' values to animate as desired."),
							
							Label(text = "A. Delay:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 9999, val = ffc.delay,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.delay = val;
								}),
							INFOBTN("The delay, in frames, before the combo begins moving."),
							Label(text = "Layer:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = -7, high = 7, val = ffc.layer,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.layer = val;
								}),
							INFOBTN("The layer (-7 to 7) the ffc will be drawn on. Ignored if 'Draw Over' is checked."
								" Will draw above the combos AND script draws on the specified layer."),
							//
							Label(text = "Link to:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = MAXFFCS, val = ffc.link,
								leftPadding = 0_px, fitParent = true,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									ffc.link = val;
								}),
							INFOBTN_T("FFC Link", "The Linked FFC, if > 0 the ffc will use the movement of the Linked FFC.")
						),
						TabPanel(ptr = &ffctabs[1],
							TabRef(name = "Basic",
								Rows_Columns<2, 5>(
									CHECKB("Draw Over", ffc_overlay, "If enabled, the FFC draws on a special timing above Layer 5, ignoring the 'Layer' setting."),
									CHECKB("Translucent", ffc_trans, "The FFC draws transparently"),
									CHECKB("Only Visible to Lens", ffc_lensvis, "If this is enabled, the FFC will not be drawn when"
										" the lens is not active. Checking 'Invisible to Lens' as well will make the FFC entirely"
										" invisible."),
									CHECKB("Invisible to Lens", ffc_lensinvis, "If this is enabled, the FFC will not be drawn when"
										" the lens is active. Checking 'Only Visible to Lens' as well will make the FFC entirely"
										" invisible."),
									CHECKB("Active during Holdup", ffc_ignoreholdup, "If disabled, the FFC will pause all activity"
										" during item holdup animations. Does not pause draws."),
									
									CHECKB("Ethereal", ffc_ethereal, "If enabled, the FFC has no 'effect', and is not checked"
										" for any combo type related effects."),
									CHECKB("Solid", ffc_solid, "If enabled, SHOULD make the FFC behave as solid."
										"\nNOTE: This is not fully implemented, and may not work at all."),
									CHECKB("Carry-Over", ffc_carryover, "If enabled, the FFC will 'Carry Over' to other screens."
										" Upon loading a new screen, the FFC of the new screen with the same index as this FFC"
										" will not be loaded, as this FFC will remain loaded in that slot instead. This FFC"
										" will carry over until either a script toggles this flag off for it, or a screen with"
										" the screen flag 'No FFC Carryover' is set is reached."),
									CHECKB("Stationary", ffc_stationary, "If enabled, the FFC will not move, even if it has"
										" movement values given to it."),
									CHECKB("Platform", ffc_platform, "If enabled, the FFC will act as a platform,"
										" moving the Hero with it if the Hero is standing on it."+QRHINT({qr_MULTI_PLATFORM_FFC}))
								)
							),
							TabRef(name = "Changer", Column(padding = 0_px,
								Rows_Columns<2, 1>(
									CHECKB("Imprecise Collision", ffc_imprecisionchanger, "Collision with changer FFCs is not"
										"subpixel-specific if enabled."),
									CHECKB("Ignore Changers", ffc_ignorechanger, "If enabled, this FFC will ignore collisions with"
										" Changer FFCs.")
								),
								HSeparator(),
								Row(
									Label(text = "Changer Only Flags"),
									INFOBTN("The flags below only work if 'Is a Changer' is checked.")
								),
								Rows_Columns<2, 3>(
									CHECKB("Is a Changer", ffc_changer, "If enabled, this FFC is a changer."
										" Changers are both invisible (not drawn) and ethereal (have no effect)."
										" When a moving FFC collides with a changer, the changer will change some properties of"
										" the colliding FFC. This can be used to make FFCs that move in patterns and such."),
									CHECKB("Swap w/ Next FFC", ffc_swapnext, "If enabled, FFCs that collide with this changer will:\n"
										"Swap its' combo, cset, delay, link, movement, effect width/height, and flags with the"
										" next combo in the list after it."),
									CHECKB("Swap w/ Prev FFC", ffc_swapprev, "If enabled, FFCs that collide with this changer will:\n"
										"Swap its' combo, cset, delay, link, movement, effect width/height, and flags with the"
										" previous combo in the list before it. Overrides 'Swap w/ Next FFC'."),
									
									CHECKB("Become This Combo+CSet", ffc_changethis, "If enabled, FFCs that collide with this changer will:\n"
										"Change its' combo and cset to the combo and cset of this changer."),
									CHECKB("Become Next Combo", ffc_changenext, "If enabled, FFCs that collide with this changer will:\n"
										"Change its' combo to the next combo in the combo list."),
									CHECKB("Become Prev Combo", ffc_changeprev, "If enabled, FFCs that collide with this changer will:\n"
										"Change its' combo to the previous combo in the combo list.")
								)
							))
						)
					)
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
						Rows<2>(
							CHECKB("Run Script at Screen Init", ffc_preload, "The script attached to this FFC runs during the"
								" screen's initialization, before scrolling to the screen begins. This is useful for scripts"
								" that want to, for instance, modify combos on the screen and have those changes visible"
								" as you scroll onto the screen."),
							CHECKB("Script Restarts when Carried Over", ffc_scriptreset, "If enabled, and 'Carry Over'"
								" is enabled, then when the FFC is carried over to a new screen, the script starts over.")
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
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	refreshScript();
	return window;
}

void FFCDialog::post_realize()
{
	prev_cache_x = prev_cache_y = -1;
	refreshSize();
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
			ffc.save(thescr, screen, ffind);
			mark_save_dirty();
			edited = true;
			[[fallthrough]];
		case message::CANCEL:
			return true;
		
		case message::MINUSCS:
			m = true;
			[[fallthrough]];
		case message::PLUSCS:
			if(ffctabs[0]) break; // only works on the main tab, where the preview is visible
			ffc.cset = WRAP_CS(ffc.cset+(m?-1:1));
			cmbsw->setCSet(ffc.cset);
			break;
	}
	return false;
}

