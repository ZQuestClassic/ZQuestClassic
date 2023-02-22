#include "set_password.h"
#include <gui/builder.h>

extern zquestheader header;
void set_questpwd(std::string_view pwd, bool use_keyfile);
void call_password_dlg()
{
	SetPasswordDialog(header.use_keyfile, set_questpwd).show();
}

SetPasswordDialog::SetPasswordDialog(bool useKeyFile,
	std::function<void(std::string_view, bool)> setPassword):
		useKeyFile(useKeyFile), setPassword(setPassword)
{}

std::shared_ptr<GUI::Widget> SetPasswordDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Set Password",
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Label(text = "Enter new password:"),
				this->pwField = TextField(
					maxLength = 255,
					focused = true),
				this->saveKeyFileCB = Checkbox(
					text = "&Save key file",
					checked = this->useKeyFile)
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

bool SetPasswordDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
	case message::OK:
		setPassword(pwField->getText(), saveKeyFileCB->getChecked());
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
}
