#ifndef ZC_DIALOG_SPRITEDATA_H
#define ZC_DIALOG_SPRITEDATA_H

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <zq/gui/tileanim_frame.h>
#include <zq/gui/seltile_swatch.h>
#include <gui/window.h>
#include <functional>

void call_sprite_dlg(int32_t index);

class SpriteDataDialog: public GUI::Dialog<SpriteDataDialog>
{
public:
	enum class message
	{
		OK, CANCEL
	};

	SpriteDataDialog(int32_t index);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::shared_ptr<GUI::SelTileSwatch> tswatch;
	std::shared_ptr<GUI::Checkbox> hflipcb, vflipcb;
	int32_t index;
	char localName[65];
	wpndata& sourceSprite;
	wpndata tempSprite;
	void updateAnimation();
};

#endif
