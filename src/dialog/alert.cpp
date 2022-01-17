#include "alert.h"
#include <gui/builder.h>
#include <utility>

AlertDialog::AlertDialog(std::string title, std::string text, std::function<void(bool)> onEnd):
	InfoDialog(title,text), onEnd(onEnd)
{}

AlertDialog::AlertDialog(std::string title, std::vector<std::string_view> lines, std::function<void(bool)> onEnd):
	InfoDialog(title,lines), onEnd(onEnd)
{}

std::shared_ptr<GUI::Widget> AlertDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = std::move(dlgTitle),
		onEnter = message::OK,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		Column(
			hPadding = 0_px, 
			Label(
				hPadding = sized(1_em,2_em),
				maxLines = 20,
				maxwidth = sized(320_px-8_px-2_em, 800_px-12_px-4_em)-1_em,
				textAlign = 1,
				text = std::move(dlgText)),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK,
					focused=true
				),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL
				)
			)
		)
	);
}

bool AlertDialog::handleMessage(const GUI::DialogMessage<int32_t>& msg)
{
	onEnd((message)msg.message==message::OK);
	return true;
}
