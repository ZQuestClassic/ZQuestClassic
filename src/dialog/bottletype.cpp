#include "bottletype.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include <boost/format.hpp>

extern miscQdata misc;

void call_bottle_dlg(int32_t index)
{
	if(index < 1 || index > 64) return;
	--index;
	BottleTypeDialog(misc.bottle_types[index]).show();
}

BottleTypeDialog::BottleTypeDialog(bottletype& theBottle):
		sourcebottle(theBottle), tempbottle(theBottle)
{}

std::shared_ptr<GUI::Widget> BottleTypeDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;

	return Window(
		Rows<3>(
			DummyWidget()
		)
	);
}

bool BottleTypeDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{

	case message::OK:
		return true;

	case message::CANCEL:
	default:
		return true;
	}
}
