#ifndef ZC_DIALOG_PALEDIT_H
#define ZC_DIALOG_PALEDIT_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_paledit_dlg(char* namebuf, byte* cdata, PALETTE* pal);

class PalEditDialog: public GUI::Dialog<PalEditDialog>
{
public:
	enum class message { OK };

	PalEditDialog(BITMAP* bmp, byte* cdata, PALETTE* pal, char* namebuf);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	BITMAP* bmp;
	byte* coldata;
	PALETTE* palt;
	char* namebuf;
};

#endif
