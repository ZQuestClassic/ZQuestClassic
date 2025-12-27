#include "dialog/subscreenwizard.h"
#include "info.h"
#include "zc_list_data.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "new_subscr.h"
#include "zq/zq_subscr.h"

#define MISC_COLOR_SEL( txt, num, basec1, basec2) \
Frame( \
	title = txt, \
	colSpan = 3, \
	misccolorsel[num-1] = MiscColorSel( \
		c1 = basec1, \
		c2 = basec2, \
		onUpdate = [=](int32_t c1, int32_t c2) \
		{ \
			basec1 = c1; \
			basec2 = c2; \
		}) \
)

extern bool sso_selection[NEW_MAXSUBSCREENITEMS];
extern ZCSubscreen subscr_edit;

void call_subscreen_wizard(subwizardtype stype, int32_t& x, int32_t& y)
{
	SubscreenWizardDialog(stype, x, y).show();
}

void group_select_widget(SubscrWidget* widg)
{
	SubscrPage& pg = subscr_edit.cur_page();
	for (int32_t q = 0; q < pg.size(); ++q)
		if (pg[q] == widg)
			sso_selection[q] = true;
}

static bool initialized[NUM_SUBWIZARD_TYPES] = {0};
static int tf_values[NUM_SUBWIZARD_TYPES][8];
static int32_t misccolors[NUM_SUBWIZARD_TYPES][3][2];
static bool cbvals[NUM_SUBWIZARD_TYPES][2];
static size_t rs_sel[NUM_SUBWIZARD_TYPES][1];
static size_t rs_sz[NUM_SUBWIZARD_TYPES][1];
SubscreenWizardDialog::SubscreenWizardDialog(subwizardtype stype, int32_t& x, int32_t& y)
	: wizard_type(stype), wizard_index(size_t(stype)), basex(x), basey(y), flags(0),
	list_font(GUI::ZCListData::fonts(false, true, true)),
	list_shadtype(GUI::ZCListData::shadow_types())
{
	if (!initialized[wizard_index]) // only set the first time you open the dialog
	{
		memset(rs_sz[wizard_index], 0, sizeof(rs_sz[wizard_index]));
		switch (stype)
		{
			case subwizardtype::SW_ITEM_GRID:
				rs_sz[wizard_index][0] = 4;
				rs_sel[wizard_index][0] = 1;
				break;
			case subwizardtype::SW_MAP_TILEBLOCK:
			{
				tf_values[wizard_index][0] = 0;
				tf_values[wizard_index][1] = 16;
				tf_values[wizard_index][2] = 16;
				tf_values[wizard_index][3] = 1;
				tf_values[wizard_index][4] = 0;
				tf_values[wizard_index][5] = -1;
				cbvals[wizard_index][0] = true;
				cbvals[wizard_index][1] = false;
				rs_sz[wizard_index][0] = 4;
				rs_sel[wizard_index][0] = 0;
				break;
			}
		}
		initialized[wizard_index] = true;
	}
}

void SubscreenWizardDialog::setRadio(size_t rs, size_t ind)
{
	for (size_t q = 0; q < rs_sz[wizard_index][rs]; ++q)
	{
		auto& radio = rset[rs][q];
		radio->setChecked(ind == q);
	}
	rs_sel[wizard_index][rs] = ind;
}
size_t SubscreenWizardDialog::getRadio(size_t rs)
{
	for (size_t q = 0; q < rs_sz[wizard_index][rs]; ++q)
	{
		if (rset[rs][q]->getChecked())
			return q;
	}
	if (rs_sz[wizard_index][rs] > 0)
		rset[rs][0]->setChecked(true);
	return 0;
}

struct TileBlockLayerData
{
	vector<vector<TilePickerData>> tile_data;
	bool you_are_here;
	word required_litems;
	dword required_scr_states, required_ex_states, required_lstate;
	int16_t required_floor = -1, required_level = -1;
	
	bool invert_litem, any_litem, invert_scrstate, any_scrstate, invert_lstate, any_lstate;
};

static vector<TileBlockLayerData> tileblock_sets;
static optional<TileBlockLayerData> copied_tileblock_set;
static size_t tab_ptrs[1];

