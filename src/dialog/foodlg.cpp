#include "foodlg.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
#endif

void call_foo_dlg()
{
	return; //This dialog for testing purposes only
	FooDialog(false, [&](int32_t v, int32_t v2)
	{
		return;
		char buf[32]={0};
		sprintf(buf, "Value: %d, %d", v, v2);
		jwin_alert("Foo", buf, NULL, NULL, "OK", NULL, 0, 0, get_zc_font(font_lfont));
	}).show();
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

FooDialog::FooDialog(int32_t v, std::function<void(int32_t,bool)> setVal):
	v(v), setVal(setVal)
{}

std::shared_ptr<GUI::Widget> FooDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Foo",
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Label(text = "Enter a value:"),
				this->datafield = TextField(
					type = GUI::TextField::type::SWAP_ZSINT,
					maxLength = 12,
					val = v,
					focused = true),
				SelTileSwatch(tile = 1, cset = 8,
					onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t)
					{
						setVal(t,c);
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

bool FooDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			setVal(datafield->getVal(),false);
			return true;
		case message::CANCEL:
			setVal(0,true);
			return true;
	}
	return false;
}
