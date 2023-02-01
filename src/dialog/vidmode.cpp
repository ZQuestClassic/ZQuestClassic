#include "vidmode.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>

extern int32_t zq_screen_w, zq_screen_h;

void call_vidmode_dlg()
{
	VidModeDialog().show();
}

VidModeDialog::VidModeDialog(){}

static int32_t resx, resy;
std::shared_ptr<GUI::Widget> VidModeDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	char resbuf[32];
	resx = al_get_display_width(all_get_display());
	resy = al_get_display_height(all_get_display());
	sprintf(resbuf, "Res: %dx%d", resx, resy);
	std::shared_ptr<GUI::Grid> resbtn_grid;
	std::shared_ptr<GUI::Window> window = Window(
		title = "Video Mode",
		onClose = message::CANCEL,
		Column(
			hPadding = 0_px, 
			Label(text = resbuf),
			Row(
				Label(text = "New Res:"),
				rx = TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					low = zq_screen_w, high = 20000, val = resx,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						resx = val;
					}),
				ry = TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					low = zq_screen_h, high = 20000, val = resy,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						resy = val;
					})
			),
			resbtn_grid = Rows<4>(
				Button(text = "1x", onPressFunc = [&]()
					{
						resx = int32_t(zq_screen_w*1);
						resy = int32_t(zq_screen_h*1);
						rx->setVal(resx);
						ry->setVal(resy);
					}),
				Button(text = "1.5x", onPressFunc = [&]()
					{
						resx = int32_t(zq_screen_w*1.5);
						resy = int32_t(zq_screen_h*1.5);
						rx->setVal(resx);
						ry->setVal(resy);
					}),
				Button(text = "2x", onPressFunc = [&]()
					{
						resx = int32_t(zq_screen_w*2);
						resy = int32_t(zq_screen_h*2);
						rx->setVal(resx);
						ry->setVal(resy);
					}),
				Button(text = "2.5x", onPressFunc = [&]()
					{
						resx = int32_t(zq_screen_w*2.5);
						resy = int32_t(zq_screen_h*2.5);
						rx->setVal(resx);
						ry->setVal(resy);
					})
			),
			Row(
				Button(
					text = "&OK",
					minwidth = 90_lpx,
					onClick = message::OK,
					focused = true),
				Button(
					text = "&Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
	if(!is_large)
	{
		resbtn_grid->add(Button(text = "3x", onPressFunc = [&]()
		{
			resx = int32_t(zq_screen_w*3);
			resy = int32_t(zq_screen_h*3);
			rx->setVal(resx);
			ry->setVal(resy);
		}));
		resbtn_grid->add(Button(text = "3.5x", onPressFunc = [&]()
		{
			resx = int32_t(zq_screen_w*3.5);
			resy = int32_t(zq_screen_h*3.5);
			rx->setVal(resx);
			ry->setVal(resy);
		}));
		resbtn_grid->add(Button(text = "4x", onPressFunc = [&]()
		{
			resx = int32_t(zq_screen_w*4);
			resy = int32_t(zq_screen_h*4);
			rx->setVal(resx);
			ry->setVal(resy);
		}));
		resbtn_grid->add(Button(text = "4.5x", onPressFunc = [&]()
		{
			resx = int32_t(zq_screen_w*4.5);
			resy = int32_t(zq_screen_h*4.5);
			rx->setVal(resx);
			ry->setVal(resy);
		}));
	}
	resx = rx->getVal();
	resy = ry->getVal();
	return window;
}

bool VidModeDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			int o_x, o_y;
			al_get_window_position(all_get_display(), &o_x, &o_y);
			int o_w = al_get_display_width(all_get_display()),
			    o_h = al_get_display_height(all_get_display());
			
			int cx = o_x + (o_w/2), cy = o_y + (o_h/2);
			int nx = cx - (resx/2), ny = cy - (resy/2);
			if(nx < 0) nx = 0;
			if(ny < 0) ny = 0;
			al_resize_display(all_get_display(), resx, resy);
			al_set_window_position(all_get_display(), nx, ny);
			return true;
		}
		case message::CANCEL:
		default:
			return true;
	}
	return true;
}

