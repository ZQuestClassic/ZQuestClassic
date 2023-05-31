#ifndef ZC_DIALOG_PALEDIT_H
#define ZC_DIALOG_PALEDIT_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/tabpanel.h>
#include <gui/button.h>
#include <zq/gui/palette_frame.h>
#include <functional>
#include <string_view>
#include "base/colors.h"

bool call_paledit_dlg(char* namebuf, byte* cdata, PALETTE* pal, int32_t offset, int32_t index);

class PalEditDialog: public GUI::Dialog<PalEditDialog>
{
public:
	enum class message { OK, CANCEL };

	PalEditDialog(BITMAP* bmp, byte* cdata, PALETTE* pal, char* namebuf, int32_t offset, int32_t index);
	void updatePal();
	void loadPal();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	BITMAP* bmp;
	byte* coldata;
	PALETTE* palt;
	char* namebuf;
	int32_t offset;
	int32_t index;
	RGB undo[16*pdLEVEL];
	RGB undo1[16*pdLEVEL];
	std::shared_ptr<GUI::PaletteFrame> frames[2];
	std::shared_ptr<GUI::TabPanel> tabpan;
	std::shared_ptr<GUI::Button> cyclebutton;
};

#endif
