#include "heroeditor.h"
#include "alert.h"
#include "gui/builder.h"
#include "gui/key.h"
#include "info.h"
#include "base/zsys.h"
#include "init.h"
#include "hero_tiles.h"
#include "tiles.h"
#include "zc_list_data.h"
#include "base/qrs.h"
#include <fmt/format.h>

extern int32_t CSet; //is this needed with the hero sprites always using CSet 6
extern bool saved;

bool call_hero_editor()
{
	HeroEditorDialog().show();
	return false;
}

static size_t hero_tabs[10] = { 0 };

HeroEditorDialog::HeroEditorDialog():
	list_deftypes(GUI::ZCListData::deftypes()),
	list_animations(GUI::ZCListData::heroanimations()),
	list_swimtypes(GUI::ZCListData::swimtypes())
{
	memcpy(local_defenses, hero_defenses, sizeof(hero_defenses));
	
}

std::shared_ptr<GUI::Widget> HeroEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	//if you add new sprites you must store the original here
	memcpy(oldWalkSpr, walkspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldStabSpr, stabspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSlashSpr, slashspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldRevSlashSpr, revslashspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldFloatSpr, floatspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSwimSpr, swimspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldDiveSpr, divespr, 4 * 3 * sizeof(int32_t));
	memcpy(oldPoundSpr, poundspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldJumpSpr, jumpspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldChargeSpr, chargespr, 4 * 3 * sizeof(int32_t));
	memcpy(oldCastSpr, castingspr, 3 * sizeof(int32_t));
	memcpy(oldsideswimCastSpr, sideswimcastingspr, 3 * sizeof(int32_t));
	memcpy(oldHoldSpr, holdspr, 2 * 3 * 3 * sizeof(int32_t));
	memcpy(oldDrownSpr, drowningspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideDrownSpr, sidedrowningspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldFallSpr, fallingspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldLavaDrownSpr, drowning_lavaspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideSwimSpr, sideswimspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideSwimSlashSpr, sideswimslashspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideSwimStabSpr, sideswimstabspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideSwimPoundSpr, sideswimpoundspr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideSwimChargeSpr, sideswimchargespr, 4 * 3 * sizeof(int32_t));
	memcpy(oldSideSwimHoldSpr, sideswimholdspr, 3 * 3 * sizeof(int32_t));
	memcpy(oldLiftingSpr, liftingspr, 4 * 4 * sizeof(int32_t));
	memcpy(oldLiftingWalkSpr, liftingwalkspr, 4 * 3 * sizeof(int32_t));
	for(int q=0; q < 4; q++)
	{
		local_liftframes[q] = vbound(liftingspr[q][spr_frames], 1, 255);
	}

	oldanimationstyle = zinit.heroAnimationStyle;
	oldswimspeed = zinit.hero_swim_speed;
	local_biglink = get_qr(qr_LTTPCOLLISION);
	local_lttpwalk = get_qr(qr_LTTPWALK);

	liftSwatch[0] = SelHeroTileSwatch(heroSprite = ls_lifting, dir = up, frames = local_liftframes[up]);
	liftSwatch[1] = SelHeroTileSwatch(heroSprite = ls_lifting, dir = down, frames = local_liftframes[down]);
	liftSwatch[2] = SelHeroTileSwatch(heroSprite = ls_lifting, dir = left, frames = local_liftframes[left]);
	liftSwatch[3] = SelHeroTileSwatch(heroSprite = ls_lifting, dir = right, frames = local_liftframes[right]);

	auto landtab = TabPanel(
		ptr = &hero_tabs[1],
		TabRef(name = "Sprites (Land)", TabPanel(
			ptr = &hero_tabs[2],
			TabRef(name = "Walk", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_walk, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_walk, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_walk, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_walk, dir = right)
					//
				)
			)),
			TabRef(name = "Slash", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_slash, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_slash, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_slash, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_slash, dir = right)
					//
				)
			)),
			TabRef(name = "Slash 2", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_revslash, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_revslash, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_revslash, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_revslash, dir = right)
					//
				)
			)),
			TabRef(name = "Stab", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_stab, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_stab, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_stab, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_stab, dir = right)
					//
				)
			)),
			TabRef(name = "Pound", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_pound, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_pound, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_pound, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_pound, dir = right)
					//
				)
			)),
			TabRef(name = "Jump", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_jump, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_jump, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_jump, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_jump, dir = right)
					//
				)
			)),
			TabRef(name = "Charge", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_charge, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_charge, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_charge, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_charge, dir = right)
					//
				)
			)),
			TabRef(name = "Misc", Column(
				Rows<4>(
					Label(text = "1 Hand", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_landhold1, dir = up),
					Label(text = "Casting", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_cast, dir = up),
					//
					Label(text = "2 Hands", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_landhold2, dir = up),
					_d
				)
			)),
			TabRef(name = "Falling", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_falling, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_falling, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_falling, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_falling, dir = right)
					//
				)
			)),
			TabRef(name = "Lifting", Column(
				Rows<4>(
					Label(text = "Lift Speed:", hAlign = 1.0),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						maxLength = 5,
						val = liftspeed,
						low = 1,
						high = 255,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
						{liftspeed = val;rerun_dlg;}
					),
					_d,
					_d,
					Label(text = "Up", hAlign = 1.0),
					liftSwatch[up],
					Label(text = "Down", hAlign = 1.0),
					liftSwatch[down],
					//
					Label(text = "Num Frames:", hAlign = 1.0),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						maxLength = 3,
						val = liftSwatch[0]->getFrames(),
						low = 1,
						high = 255,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{liftSwatch[0]->setFrames(val); rerun_dlg; }
					),
					Label(text = "Num Frames:", hAlign = 1.0),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						maxLength = 3,
						val = liftSwatch[1]->getFrames(),
						low = 1,
						high = 255,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{liftSwatch[1]->setFrames(val); rerun_dlg; }
					),
					//
					Label(text = "Left", hAlign = 1.0),
					liftSwatch[left],
					Label(text = "Right", hAlign = 1.0),
					liftSwatch[right],
					//
					Label(text = "Num Frames:", hAlign = 1.0),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						maxLength = 3,
						val = liftSwatch[2]->getFrames(),
						low = 1,
						high = 255,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{liftSwatch[2]->setFrames(val); rerun_dlg; }
					),
					Label(text = "Num Frames:", hAlign = 1.0),
					TextField(
						type = GUI::TextField::type::INT_DECIMAL,
						maxLength = 3,
						val = liftSwatch[3]->getFrames(),
						low = 1,
						high = 255,
						fitParent = true,
						onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{liftSwatch[3]->setFrames(val); rerun_dlg; }
					)
					//
				)
			)),
			TabRef(name = "Lift + Walk", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_liftwalk, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_liftwalk, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_liftwalk, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_liftwalk, dir = right)
					//
				)
			))
		))
	);
	


	auto defensetab = TabPanel(
		ptr = &hero_tabs[7],
		TabRef(name = "Defenses", TabPanel(
			ptr = &hero_tabs[8],
			TabRef(name = "Enemy 1", Rows<3>(
				Label(text = "Fireball Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[0] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewFireball],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewFireball] = val;
					}),
				Button(
					fitParent = true,
					text = "Set All",
					minwidth = 40_px,
					height = 1.5_em,
					onClick = message::SETALLDEFENSE
				),
				Label(text = "Boomerang Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[1] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewBrang],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewBrang] = val;
					}),
				_d,
				Label(text = "Arrow Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[2] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewArrow],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewArrow] = val;
					}),
				_d,
				Label(text = "Sword Beam Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[3] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewSword],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewSword] = val;
					}),
				_d,
				Label(text = "Rock Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[4] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewRock],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewRock] = val;
					}),
				_d,
				Label(text = "Magic Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[5] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewMagic],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewMagic] = val;
					}),
				_d,
				Label(text = "Bomb (object) Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[6] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewBomb],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewBomb] = val;
					}),
				_d,
				Label(text = "S. Bomb (object) Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[7] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewSBomb],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewSBomb] = val;
					}),
				_d,
				Label(text = "Bomb (explode) Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[8] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewLitBomb],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewLitBomb] = val;
					}),
				_d,
				Label(text = "S. Bomb (explode) Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[9] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewLitSBomb],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewLitSBomb] = val;
					}),
				_d
			)),
			TabRef(name = "Enemy 2", Rows<3>(
				Label(text = "Fire Trail Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[10] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewFireTrail],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewFireTrail] = val;
					}),
				_d,
				Label(text = "Flame Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[11] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewFlame],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewFlame] = val;
					}),
				_d,
				Label(text = "Wind Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[12] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewWind],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewWind] = val;
					}),
				_d,
				Label(text = "Flame 2 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[13] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewFlame2],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewFlame2] = val;
					}),
				_d,
				Label(text = "Flame 2 Trail Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[14] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewFlame2Trail],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewFlame2Trail] = val;
					}),
				_d,
				Label(text = "Ice Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[15] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewIce],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewIce] = val;
					}),
				_d,
				Label(text = "Fireball 2 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[16] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[ewFireball2],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[ewFireball2] = val;
					}),
				_d
			)),
			TabRef(name = "Other", Rows<3>(
				Label(text = "Candle Fire Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[17] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wFire],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wFire] = val;
					}),
				_d,
				Label(text = "Player Bomb Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[18] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wBomb],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wBomb] = val;
					}),
				_d,
				Label(text = "Refl. Magic Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[19] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefMagic],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefMagic] = val;
					}),
				_d,
				Label(text = "Refl. Fireball Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[20] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefFireball],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefFireball] = val;
					}),
				_d,
				Label(text = "Refl. Rock Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[21] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefRock],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefRock] = val;
					}),
				_d,
				Label(text = "Refl. Sword Beam Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[22] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefBeam],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefBeam] = val;
					}),
				_d,
				Label(text = "Refl. Arrow Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[23] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefArrow],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefArrow] = val;
					}),
				_d,
				Label(text = "Refl. Fire Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[24] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefFire],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefFire] = val;
					}),
				_d,
				Label(text = "Refl. Fire 2 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[25] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wRefFire2],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wRefFire2] = val;
					}),
				_d
			)),
			TabRef(name = "Custom", Rows<3>(
				Label(text = "Custom Weapon 1 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[26] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript1],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript1] = val;
					}),
				_d,
				Label(text = "Custom Weapon 2 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[27] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript2],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript2] = val;
					}),
				_d,
				Label(text = "Custom Weapon 3 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[28] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript3],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript3] = val;
					}),
				_d,
				Label(text = "Custom Weapon 4 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[29] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript4],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript4] = val;
					}),
				_d,
				Label(text = "Custom Weapon 5 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[30] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript5],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript5] = val;
					}),
				_d,
				Label(text = "Custom Weapon 6 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[31] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript6],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript6] = val;
					}),
				_d,
				Label(text = "Custom Weapon 7 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[32] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript7],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript7] = val;
					}),
				_d,
				Label(text = "Custom Weapon 8 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[33] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript8],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript8] = val;
					}),
				_d,
				Label(text = "Custom Weapon 9 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[34] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript9],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript9] = val;
					}),
				_d,
				Label(text = "Custom Weapon 10 Defense:", hAlign = 1.0, rightPadding = DEFAULT_PADDING * 1.5),
				ddl_defenses[35] = DropDownList(
					data = list_deftypes,
					hAlign = 0.0,
					selectedValue = local_defenses[wScript10],
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						local_defenses[wScript10] = val;
					}),
				_d
			))
		))
	);

	auto liquidtab = TabPanel(
		ptr = &hero_tabs[3],
		TabRef(name = "Sprites (Liquid)", TabPanel(
			ptr = &hero_tabs[4],
			TabRef(name = "Float", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_float, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_float, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_float, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_float, dir = right)
					//
				)
			)),
			TabRef(name = "Swim", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_swim, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_swim, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_swim, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_swim, dir = right)
					//
				)
			)),
			TabRef(name = "Dive", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_dive, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_dive, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_dive, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_dive, dir = right)
					//
				)
			)),
			TabRef(name = "Drown", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_drown, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_drown, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_drown, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_drown, dir = right)
					//
				)
			)),
			TabRef(name = "Hold", Column(
				Rows<2>(
					Label(text = "1 Hand", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_waterhold1, dir = up),
					Label(text = "2 Hands", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_waterhold2, dir = up)
				)
			)),
			TabRef(name = "Lava Drown", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_lavadrown, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_lavadrown, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_lavadrown, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_lavadrown, dir = right)
					//
				)
			))
		))
	);

	auto sideliquidtab = TabPanel(
		ptr = &hero_tabs[5],
		TabRef(name = "Sprites (Side Liquid)", TabPanel(
			ptr = &hero_tabs[6],
			TabRef(name = "Swim", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswim, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswim, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswim, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswim, dir = right)
					//
				)
			)),
			TabRef(name = "S. Slash", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimslash, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimslash, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimslash, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimslash, dir = right)
					//
				)
			)),
			TabRef(name = "S. Stab", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimstab, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimstab, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimstab, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimstab, dir = right)
					//
				)
			)),
			TabRef(name = "S. Pound", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimpound, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimpound, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimpound, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimpound, dir = right)
					//
				)
			)),
			TabRef(name = "Charge", Column(
				Rows<4>(
					Label(text = "Up", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimcharge, dir = up),
					Label(text = "Down", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimcharge, dir = down),
					//
					Label(text = "Left", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimcharge, dir = left),
					Label(text = "Right", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimcharge, dir = right)
					//
				)
			)),
			TabRef(name = "Misc", Column(
				Rows<4>(
					Label(text = "1 Hand", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sidewaterhold1, dir = up),
					Label(text = "2 Hands", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sidewaterhold2, dir = up),
					//
					Label(text = "Casting", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sideswimcast, dir = up),
					Label(text = "Drown", hAlign = 1.0),
					SelHeroTileSwatch(heroSprite = ls_sidedrown, dir = up)
					//
				)
			))
		))
	);

	auto optionstab = TabPanel(
		ptr = &hero_tabs[9],
		TabRef(name = "Options", Rows<2>(width = 24_em,
			Label(text = "Animation Style:", hAlign = 1.0),
			animation_style = DropDownList(
				data = list_animations,
				hAlign = 0.0,
				selectedValue = zinit.heroAnimationStyle,
				fitParent = true,
				onSelectFunc = [&](int32_t val) {zinit.heroAnimationStyle = val; }),
			Label(text = "Swim Speed:", hAlign = 1.0),
			swim_speed = DropDownList(
				data = list_swimtypes,
				hAlign = 0.0,
				selectedValue = zinit.hero_swim_speed,
				fitParent = true,
				onSelectFunc = [&](int32_t val) {zinit.hero_swim_speed = val; }),
			biglinkhitbox = Checkbox(fitParent = true, text = "Big Link Hitbox", checked = local_biglink,
				onToggleFunc = [&](bool state) {local_biglink = state; }),
			_d,
			diagonalmovement = Checkbox(fitParent = true, text = "Diagonal Movement", checked = local_lttpwalk,
				onToggleFunc = [&](bool state) {local_lttpwalk = state; }),
			_d
		))
	);

	window = Window(
		use_vsync = true,
		title = "Hero Sprites",
		info = "Edit the Hero's sprites, defenses, and animation rules",
		onClose = message::CANCEL,

		Column(
			TabPanel(
				ptr = &hero_tabs[0],
				TabRef(name = "Sprites (Land)", landtab),
				TabRef(name = "Sprites (Liquid)", liquidtab),
				TabRef(name = "Sprites (Side Liquid)", sideliquidtab),
				TabRef(name = "Defenses", defensetab),
				TabRef(name = "Options", optionstab)
			),
			Row(
				Button(
					fitParent = true,
					text = "Ok",
					width = 6_em,
					height = 3_em,
					onClick = message::OK
				),
				_d,
				Button(
					fitParent = true,
					text = "Cancel",
					width = 6_em,
					height = 3_em,
					onClick = message::CANCEL
				)
			)
		)
	);
	return window;
}

