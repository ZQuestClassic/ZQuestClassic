#include "about.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>

AboutDialog::AboutDialog(std::string title, std::string text):
	dlgTitle(title),
	dlgText(text)
{
	//erase trailing newlines
	if(dlgText.at(dlgText.size()-1) == '\n')
		dlgText.erase(dlgText.find_last_not_of('\n')+1);
}

std::shared_ptr<GUI::Widget> AboutDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	bool devmode = CHECK_CTRL_CMD;
	
	std::shared_ptr<GUI::Grid> windowGrid;
	std::shared_ptr<GUI::Window> window = Window(
		title = std::move(dlgTitle),
		onClose = message::OK,
		hPadding = 0_px, 
		windowGrid = Column(
			hPadding = 0_px, 
			Label(noHLine = true,
				hPadding = 2_em,
				maxLines = 20,
				maxwidth = Size::pixels(zq_screen_w)-12_px-5_em,
				textAlign = 1,
				text = std::move(dlgText)),
			Row(
				Button(text = "Copy Report Info",
					topPadding = 0.5_em,
					onPressFunc = []()
					{
						al_set_clipboard_text(all_get_display(), get_dbreport_string().c_str());
						InfoDialog("Copied", "Report info copied to clipboard!").show();
					}),
				Button(
					text = "&Close",
					topPadding = 0.5_em,
					onClick = message::OK,
					focused = true)
			)
		)
	);
	if(!devmode) return window;
	std::shared_ptr<GUI::Grid> devrow =
		Row(padding = 0_px,
			Button(text = "Load Report Info",
				topPadding = 0.5_em,
				onPressFunc = []()
				{
					if(load_dev_info_clipboard())
						return;
					InfoDialog("Error", "No Report Info could be loaded from the clipboard").show();
				})
		);
	windowGrid->add(devrow);
	return window;
}

bool AboutDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			return true;
	}
	return false;
}

