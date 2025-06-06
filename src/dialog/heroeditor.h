#ifndef ZC_DIALOG_HEROEDITOR_H_
#define ZC_DIALOG_HEROEDITOR_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/drop_down_list.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <zq/gui/selherotile_swatch.h>
#include <functional>
#include <string_view>
#include <map>
#include <vector>

bool call_hero_editor();

class HeroEditorDialog : public GUI::Dialog<HeroEditorDialog>
{
public:
	enum class message {
		REFR_INFO, SETALLDEFENSE, OK, CANCEL
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	HeroEditorDialog();

	int32_t oldWalkSpr[4][3];
	int32_t oldStabSpr[4][3];
	int32_t oldSlashSpr[4][3];
	int32_t oldRevSlashSpr[4][3];
	int32_t oldFloatSpr[4][3];
	int32_t oldSwimSpr[4][3];
	int32_t oldDiveSpr[4][3];
	int32_t oldPoundSpr[4][3];
	int32_t oldJumpSpr[4][3];
	int32_t oldChargeSpr[4][3];
	int32_t oldCastSpr[3];
	int32_t oldsideswimCastSpr[3];
	int32_t oldHoldSpr[2][3][3];
	int32_t oldDrownSpr[4][3];
	int32_t oldSideDrownSpr[4][3];
	int32_t oldFallSpr[4][3];
	int32_t oldLavaDrownSpr[4][3];
	int32_t oldSideSwimSpr[4][3];
	int32_t oldSideSwimSlashSpr[4][3];
	int32_t oldSideSwimStabSpr[4][3];
	int32_t oldSideSwimPoundSpr[4][3];
	int32_t oldSideSwimChargeSpr[4][3];
	int32_t oldSideSwimHoldSpr[3][3];
	int32_t oldLiftingSpr[4][4];
	int32_t oldLiftingWalkSpr[4][3];

	int32_t local_liftframes[4];
	byte local_defenses[wMAX];

	int32_t oldanimationstyle;
	int32_t oldswimspeed;
	bool local_biglink;
	bool local_lttpwalk;

	GUI::ListData list_defensenames, list_defenses, list_deftypes, list_animations, list_swimtypes;
	
	std::shared_ptr<GUI::SelHeroTileSwatch> liftSwatch[4];
	std::shared_ptr<GUI::DropDownList> ddl_defenses[36];
	std::shared_ptr<GUI::DropDownList> animation_style;
	std::shared_ptr<GUI::DropDownList> swim_speed;
	std::shared_ptr<GUI::Checkbox> biglinkhitbox;
	std::shared_ptr<GUI::Checkbox> diagonalmovement;

	std::shared_ptr<GUI::Window> window;
		
	friend bool call_hero_editor();
};

#endif