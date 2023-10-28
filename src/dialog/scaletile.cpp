#include "scaletile.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"

ScaleTileDialog::ScaleTileDialog(int32_t *w, int32_t *h):
	swid(*w), shei(*h), tw(*w), th(*h), twid(w), thei(h)
{}

std::shared_ptr<GUI::Widget> ScaleTileDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Scale Tile",
		onClose = message::CANCEL,
		Column(
			Columns<2>(
				Label(text = "Source Wid:"),
				Label(text = "Source Hei:"),
				Label(text = std::to_string(swid)),
				Label(text = std::to_string(shei)),
				Label(text = "Dest Wid:"),
				Label(text = "Dest Hei:"),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					maxLength = 3, low = 1, high = 20,
					val = swid,
					focused = true,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						tw = val;
					}),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					maxLength = 3, low = 1, high = 20,
					val = shei,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						th = val;
					})
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
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
}

bool ScaleTileDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			*twid = tw;
			*thei = th;
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}
