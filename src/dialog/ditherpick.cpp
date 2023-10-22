#include "ditherpick.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "gui/use_size.h"

void call_edit_dither(byte& ditherTy, byte& ditherArg, byte& ditherCol, bool editcol)
{
	DitherPickDialog(ditherTy,ditherArg,ditherCol,editcol).show();
}

DitherPickDialog::DitherPickDialog(byte& ditherTy, byte& ditherArg, byte& ditherCol, bool editcol)
	: dest_dtype(ditherTy), dest_darg(ditherArg),
	dest_dcol(ditherCol), edit_color(editcol),
	dtype(ditherTy), darg(ditherArg),
	dcol(ditherCol)
{}

static GUI::ListData list_dithertype
{
	{ "Checkerboard", dithChecker },
	{ "Criss-Cross", dithCrissCross },
	{ "Diagonal (\\)", dithDiagULDR },
	{ "Diagonal (/)", dithDiagURDL },
	{ "Rows", dithRow },
	{ "Columns", dithCol },
	{ "Dots", dithDots },
	{ "Grid", dithGrid },
	{ "Static 1", dithStatic },
	{ "Static 2", dithStatic2 },
	{ "Static 3", dithStatic3 },
	{ "Dots 2", dithDots2 },
	{ "Dots 3", dithDots3 },
	{ "Dots 4", dithDots4 },
};

std::shared_ptr<GUI::Widget> DitherPickDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Grid> wingrid;
	
	window = Window(
		title = "Dither Editor",
		minwidth = 30_em,
		info = "Select a list of numbers.",
		onClose = message::CANCEL,
		Column(
			Row(
				wingrid = Rows<3>(),
				dithprev = DitherPreview(dither_type = dtype,
					dither_arg = darg, dither_color = dcol,
					preview_scale = 2)
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
	//
	wingrid->add(Label(text = "Type: ", hAlign = 1.0));
	wingrid->add(DropDownList(data = list_dithertype,
			fitParent = true,
			selectedValue = dtype&0xFE,
			onSelectFunc = [&](int32_t val)
			{
				dtype = val|(dtype&1);
				dithprev->setDitherType(dtype);
			}
		));
	wingrid->add(INFOBTN("The type of dither pattern to use"));
	//
	wingrid->add(Checkbox(checked = dtype%2, text = "Inverted",
		_EX_RBOX, colSpan = 2,
		onToggleFunc = [&](bool state)
		{
			SETFLAG(dtype, 1, state);
			dithprev->setDitherType(dtype);
		}));
	wingrid->add(INFOBTN("Should the pattern be inverted?"));
	//
	wingrid->add(Label(text = "Arg: ", hAlign = 1.0));
	wingrid->add(TextField(
		type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
		low = 0, high = 255, val = darg,
		onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
		{
			darg = val;
			dithprev->setDitherArg(val);
		}));
	wingrid->add(INFOBTN("The dither argument. This usually represents some form"
		" of 'size' and/or 'spacing' in the pattern, though it differs for each type."));
	//
	if(edit_color)
	{
		wingrid->add(Label(text = "Color", hAlign = 1.0));
		wingrid->add(ColorSel(val = dcol,
			fitParent = true, onValChangedFunc = [&](byte val)
			{
				dcol = val;
				dithprev->setDitherColor(val);
			}));
		wingrid->add(INFOBTN("The color used by the dither"));
	}
	return window;
}

bool DitherPickDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			dest_dtype = dtype;
			dest_darg = darg;
			if(edit_color)
				dest_dcol = dcol;
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

