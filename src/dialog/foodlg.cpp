#include "foodlg.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"

void call_foo_dlg()
{
	return; //This dialog for testing purposes only
	FooDialog(false, [&](int v, int v2)
	{
		return;
		char buf[32]={0};
		sprintf(buf, "Value: %d, %d", v, v2);
		jwin_alert("Foo", buf, NULL, NULL, "OK", NULL, 0, 0, lfont);
	}).show();
}

FooDialog::FooDialog(int v, std::function<void(int,bool)> setVal):
	v(v), setVal(setVal)
{}

std::shared_ptr<GUI::Widget> FooDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Foo",
		onEnter = message::OK,
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
					onSelectFunc = [&](int t, int c)
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
					onClick = message::OK),
				Button(
					text = "Cancel",
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
		default:
			return true;
	}
}