bool SubscreenWizardDialog::finalize()
{
#define HANDLE_FAILURE(widget) \
if (!widget) \
{ \
	displayinfo("Error", "Failed creating a widget; subscreen page might be full, or else an unexpected error occurred."); \
	return true; \
}
	memset(sso_selection, 0, sizeof(sso_selection));
	switch (wizard_type)
	{
		case subwizardtype::SW_ITEM_GRID:
		{
			int32_t w = tfs[3]->getVal();
			int32_t h = tfs[4]->getVal();
			bool non_selectable = rset[0][0]->getChecked();
			bool non_equippable = cboxes[0]->getChecked();
			for (int32_t y = 0; y < h; ++y)
			{
				for (int32_t x = 0; x < w; ++x)
				{
					SubscrWidget* widg = (SW_ItemSlot*)create_new_widget_of(widgITEMSLOT, tfs[1]->getVal() + x * tfs[5]->getVal(), tfs[2]->getVal() + y * tfs[6]->getVal(), false);
					HANDLE_FAILURE(widg)
					SETFLAG(widg->flags, SUBSCR_CURITM_NONEQP, non_equippable);
					group_select_widget(widg);
					if (non_selectable)
						widg->genflags &= ~SUBSCRFLAG_SELECTABLE;
					else
					{
						widg->genflags |= SUBSCRFLAG_SELECTABLE;
						widg->pos = tfs[0]->getVal() + w * y + x;
						if (rset[0][1]->getChecked()) // Loop L/R
						{
							widg->pos_up = widg->pos + (y == 0 ? (w * (h - 1)) : -w);
							widg->pos_down = widg->pos + (y == h - 1 ? -(w * (h - 1)) : w);
							widg->pos_left = widg->pos + ((x == 0 && y == 0) ? (w * h - 1) : -1);
							widg->pos_right = widg->pos + ((x == w - 1 && y == h - 1) ? -(w * h - 1) : 1);
						}
						else if (rset[0][2]->getChecked()) // Wrap Sides
						{
							widg->pos_up = widg->pos + (y == 0 ? (w * (h - 1)) : -w);
							widg->pos_down = widg->pos + (y == h - 1 ? -(w * (h - 1)) : w);
							widg->pos_left = widg->pos + (x == 0 ? w - 1 : -1);
							widg->pos_right = widg->pos + (x == w - 1 ? -(w - 1) : 1);
						}
						else if (rset[0][3]->getChecked()) // No Wrap
						{
							widg->pos_up = y == 0 ? 255 : widg->pos - w;
							widg->pos_down = y == h - 1 ? 255 : widg->pos + w;
							widg->pos_left = x == 0 ? 255 : widg->pos - 1;
							widg->pos_right = x == w - 1 ? 255 : widg->pos + 1;
						}
					}
				}
			}
			break;
		}
		case subwizardtype::SW_COUNTER_BLOCK:
		{
			int32_t x = tfs[0]->getVal();
			int32_t y = tfs[1]->getVal();
			int32_t fontid = ddls[0]->getSelectedValue();
			int32_t shadtype = ddls[1]->getSelectedValue();
			int32_t maxc = tfs[7]->getVal();
			std::string_view divider = tfs[4]->getText();
			int32_t dividerw = text_length(get_zc_font(fontid), std::string(divider).c_str());
			int32_t width = 8 + dividerw + maxc * text_length(get_zc_font(fontid), std::string("8").c_str()) + tfs[3]->getVal();
			int32_t height = std::max(8, text_height(get_zc_font(fontid)) + tfs[3]->getVal());
			int32_t yoff = (8 - text_height(get_zc_font(fontid))) / 2;
			for (int32_t q = 0; q < tfs[2]->getVal(); ++q)
			{
				SW_MiniTile* mt = (SW_MiniTile*)create_new_widget_of(widgMINITILE, x, y, false);
				HANDLE_FAILURE(mt)
				group_select_widget(mt);
				mt->tile = tswatches[0]->getTile();
				mt->cs.set_int_cset(tswatches[0]->getCSet());
				mt->crn = tswatches[0]->getMiniCrn();

				SW_Text* txt = (SW_Text*)create_new_widget_of(widgTEXT, x + 8 + dividerw, y + yoff, false);
				HANDLE_FAILURE(txt)
				group_select_widget(txt);
				txt->fontid = fontid;
				txt->shadtype = shadtype;
				txt->align = ALIGN_RIGHT;
				txt->text = tfs[4]->getText();
				txt->c_text.type = misccolors[wizard_index][0][0];
				txt->c_text.color = misccolors[wizard_index][0][1];
				txt->c_shadow.type = misccolors[wizard_index][1][0];
				txt->c_shadow.color = misccolors[wizard_index][1][1];
				txt->c_bg.type = misccolors[wizard_index][2][0];
				txt->c_bg.color = misccolors[wizard_index][2][1];

				SW_Counter* ctr = (SW_Counter*)create_new_widget_of(widgCOUNTER, x + 8 + dividerw, y + yoff, false);
				HANDLE_FAILURE(ctr)
				group_select_widget(ctr);
				ctr->fontid = fontid;
				ctr->shadtype = shadtype;
				ctr->align = ALIGN_LEFT;
				std::string_view infchar_sv = tfs[6]->getText();
				ctr->infchar = infchar_sv.empty() ? 'A' : infchar_sv.at(0);
				ctr->mindigits = tfs[5]->getVal();
				ctr->maxdigits = maxc;
				if(cboxes[1]->getChecked())
					ctr->flags |= SUBSCR_COUNTER_SHOW0;
				ctr->c_text.type = misccolors[wizard_index][0][0];
				ctr->c_text.color = misccolors[wizard_index][0][1];
				ctr->c_shadow.type = misccolors[wizard_index][1][0];
				ctr->c_shadow.color = misccolors[wizard_index][1][1];
				ctr->c_bg.type = misccolors[wizard_index][2][0];
				ctr->c_bg.color = misccolors[wizard_index][2][1];

				if (cboxes[0]->getChecked())
					x += width;
				else
					y += height;
			}
			break;
		}
		case subwizardtype::SW_MAP_TILEBLOCK:
		{
			int32_t w = tfs[3]->getVal();
			int32_t h = tfs[4]->getVal();
			int32_t mapid = tfs[7]->getVal();
			int32_t scrid = tfs[8]->getVal();
			int32_t reqlvl = tfs[9]->getVal();
			bool non_selectable = rset[0][0]->getChecked();
			bool overlay = cboxes[0]->getChecked();
			bool transparent = cboxes[1]->getChecked();
			if (auto badscr = (scrid + (w-1) + ((h-1) * 0x10)); badscr >= MAPSCRSNORMAL)
			{
				if (!alert_confirm("Screen value out of bounds",
					fmt::format("Attempting to create a grid of '{}x{}' screens starting with screen '0x{:02X}'"
					" would go out of bounds to invalid screen '0x{:02X}'. Tiles that would match an invalid"
					" screen will be ignored, their TileBlock widget not being created, if you continue.", w, h, scrid, badscr)))
					return false;
			}
			if (get_qr(qr_ONLY_MARK_SCREENS_VISITED_IF_MAP_VIEWABLE))
			{
				InfoDialog("Conflicting QR", "A conflicting compat rule \"Only 'Visit' Screens On Mappable DMaps\""
					" is enabled. This may prevent screens from properly being marked 'Visited'."
					+ QRHINT({qr_ONLY_MARK_SCREENS_VISITED_IF_MAP_VIEWABLE})).show();
			}
			for (int idx = 0; idx < tileblock_sets.size(); ++idx)
			{
				auto& data = tileblock_sets[idx];
				data.tile_data.resize(h);
				for (int32_t y = 0; y < h; ++y)
				{
					auto& row_data = data.tile_data[y];
					row_data.resize(w);
					for (int32_t x = 0; x < w; ++x)
					{
						int screen = scrid + x + (0x10 * y);
						if (unsigned(screen) >= MAPSCRSNORMAL)
							continue; // invalid screen, don't create matching widget
						TilePickerData const& tile_data = row_data[x];
						if (!tile_data.tile) // don't create for tile 0
							continue;
						SW_TileBlock* widg = (SW_TileBlock*)create_new_widget_of(widgTILEBLOCK, tfs[1]->getVal() + x * tfs[5]->getVal(), tfs[2]->getVal() + y * tfs[6]->getVal(), false);
						HANDLE_FAILURE(widg)
						group_select_widget(widg);
						if (non_selectable)
							widg->genflags &= ~SUBSCRFLAG_SELECTABLE;
						else
						{
							widg->genflags |= SUBSCRFLAG_SELECTABLE;
							widg->pos = tfs[0]->getVal() + w * y + x;
							if (rset[0][1]->getChecked()) // Loop L/R
							{
								widg->pos_up = widg->pos + (y == 0 ? (w * (h - 1)) : -w);
								widg->pos_down = widg->pos + (y == h - 1 ? -(w * (h - 1)) : w);
								widg->pos_left = widg->pos + ((x == 0 && y == 0) ? (w * h - 1) : -1);
								widg->pos_right = widg->pos + ((x == w - 1 && y == h - 1) ? -(w * h - 1) : 1);
							}
							else if (rset[0][2]->getChecked()) // Wrap Sides
							{
								widg->pos_up = widg->pos + (y == 0 ? (w * (h - 1)) : -w);
								widg->pos_down = widg->pos + (y == h - 1 ? -(w * (h - 1)) : w);
								widg->pos_left = widg->pos + (x == 0 ? w - 1 : -1);
								widg->pos_right = widg->pos + (x == w - 1 ? -(w - 1) : 1);
							}
							else if (rset[0][3]->getChecked()) // No Wrap
							{
								widg->pos_up = y == 0 ? 255 : widg->pos - w;
								widg->pos_down = y == h - 1 ? 255 : widg->pos + w;
								widg->pos_left = x == 0 ? 255 : widg->pos - 1;
								widg->pos_right = x == w - 1 ? 255 : widg->pos + 1;
							}
						}
						
						widg->tile = tile_data.tile;
						widg->cs.set_int_cset(tile_data.cset);
						widg->flip = tile_data.flip;
						SETFLAG(widg->flags, SUBSCR_TILEBL_OVERLAY, overlay);
						SETFLAG(widg->flags, SUBSCR_TILEBL_TRANSP, transparent);
						
						SETFLAG(widg->genflags, SUBSCRFLAG_REQ_INVERT_LITEM, data.invert_litem);
						SETFLAG(widg->genflags, SUBSCRFLAG_REQ_ANY_LITEM, data.any_litem);
						SETFLAG(widg->genflags, SUBSCRFLAG_REQ_INVERT_SCRSTATE, data.invert_scrstate);
						SETFLAG(widg->genflags, SUBSCRFLAG_REQ_ANY_SCRSTATE, data.any_scrstate);
						SETFLAG(widg->genflags, SUBSCRFLAG_REQ_INVERT_LSTATE, data.invert_lstate);
						SETFLAG(widg->genflags, SUBSCRFLAG_REQ_ANY_LSTATE, data.any_lstate);
						
						widg->req_litems = data.required_litems;
						widg->req_lvlstate = data.required_lstate;
						if (data.required_litems || data.required_lstate)
							widg->req_litem_level = reqlvl;
						
						widg->req_scrstate = data.required_scr_states;
						widg->req_exstate = data.required_ex_states;
						if (data.required_scr_states || data.required_ex_states)
						{
							widg->req_scrstate_map = mapid;
							widg->req_scrstate_scr = screen;
						}
						if (data.you_are_here)
						{
							widg->req_maps = {(word)mapid};
							widg->req_screens = {(byte)screen};
						}
						widg->req_dmap_floors.clear();
						if (data.required_floor >= 0)
							widg->req_dmap_floors = { (byte)data.required_floor };
						widg->req_dmap_levels.clear();
						if (data.required_level >= 0)
							widg->req_dmap_levels = { (word)data.required_level };
					}
				}
			}
			break;
		}
	}
	return true;
}
#define IH_BTN(hei, inf) \
Button(height = hei, text = "?", \
	onPressFunc = [=]() \
	{ \
		InfoDialog("Info",inf).show(); \
	})
