#ifndef ZC_GUI_QRCHECKBOX_H
#define ZC_GUI_QRCHECKBOX_H

#include "widget.h"
#include "checkbox.h"
#include "dialog_ref.h"

namespace GUI
{

class QRCheckbox: public Checkbox
{
public:
	QRCheckbox();
	
	void setQR(int newqr);
	
private:
	int qr;

	int onEvent(int event, MessageDispatcher& sendMessage) override;
};

}

#endif
