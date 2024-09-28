#include "misccolors.h"
#include "gui/builder.h"
#include "base/misctypes.h"
#include "base/util.h"

bool extern saved;
extern miscQdata QMisc;

bool call_misccolor_dialog()
{
	MiscColorDialog().show();
	return true;
}

MiscColorDialog::MiscColorDialog() :
	si(&(QMisc.colors.text))
{
	for (int32_t i = 0; i < 16; i++)
	{
		color_buf[i] = *(si++);	
	}
	color_buf[16] = QMisc.colors.msgtext;
}

std::shared_ptr<GUI::Widget> MiscColorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	std::shared_ptr<GUI::Grid> colors1_tab = Rows<2>();
	std::shared_ptr<GUI::Grid> colors2_tab = Rows<2>();
	std::shared_ptr<GUI::Grid> colors3_tab = Rows<2>();
	
	swatch = MiscColorSwatch(
		hexclicked = -1
	);
	for (int q = 0; q < 17; ++q)
	{
		auto& tab = q < 6 ? colors1_tab : q < 12 ? colors2_tab : colors3_tab;
		tab->add(Label(text=color_strings[q], hAlign = 0.0, rightPadding = 2_em));
		tab->add(TextField(
			hAlign = 1.0,
			fitParent = true, minwidth = 2_em,
			type = GUI::TextField::type::INT_HEX,
			low = 0x00, high = 0xFF, val = color_buf[q],
			onValChangedFunc = [&,q](GUI::TextField::type, std::string_view, int32_t val)
			{
				color_buf[q] = (byte)val;
				swatch->setHexClicked(q);
			}));
	}
	colors3_tab->add(_d);

	window = Window(
		use_vsync = true,
		title = "Misc Colors",
		onClose = message::CANCEL,
		Column(
			Row(
				swatch,
				TabPanel(
					TabRef(name = "1", Row(colors1_tab)),
					TabRef(name = "2", Row(colors2_tab)),
					TabRef(name = "3", Row(colors3_tab))
				)
			),
			Row(vAlign = 1.0, spacing = 2_em,
				Button(
					focused = true,
					text = "Ok",
					minwidth = 90_px,
					onClick = message::OK
				),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL
				)
			)
		)
	);
	return window;
}

bool MiscColorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::OK:
		saved = false;
		si = &(QMisc.colors.text);

		for (int32_t i = 0; i < 16; i++)
		{
			*si = color_buf[i];
			++si;
		}
		QMisc.colors.msgtext = color_buf[16];
		[[fallthrough]];
	case message::CANCEL:
		return true;
	}
	return false;
}