#define DDH 21_px

void SubscreenWizardDialog::updateTitle()
{
	string info;
	switch (wizard_type)
	{
		case subwizardtype::SW_ITEM_GRID:
			tyname = "Item Grid";
			info = "Creates a grid of items, with selection directions linked between them.";
			break;
		case subwizardtype::SW_COUNTER_BLOCK:
			tyname = "Counter Block";
			info = "Creates a block of counters.";
			break;
		case subwizardtype::SW_MAP_TILEBLOCK:
			tyname = "Map Tile Grid";
			info = "Creates 2 blocks of TileBlocks, overlapping. The lower blocks require having the Map, while the "
				" higher blocks require having visited specific screens. This allows creating a custom 'TileBlock Map',"
				" with a layer that reveals for the whole dungeon when you have the map, and a layer that reveals as you"
				" visit rooms.";
			break;
	}
	window->setTitle("Subscreen Wizard (" + tyname + ")");
	window->setHelp(info);
}

std::shared_ptr<GUI::Widget> SubscreenWizardDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	std::shared_ptr<GUI::Grid> windowRow;
	window = Window(
		//use_vsync = true,
		onClose = message::CANCEL,
		Column(
			windowRow = Row(padding = 0_px),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);

	bool wip = false;
	switch (wizard_type)
	{
		case subwizardtype::SW_ITEM_GRID:
		{
			windowRow->add(
				Rows<2>(
					Rows<3>(
						Label(text = "Starting Pos:", hAlign = 1.0),
						tfs[0] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 254, val = 0
						),
						INFOBTN("The first selector position used by the grid. It will use as many positions in order as there are items, unless it's Non-Selectable."),
						Label(text = "X:", hAlign = 1.0),
						tfs[1] = TextField(
							colSpan = 2,
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -15, high = 256, val = basex
						),
						Label(text = "Y:", hAlign = 1.0),
						tfs[2] = TextField(
							colSpan = 2,
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -15, high = 176, val = basey
						),
						Label(text = "Width:", hAlign = 1.0),
						tfs[3] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 16, val = 5
						),
						INFOBTN("Sets the number of items in the grid along the X-axis"),
						Label(text = "Height:", hAlign = 1.0),
						tfs[4] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 16, val = 3
						),
						INFOBTN("Sets the number of items in the grid along the Y-axis"),
						Label(text = "X Spacing:", hAlign = 1.0),
						tfs[5] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 32, val = 16
						),
						INFOBTN("Sets the space in pixels between each item in the grid on the X-axis"),
						Label(text = "Y Spacing:", hAlign = 1.0),
						tfs[6] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 32, val = 16
						),
						INFOBTN("Sets the space in pixels between each item in the grid on the Y-axis"),
						cboxes[0] = Checkbox(
							colSpan = 2, _EX_RBOX, text = "Non-Equippable"
						),
						INFOBTN("If checked, items in this grid default to being non equippable, but may still be selectable.")
					),
					Rows<2>(
						framed = true, frameText = "Connections",
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = rs_sel[wizard_index][0] == 0,
							text = "Non Selectable",
							indx = 0,
							onToggle = message::RSET0
						),
						INFOBTN_VS("Items in this grid are non selectable.", 0_px),
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = rs_sel[wizard_index][0] == 1,
							text = "Loop L/R End to End",
							indx = 1,
							onToggle = message::RSET0
						),
						INFOBTN_VS("Items in this grid will form a complete loop left to right going through every item.\nUse this if you plan to have L/R inventory shifting.", 0_px),
						rset[0][2] = Radio(
							hAlign = 0.0,
							checked = rs_sel[wizard_index][0] == 2,
							text = "Wrap At Sides",
							indx = 2,
							onToggle = message::RSET0
						),
						INFOBTN_VS("Items in this grid will wrap to opposite ends when selecting off the edge.\nThis is not compatible with L/R inventory shifting.", 0_px),
						rset[0][3] = Radio(
							hAlign = 0.0,
							checked = rs_sel[wizard_index][0] == 3,
							text = "No Wrapping",
							indx = 3,
							onToggle = message::RSET0
						),
						INFOBTN_VS("Items in this grid will not connect to anything at the edges.\nThis is not compatible with L/R inventory shifting.", 0_px)
					)
				)
			);
			break;
		}
		case subwizardtype::SW_COUNTER_BLOCK:
		{
			misccolors[wizard_index][0][0] = ssctMISC;
			misccolors[wizard_index][0][1] = 0;
			misccolors[wizard_index][1][0] = 0;
			misccolors[wizard_index][1][1] = 0;
			misccolors[wizard_index][2][0] = ssctSYSTEM;
			misccolors[wizard_index][2][1] = -1;
			windowRow->add(
				Rows<2>(
					Column(
						Rows<6>(
							Label(text = "X:", hAlign = 1.0),
							tfs[0] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -16, high = 256, val = basex
							),
							DummyWidget(),
							Label(text = "Y:", hAlign = 1.0),
							tfs[1] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -16, high = 176, val = basey
							),
							DummyWidget(),
							Label(text = "Count:", hAlign = 1.0),
							tfs[2] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 2, high = 32, val = 4
							),
							INFOBTN("How many counters to create"),
							Label(text = "Spacing:", hAlign = 1.0),
							tfs[3] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -8, high = 255, val = 0
							),
							INFOBTN("How spaced apart the counters should be"),
							Label(text = "Tile:", hAlign = 1.0),
							tswatches[0] = SelTileSwatch(
								hAlign = 0.0,
								tile = 0,
								cset = 1,
								tilewid = 1,
								mini = true,
								minicorner = 0,
								showvals = false,
								showT0 = true
							),
							DummyWidget(colSpan = 4),
							Label(text = "Divider:", hAlign = 1.0),
							tfs[4] = TextField(
								forceFitW = true,
								text = "X",
								maxLength = 256,
								type = GUI::TextField::type::TEXT
							),
							INFOBTN("Dividing character drawn between the minitile and counter"),
							Label(text = "Min Digits:", hAlign = 1.0),
							tfs[5] = TextField(
								fitParent = true, minwidth = 3_em,
								val = 0,
								low = 0, high = 5,
								type = GUI::TextField::type::INT_DECIMAL
							),
							DummyWidget(),
							Label(text = "Inf Char:", hAlign = 1.0),
							tfs[6] = TextField(
								fitParent = true, minwidth = 3_em,
								text = "A",
								type = GUI::TextField::type::TEXT
							),
							DummyWidget(),
							Label(text = "Max Digits:", hAlign = 1.0),
							tfs[7] = TextField(
								fitParent = true, minwidth = 3_em,
								val = 3,
								low = 0, high = 5,
								type = GUI::TextField::type::INT_DECIMAL
							),
							DummyWidget()
						),
						Rows<2>(
							INFOBTN("The counters will be spaced horizontally instead of vertically"),
							cboxes[0] = Checkbox(
								text = "Horizontal"
							),
							DummyWidget(),
							cboxes[1] = Checkbox(
								text = "Show Zero", checked = true
							)
						),
						Rows<2>(
							Label(text = "Font:", hAlign = 1.0),
							ddls[0] = DropDownList(data = list_font,
								fitParent = true,
								selectedValue = 0
							),
							Label(text = "Format:", hAlign = 1.0),
							ddls[1] = DropDownList(data = list_shadtype,
								fitParent = true,
								selectedValue = 0
							)
						)
					),
					Rows<3>(
						MISC_COLOR_SEL("Text Color", 1, misccolors[wizard_index][0][0], misccolors[wizard_index][0][1]),
						MISC_COLOR_SEL("Shadow Color", 2, misccolors[wizard_index][1][0], misccolors[wizard_index][1][1]),
						MISC_COLOR_SEL("Background Color", 3, misccolors[wizard_index][2][0], misccolors[wizard_index][2][1])
					)
				)
			);
			break;
		}
		case subwizardtype::SW_MAP_TILEBLOCK:
		{
			std::shared_ptr<GUI::TabPanel> tabs;
			windowRow->add(
				Rows<2>(
					Column(
						Rows<6>(
							Label(text = "X:", hAlign = 1.0),
							tfs[1] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -15, high = 256, val = basex,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									basex = val;
								}
							),
							INFOBTN("Sets the X-position of the top-left widget."),
							
							Label(text = "Width:", hAlign = 1.0),
							tfs[3] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 16, val = tf_values[wizard_index][6],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][6] = val;
								}
							),
							INFOBTN("Sets the number of widgets in the grid along the X-axis"),
							
							
							Label(text = "Y:", hAlign = 1.0),
							tfs[2] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -15, high = 176, val = basey,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									basey = val;
								}
							),
							INFOBTN("Sets the Y-position of the top-left widget."),
							
							Label(text = "Height:", hAlign = 1.0),
							tfs[4] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 8, val = tf_values[wizard_index][6],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][6] = val;
								}
							),
							INFOBTN("Sets the number of widgets in the grid along the Y-axis"),
							
							Label(text = "Map:", hAlign = 1.0),
							tfs[7] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = map_count, val = tf_values[wizard_index][3],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][3] = val;
								}
							),
							INFOBTN("Sets the Map to check for conditions"),
							
							Label(text = "X Spacing:", hAlign = 1.0),
							tfs[5] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 32, val = tf_values[wizard_index][1],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][1] = val;
								}
							),
							INFOBTN("Sets the space in pixels between each item in the grid on the X-axis"),
							
							Label(text = "Screen:", hAlign = 1.0),
							tfs[8] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
								swap_type = nswapHEX,
								low = 0, high = MAPSCRSNORMAL-1, val = tf_values[wizard_index][4],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][4] = val;
								}
							),
							INFOBTN("Sets the *top-left* Screen to check for conditions."
								" Each tile in the grid checks a different screen, based on it's position."),
							
							Label(text = "Y Spacing:", hAlign = 1.0),
							tfs[6] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = 32, val = tf_values[wizard_index][2],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][2] = val;
								}
							),
							INFOBTN("Sets the space in pixels between each item in the grid on the Y-axis"),
							
							Label(text = "Target Level:", hAlign = 1.0),
							tfs[9] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -1, high = MAXLEVELS-1, val = tf_values[wizard_index][5],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][5] = val;
								}
							),
							INFOBTN("Sets which 'level' level items / states are required for LItem / LState conditions. '-1' represents 'current level'.")
						),
						Rows<2>(
							INFOBTN("The state of the 'Overlay' box for each TileBlock."),
							cboxes[0] = Checkbox(text = "Overlay", hAlign = 0.0,
								checked = cbvals[wizard_index][0],
								onToggleFunc = [&](bool state)
								{
									cbvals[wizard_index][0] = state;
								}
							),
							INFOBTN("The state of the 'Transparent' box for each TileBlock."),
							cboxes[1] = Checkbox(text = "Transparent", hAlign = 0.0,
								checked = cbvals[wizard_index][1],
								onToggleFunc = [&](bool state)
								{
									cbvals[wizard_index][1] = state;
								}
							)
						)
					),
					Column(
						Rows<3>(
							Label(text = "Starting Pos:", hAlign = 1.0),
							tfs[0] = TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 254, val = tf_values[wizard_index][0],
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									tf_values[wizard_index][0] = val;
								}
							),
							INFOBTN("The first selector position used by the grid. It will use as many positions in order as there are widgets, unless it's Non-Selectable.")
						),
						Rows<2>(
							framed = true, frameText = "Connections",
							rset[0][0] = Radio(
								hAlign = 0.0,
								checked = rs_sel[wizard_index][0] == 0,
								text = "Non Selectable",
								indx = 0,
								onToggle = message::RSET0
							),
							INFOBTN_VS("Widgets in this grid are non selectable.", 0_px),
							rset[0][1] = Radio(
								hAlign = 0.0,
								checked = rs_sel[wizard_index][0] == 1,
								text = "Loop L/R End to End",
								indx = 1,
								onToggle = message::RSET0
							),
							INFOBTN_VS("Widgets in this grid will form a complete loop left to right going through every item.\nUse this if you plan to have L/R inventory shifting.", 0_px),
							rset[0][2] = Radio(
								hAlign = 0.0,
								checked = rs_sel[wizard_index][0] == 2,
								text = "Wrap At Sides",
								indx = 2,
								onToggle = message::RSET0
							),
							INFOBTN_VS("Widgets in this grid will wrap to opposite ends when selecting off the edge.\nThis is not compatible with L/R inventory shifting.", 0_px),
							rset[0][3] = Radio(
								hAlign = 0.0,
								checked = rs_sel[wizard_index][0] == 3,
								text = "No Wrapping",
								indx = 3,
								onToggle = message::RSET0
							),
							INFOBTN_VS("Widgets in this grid will not connect to anything at the edges.\nThis is not compatible with L/R inventory shifting.", 0_px)
						)
					),
					tabs = TabPanel(colSpan = 2, ptr = &tab_ptrs[0])
				)
			);
			const auto btnsz = 32_px;
			size_t idx = 0;
			for (; idx < tileblock_sets.size(); ++idx)
			{
				auto navgrid = Column(
					Row(
						Button(type = GUI::Button::type::ICON,
							icon = BTNICON_ARROW_LEFT,
							disabled = (idx <= 0),
							width = btnsz, height = btnsz,
							onPressFunc = [&, idx]()
							{
								zc_swap(tileblock_sets[idx], tileblock_sets[idx-1]);
								--tab_ptrs[0];
								refresh_dlg();
							}),
						Button(type = GUI::Button::type::ICON,
							icon = BTNICON_TRASH,
							width = btnsz, height = btnsz,
							onPressFunc = [&, idx]()
							{
								if (!alert_confirm("Are you sure?", "This layer of tile settings will be erased."))
									return;
								auto it = tileblock_sets.begin();
								std::advance(it, idx);
								tileblock_sets.erase(it);
								if (idx > 0)
									--tab_ptrs[0];
								refresh_dlg();
							}),
						Button(type = GUI::Button::type::ICON,
							icon = BTNICON_ARROW_RIGHT,
							disabled = (idx >= tileblock_sets.size()-1),
							width = btnsz, height = btnsz,
							onPressFunc = [&, idx]()
							{
								zc_swap(tileblock_sets[idx], tileblock_sets[idx+1]);
								++tab_ptrs[0];
								refresh_dlg();
							})
					),
					Rows<2>(
						Button(text = "Copy",
							onPressFunc = [&, idx]()
							{
								bool was_copied = copied_tileblock_set.has_value();
								copied_tileblock_set = tileblock_sets[idx];
								if (!was_copied)
									refresh_dlg(); // un-disable paste buttons
							}),
						Button(text = "Paste",
							disabled = !copied_tileblock_set,
							onPressFunc = [&, idx]()
							{
								tileblock_sets[idx] = *copied_tileblock_set;
								refresh_dlg();
							})
					)
				);
				tabs->add(TabRef(name = to_string(idx),
					Row(
						Rows<3>(
							Button(text = "Edit Req LItems", fitParent = true, prefheight = 2.5_em,
								onPressFunc = [&, idx]()
								{
									dword flags = tileblock_sets[idx].required_litems;
									auto const litem_names = GUI::ZCCheckListData::level_items();
									if(!call_checklist_dialog("Select 'Level Items'",litem_names,flags))
										return;
									tileblock_sets[idx].required_litems = (word)flags;
								}),
							INFOBTN("These level items are required for this layer of tile widgets to appear. (ex. 'only visible with map')"),
							Frame(fitParent = true, padding = 0_px,
								Rows<2>(vPadding = 0_px, rowSpacing = 0_px,
									Checkbox(_EX_RBOX, text = "Invert", vPadding = 1_px,
										checked = tileblock_sets[idx].invert_litem,
										onToggleFunc = [&, idx](bool state)
										{
											tileblock_sets[idx].invert_litem = state;
										}),
									INFOBTN_VS("Invert the LItem requirement (ex. require having none instead of all)", 0_px),
									Checkbox(_EX_RBOX, text = "Any", vPadding = 1_px,
										checked = tileblock_sets[idx].any_litem,
										onToggleFunc = [&, idx](bool state)
										{
											tileblock_sets[idx].any_litem = state;
										}),
									INFOBTN_VS("Make the LItem requirement looser (ex. require having any instead of all)", 0_px)
								)
							),
							Button(text = "Edit Req LStates", fitParent = true, prefheight = 2.5_em,
								onPressFunc = [&, idx]()
								{
									dword flags = tileblock_sets[idx].required_lstate;
									auto const lstates = GUI::ZCCheckListData::level_states();
									if(!call_checklist_dialog("Select 'Level States'", lstates, flags, 8))
										return;
									tileblock_sets[idx].required_lstate = flags;
								}),
							INFOBTN("These level states are required for this layer of tile widgets to appear. (ex. 'only visible with red switch up')"),
							Frame(fitParent = true, padding = 0_px,
								Rows<2>(vPadding = 0_px, rowSpacing = 0_px,
									Checkbox(_EX_RBOX, text = "Invert", vPadding = 1_px,
										checked = tileblock_sets[idx].invert_lstate,
										onToggleFunc = [&, idx](bool state)
										{
											tileblock_sets[idx].invert_lstate = state;
										}),
									INFOBTN_VS("Invert the LevelState requirement (ex. require having none instead of all)", 0_px),
									Checkbox(_EX_RBOX, text = "Any", vPadding = 1_px,
										checked = tileblock_sets[idx].any_lstate,
										onToggleFunc = [&, idx](bool state)
										{
											tileblock_sets[idx].any_lstate = state;
										}),
									INFOBTN_VS("Make the LevelState requirement looser (ex. require having any instead of all)", 0_px)
								)
							),
							Button(text = "Edit Req Screen States", fitParent = true, prefheight = 2.5_em,
								onPressFunc = [&, idx]()
								{
									dword flags = tileblock_sets[idx].required_scr_states;
									auto const scrstate_names = GUI::ZCCheckListData::screen_state();
									if(!call_checklist_dialog("Select 'Screen States'",scrstate_names,flags))
										return;
									tileblock_sets[idx].required_scr_states = flags;
								}),
							INFOBTN("These screen states are required on the matching screen for each tile widget of this layer for them to appear. (ex. 'visited this screen')"),
							Frame(fitParent = true, padding = 0_px, rowSpan = 2,
								Rows<2>(vPadding = 0_px, rowSpacing = 0_px,
									Checkbox(_EX_RBOX, text = "Invert", vPadding = 1_px,
										checked = tileblock_sets[idx].invert_scrstate,
										onToggleFunc = [&, idx](bool state)
										{
											tileblock_sets[idx].invert_scrstate = state;
										}),
									INFOBTN_VS("Invert the Screen/Ex State requirement (ex. require having none instead of all)", 0_px),
									Checkbox(_EX_RBOX, text = "Any", vPadding = 1_px,
										checked = tileblock_sets[idx].any_scrstate,
										onToggleFunc = [&, idx](bool state)
										{
											tileblock_sets[idx].any_scrstate = state;
										}),
									INFOBTN_VS("Make the Screen/Ex State requirement looser (ex. require having any instead of all)", 0_px)
								)
							),
							Button(text = "Edit Req Screen ExStates", fitParent = true, prefheight = 2.5_em,
								onPressFunc = [&, idx]()
								{
									dword flags = tileblock_sets[idx].required_ex_states;
									auto const exstate_names = GUI::ZCCheckListData::ex_state();
									if(!call_checklist_dialog("Select 'Ex States'",exstate_names,flags))
										return;
									tileblock_sets[idx].required_ex_states = flags;
								}),
							INFOBTN("These ex states are required on the matching screen for each tile widget of this layer for them to appear."),
							Button(text = "Edit Tiles", fitParent = true, prefheight = 2.5_em,
								onPressFunc = [&, idx]()
								{
									int32_t w = tfs[3]->getVal();
									int32_t h = tfs[4]->getVal();
									tileblock_sets[idx].tile_data.resize(h);
									for (auto& vec : tileblock_sets[idx].tile_data)
										vec.resize(w);
									if (w == 0 || h == 0) return;
									call_tilepicker_dlg("Tiles " + to_string(idx), tileblock_sets[idx].tile_data);
								}),
							INFOBTN("Edit the tiles that will be assigned to the tile blocks."
								" The 'Width' and 'Height' should be set first, as the popup uses those"
								" to determine how many tiles to display editors for.")
						),
						Column(
							Frame(hAlign = 1.0, navgrid),
							Rows<3>(
								Checkbox(_EX_RBOX, text = "Require Current Screen", colSpan = 2,
									checked = tileblock_sets[idx].you_are_here,
									onToggleFunc = [&, idx](bool state)
									{
										tileblock_sets[idx].you_are_here = state;
									}
								),
								INFOBTN("This layer of tiles will only be visible when the hero is currently on the corresponding screen. (ex. 'you are here' highlight)"),
								
								Label(text = "Require Floor:", hAlign = 1.0),
								TextField(
									fitParent = true, minwidth = 3_em,
									type = GUI::TextField::type::INT_DECIMAL,
									low = -1, high = 255, val = tileblock_sets[idx].required_floor,
									onValChangedFunc = [&, idx](GUI::TextField::type,std::string_view,int32_t val)
									{
										tileblock_sets[idx].required_floor = val;
									}
								),
								INFOBTN("If >-1, this layer of tiles will only be visible when the dmap the hero is currently on has a matching 'Floor' value"),
								Label(text = "Require Level:", hAlign = 1.0),
								TextField(
									fitParent = true, minwidth = 3_em,
									type = GUI::TextField::type::INT_DECIMAL,
									low = -1, high = MAXLEVELS-1, val = tileblock_sets[idx].required_level,
									onValChangedFunc = [&, idx](GUI::TextField::type,std::string_view,int32_t val)
									{
										tileblock_sets[idx].required_level = val;
									}
								),
								INFOBTN("If >-1, this layer of tiles will only be visible when the dmap the hero is currently on has a matching 'Level' value")
							)
						)
					)
				));
			}
			tabs->add(TabRef(name = "+",
				Column(
					Button(text = "Add New TileBlock Layer",
						onPressFunc = [&]()
						{
							tileblock_sets.emplace_back();
							refresh_dlg();
						}),
					Button(text = "Paste New TileBlock Layer",
						disabled = !copied_tileblock_set,
						onPressFunc = [&]()
						{
							tileblock_sets.push_back(*copied_tileblock_set);
							refresh_dlg();
						})
				)
			));
			break;
		}
	}
	updateTitle();
	return window;
}

bool SubscreenWizardDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::UPDATE:
		return false;
	case message::OK:
		return finalize();
	case message::CANCEL:
		return true;

	case message::RSET0: case message::RSET1: case message::RSET2: case message::RSET3: case message::RSET4:
	case message::RSET5: case message::RSET6: case message::RSET7: case message::RSET8: case message::RSET9:
		setRadio(int32_t(msg.message) - int32_t(message::RSET0), int32_t(msg.argument));
		return false;
	}
	return false;
}
