#include "common.h"
#include <gui/builder.h>
#include "subscr_grid_dlg.h"
#include "base/initdata.h"
#include "subscr.h"

void mark_save_dirty();

void call_subscr_edit_grid()
{
	SubscrGridDialog().show();
}

SubscrGridDialog::SubscrGridDialog() :
	grid_x(zinit.ss_grid_x), grid_y(zinit.ss_grid_y),
	grid_xofs(zinit.ss_grid_xofs), grid_yofs(zinit.ss_grid_yofs),
	grid_color(zinit.ss_grid_color), show_grid(zinit.ss_flags & ssflagSHOWGRID),
	grid_dots(!(zinit.ss_flags & ssflagGRIDLINES))
{}

static const GUI::ListData sys_colors
{
	{ "Black", 0 },
	{ "Blue", 1 },
	{ "Green", 2 },
	{ "Cyan", 3 },
	{ "Red", 4 },
	{ "Magenta", 5 },
	{ "Brown", 6 },
	{ "Light Gray", 7 },
	{ "Dark Gray", 8 },
	{ "Light Blue", 9 },
	{ "Light Green", 10 },
	{ "Light Cyan", 11 },
	{ "Light Red", 12 },
	{ "Light Magenta", 13 },
	{ "Yellow", 14 },
	{ "White", 15 },
};

std::shared_ptr<GUI::Widget> SubscrGridDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window = Window(
		title = "Subscreen Grid",
		info = "The grid is used by several subscreen editor featues. It can be visible,"
			" to help you align objects- and it is also used for movement and alignment features,"
			" such as holding shift while using the arrow keys moving a widget by an entire grid-size"
			" at a time instead of a pixel at a time.",
		onClose = message::CANCEL,
		Column(
			Column(
				Rows_Columns<2,2>(
					Label(text = "X Size:"),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						low = 1, high = 256,
						val = grid_x,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							grid_x = val;
						}
					),
					Label(text = "Y Size:"),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						low = 1, high = 256,
						val = grid_y,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							grid_y = val;
						}
					),
					Label(text = "X Offset:"),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						low = -256, high = 256,
						val = grid_xofs,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							grid_xofs = val;
						}
					),
					Label(text = "Y Offset:"),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						low = -256, high = 256,
						val = grid_yofs,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							grid_yofs = val;
						}
					)
				),
				Row(
					Label(text = "Color:"),
					DropDownList(data = sys_colors, fitParent = true,
						selectedValue = grid_color,
						onSelectFunc = [&](int32_t val)
						{
							grid_color = val;
						}
					)
				),
				Rows<2>(
					Checkbox(
						text = "Visible Grid", hAlign = 0.0,
						checked = show_grid,
						onToggleFunc = [&](bool state)
						{
							show_grid = state;
						}
					),
					INFOBTN("The grid will be visible in the editor"),
					Checkbox(
						text = "Draw as Dots", hAlign = 0.0,
						checked = grid_dots,
						onToggleFunc = [&](bool state)
						{
							grid_dots = state;
						}
					),
					INFOBTN("If the grid is visible, this determines if it will be drawn as full"
						" grid lines, or just as dots at the intersection points.")
				)
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
	return window;
}

bool SubscrGridDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			zinit.ss_grid_x = grid_x;
			zinit.ss_grid_y = grid_y;
			zinit.ss_grid_xofs = grid_xofs;
			zinit.ss_grid_yofs = grid_yofs;
			zinit.ss_grid_color = grid_color;
			SETFLAG(zinit.ss_flags, ssflagSHOWGRID, show_grid);
			SETFLAG(zinit.ss_flags, ssflagGRIDLINES, !grid_dots);
			mark_save_dirty();
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}
