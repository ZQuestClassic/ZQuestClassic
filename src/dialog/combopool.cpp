#include "combopool.h"
#include "base/cpool.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "zq/zq_tiles.h"

extern bool saved;
combo_pool temp_cpool;
combo_pool copy_cpool;
static bool copied_cpool = false;
static combo_pool* retptr;
static int32_t cursor_cset;
static bool new_usecs = false;

extern int32_t CSet;
extern combo_pool combo_pools[];

void call_cpool_dlg(int32_t index)
{
	cursor_cset = CSet;
	retptr = &combo_pools[index];
	temp_cpool = *retptr;
	ComboPoolDialog().show();
}

void animate_combos();
static int32_t scroll_pos1 = 0;
std::shared_ptr<GUI::Widget> ComboPoolDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Grid> wingrid, sgrid;
	size_t per_row = 4;
	size_t vis_rows = 3;
	sgrid = Rows<4>();
	window = Window(
		title = "Combo Pool Editor",
		use_vsync = true,
		onTick = [&](){animate_combos(); return ONTICK_REDRAW;},
		minwidth = 30_em,
		info = "When placing a pool, a random combo in the pool is selected to place."
			"\n'Copy' copies the current data to a clipboard that can be"
			"\n'Paste'd later to any combo pool."
			"\n'Tidy' merges duplicate entries, and removes combos with weight 0."
			"\n'Tidy' will automatically be done when clicking 'OK'."
			"\nWhen using '+' or '-', holding SHIFT increments by 10, and CTRL by 100."
			"\nWhen clicking on a combo proc, holding CTRL will copy its' combo to a clipboard,"
			"\nand ALT will paste to it.",
		onClose = message::CANCEL,
		Column(
			wingrid = Column(padding=0_px),
			Row(
				Checkbox(colSpan = 3,
					text = "New Combos default 'Use CSet'", topPadding = 0_px,
					checked = new_usecs,
					onToggleFunc = [&](bool state)
					{
						new_usecs = state;
					})
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(text = "Tidy",
					minwidth = 90_px,
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						temp_cpool.trim();
					}),
				Button(text = "Copy",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						copy_cpool = temp_cpool;
						copied_cpool = true;
						pastebtn->setDisabled(false);
					}),
				pastebtn = Button(text = "Paste",
					minwidth = 90_px,
					disabled = !copied_cpool,
					onClick = message::RELOAD,
					onPressFunc = [&]()
					{
						if(copied_cpool)
							temp_cpool = copy_cpool;
					})
			),
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
	size_t ind = 0;
	size_t hei = 0, wid = 0;
	widgs.clear();
	for(auto& entry : temp_cpool.combos)
	{
		widgs.emplace_back();
		cpool_widg& widg = widgs.back();
		std::shared_ptr<GUI::Grid> row;
		sgrid->add(
			row=Rows<3>(
				framed = true,
				/*widg.left_btn = */Button(text = "<",
					padding = 0_px,
					minheight = Size::pixels(text_height(GUI_DEF_FONT))+DEFAULT_PADDING,
					forceFitH = true,
					disabled = ind==0,
					onClick = message::RELOAD,
					onPressFunc = [&,ind]()
					{
						temp_cpool.swap(ind,ind-1);
					}),
				/*widg.xbtn = */Button(text = "X",
					padding = 0_px,
					minheight = Size::pixels(text_height(GUI_DEF_FONT))+DEFAULT_PADDING,
					forceFitH = true,
					onClick = message::RELOAD,
					onPressFunc = [&,ind]()
					{
						temp_cpool.erase(ind);
					}),
				/*widg.right_btn = */Button(text = ">",
					padding = 0_px,
					minheight = Size::pixels(text_height(GUI_DEF_FONT))+DEFAULT_PADDING,
					forceFitH = true,
					disabled = ind >= temp_cpool.combos.size()-1,
					onClick = message::RELOAD,
					onPressFunc = [&,ind]()
					{
						temp_cpool.swap(ind,ind+1);
					}),
				//Row2
				widg.cpane = SelComboSwatch(colSpan = 3,
					combo = entry.cid,
					cset = entry.cset < 0 ? cursor_cset : entry.cset,
					showvals = true,
					onSelectFunc = [&,ind](int32_t cmb, int32_t c)
					{
						entry.cid = cmb;
						if(entry.cset < 0)
							widgs.at(ind).cpane->setCSet(cursor_cset);
						else
							entry.cset = c;
					}),
				//Row3
				widg.use_cs = Checkbox(colSpan = 3,
					text = "Use CSet", topPadding = 0_px,
					checked = entry.cset > -1,
					onToggleFunc = [&,ind](bool state)
					{
						if(state)
						{
							entry.cset = cursor_cset;
						}
						else
						{
							entry.cset = -1;
							widgs.at(ind).cpane->setCSet(cursor_cset);
						}
					}),
				//Row4
				/*widg.minus_btn = */Button(text = "-",
					padding = 0_px,
					minheight = Size::pixels(text_height(GUI_DEF_FONT))+DEFAULT_PADDING,
					forceFitH = true,
					onPressFunc = [&,ind]()
					{
						size_t inc = 1;
						if(key_shifts & KB_CTRL_FLAG) inc = 100;
						else if(key_shifts & KB_SHIFT_FLAG) inc = 10;
						
						if(entry.quant > inc)
							entry.quant -= inc;
						else entry.quant = 0;
						widgs.at(ind).weight_txt->setVal(entry.quant);
					}),
				Column(
					padding = 0_px,
					Label(text = "Weight:",padding = 0_px),
					widg.weight_txt = TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						low = 0, high = 65535, val = entry.quant,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							entry.quant = val;
						})
				),
				/*widg.plus_btn = */Button(text = "+",
					padding = 0_px,
					minheight = Size::pixels(text_height(GUI_DEF_FONT))+DEFAULT_PADDING,
					forceFitH = true,
					onPressFunc = [&,ind]()
					{
						size_t inc = 1;
						if(key_shifts & KB_CTRL_FLAG) inc = 100;
						else if(key_shifts & KB_SHIFT_FLAG) inc = 10;
						
						if(entry.quant < 65535-inc)
							entry.quant += inc;
						else entry.quant = 65535;
						widgs.at(ind).weight_txt->setVal(entry.quant);
					})
			)
		);
		if(!hei)
		{
			row->calculateSize();
			hei = row->getTotalHeight();
			wid = row->getTotalWidth();
		}
		++ind;
	}
	sgrid->add(Frame(
		minheight = Size::pixels(hei),
		minwidth = Size::pixels(wid),
		padding = 0_px,
		vAlign = 0.5,
		Button(text = "Add Combo",
			vAlign = 0.5,
			onClick = message::RELOAD,
			onPressFunc = [&]()
			{
				int32_t cmb,cs;
				if(select_combo_3(cmb,cs))
					temp_cpool.add(cmb,new_usecs ? cs : -1,1);
			})
	));
	if(ind/per_row >= vis_rows)
	{
		wingrid->add(ScrollingPane(
			ptr = &scroll_pos1,
			minheight = Size::pixels(hei*vis_rows+DEFAULT_PADDING_INT*2),
			sgrid));
	}
	else
	{
		scroll_pos1 = 0;
		wingrid->add(sgrid);
	}
	return window;
}

bool ComboPoolDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			rerun_dlg = true;
			return true;
		case message::OK:
			if(retptr)
			{
				temp_cpool.trim();
				*retptr = temp_cpool;
				saved = false;
			}
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}