bool HeroEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::SETALLDEFENSE:
	{
		for (int32_t i = 0; i < 256; i++)
		{
			local_defenses[i] = local_defenses[ewFireball];
		}
		return false;
	}
	case message::OK:
	{
		saved = false;
		set_qr(qr_LTTPCOLLISION, local_biglink ? 1 : 0);
		set_qr(qr_LTTPWALK, local_lttpwalk ? 1 : 0);

		//Save Hero defenses
		for (int32_t i = 0; i < wMax - wEnemyWeapons - 1; i++)
		{
			hero_defenses[wEnemyWeapons + i] = local_defenses[wEnemyWeapons + i];
		}
		hero_defenses[wFire] = local_defenses[wFire];
		hero_defenses[wBomb] = local_defenses[wBomb];
		hero_defenses[wRefMagic] = local_defenses[wRefMagic];
		hero_defenses[wRefFireball] = local_defenses[wRefFireball];
		hero_defenses[wRefRock] = local_defenses[wRefRock];
		hero_defenses[wRefBeam] = local_defenses[wRefBeam];

		hero_defenses[wScript1] = local_defenses[wScript1];
		hero_defenses[wScript2] = local_defenses[wScript2];
		hero_defenses[wScript3] = local_defenses[wScript3];
		hero_defenses[wScript4] = local_defenses[wScript4];
		hero_defenses[wScript5] = local_defenses[wScript5];
		hero_defenses[wScript6] = local_defenses[wScript6];
		hero_defenses[wScript7] = local_defenses[wScript7];
		hero_defenses[wScript8] = local_defenses[wScript8];
		hero_defenses[wScript9] = local_defenses[wScript9];
		hero_defenses[wScript10] = local_defenses[wScript10];

		return true;
		break;
	}
	case message::CANCEL:
	{
		zinit.heroAnimationStyle = oldanimationstyle;
		zinit.hero_swim_speed = oldswimspeed;
		memcpy(walkspr, oldWalkSpr, 4 * 3 * sizeof(int32_t));
		memcpy(stabspr, oldStabSpr, 4 * 3 * sizeof(int32_t));
		memcpy(slashspr, oldSlashSpr, 4 * 3 * sizeof(int32_t));
		memcpy(revslashspr, oldRevSlashSpr, 4 * 3 * sizeof(int32_t));
		memcpy(floatspr, oldFloatSpr, 4 * 3 * sizeof(int32_t));
		memcpy(swimspr, oldSwimSpr, 4 * 3 * sizeof(int32_t));
		memcpy(divespr, oldDiveSpr, 4 * 3 * sizeof(int32_t));
		memcpy(poundspr, oldPoundSpr, 4 * 3 * sizeof(int32_t));
		memcpy(jumpspr, oldJumpSpr, 4 * 3 * sizeof(int32_t));
		memcpy(chargespr, oldChargeSpr, 4 * 3 * sizeof(int32_t));
		memcpy(castingspr, oldCastSpr, 3 * sizeof(int32_t));
		memcpy(sideswimcastingspr, oldsideswimCastSpr, 3 * sizeof(int32_t));
		memcpy(holdspr, oldHoldSpr, 2 * 3 * 3 * sizeof(int32_t));
		memcpy(drowningspr, oldDrownSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sidedrowningspr, oldSideDrownSpr, 4 * 3 * sizeof(int32_t));
		memcpy(fallingspr, oldFallSpr, 4 * 3 * sizeof(int32_t));
		memcpy(drowning_lavaspr, oldLavaDrownSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sideswimspr, oldSideSwimSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sideswimslashspr, oldSideSwimSlashSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sideswimstabspr, oldSideSwimStabSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sideswimpoundspr, oldSideSwimPoundSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sideswimchargespr, oldSideSwimChargeSpr, 4 * 3 * sizeof(int32_t));
		memcpy(sideswimholdspr, oldSideSwimHoldSpr, 3 * 3 * sizeof(int32_t));
		memcpy(liftingspr, oldLiftingSpr, 4 * 4 * sizeof(int32_t));
		memcpy(liftingwalkspr, oldLiftingWalkSpr, 4 * 3 * sizeof(int32_t));
		return true;
		break;
	}
	}
	return false;
}