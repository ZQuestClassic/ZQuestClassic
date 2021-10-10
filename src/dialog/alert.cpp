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
		Column(
			Label(
				maxwidth = 20_em,
				hPadding = 1_em,
				maxLines = 10,
				text = std::move(dlgText)),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					onClick = message::OK,
					focused=true
				),
				Button(
					text = "Cancel",
					onClick = message::CANCEL
				)
			)
		)
	);
}

bool AlertDialog::handleMessage(const GUI::DialogMessage<int>& msg)
{
	onEnd((message)msg.message==message::OK);
	return true;
}
