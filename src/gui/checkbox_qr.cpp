#include "checkbox_qr.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>

#define FONT sized(nfont, lfont_l)

namespace GUI
{

QRCheckbox::QRCheckbox(): Checkbox(), qr(0)
{}

void QRCheckbox::setQR(int newqr)
{
	qr = newqr;
}

int QRCheckbox::onEvent(int event, MessageDispatcher& sendMessage)
{
	assert(event == geTOGGLE);
	if(message >= 0)
		sendMessage(message, qr);
	return -1;
}

}
