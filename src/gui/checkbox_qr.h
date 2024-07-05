#ifndef ZC_GUI_QRCHECKBOX_H_
#define ZC_GUI_QRCHECKBOX_H_

#include "gui/widget.h"
#include "gui/checkbox.h"
#include "gui/dialog_ref.h"

namespace GUI
{

class QRCheckbox: public Checkbox
{
public:
	QRCheckbox();
	
	void setQR(int32_t newqr);
	
private:
	int32_t qr;

	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
