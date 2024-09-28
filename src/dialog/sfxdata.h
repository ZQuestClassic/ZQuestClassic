#ifndef ZC_DIALOG_SFXDATA_H_
#define ZC_DIALOG_SFXDATA_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>
#include <string_view>

bool call_sfxdata_dialog(int32_t index);

class SFXDataDialog : public GUI::Dialog<SFXDataDialog>
{
public:
	enum class message
	{
		REFR_INFO, SAVE, LOAD, DEFAULT, PLAY, STOP, OK, CANCEL
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	SFXDataDialog(int32_t index);

	std::shared_ptr<GUI::Window> window;
	int32_t index;
	string sfxname;
	uint8_t customsfx;

	friend bool call_sfxdata_dialog(int32_t index);
};

#endif
