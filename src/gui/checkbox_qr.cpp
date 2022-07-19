#include "checkbox_qr.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include <cassert>
#include <utility>

namespace GUI
{

QRCheckbox::QRCheckbox(): Checkbox(), qr(0)
{}

void QRCheckbox::setQR(int32_t newqr)
{
	qr = newqr;
}

int32_t QRCheckbox::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geTOGGLE);
	if(message >= 0)
		sendMessage(message, qr);
	return -1;
}

}
