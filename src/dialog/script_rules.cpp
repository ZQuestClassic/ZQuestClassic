#include "script_rules.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include "gui/use_size.h"

extern GUI::ListData scriptRulesList;
extern GUI::ListData instructionRulesList;
extern GUI::ListData objectRulesList;
extern GUI::ListData drawingRulesList;
extern GUI::ListData bugfixRulesList;

ScriptRulesDialog::ScriptRulesDialog(byte const* qrs, size_t qrs_per_tab, std::function<void(byte*)> setQRs):
	setQRs(setQRs), qrs_per_tab(qrs_per_tab)
{
	memcpy(local_qrs, qrs, QR_SZ);
}

std::shared_ptr<GUI::Widget> ScriptRulesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	return Window(
		title = "Script Rules",
		onClose = message::CANCEL,
		Column(
			TabPanel(
				maxwidth = Size::pixels(zq_screen_w),
				TabRef(
					name = "Script",
					QRPanel(
						margins = 0_px,
						padding = 3_px,
						onToggle = message::TOGGLE_QR,
						qr_ptr = local_qrs,
						count = qrs_per_tab,
						data = scriptRulesList
					)
				),
				TabRef(
					name = "Instructions",
					QRPanel(
						margins = 0_px,
						padding = 3_px,
						onToggle = message::TOGGLE_QR,
						qr_ptr = local_qrs,
						count = qrs_per_tab,
						data = instructionRulesList
					)
				),
				TabRef(
					name = "Object",
					QRPanel(
						margins = 0_px,
						padding = 3_px,
						onToggle = message::TOGGLE_QR,
						qr_ptr = local_qrs,
						count = qrs_per_tab,
						data = objectRulesList
					)
				),
				TabRef(
					name = "Drawing",
					QRPanel(
						margins = 0_px,
						padding = 3_px,
						onToggle = message::TOGGLE_QR,
						qr_ptr = local_qrs,
						count = qrs_per_tab,
						data = drawingRulesList
					)
				),
				TabRef(
					name = "Compat",
					QRPanel(
						margins = 0_px,
						padding = 3_px,
						onToggle = message::TOGGLE_QR,
						qr_ptr = local_qrs,
						count = qrs_per_tab,
						data = bugfixRulesList
					)
				)
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

bool ScriptRulesDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::TOGGLE_QR:
			toggle_bit(local_qrs, msg.argument);
			return false;
		case message::OK:
			setQRs(local_qrs);
			[[fallthrough]];
		case message::CANCEL:
			return true;
	}
	return false;
}
