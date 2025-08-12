#ifndef ZC_DIALOG_QRS_H_
#define ZC_DIALOG_QRS_H_

#include "base/qrs.h"
#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/list_data.h>
#include <gui/window.h>
#include <functional>
#include <string_view>

void call_qr_dialog(size_t qrs_per_tab, std::function<void(byte*)> setQRs);
void call_qrsearch_dialog(std::function<void(byte*)> setQRs);
GUI::ListData const& combinedQRList();
GUI::ListData const& combinedZSRList();

class QRDialog: public GUI::Dialog<QRDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TOGGLE_QR, HEADER, RULESET,
		CHEATS, RULETMP, QRSTR_CPY, QRSTR_LOAD, RERUN, SEARCH };

	QRDialog(byte const* qrs, size_t qrs_per_tab, std::function<void(byte*)> setQRs);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	void reloadQRs();
	
	bool searchmode;
private:
	std::shared_ptr<GUI::TextField> mapCountTF;
	std::shared_ptr<GUI::Window> window;
	
	std::function<void(byte*)> setQRs;
	byte local_qrs[QR_SZ];
	byte const* realqrs;
	size_t qrs_per_tab;
	size_t cur_tab;
	size_t qr_subtabs[10];
	
	friend void call_qrsearch_dialog(std::function<void(byte*)> setQRs);
};

#endif

