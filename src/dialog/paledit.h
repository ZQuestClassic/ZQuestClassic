#ifndef ZC_DIALOG_PALEDIT_H
#define ZC_DIALOG_PALEDIT_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/palette_frame.h>
#include <gui/tabpanel.h>
#include <functional>
#include <string_view>

void call_paledit_dlg(char* namebuf, byte* cdata, PALETTE* pal, int32_t offset);

class PalEditDialog: public GUI::Dialog<PalEditDialog>
{
public:
	enum class message { OK };

	PalEditDialog(BITMAP* bmp, byte* cdata, PALETTE* pal, char* namebuf, int32_t offset);
	void updatePal();
	void loadPal(size_t tab);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	BITMAP* bmp;
	byte* coldata;
	PALETTE* palt;
	char* namebuf;
	int32_t offset;
	std::shared_ptr<GUI::PaletteFrame> frames[2];
	std::shared_ptr<GUI::TabPanel> tabpan;
};

#endif
