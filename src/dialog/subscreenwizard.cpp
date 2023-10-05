#include "dialog/subscreenwizard.h"
#include "info.h"
#include "dialog/info.h"
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

extern bool sso_selection[MAXSUBSCREENITEMS];
extern ZCSubscreen subscr_edit;

void call_subscreen_wizard(subwizardtype stype, int32_t x, int32_t y)
{
	SubscreenWizardDialog(stype, x, y).show();
}

void group_select_widget(SubscrWidget* widg)
{
	SubscrPage& pg = subscr_edit.cur_page();
	for (int32_t q = 0; q < pg.size(); ++q)
	{
		if (pg[q] == widg)
		{
			sso_selection[q] = true;
		}
	}
}

SubscreenWizardDialog::SubscreenWizardDialog(subwizardtype stype, int32_t x, int32_t y) : wizard_type(stype),
basex(x), basey(y), flags(0),
list_font(GUI::ZCListData::fonts(false, true, true)),
list_shadtype(GUI::ZCListData::shadow_types())
{
	memset(rs_sz, 0, sizeof(rs_sz));
}

void SubscreenWizardDialog::setRadio(size_t rs, size_t ind)
{
	for (size_t q = 0; q < rs_sz[rs]; ++q)
	{
		auto& radio = rset[rs][q];
		radio->setChecked(ind == q);
	}
}
size_t SubscreenWizardDialog::getRadio(size_t rs)
{
	for (size_t q = 0; q < rs_sz[rs]; ++q)
	{
		if (rset[rs][q]->getChecked())
			return q;
	}
	if (rs_sz[rs] > 0)
		rset[rs][0]->setChecked(true);
	return 0;
}

#define RESET(member) (local_ref.member = src_ref.member)
#define ZERO(member) (local_ref.member = 0)
#define RESET_ZERO(member,flag) (local_ref.member = (flag ? 0 : src_ref.member))

void SubscreenWizardDialog::endUpdate()
{
	for (int32_t q = 0; q < MAXSUBSCREENITEMS; ++q)
	{
		sso_selection[q] = false;
	}
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
				group_select_widget(mt);
				mt->tile = tswatches[0]->getTile();
				mt->cs.set_int_cset(tswatches[0]->getCSet());
				mt->crn = tswatches[0]->getMiniCrn();

				SW_Text* txt = (SW_Text*)create_new_widget_of(widgTEXT, x + 8 + dividerw, y + yoff, false);
				group_select_widget(txt);
				txt->fontid = fontid;
				txt->shadtype = shadtype;
				txt->align = sstaRIGHT;
				txt->text = tfs[4]->getText();
				txt->c_text.type = misccolors[0][0];
				txt->c_text.color = misccolors[0][1];
				txt->c_shadow.type = misccolors[1][0];
				txt->c_shadow.color = misccolors[1][1];
				txt->c_bg.type = misccolors[2][0];
				txt->c_bg.color = misccolors[2][1];

				SW_Counter* ctr = (SW_Counter*)create_new_widget_of(widgCOUNTER, x + 8 + dividerw, y + yoff, false);
				group_select_widget(ctr);
				ctr->fontid = fontid;
				ctr->shadtype = shadtype;
				ctr->align = sstaLEFT;
				ctr->infchar = tfs[6]->getText().at(0);
				ctr->mindigits = tfs[5]->getVal();
				ctr->maxdigits = maxc;
				if(cboxes[1]->getChecked())
					ctr->flags |= SUBSCR_COUNTER_SHOW0;
				ctr->c_text.type = misccolors[0][0];
				ctr->c_text.color = misccolors[0][1];
				ctr->c_shadow.type = misccolors[1][0];
				ctr->c_shadow.color = misccolors[1][1];
				ctr->c_bg.type = misccolors[2][0];
				ctr->c_bg.color = misccolors[2][1];

				if (cboxes[0]->getChecked())
					x += width;
				else
					y += height;
			}
			break;
		}
	}
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
	switch (wizard_type)
	{
		case subwizardtype::SW_ITEM_GRID:
			tyname = "Item Grid";
			break;
		case subwizardtype::SW_COUNTER_BLOCK:
			tyname = "Counter Set";
			break;
	}
	window->setTitle("Subscreen Wizard (" + tyname + ")");
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
							low = -16, high = 256, val = basex
						),
						Label(text = "Y:", hAlign = 1.0),
						tfs[2] = TextField(
							colSpan = 2,
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -16, high = 176, val = basey
						),
						Label(text = "Width:", hAlign = 1.0),
						tfs[3] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 32, val = 5
						),
						INFOBTN("Sets the number of items in the grid along the X-axis"),
						Label(text = "Height:", hAlign = 1.0),
						tfs[4] = TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 32, val = 3
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
							checked = false,
							text = "Non Selectable",
							indx = 0,
							onToggle = message::RSET0
						),
						INFOBTN("Items in this grid are non selectable."),
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = true,
							text = "Loop L/R End to End",
							indx = 1,
							onToggle = message::RSET0
						),
						INFOBTN("Items in this grid will form a complete loop left to right going through every item.\nUse this if you plan to have L/R inventory shifting."),
						rset[0][2] = Radio(
							hAlign = 0.0,
							checked = false,
							text = "Wrap At Sides",
							indx = 2,
							onToggle = message::RSET0
						),
						INFOBTN("Items in this grid will wrap to opposite ends when selecting off the edge.\nThis is not compatible with L/R inventory shifting."),
						rset[0][3] = Radio(
							hAlign = 0.0,
							checked = false,
							text = "No Wrapping",
							indx = 3,
							onToggle = message::RSET0
						),
						INFOBTN("Items in this grid will not connect to anything at the edges.\nThis is not compatible with L/R inventory shifting.")
						)
					)
			);
			break;
		}
		case subwizardtype::SW_COUNTER_BLOCK:
		{
			misccolors[0][0] = ssctMISC;
			misccolors[0][1] = 0;
			misccolors[1][0] = 0;
			misccolors[1][1] = 0;
			misccolors[2][0] = ssctSYSTEM;
			misccolors[2][1] = -1;
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
						MISC_COLOR_SEL("Text Color", 1, misccolors[0][0], misccolors[0][1]),
						MISC_COLOR_SEL("Shadow Color", 2, misccolors[1][0], misccolors[1][1]),
						MISC_COLOR_SEL("Background Color", 3, misccolors[2][0], misccolors[2][1])
					)
				)
			);
			break;
		}
	}
	return window;
}

bool SubscreenWizardDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::UPDATE:
		return false;
	case message::OK:
		endUpdate();
		return true;
	case message::CANCEL:
		return true;

	case message::RSET0: case message::RSET1: case message::RSET2: case message::RSET3: case message::RSET4:
	case message::RSET5: case message::RSET6: case message::RSET7: case message::RSET8: case message::RSET9:
		setRadio(int32_t(msg.message) - int32_t(message::RSET0), int32_t(msg.argument));
		return false;
	}
	return false;
}
