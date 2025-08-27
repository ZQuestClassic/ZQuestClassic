#include "combo_trigger_editor.h"
#include "comboeditor.h"
#include "gui/key.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include "tiles.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "items.h"
#include "base/qrs.h"
#include <fmt/format.h>
#include "zinfo.h"
#include "base/combo.h"

using std::string;
using std::to_string;

bool hasCTypeEffects(int32_t type);
bool hasCTypeCauses(int32_t type);

static bool edited = false;

bool call_trigger_editor(ComboEditorDialog& parentdlg, size_t index)
{
	if(index >= parentdlg.local_comboref.triggers.size()) return false;
	edited = false;
	ComboTriggerDialog(parentdlg, parentdlg.local_comboref.triggers[index], index).show();
	return edited;
}

ComboTriggerDialog::ComboTriggerDialog(ComboEditorDialog& parentdlg, combo_trigger& trigger, size_t index):
	local_ref(parentdlg.local_comboref.triggers[index]), parent(parentdlg),
	parent_comboref(parentdlg.local_comboref), index(index), _ice_cs(0)
{
	load_trigger();
}

void ComboTriggerDialog::load_trigger()
{
	force_ice_combo = local_ref.force_ice_combo > -1;
	ice_combo = force_ice_combo ? local_ref.force_ice_combo : 0;
}

//{ Macros

std::shared_ptr<GUI::Checkbox> ComboTriggerDialog::TRIGFLAG(int index, const char* str, int cspan, bool right)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if(index < 0)
	{
		return Checkbox(
			text = str, hAlign = right ? 1.0 : 0.0, colSpan = cspan,
			boxPlacement = right ? GUI::Checkbox::boxPlacement::RIGHT : GUI::Checkbox::boxPlacement::LEFT,
			fitParent = true, disabled = true
		);
	}
	
	return Checkbox(
		text = str, hAlign = right ? 1.0 : 0.0,
		boxPlacement = right ? GUI::Checkbox::boxPlacement::RIGHT : GUI::Checkbox::boxPlacement::LEFT,
		checked = local_ref.trigger_flags.get(index),
		fitParent = true, colSpan = cspan,
		onToggleFunc = [&, index](bool state)
		{
			local_ref.trigger_flags.set(index, state);
		}
	);
}

#define IBTN(info) \
Button( \
	width = 1.5_em, padding = 0_px, forceFitH = true, \
	text = "?", hAlign = 1.0, onPressFunc = [&]() \
	{ \
		InfoDialog("Info", info).show(); \
	} \
)

#define IBTN_T(title, info) \
Button( \
	width = 1.5_em, padding = 0_px, forceFitH = true, \
	text = "?", hAlign = 1.0, onPressFunc = [&]() \
	{ \
		InfoDialog(title, info).show(); \
	} \
)
//}

static bool has_weapon_cause(combo_trigger const& trig)
{
	if (trig.trigger_flags.any({
		TRIGFLAG_SWORD,TRIGFLAG_SWORDBEAM,TRIGFLAG_BRANG,TRIGFLAG_BOMB,TRIGFLAG_SBOMB,
		TRIGFLAG_LITBOMB,TRIGFLAG_LITSBOMB,TRIGFLAG_ARROW,TRIGFLAG_FIRE,TRIGFLAG_WHISTLE,
		TRIGFLAG_BAIT,TRIGFLAG_WAND,TRIGFLAG_MAGIC,TRIGFLAG_WIND,TRIGFLAG_REFMAGIC,
		TRIGFLAG_REFFIREBALL,TRIGFLAG_REFROCK,TRIGFLAG_HAMMER,TRIGFLAG_EWFIREBALL,
		TRIGFLAG_HOOKSHOT,TRIGFLAG_SPARKLE,TRIGFLAG_BYRNA,TRIGFLAG_REFBEAM,TRIGFLAG_STOMP,
		TRIGFLAG_SCRIPT01,TRIGFLAG_SCRIPT02,TRIGFLAG_SCRIPT03,TRIGFLAG_SCRIPT04,TRIGFLAG_SCRIPT05,
		TRIGFLAG_SCRIPT06,TRIGFLAG_SCRIPT07,TRIGFLAG_SCRIPT08,TRIGFLAG_SCRIPT09,TRIGFLAG_SCRIPT10,
		TRIGFLAG_EWARROW,TRIGFLAG_EWBRANG,TRIGFLAG_EWSWORD,TRIGFLAG_EWROCK,
		TRIGFLAG_EWSCRIPT01,TRIGFLAG_EWSCRIPT02,TRIGFLAG_EWSCRIPT03,TRIGFLAG_EWSCRIPT04,TRIGFLAG_EWSCRIPT05,
		TRIGFLAG_EWSCRIPT06,TRIGFLAG_EWSCRIPT07,TRIGFLAG_EWSCRIPT08,TRIGFLAG_EWSCRIPT09,TRIGFLAG_EWSCRIPT10,
		TRIGFLAG_EWMAGIC,TRIGFLAG_EWBBLAST,TRIGFLAG_EWSBBLAST,TRIGFLAG_EWLITBOMB,TRIGFLAG_EWLITSBOMB,
		TRIGFLAG_EWFIRETRAIL,TRIGFLAG_EWFLAME,TRIGFLAG_EWWIND,TRIGFLAG_EWFLAME2,
		TRIGFLAG_THROWN,TRIGFLAG_QUAKESTUN,TRIGFLAG_SQUAKESTUN,TRIGFLAG_ANYFIRE,
		TRIGFLAG_STRONGFIRE,TRIGFLAG_MAGICFIRE,TRIGFLAG_DIVINEFIRE,
		TRIGFLAG_LWREFARROW,TRIGFLAG_LWREFFIRE,TRIGFLAG_LWREFFIRE2})) return true;
	return false;
}
static bool has_trigger_cause(combo_trigger const& trig)
{
	if(trig.triggerbtn && (trig.trigger_flags.any({TRIGFLAG_BTN_TOP,TRIGFLAG_BTN_BOTTOM,
		TRIGFLAG_BTN_LEFT,TRIGFLAG_BTN_RIGHT}))) return true;
	if(trig.trigtimer) return true;
	if(trig.trigger_flags.any({
		TRIGFLAG_SWORD,TRIGFLAG_SWORDBEAM,TRIGFLAG_BRANG,TRIGFLAG_BOMB,TRIGFLAG_SBOMB,
		TRIGFLAG_LITBOMB,TRIGFLAG_LITSBOMB,TRIGFLAG_ARROW,TRIGFLAG_FIRE,TRIGFLAG_WHISTLE,
		TRIGFLAG_BAIT,TRIGFLAG_WAND,TRIGFLAG_MAGIC,TRIGFLAG_WIND,TRIGFLAG_REFMAGIC,
		TRIGFLAG_REFFIREBALL,TRIGFLAG_REFROCK,TRIGFLAG_HAMMER,TRIGFLAG_STEP,TRIGFLAG_STEPSENS,
		TRIGFLAG_SHUTTER,TRIGFLAG_EWFIREBALL,
		TRIGFLAG_HOOKSHOT,TRIGFLAG_SPARKLE,TRIGFLAG_BYRNA,TRIGFLAG_REFBEAM,TRIGFLAG_STOMP,
		TRIGFLAG_SCRIPT01,TRIGFLAG_SCRIPT02,TRIGFLAG_SCRIPT03,TRIGFLAG_SCRIPT04,TRIGFLAG_SCRIPT05,
		TRIGFLAG_SCRIPT06,TRIGFLAG_SCRIPT07,TRIGFLAG_SCRIPT08,TRIGFLAG_SCRIPT09,TRIGFLAG_SCRIPT10,
		TRIGFLAG_AUTOMATIC,TRIGFLAG_LIGHTON,TRIGFLAG_LIGHTOFF,TRIGFLAG_PUSH,TRIGFLAG_LENSON,
		TRIGFLAG_LENSOFF,TRIGFLAG_EWARROW,TRIGFLAG_EWBRANG,TRIGFLAG_EWSWORD,TRIGFLAG_EWROCK,
		TRIGFLAG_EWSCRIPT01,TRIGFLAG_EWSCRIPT02,TRIGFLAG_EWSCRIPT03,TRIGFLAG_EWSCRIPT04,TRIGFLAG_EWSCRIPT05,
		TRIGFLAG_EWSCRIPT06,TRIGFLAG_EWSCRIPT07,TRIGFLAG_EWSCRIPT08,TRIGFLAG_EWSCRIPT09,TRIGFLAG_EWSCRIPT10,
		TRIGFLAG_EWMAGIC,TRIGFLAG_EWBBLAST,TRIGFLAG_EWSBBLAST,TRIGFLAG_EWLITBOMB,TRIGFLAG_EWLITSBOMB,
		TRIGFLAG_EWFIRETRAIL,TRIGFLAG_EWFLAME,TRIGFLAG_EWWIND,TRIGFLAG_EWFLAME2,TRIGFLAG_ENEMIESKILLED,
		TRIGFLAG_SECRETSTR,TRIGFLAG_THROWN,TRIGFLAG_QUAKESTUN,TRIGFLAG_SQUAKESTUN,TRIGFLAG_ANYFIRE,
		TRIGFLAG_STRONGFIRE,TRIGFLAG_MAGICFIRE,TRIGFLAG_DIVINEFIRE,
		TRIGFLAG_TRIGLEVELSTATE,TRIGFLAG_TRIGGLOBALSTATE,TRIGFLAG_TGROUP_LESS,TRIGFLAG_TGROUP_GREATER,
		TRIGFLAG_PUSHEDTRIG,TRIGFLAG_DIVETRIG,TRIGFLAG_DIVESENSTRIG,TRIGFLAG_LWREFARROW,TRIGFLAG_LWREFFIRE,
		TRIGFLAG_LWREFFIRE2,TRIGFLAG_SCREENLOAD,TRIGFLAG_PLAYERLANDHERE,
		TRIGFLAG_PLAYERLANDANYWHERE,TRIGFLAG_CMBTYPECAUSES})) return true;
	if(trig.exstate != -1 && !(trig.trigger_flags.get(TRIGFLAG_UNSETEXSTATE))) return true;
	if(trig.exdoor_dir != -1 && !(trig.trigger_flags.get(TRIGFLAG_UNSETEXDOOR))) return true;
	if(trig.trigcopycat) return true;
	return false;
}
static bool has_trigger_effect(combo_trigger const& trig)
{
	if (trig.trigger_flags.any({ TRIGFLAG_RESETANIM,TRIGFLAG_CMBTYPEFX,TRIGFLAG_KILLWPN,
		TRIGFLAG_CONSUMEITEM,TRIGFLAG_COUNTEREAT,TRIGFLAG_SECRETS,TRIGFLAG_LEVELSTATE,
		TRIGFLAG_GLOBALSTATE,TRIGFLAG_KILLENEMIES,TRIGFLAG_CLEARENEMIES,TRIGFLAG_CLEARLWEAPONS,
		TRIGFLAG_CLEAREWEAPONS,TRIGFLAG_IGNITE_ANYFIRE,TRIGFLAG_IGNITE_STRONGFIRE,
		TRIGFLAG_IGNITE_MAGICFIRE,TRIGFLAG_IGNITE_DIVINEFIRE,TRIGFLAG_TOGGLEDARK,
		TRIGFLAG_LITEM_SET,TRIGFLAG_LITEM_UNSET,TRIGFLAG_TINT_CLEAR,TRIGFLAG_SETPLAYER_X_ABS,
		TRIGFLAG_SETPLAYER_X_REL_CMB,TRIGFLAG_SETPLAYER_Y_ABS,
		TRIGFLAG_SETPLAYER_Y_REL_CMB,TRIGFLAG_SETPLAYER_Z_ABS,
		TRIGFLAG_FORCE_ICE_VX,TRIGFLAG_FORCE_ICE_VY,TRIGFLAG_CANCEL_TRIGGER,
		TRIGFLAG_SET_GRAVITY, TRIGFLAG_REVERT_GRAVITY })) return true;
	if(trig.dest_player_x || trig.dest_player_y || trig.dest_player_z) return true;
	if(trig.force_ice_combo > -1) return true;
	if(trig.dest_player_dir > -1) return true;
	if(trig.trigsfx) return true;
	if(trig.trigchange) return true;
	if(trig.trigcschange) return true;
	if(trig.spawnitem) return true;
	if(trig.spawnenemy) return true;
	if(trig.exdoor_dir != -1) return true;
	if(trig.exstate != -1) return true;
	if(trig.trigcopycat) return true;
	if(trig.trigcooldown) return true;
	if(trig.trig_genscr) return true;
	if(trig.trigtint[0]) return true;
	if(trig.trigtint[1]) return true;
	if(trig.trigtint[2]) return true;
	if(trig.triglvlpalette != -1) return true;
	if(trig.trigbosspalette != -1) return true;
	if(trig.trigwavytime != -1) return true;
	if(trig.trigquaketime != -1) return true;
	if(trig.trig_swjinxtime != -2) return true;
	if(trig.trig_itmjinxtime != -2) return true;
	if(trig.trig_shieldjinxtime != -2) return true;
	if(trig.trig_stuntime != -2) return true;
	if(trig.trig_bunnytime != -2) return true;
	if(trig.trig_msgstr || trig.fail_msgstr) return true;
	if(trig.player_bounce) return true;
	return false;
}

void ComboTriggerDialog::updateWarnings()
{
	warnings.clear();
	bool cause = has_trigger_cause(local_ref);
	bool effect = has_trigger_effect(local_ref);
	bool tgroup = local_ref.trigger_flags.get(TRIGFLAG_TGROUP_CONTRIB);
	if(!tgroup || (cause != effect))
	{
		if(!cause)
			warnings.emplace_back("Trigger has no 'Cause', and will never be triggered!");
		if(!effect)
			warnings.emplace_back("Trigger has no 'Effect', and will do nothing!");
	}
	bool has_ignite = local_ref.trigger_flags.any({
		TRIGFLAG_IGNITE_ANYFIRE,TRIGFLAG_IGNITE_STRONGFIRE,
		TRIGFLAG_IGNITE_MAGICFIRE,TRIGFLAG_IGNITE_DIVINEFIRE
	});
	if(has_ignite || local_ref.trigger_flags.any({TRIGFLAG_KILLWPN,TRIGFLAG_SEPARATEWEAPON}))
	{
		if(!has_weapon_cause(local_ref))
			warnings.emplace_back("Trigger has weapon-specific 'Effect'(s), but no weapon-based causes!");
	}
	if(!has_ignite && local_ref.trigger_flags.get(TRIGFLAG_UNIGNITE_WEAPONS))
		warnings.emplace_back("Trigger has '...Unignite' flag, but no 'Ignite' flags are used!");
	if (local_ref.trigger_flags.all({ TRIGFLAG_SET_GRAVITY, TRIGFLAG_REVERT_GRAVITY }))
		warnings.emplace_back("'Set Gravity' does nothing when 'Revert Gravity' is checked!");
	if(local_ref.trigger_flags.get(TRIGFLAG_UNSETEXSTATE) && local_ref.exstate < 0)
		warnings.emplace_back("Can't unset ExState -1!");
	if(local_ref.trigger_flags.get(TRIGFLAG_UNSETEXDOOR) && local_ref.exdoor_dir < 0)
		warnings.emplace_back("Can't unset ExDoor '(None)' dir!");
	
	
	
	warnbtn->setDisabled(warnings.empty());
}

static size_t trig_tabs[5] = {0};
const std::string minstr = "Min Level (Applies to all):";
const std::string maxstr = "Max Level (Applies to all):";
std::shared_ptr<GUI::Widget> ComboTriggerDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	std::shared_ptr<GUI::Checkbox> cteff_tflag, ctcause_tflag;
	
	window = Window(
		use_vsync = true,
		title = fmt::format("Combo Trigger Editor ({}: {})", index, local_ref.label),
		info = "Edit combo triggers, setting up their causes, conditions, and effects."
			"Hotkeys:\n"
			"Shift -/+: Change Trigger\n",
		onClose = message::CANCEL,
		shortcuts={
			Shift+Minus=message::MINUSTRIGGER,
			Shift+MinusPad=message::MINUSTRIGGER,
			Shift+Equals=message::PLUSTRIGGER,
			Shift+PlusPad=message::PLUSTRIGGER,
		},
		Column(
			Row(
				Label(text = "Label:"),
				TextField(
					text = local_ref.label,
					maxLength = 64,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
					{
						local_ref.label = str;
						window->setTitle(fmt::format("Combo Trigger Editor ({}: {})", index, local_ref.label));
					}
				),
				INFOBTN("Name of this trigger, shown in the combo editor menu.")
			),
			TabPanel(
				ptr = &trig_tabs[0],
				TabRef(name = "Weapons", TabPanel(
					ptr = &trig_tabs[1],
					TabRef(name = "LWeapons", Row(
						Frame(title = "LW Types", info = "Triggered by any lweapon matching these types and this level requirement",
							Column(
								Row(
									l_minmax_trig = Label(text = "Min Level (Applies to all):"),
									TextField(
										fitParent = true,
										vPadding = 0_px,
										type = GUI::TextField::type::INT_DECIMAL,
										low = 0, high = 214748, val = local_ref.triggerlevel,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.triggerlevel = val;
										}),
									Checkbox(
										text = "Max level instead", hAlign = 0.0,
										checked = local_ref.trigger_flags.get(TRIGFLAG_INVERTMINMAX),
										onToggleFunc = [&](bool state)
										{
											local_ref.trigger_flags.set(TRIGFLAG_INVERTMINMAX,state);
											l_minmax_trig->setText(state ? maxstr : minstr);
										})
								),
								Rows<4>(
									TRIGFLAG(TRIGFLAG_SWORD,"Sword"),
									TRIGFLAG(TRIGFLAG_SWORDBEAM,"Sword Beam"),
									TRIGFLAG(TRIGFLAG_BRANG,"Boomerang"),
									TRIGFLAG(TRIGFLAG_BOMB,"Bomb Boom"),
									TRIGFLAG(TRIGFLAG_SBOMB,"Super Bomb Boom"),
									TRIGFLAG(TRIGFLAG_LITBOMB,"Placed Bomb"),
									TRIGFLAG(TRIGFLAG_LITSBOMB,"Placed Super Bomb"),
									TRIGFLAG(TRIGFLAG_ARROW,"Arrow"),
									TRIGFLAG(TRIGFLAG_FIRE,"Fire"),
									TRIGFLAG(TRIGFLAG_WHISTLE,"Whistle"),
									TRIGFLAG(TRIGFLAG_BAIT,"Bait"),
									TRIGFLAG(TRIGFLAG_WAND,"Wand"),
									TRIGFLAG(TRIGFLAG_MAGIC,"Magic"),
									TRIGFLAG(TRIGFLAG_WIND,"Wind"),
									TRIGFLAG(TRIGFLAG_REFMAGIC,"Refl. Magic"),
									TRIGFLAG(TRIGFLAG_REFFIREBALL,"Refl. Fireball"),
									TRIGFLAG(TRIGFLAG_REFROCK,"Refl. Rock"),
									TRIGFLAG(TRIGFLAG_LWREFARROW,"Refl. Arrow"),
									TRIGFLAG(TRIGFLAG_LWREFFIRE,"Refl. Fire"),
									TRIGFLAG(TRIGFLAG_LWREFFIRE2,"Refl. Fire 2"),
									TRIGFLAG(TRIGFLAG_HAMMER,"Hammer"),
									TRIGFLAG(TRIGFLAG_HOOKSHOT,"Hookshot"),
									TRIGFLAG(TRIGFLAG_SPARKLE,"Sparkle"),
									TRIGFLAG(TRIGFLAG_BYRNA,"Byrna"),
									TRIGFLAG(TRIGFLAG_REFBEAM,"Refl. Beam"),
									TRIGFLAG(TRIGFLAG_STOMP,"Stomp"),
									TRIGFLAG(TRIGFLAG_THROWN, "Thrown"),
									TRIGFLAG(TRIGFLAG_QUAKESTUN, "Quake Hammer"),
									TRIGFLAG(TRIGFLAG_SQUAKESTUN, "S. Quake Hammer"),
									TRIGFLAG(TRIGFLAG_SCRIPT01,"Custom Weapon 1"),
									TRIGFLAG(TRIGFLAG_SCRIPT02,"Custom Weapon 2"),
									TRIGFLAG(TRIGFLAG_SCRIPT03,"Custom Weapon 3"),
									TRIGFLAG(TRIGFLAG_SCRIPT04,"Custom Weapon 4"),
									TRIGFLAG(TRIGFLAG_SCRIPT05,"Custom Weapon 5"),
									TRIGFLAG(TRIGFLAG_SCRIPT06,"Custom Weapon 6"),
									TRIGFLAG(TRIGFLAG_SCRIPT07,"Custom Weapon 7"),
									TRIGFLAG(TRIGFLAG_SCRIPT08,"Custom Weapon 8"),
									TRIGFLAG(TRIGFLAG_SCRIPT09,"Custom Weapon 9"),
									TRIGFLAG(TRIGFLAG_SCRIPT10,"Custom Weapon 10")
								)
							)
						)
					)),
					TabRef(name = "EWeapons", Row(
						Frame(title = "EW Types", info = "Triggered by any eweapon matching these types",
							Column(
								Rows<4>(
									TRIGFLAG(TRIGFLAG_EWFIREBALL,"Fireball"),
									TRIGFLAG(TRIGFLAG_EWARROW,"Arrow"),
									TRIGFLAG(TRIGFLAG_EWBRANG,"Boomerang"),
									TRIGFLAG(TRIGFLAG_EWSWORD,"Sword"),
									TRIGFLAG(TRIGFLAG_EWROCK,"Rock"),
									TRIGFLAG(TRIGFLAG_EWMAGIC,"Magic"),
									TRIGFLAG(TRIGFLAG_EWBBLAST,"Bomb Blast"),
									TRIGFLAG(TRIGFLAG_EWSBBLAST,"SBomb Blast"),
									TRIGFLAG(TRIGFLAG_EWLITBOMB,"Lit Bomb"),
									TRIGFLAG(TRIGFLAG_EWLITSBOMB,"Lit SBomb"),
									TRIGFLAG(TRIGFLAG_EWFIRETRAIL,"Fire Trail"),
									TRIGFLAG(TRIGFLAG_EWFLAME,"Flame"),
									TRIGFLAG(TRIGFLAG_EWWIND,"Wind"),
									TRIGFLAG(TRIGFLAG_EWFLAME2,"Flame 2"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT01,"Custom Weapon 1"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT02,"Custom Weapon 2"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT03,"Custom Weapon 3"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT04,"Custom Weapon 4"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT05,"Custom Weapon 5"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT06,"Custom Weapon 6"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT07,"Custom Weapon 7"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT08,"Custom Weapon 8"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT09,"Custom Weapon 9"),
									TRIGFLAG(TRIGFLAG_EWSCRIPT10,"Custom Weapon 10")
								)
							)
						)
					)),
					TabRef(name = "Other", Row(
						Frame(title = "Burning", info = "Triggered by weapons (lweapon or eweapon) burning with particular levels of fire"
							+ QRHINT({qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS}),
							Rows<2>(
								IBTN("Triggered by weapons burning with 'Normal' type fire.\nNote: Many engine fire sources will *always* count as 'Normal Fire', regardless of other settings."),
								TRIGFLAG(TRIGFLAG_ANYFIRE, "Normal Fire"),
								IBTN("Triggered by weapons burning with 'Strong' type fire"),
								TRIGFLAG(TRIGFLAG_STRONGFIRE, "Strong Fire"),
								IBTN("Triggered by weapons burning with 'Magic' type fire"),
								TRIGFLAG(TRIGFLAG_MAGICFIRE, "Magic Fire"),
								IBTN("Triggered by weapons burning with 'Divine' type fire"),
								TRIGFLAG(TRIGFLAG_DIVINEFIRE, "Divine Fire")
							)
						),
						Frame(
							Rows<2>(
								IBTN("Only weapons with Z=0 will count for this trigger. (No effect in sideview)"),
								TRIGFLAG(TRIGFLAG_ONLY_GROUND_WPN,"Only Grounded Weapons")
							)
						)
					)),
					TabRef(name = "Weapon Effects", Column(
						Frame(fitParent = true, title = "Weapon Effects",
							info = "Trigger effects related to the weapon that triggered this trigger. These"
								" effects will not occur if the combo is triggered by a non-weapon trigger.",
							Rows<2>(hAlign = 0.0,
								IBTN("Destroy the triggering weapon"),
								TRIGFLAG(TRIGFLAG_KILLWPN, "Kill Triggering Weapon"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Normal Fire' triggers."),
								TRIGFLAG(TRIGFLAG_IGNITE_ANYFIRE, "Ignite Weapon (Normal)"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Strong Fire' triggers."),
								TRIGFLAG(TRIGFLAG_IGNITE_STRONGFIRE, "Ignite Weapon (Strong)"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Magic Fire' triggers."),
								TRIGFLAG(TRIGFLAG_IGNITE_MAGICFIRE, "Ignite Weapon (Magic)"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Divine Fire' triggers."),
								TRIGFLAG(TRIGFLAG_IGNITE_DIVINEFIRE, "Ignite Weapon (Divine)"),
								IBTN("Changes the 'Ignite' triggers above to 'Unignite' weapons instead."),
								TRIGFLAG(TRIGFLAG_UNIGNITE_WEAPONS, "...Unignite Instead")
							)
						),
						Frame(vPadding = 0_px, fitParent = true,
							Rows<2>(hAlign = 0.0,
								IBTN("If triggered by a weapon, only the effects from the 'Weapon Effects'"
									" section will occur- all other flags will only apply to non-weapon triggers."),
								TRIGFLAG(TRIGFLAG_SEPARATEWEAPON,"Weapon Separate Triggers")
							)
						)
					))
				)),
				TabRef(name = "Other", Column(
					Row(padding = 0_px,
						Column(framed = true, vAlign = 1.0,
							Row(padding = 0_px,
								Label(text = "Buttons:"),
								trig_buttons_field = TextField(
									fitParent = true,
									bottomPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 0, high = 255, val = local_ref.triggerbtn,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.triggerbtn = val;
									}),
								Button(
									width = 1.5_em, padding = 0_px, forceFitH = true,
									text = "P", hAlign = 1.0, onPressFunc = [&]()
									{
										int32_t flags = local_ref.triggerbtn;
										auto& button_names = GUI::ZCCheckListData::buttons();
										if(!call_checklist_dialog("Select 'Buttons'",button_names,flags))
											return;
										local_ref.triggerbtn = flags;
										trig_buttons_field->setVal(local_ref.triggerbtn);
									}
								),
								IBTN_T("Button Triggers","Buttons used while standing"
									" against the combo from a direction with the 'Btn: [dir]'"
									" flag checked for that side will trigger the combo."
									"\nUse the 'P' button to pick the flags for this value.")
							),
							Column(
								TRIGFLAG(TRIGFLAG_BTN_TOP,"Btn: Top"),
								TRIGFLAG(TRIGFLAG_BTN_BOTTOM,"Btn: Bottom"),
								TRIGFLAG(TRIGFLAG_BTN_LEFT,"Btn: Left"),
								TRIGFLAG(TRIGFLAG_BTN_RIGHT,"Btn: Right")
							)
						),
						Rows<3>(framed = true, vAlign = 1.0,
							Label(text = "Button Prompt:"),
							SelComboSwatch(
									showvals = true,
									combo = local_ref.prompt_cid,
									cset = local_ref.prompt_cs,
									onSelectFunc = [&](int32_t cmb, int32_t c)
									{
										local_ref.prompt_cid = cmb;
										local_ref.prompt_cs = c;
									}
								),
							IBTN("Combo to display when within range to press the triggering button."),
							Label(text = "Fail Prompt:"),
							SelComboSwatch(
									showvals = true,
									combo = local_ref.fail_prompt_cid,
									cset = local_ref.fail_prompt_cs,
									onSelectFunc = [&](int32_t cmb, int32_t c)
									{
										local_ref.fail_prompt_cid = cmb;
										local_ref.fail_prompt_cs = c;
									}
								),
							IBTN("Combo to display when within range to press the triggering button, but the combo trigger's conditions fail"),
							Label(text = "Prompt Xoffset:"),
							TextField(
								fitParent = true,
								bottomPadding = 0_px,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -32768, high = 32767, val = local_ref.prompt_x,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.prompt_x = val;
								}),
							IBTN("X Offset of the prompt combo from the Hero."),
							Label(text = "Prompt Yoffset:"),
							TextField(
								fitParent = true,
								bottomPadding = 0_px,
								type = GUI::TextField::type::INT_DECIMAL,
								low = -32768, high = 32767, val = local_ref.prompt_y,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.prompt_y = val;
								}),
							IBTN("Y Offset of the prompt combo from the Hero.")
						),
						Rows<3>(framed = true, vAlign = 1.0,
							Label(text = "LightBeam:", fitParent = true),
							TextField(
								fitParent = true,
								vPadding = 0_px,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 32, val = local_ref.triglbeam,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.triglbeam = (byte)val;
								}),
							IBTN_T("LightBeam Requirement", "Interacts with the 'Light On' / 'Light Off' flags."
								" If '0', any light beam counts for those flags."
								"\nIf '1-32', only a matching light beam counts for those flags."),
							Label(text = "Timer:", fitParent = true),
							TextField(
								fitParent = true,
								vPadding = 0_px,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 65535, val = local_ref.trigtimer,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.trigtimer = val;
								}),
							IBTN_T("Timed Trigger", "If the value is >0, the combo will"
								" trigger itself every 'n' frames."),
							Label(text = "Push Time:", fitParent = true),
							TextField(
								fitParent = true,
								vPadding = 0_px,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_ref.trig_pushtime,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.trig_pushtime = val;
								}),
							IBTN_T("Push Time", "The number of frames you need to push against a block for 'Push->' to activate.")
						)
					),
					Row(padding = 0_px,
						Rows_Columns<4,5>(framed = true, vAlign = 0.0,
							IBTN("Triggers when stepped on"),
							TRIGFLAG(TRIGFLAG_STEP,"Step->"),
							IBTN("Triggers when stepped on by even a pixel"),
							TRIGFLAG(TRIGFLAG_STEPSENS,"Step-> (Sensitive)"),
							//
							IBTN("Triggered when hit by a Light Beam matching the 'LightBeam' value"),
							TRIGFLAG(TRIGFLAG_LIGHTON,"Light On->"),
							IBTN("Triggered when NOT hit by a Light Beam matching the 'LightBeam' value"),
							TRIGFLAG(TRIGFLAG_LIGHTOFF,"Light Off->"),
							//
							IBTN("Triggered when a " + std::string(ZI.getItemClassName(itype_lens))
								+ " with 'Triggers Lens Trigflag' checked is activated."),
							TRIGFLAG(TRIGFLAG_LENSON,"Lens On->"),
							IBTN("Triggered when a " + std::string(ZI.getItemClassName(itype_lens))
								+ " with 'Triggers Lens Trigflag' checked is NOT activated."),
							TRIGFLAG(TRIGFLAG_LENSOFF,"Lens Off->"),
							//
							IBTN("Triggered when the Hero pushes against the combo"),
							TRIGFLAG(TRIGFLAG_PUSH,"Push->"),
							IBTN("Triggered when the combo is pushed as a pushblock (after it settles into the new position)."),
							TRIGFLAG(TRIGFLAG_PUSHEDTRIG,"Pushed->"),
							//
							IBTN("Triggers when the Hero dives on this combo"),
							TRIGFLAG(TRIGFLAG_DIVETRIG, "Dive->"),
							IBTN("Triggers when the Hero dives on this combo (more sensitive hitbox)"),
							TRIGFLAG(TRIGFLAG_DIVESENSTRIG, "Dive-> (Sensitive)"),
							//
							IBTN("Triggers when screen/region loads, after levelstates and exstates are applied"),
							TRIGFLAG(TRIGFLAG_SCREENLOAD,"Triggers when screen loads"),
							IBTN("Triggers every frame automatically"),
							TRIGFLAG(TRIGFLAG_AUTOMATIC,"Always Triggered"),
							IBTN("Triggers when all enemies are defeated"),
							TRIGFLAG(TRIGFLAG_ENEMIESKILLED, "Enemies->"),
							IBTN("Triggers when room shutters would open"),
							TRIGFLAG(TRIGFLAG_SHUTTER,"Shutter->"),
							IBTN("Triggered when the player lands on this combo (after the general tab 'Landing' SFX plays)"),
							TRIGFLAG(TRIGFLAG_PLAYERLANDHERE, "Land Here->"),
							IBTN("Triggered when the player lands from the air (anywhere on the screen)"),
							TRIGFLAG(TRIGFLAG_PLAYERLANDANYWHERE, "Land Anywhere->"),
							IBTN("Triggers when screen secrets trigger"),
							TRIGFLAG(TRIGFLAG_SECRETSTR, "Secrets->"),
							IBTN("Triggers when the combo's inherent type-based *cause* occurs"
								" (Ex. the Player falling down a Pitfall or drowning in Liquid)."
								" Not available for all combo types; will be greyed out when unavailable."),
							ctcause_tflag = TRIGFLAG(TRIGFLAG_CMBTYPECAUSES, "ComboType Causes->")
						)
					)
				)),
				TabRef(name = "Effects", TabPanel(
					ptr = &trig_tabs[2],
					TabRef(name = "Misc",
						Row(padding = 0_px,
							Column(vAlign = 0.0,
								Rows<3>(framed = true, padding = DEFAULT_PADDING*1.5, hAlign = 1.0,
									Label(text = "Combo Change:", fitParent = true),
									TextField(
										fitParent = true,
										vPadding = 0_px,
										type = GUI::TextField::type::INT_DECIMAL,
										low = -65535, high = 65535, val = local_ref.trigchange,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.trigchange = val;
										}),
									IBTN_T("Combo Change","If the value is not 0, the combo will"
										" change by that much when triggered."
										"\nEx. '1' causes '->Next', '-1' causes '->Prev'."),
									Label(text = "CSet Change:", fitParent = true),
									TextField(
										fitParent = true,
										vPadding = 0_px,
										type = GUI::TextField::type::INT_DECIMAL,
										low = -15, high = 15, val = local_ref.trigcschange,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.trigcschange = val;
										}),
									IBTN_T("CSet Change","If the value is not 0, the cset will"
										" change by that much when triggered."
										"\nEx. '1' causes '->Next CSet', '-1' causes '->Prev CSet'.")
								),
								Rows_Columns<2,5>(framed = true, hAlign = 1.0,
									IBTN("Triggering the combo will trigger screen secrets. Will be permanent,"
										" unless 'Temporary Secrets' screen data flag is checked."),
									TRIGFLAG(TRIGFLAG_SECRETS,"Triggers Secrets"),
									IBTN("Triggering the combo toggles the screen's \"darkness\". This resets upon leaving the screen."),
									TRIGFLAG(TRIGFLAG_TOGGLEDARK,"Toggle Darkness"),
									IBTN("After triggering, the combo animation is reset. If the combo has changed"
										" (by any trigger effect), the new combo is the one that resets."),
									TRIGFLAG(TRIGFLAG_RESETANIM,"Reset Anim"),
									IBTN("Triggers on this combo listed below this trigger will not trigger from this cause."),
									TRIGFLAG(TRIGFLAG_CANCEL_TRIGGER,"Cancel Further Triggers"),
									IBTN("Triggering the combo will cause its inherent type-based effects to occur."
										" Ex. Triggering a 'Signpost' displays its' string, triggering a chest opens it."
										" Not available for all combo types; will be greyed out when unavailable."),
									cteff_tflag = TRIGFLAG(TRIGFLAG_CMBTYPEFX,"->ComboType Effects"),
									//
									IBTN("Kill all enemies on screen (same as 'kill all enemies' item)"),
									TRIGFLAG(TRIGFLAG_KILLENEMIES, "Kill Enemies"),
									IBTN("Delete all enemies on screen."),
									TRIGFLAG(TRIGFLAG_CLEARENEMIES, "Clear Enemies"),
									IBTN("Delete all LWeapons on screen."),
									TRIGFLAG(TRIGFLAG_CLEARLWEAPONS, "Clear LWeapons"),
									IBTN("Delete all EWeapons on screen."),
									TRIGFLAG(TRIGFLAG_CLEAREWEAPONS, "Clear EWeapons")
								)
							),
							Column(vAlign = 0.0,
								Rows<3>(framed = true, padding = DEFAULT_PADDING*1.5,
									hAlign = 0.0,
									Label(text = "SFX:", hAlign = 1.0),
									DropDownList(data = parent.list_sfx,
										vPadding = 0_px, maxwidth = 250_px,
										fitParent = true, selectedValue = local_ref.trigsfx,
										onSelectFunc = [&](int32_t val)
										{
											local_ref.trigsfx = val;
										}),
									IBTN_T("Trigger SFX", "If the value is >0, the combo will"
										" play the specified SFX when triggered."),
									Label(text = "RunFrozen:", hAlign = 1.0),
									DropDownList(data = parent.list_genscr,
										vPadding = 0_px, maxwidth = 250_px,
										fitParent = true, selectedValue = local_ref.trig_genscr,
										onSelectFunc = [&](int32_t val)
										{
											local_ref.trig_genscr = val;
										}),
									IBTN_T("Run Frozen Generic Script", "The selected generic script will be run in the 'Frozen' mode. (See 'genericdata->RunFrozen()' documentation)"),
									//
									Label(text = "Trigger String:", hAlign = 1.0),
									DropDownList(data = parent.list_strings,
										vPadding = 0_px, forceFitW = true,
										fitParent = true, selectedValue = local_ref.trig_msgstr/10000,
										onSelectFunc = [&](int32_t val)
										{
											local_ref.trig_msgstr = val*10000;
										}),
									IBTN_T("Trigger String", "The string to play when triggered. Negative values are special, reading the string number from somewhere else."),
									Label(text = "Fail String:", hAlign = 1.0),
									DropDownList(data = parent.list_strings,
										vPadding = 0_px, forceFitW = true,
										fitParent = true, selectedValue = local_ref.fail_msgstr/10000,
										onSelectFunc = [&](int32_t val)
										{
											local_ref.fail_msgstr = val*10000;
										}),
									IBTN_T("Fail Trigger String", "The string to play when triggered, but the combo trigger's conditions fail. Negative values are special, reading the string number from somewhere else.")
								),
								Column(framed = true, padding = DEFAULT_PADDING*1.5,
									hAlign = 0.0,
									Rows<2>(
										IBTN("Sets the screen's gravity to the specified value."),
										TRIGFLAG(TRIGFLAG_SET_GRAVITY, "Set Gravity"),
										IBTN("Reverts the screen's gravity to editor settings."),
										TRIGFLAG(TRIGFLAG_REVERT_GRAVITY, "Revert Gravity")
									),
									Rows<2>(
										Label(text = "Gravity:", hAlign = 1.0),
										TextField(maxLength = 11,
											type = GUI::TextField::type::NOSWAP_ZSINT,
											swap_type = nswapDEC, val = local_ref.trig_gravity.getZLong(),
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.trig_gravity = zslongToFix(val);
											}),
										Label(text = "Terminal Velocity:", hAlign = 1.0),
										TextField(maxLength = 11,
											type = GUI::TextField::type::NOSWAP_ZSINT,
											swap_type = nswapDEC, val = local_ref.trig_terminal_v.getZLong(),
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.trig_terminal_v = zslongToFix(val);
											})
									)
								)
							)
						)
					),
					TabRef(name = "Player",
						Column(padding = 0_px,
							Frame(title = "Move Player",
								Column(
									Row(
										Rows<3>(padding = 0_px, vAlign = 0.0,	
											Label(text = "Dest X:", fitParent = true),
											TextField(
												fitParent = true, vPadding = 0_px,
												maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
												places = 4,
												val = local_ref.dest_player_x.getZLong(),
												onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
												{
													local_ref.dest_player_x = zslongToFix(val);
												}),
											IBTN("Move the Player's X by this much"),
											Label(text = "Dest Y:", fitParent = true),
											TextField(
												fitParent = true, vPadding = 0_px,
												maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
												places = 4,
												val = local_ref.dest_player_y.getZLong(),
												onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
												{
													local_ref.dest_player_y = zslongToFix(val);
												}),
											IBTN("Move the Player's Y by this much"),
											Label(text = "Dest Z:", fitParent = true),
											TextField(
												fitParent = true, vPadding = 0_px,
												maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
												places = 4,
												val = local_ref.dest_player_z.getZLong(),
												onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
												{
													local_ref.dest_player_z = zslongToFix(val);
												}),
											IBTN("Move the Player's Z by this much"),
											Label(text = "Player Bounce:", fitParent = true),
											TextField(
												fitParent = true, vPadding = 0_px,
												maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
												places = 4,
												val = local_ref.player_bounce.getZLong(),
												onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
												{
													local_ref.player_bounce = zslongToFix(val);
												}),
											IBTN_T("Player Bounce","If the value is not 0, the player will"
												" 'Jump' with that force. Negative values make the player 'fall'.")
										),
										Rows<4>(padding = 0_px, vAlign = 0.0,
											IBTN("Set the Player's 'X' to the 'Dest X' value exactly."
												" No collision checks are performed; the player simply teleports."),
											TRIGFLAG(TRIGFLAG_SETPLAYER_X_ABS, "Set X (Abs)"),
											IBTN("Set the Player's 'X' to the 'Dest X' value relative to this combo."
												" For FFCs, relative to the center of the hitbox."
												" No collision checks are performed; the player simply teleports."),
											TRIGFLAG(TRIGFLAG_SETPLAYER_X_REL_CMB, "Set X (Rel)"),
											IBTN("Set the Player's 'Y' to the 'Dest Y' value exactly."),
											TRIGFLAG(TRIGFLAG_SETPLAYER_Y_ABS, "Set Y (Abs)"),
											IBTN("Set the Player's 'Y' to the 'Dest Y' value relative to this combo."
												" For FFCs, relative to the center of the hitbox."
												" No collision checks are performed; the player simply teleports."),
											TRIGFLAG(TRIGFLAG_SETPLAYER_Y_REL_CMB, "Set Y (Rel)"),
											IBTN("Set the Player's 'Z' to the 'Dest Z' value exactly."),
											TRIGFLAG(TRIGFLAG_SETPLAYER_Z_ABS, "Set Z (Abs)")
										)
									),
									Rows<3>(hAlign = 0.0,
										Label(text = "Player Dir:", fitParent = true),
										DropDownList(data = parent.list_dirs4n, fitParent = true,
											selectedValue = local_ref.dest_player_dir,
											onSelectFunc = [&](int32_t val)
											{
												local_ref.dest_player_dir = val;
											}),
										IBTN("Forcibly turn the player this direction.")
									)
								)
							),
							Row(
								Frame(title = "Status Effects", vAlign = 0.0,
									Rows<3>(padding = 0_px,
										Label(text = "Sword Jinx:", fitParent = true),
										TextField(
											fitParent = true,
											vPadding = 0_px,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -2, high = MAX_ZSCRIPT_INT, val = local_ref.trig_swjinxtime,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.trig_swjinxtime = val;
											}),
										IBTN("Sets the duration of the 'Sword Jinx' status effect."
											"\n'-2' indicates not to do anything."
											"\n'-1' inflicts the status indefinitely, until cured."
											"\n'0' cures the status."
											"\nAny value above 0 inflicts the status for that many frames."),
										//
										Label(text = "Item Jinx:", fitParent = true),
										TextField(
											fitParent = true,
											vPadding = 0_px,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -2, high = MAX_ZSCRIPT_INT, val = local_ref.trig_itmjinxtime,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.trig_itmjinxtime = val;
											}),
										IBTN("Sets the duration of the 'Item Jinx' status effect."
											"\n'-2' indicates not to do anything."
											"\n'-1' inflicts the status indefinitely, until cured."
											"\n'0' cures the status."
											"\nAny value above 0 inflicts the status for that many frames."),
										//
										Label(text = "Shield Jinx:", fitParent = true),
										TextField(
											fitParent = true,
											vPadding = 0_px,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -2, high = MAX_ZSCRIPT_INT, val = local_ref.trig_shieldjinxtime,
											onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
											{
												local_ref.trig_shieldjinxtime = val;
											}),
										IBTN("Sets the duration of the 'Shield Jinx' status effect."
											"\n'-2' indicates not to do anything."
											"\n'-1' inflicts the status indefinitely, until cured."
											"\n'0' cures the status."
											"\nAny value above 0 inflicts the status for that many frames."),
										//
										Label(text = "Stun:", fitParent = true),
										TextField(
											fitParent = true,
											vPadding = 0_px,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -2, high = MAX_ZSCRIPT_INT, val = local_ref.trig_stuntime,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.trig_stuntime = val;
											}),
										IBTN("Sets the duration of the 'Stun' status effect."
											"\n'-2' indicates not to do anything."
											"\n'-1' inflicts the status indefinitely, until cured."
											"\n'0' cures the status."
											"\nAny value above 0 inflicts the status for that many frames."),
										//
										Label(text = "Bunny:", fitParent = true),
										TextField(
											fitParent = true,
											vPadding = 0_px,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -2, high = MAX_ZSCRIPT_INT, val = local_ref.trig_bunnytime,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.trig_bunnytime = val;
											}),
										IBTN("Sets the duration of the 'Bunny' status effect."
											"\n'-2' indicates not to do anything."
											"\n'-1' inflicts the status indefinitely, until cured."
											"\n'0' cures the status."
											"\nAny value above 0 inflicts the status for that many frames.")
									)
								),
								Frame(title = "Ice Physics",
									Column(
										Row(
											IBTN("Changes which 'Icy Floor' combo has it's physics being forcibly applied to the player."
												" Combo 0 indicates \"don't force any ice physics\"."),
											Checkbox(
												text = "Force Ice Physics",
												checked = force_ice_combo,
												onToggleFunc = [&](bool state)
												{
													force_ice_combo = state;
												})
										),
										SelComboSwatch(
											showvals = true,
											combo = ice_combo, cset = _ice_cs,
											onSelectFunc = [&](int32_t cmb, int32_t c)
											{
												ice_combo = cmb;
												_ice_cs = c;
											}
										),
										Rows<3>(
											TRIGFLAG(TRIGFLAG_FORCE_ICE_VX, "Force Vx:"),
											TextField(
												fitParent = true, vPadding = 0_px,
												maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
												places = 4,
												val = local_ref.force_ice_vx.getZLong(),
												onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
												{
													local_ref.force_ice_vx = zslongToFix(val);
												}),
											IBTN("Set the player's Ice Physics X velocity to this value"),
											TRIGFLAG(TRIGFLAG_FORCE_ICE_VY, "Force Vy:"),
											TextField(
												fitParent = true, vPadding = 0_px,
												maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
												places = 4,
												val = local_ref.force_ice_vy.getZLong(),
												onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
												{
													local_ref.force_ice_vy = zslongToFix(val);
												}),
											IBTN("Set the player's Ice Physics Y velocity to this value")
										)
									)
								)
							)
						)
					)
				)),
				TabRef(name = "Conditions", Frame(
					info = "These are 'Conditions'. They won't trigger the combo on their own, but they must apply for other triggers to work.",
					Column(
						Frame(title = "Player Position", topPadding = DEFAULT_PADDING*2.5,
							Rows<9>(
								Label(text = "Player X:", fitParent = true),
								TextField(
									fitParent = true, vPadding = 0_px,
									maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
									places = 4,
									val = local_ref.req_player_x.getZLong(),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.req_player_x = zslongToFix(val);
									}),
								IBTN("Does nothing on it's own, see flags to the right"),
								TRIGFLAG(TRIGFLAG_REQ_X_GE, "Req X >=", 1, true),
								IBTN("The combo will only trigger if the Hero's X value is >= 'Player X'."),
								TRIGFLAG(TRIGFLAG_REQ_X_LE, "Req X <=", 1, true),
								IBTN("The combo will only trigger if the Hero's X value is <= 'Player X'."),
								TRIGFLAG(TRIGFLAG_REQ_X_REL, "...Relative", 1, true),
								IBTN("'Player X' is treated as relative to where the player's X would be if"
									" the player were standing centered on the combo."),
								//
								Label(text = "Player Y:", fitParent = true),
								TextField(
									fitParent = true, vPadding = 0_px,
									maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
									places = 4,
									val = local_ref.req_player_y.getZLong(),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.req_player_y = zslongToFix(val);
									}),
								IBTN("Does nothing on it's own, see flags to the right"),
								TRIGFLAG(TRIGFLAG_REQ_Y_GE, "Req Y >=", 1, true),
								IBTN("The combo will only trigger if the Hero's Y value is >= 'Player Y'."),
								TRIGFLAG(TRIGFLAG_REQ_Y_LE, "Req Y <=", 1, true),
								IBTN("The combo will only trigger if the Hero's Y value is <= 'Player Y'."),
								TRIGFLAG(TRIGFLAG_REQ_Y_REL, "...Relative", 1, true),
								IBTN("'Player Y' is treated as relative to where the player's Y would be if"
									" the player were standing centered on the combo."),
								Label(text = "Player Z:", fitParent = true),
								TextField(
									fitParent = true, vPadding = 0_px,
									maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
									places = 4,
									val = local_ref.req_player_z.getZLong(),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.req_player_z = zslongToFix(val);
									}),
								IBTN_T("Player Z Requirement","The combo"
									" will only trigger if the Hero's Z value is >= this value."),
								TRIGFLAG(TRIGFLAG_INVERT_PLAYER_Z, "Invert Player Z Req", 1, true),
								IBTN("'Player Z:' requires that the Hero be < the specified Z, instead of >=."),
								_d, _d, _d, _d,
								//
								Label(text = "Player Jump:", fitParent = true),
								TextField(
									fitParent = true, vPadding = 0_px,
									maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
									places = 4,
									val = local_ref.req_player_jump.getZLong(),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.req_player_jump = zslongToFix(val);
									}),
								IBTN("Does nothing on it's own, see flags to the right"),
								TRIGFLAG(TRIGFLAG_REQ_JUMP_GE, "Req Jump >=", 1, true),
								IBTN("The combo will only trigger if the Hero's Jump value is >= 'Player Jump'."),
								TRIGFLAG(TRIGFLAG_REQ_JUMP_LE, "Req Jump <=", 1, true),
								IBTN("The combo will only trigger if the Hero's Jump value is <= 'Player Jump'.")
							)
						),
						Row(
							Column(
								Rows<3>(
									Label(text = "Proximity:", fitParent = true),
									TextField(
										fitParent = true,
										vPadding = 0_px,
										type = GUI::TextField::type::INT_DECIMAL,
										low = 0, high = 5000, val = local_ref.trigprox,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.trigprox = (word)val;
										}),
									IBTN_T("Proximity Requirement","If the value is >0, the combo "
										" will only trigger if the Hero is within that number of pixels of the combo."
										"\nIf 'Invert Proximity Req' is checked, the Hero must be FARTHER than that distance instead."),
									Label(text = "Cooldown:", fitParent = true),
									TextField(
										fitParent = true,
										vPadding = 0_px,
										type = GUI::TextField::type::INT_DECIMAL,
										low = 0, high = 255, val = local_ref.trigcooldown,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.trigcooldown = val;
										}),
									IBTN_T("Trigger Cooldown", "If the value is >0, the combo will"
										" be unable to be triggered for 'n' frames after being triggered.")
								),
								Rows<3>(
									Label(text = "Player Facing:", fitParent = true),
									Button(
										width = 1.5_em, padding = 0_px, forceFitH = true,
										text = "P", hAlign = 1.0, onPressFunc = [&]()
										{
											int32_t flags = local_ref.req_player_dir;
											auto& dir_names = GUI::ZCCheckListData::dir_4();
											if(!call_checklist_dialog("Select 'Player Facing'",dir_names,flags))
												return;
											local_ref.req_player_dir = byte(flags);
										}
									),
									IBTN_T("Player Facing","Player must be facing one of these directions"
										" for the combo to trigger (if any are checked).")
								)
							),
							Rows_Columns<2, 3>(
								IBTN("Can only trigger if the room is darkened."),
								TRIGFLAG(TRIGFLAG_COND_DARK, "Req. Darkness"),
								IBTN("Can only trigger if the room is NOT darkened."),
								TRIGFLAG(TRIGFLAG_COND_NODARK, "Req. No Darkness"),
								IBTN("'Proximity:' requires the Hero to be far away, instead of close"),
								TRIGFLAG(TRIGFLAG_INVERTPROX,"Invert Proximity Req"),
								IBTN("Can only trigger if the player is standing on the ground. Handles 'standing' in sideview as well."),
								TRIGFLAG(TRIGFLAG_PLAYER_STANDING, "Req. Player Standing"),
								IBTN("Can only trigger if the player is NOT standing on the ground. Handles 'standing' in sideview as well."),
								TRIGFLAG(TRIGFLAG_PLAYER_NOTSTANDING, "Req. Player Not Standing")
							)
						)
					)
				)),
				TabRef(name = "Counters/Items", Column(
					Frame(title = "Counters",
						Column(padding = 0_px,
							Rows<3>(
								Label(text = "Counter:", fitParent = true),
								DropDownList(data = parent.list_ss_counters_nn,
									fitParent = true, maxwidth = 350_px,
									selectedValue = local_ref.trigctr,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.trigctr = val;
									}
								),
								IBTN_T("Counter", "Which counter to use for the various counter effects"),
								Label(text = "Amount:", fitParent = true),
								TextField(
									fitParent = true,
									vPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									low = -65535, high = 65535, val = local_ref.trigctramnt,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.trigctramnt = val;
									}),
								IBTN_T("Counter Amount", "The amount of the counter to use for the various counter effects")
							),
							Rows_Columns<2,4>(
								IBTN("Only trigger if the specified counter has at least the specified amount."
									"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work."),
								TRIGFLAG(TRIGFLAG_COUNTERGE,"Require >="),
								IBTN("Only trigger if the specified counter has less than the specified amount."
									"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work."),
								TRIGFLAG(TRIGFLAG_COUNTERLT,"Require <"),
								IBTN("The Counter Amount is a percentage of the max."),
								TRIGFLAG(TRIGFLAG_COUNTER_PERCENT,"...Is Percent"),
								IBTN(fmt::format("The Counter Amount will be discounted based on the Hero's current '{}' item.", ZI.getItemClassName(itype_wealthmedal))),
								TRIGFLAG(TRIGFLAG_COUNTERDISCOUNT,"Apply Discount"),
								IBTN("If the counter has the specified amount, consume it."
									" Negative amount will add to the counter."),
								TRIGFLAG(TRIGFLAG_COUNTEREAT,"Consume Amount"),
								IBTN("The 'Consume Amount' will be drained/granted gradually, instead of at once."),
								TRIGFLAG(TRIGFLAG_COUNTER_GRADUAL,"...Gradual"),
								IBTN("The 'Consume Amount' will occur even if the combo does not meet its' *counter based* trigger conditions."),
								TRIGFLAG(TRIGFLAG_CTRNONLYTRIG,"Consume w/o trig")
							)
						)
					),
					Row(padding = 0_px,
						Frame(title = "Items", vAlign = 0.0,
							Column(padding = 0_px,
								Rows<3>(
									Label(text = "Req Item:", fitParent = true),
									DropDownList(data = parent.list_items_0none,
										vPadding = 0_px, maxwidth = 350_px,
										fitParent = true, selectedValue = local_ref.triggeritem,
										onSelectFunc = [&](int32_t val)
										{
											local_ref.triggeritem = val;
										}),
									IBTN_T("Item Requirement","If the value is >0, the item "
										" id set here must be owned to trigger the combo."
										"\nIf 'Invert Item Req' is checked, the item must NOT be owned instead."
										"\nIf 'Consume Item Req' is checked, the item will be removed upon triggering."
										"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work.")
								),
								Row(
									TRIGFLAG(TRIGFLAG_INVERTITEM,"Invert Item Req"),
									IBTN("'Req Item:' must NOT be owned to trigger"),
									TRIGFLAG(TRIGFLAG_CONSUMEITEM,"Consume Item Req"),
									IBTN("'Req Item:' will be taken when triggering")
								)
							)
						)
					)
				)),
				TabRef(name = "States/Spawning", Row(
					Rows<4>(framed = true, vAlign = 0.0,
						Label(text = "Spawn Item:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -255, high = 255, val = local_ref.spawnitem,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.spawnitem = val;
							}),
						DummyWidget(),
						IBTN_T("Spawn Item", "If the value is >0, the item "
							" id set here will be spawned when the combo is triggered."
							"\nIf the value is <0, it will be treated as a dropset to drop."
							"\nIf 'Spawns Special Item' is checked, the item will count as the room's special item,"
							"\nnot spawning if the special item state is already set."
							"\nIf 'Trigger ExState after item pickup' is checked, the combo will not set its'"
							"\nExState on being triggered, instead setting it when the item is picked up."),
						//
						Label(text = "Spawned Item Pickup:", fitParent = true),
						spawned_ip_field = TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							maxLength = 12, val = local_ref.spawnip,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.spawnip = val;
							}),
						Button(
							width = 1.5_em, padding = 0_px, forceFitH = true,
							text = "P", hAlign = 1.0, onPressFunc = [&]()
							{
								int32_t flags = local_ref.spawnip;
								static const vector<CheckListInfo> pickups =
								{
									{ CheckListInfo::DISABLED, "Large Collision Rectangle (INTERNAL)" },
									{ "Hold Up Item" },
									{ CheckListInfo::DISABLED, "Sets Screen State ST_ITEM" },
									{ CheckListInfo::DISABLED, "Dummy Item" },
									{ CheckListInfo::DISABLED, "Shop Item (INTERNAL)" },
									{ CheckListInfo::DISABLED, "Pay for Info (INTERNAL)" },
									{ CheckListInfo::DISABLED, "Item Fades" },
									{ CheckListInfo::DISABLED, "Enemy Carries Item" },
									{ "Item Disappears" },
									{ CheckListInfo::DISABLED, "Big McGuffin (INTERNAL)" },
									{ CheckListInfo::DISABLED, "Invisible" },
									{ CheckListInfo::DISABLED, "Triggers Screen State ST_SP_ITEM" },
									{ "Triggers Screen Secrets" },
									{ "Always Grabbable" },
									{ CheckListInfo::DISABLED },
									{ CheckListInfo::DISABLED },
								};
								if(!call_checklist_dialog("Select 'Spawned Item Pickup'",pickups,flags))
									return;
								local_ref.spawnip = flags;
								spawned_ip_field->setVal(local_ref.spawnip);
							}
						),
						IBTN_T("Spawn Item Pickup", "Represents the pickup flags of the spawned item."
							" Only some flags are valid. Click the 'P' button for a selector list."),
						//
						Label(text = "Spawn Enemy:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 511, val = local_ref.spawnenemy,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.spawnenemy = val;
							}),
						DummyWidget(),
						IBTN_T("Spawn Enemy","If the value is >0, the enemy "
							" id set here will be spawned when the combo is triggered."
							"\nIf 'Trigger ExState after enemy kill' is checked, the combo will not set its'"
							"\nExState on being triggered, instead setting it when the enemy is defeated."),
						//
						Label(text = "ExState:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -1, high = 31, val = local_ref.exstate,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.exstate = val;
							}),
						DummyWidget(),
						IBTN_T("ExState", "If the value is >=0, the exstate"
							" id set here will be set when the combo is triggered,"
							"\nand if the exstate set here is already set, the combo will automatically trigger"
							"\nwithout any effects other than combo/cset change."),
						//
						Label(text = "Copycat:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 255, val = local_ref.trigcopycat,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trigcopycat = val;
							}),
						DummyWidget(),
						IBTN_T("Copycat", "If the value is > 0, the combo is linked to that copycat ID."
							"\nIf this trigger is triggered, all other linked combos will also trigger,"
							"\nand if any other linked trigger triggers, this trigger will trigger."),
						//
						Label(text = "LevelState:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 31, val = local_ref.trig_lstate,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trig_lstate = val;
							}),
						DummyWidget(),
						IBTN_T("LevelState", "The LevelState used by the flags"
							" '->LevelState' and 'LevelState->'. 0-31."),
						//
						Label(text = "GlobalState:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 255, val = local_ref.trig_gstate,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trig_gstate = val;
							}),
						DummyWidget(),
						IBTN_T("GlobalState", "The GlobalState used by the flags"
							" '->GlobalState' and 'GlobalState->'. 0-255."),
						//
						Label(text = "GlobalState Timer:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 214748, val = local_ref.trig_statetime,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trig_statetime = val;
							}),
						DummyWidget(),
						IBTN_T("GlobalState Timer", "If this value is >0,"
							" then the 'GlobalState->' flag will trigger a timed global"
							" state with this duration, in frames, instead of toggling"
							" the global state."),
						//
						Label(text = "Trigger Group:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 255, val = local_ref.trig_group,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trig_group = val;
							}),
						DummyWidget(),
						IBTN_T("Trigger Group", "The Trigger Group used by the flags"
							" 'TrigGroup Less->', 'TrigGroup Greater->', and '->TrigGroup'. 0-255."),
						//
						Label(text = "Trigger Group Val:", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 65535, val = local_ref.trig_group_val,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trig_group_val = val;
							}),
						DummyWidget(),
						IBTN_T("Trigger Group Val", "The value used by the flags"
							" 'TrigGroup Less->' and 'TrigGroup Greater->'. 0-65535."),
						//
						Label(text = "ExDoor Dir:", fitParent = true),
						DropDownList(data = parent.list_dirs4n, fitParent = true,
							selectedValue = local_ref.exdoor_dir,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.exdoor_dir = val;
							}),
						DummyWidget(),
						IBTN("If not '(None)', triggering this trigger sets the extra doorstate"
							" in the specified direction (of the index set for 'ExDoor Index')."
							" Additionally, if that door state is already set, the combo will"
							" automatically trigger without any effects other than combo/cset change."),
						//
						Label(text = "ExDoor Index:", fitParent = true),
						DropDownList(data = parent.list_0_7, fitParent = true,
							selectedValue = local_ref.exdoor_ind,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.exdoor_ind = val;
							}),
						DummyWidget(),
						IBTN("Which door index of the specified direction to use. (See ? for 'ExDoor Dir' for more info)")
					),
					Column(vAlign = 0.0, padding = 0_px,
						Rows<4>(framed = true, fitParent = true,
							IBTN("'Spawn Item' will be linked to the room's Special Item state"),
							TRIGFLAG(TRIGFLAG_SPCITEM, "Spawns Special Item",3),
							IBTN("The item spawned by the combo will automatically be collected by the Hero."),
							TRIGFLAG(TRIGFLAG_AUTOGRABITEM, "Spawned Item auto-collects",3),
							IBTN("The combo's 'ExState' will be set when the spawned item is picked up, rather than when it is triggered."),
							TRIGFLAG(TRIGFLAG_EXSTITEM, "Trigger ExState after item pickup",3),
							IBTN("The combo's 'ExState' will be set when the spawned enemy is defeated, rather than when it is triggered."),
							TRIGFLAG(TRIGFLAG_EXSTENEMY, "Trigger ExState after enemy kill",3),
							IBTN("The combo's 'ExState' will be unset instead of set when triggered."
								" (Does not necessarily revert combos that were changed by the ExState on it's own)"),
							TRIGFLAG(TRIGFLAG_UNSETEXSTATE, "Clear ExState on trigger",3),
							IBTN("The combo's 'ExDoor' will be unset instead of set when triggered."
								" (Does not necessarily revert combos that were changed by the ExDoor on it's own)"),
							TRIGFLAG(TRIGFLAG_UNSETEXDOOR, "Clear ExDoor on trigger",3),
							IBTN("This combo is triggered when the level-based switch state specified as 'LevelState' is toggled."),
							TRIGFLAG(TRIGFLAG_TRIGLEVELSTATE, "LevelState->"),
							IBTN("When triggered, toggles the level-based switch state specified as 'LevelState'."),
							TRIGFLAG(TRIGFLAG_LEVELSTATE, "->LevelState"),
							IBTN("This combo is triggered when the globalswitch state specified as 'GlobalState' is toggled."),
							TRIGFLAG(TRIGFLAG_TRIGGLOBALSTATE, "GlobalState->"),
							IBTN("When triggered, toggles the global switch state specified as 'GlobalState'."
								"\nIf 'GlobalState Timer' is >0, resets the timer of the state to the specified value instead of toggling it."),
							TRIGFLAG(TRIGFLAG_GLOBALSTATE, "->GlobalState"),
							IBTN("This combo contributes to its Trigger Group."),
							TRIGFLAG(TRIGFLAG_TGROUP_CONTRIB, "Contributes To TrigGroup",3),
							IBTN("When the number of combos that contribute to this trigger's Trigger Group is LESS than the Trigger Group Val, trigger this trigger."),
							TRIGFLAG(TRIGFLAG_TGROUP_LESS, "TrigGroup Less->"),
							IBTN("When the number of combos that contribute to this trigger's Trigger Group is GREATER than the Trigger Group Val, trigger this trigger."),
							TRIGFLAG(TRIGFLAG_TGROUP_GREATER, "TrigGroup Greater->"),
							IBTN("The 'Copycat' will not cause this combo to trigger; it will only be used to trigger other combos when this combo triggers."),
							TRIGFLAG(TRIGFLAG_NO_COPYCAT_CAUSE, "Copycat doesn't trigger this")
						),
						Frame(title = "GlobalState Conditions",
							info = "These are 'Conditions'. They won't trigger the combo on their own, but they must apply for other triggers to work.",
							Rows<3>(
								Label(text = "Req States:", fitParent = true),
								Button(
									width = 1.5_em, padding = 0_px, forceFitH = true,
									text = "P", hAlign = 1.0, onPressFunc = [&]()
									{
										auto flags = local_ref.req_global_state;
										auto& gstates = GUI::ZCCheckListData::global_states();
										if(!call_checklist_dialog("Select 'Req States'",gstates,flags,16))
											return;
										local_ref.req_global_state = flags;
									}
								),
								IBTN("These GlobalStates must be set for this trigger to activate."
									"\nUse the 'P' button to pick the flags for this value."),
								//
								Label(text = "Unreq States:", fitParent = true),
								Button(
									width = 1.5_em, padding = 0_px, forceFitH = true,
									text = "P", hAlign = 1.0, onPressFunc = [&]()
									{
										auto flags = local_ref.unreq_global_state;
										auto& gstates = GUI::ZCCheckListData::global_states();
										if(!call_checklist_dialog("Select 'Unreq States'",gstates,flags,16))
											return;
										local_ref.unreq_global_state = flags;
									}
								),
								IBTN("These GlobalStates must NOT be set for this trigger to activate."
									"\nUse the 'P' button to pick the flags for this value.")
							)
						)
					)
				)),
				TabRef(name = "Graphics", Rows<2>(
					Frame(title = "Tint", fitParent = true, Column(
						Rows<2>(
							Label(text = "Tint Palette R/G/B:", fitParent = true),
							_d,
							Row(
								TextField(
									fitParent = true, padding = 0_px,
									type = GUI::TextField::type::NOSWAP_ZSINT,
									swap_type = nswapLDEC,
									low = -255, high = 255, val = local_ref.trigtint[0],
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.trigtint[0] = val;
									}),
								TextField(
									fitParent = true, padding = 0_px,
									type = GUI::TextField::type::NOSWAP_ZSINT,
									swap_type = nswapLDEC,
									low = -255, high = 255, val = local_ref.trigtint[1],
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.trigtint[1] = val;
									}),
								TextField(
									fitParent = true, padding = 0_px,
									type = GUI::TextField::type::NOSWAP_ZSINT,
									swap_type = nswapLDEC,
									low = -255, high = 255, val = local_ref.trigtint[2],
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.trigtint[2] = val;
									})
							),
							IBTN("Applies a tint to the palette, same as the script 'Graphics->Tint()'."
								"Can be cleared using the '->ClearTint' flag."),
							TRIGFLAG(TRIGFLAG_TINT_CLEAR,"->ClearTint",1,true),
							IBTN("Clears all 'tint' from the palette, same as the script 'Graphics->ClearTint()'."
								" Runs before the above 'Tint Palette' effect, if both are set.")
						)
					)),
					Frame(title = "Palette", fitParent = true, Rows<3>(
						Label(text = "Load Level Palette", fitParent = true),
						TextField(
							fitParent = true, padding = 0_px,
							type = GUI::TextField::type::SWAP_SSHORT,
							swap_type = nswapHEX,
							low = -2, high = 512, val = local_ref.triglvlpalette,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.triglvlpalette = val;
							}),
						IBTN("Loads the specified level palette over the current level palette."
							" '-1' for 'none', '-2' for 'current screen editor palette'."),
						//
						Label(text = "Load Boss Palette", fitParent = true),
						TextField(
							fitParent = true, padding = 0_px,
							type = GUI::TextField::type::SWAP_SSHORT,
							swap_type = nswapDEC,
							low = -1, high = 29, val = local_ref.trigbosspalette,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trigbosspalette = val;
							}),
						IBTN("Loads the specified level palette over the current boss palette."
							" '-1' for 'none'.")
					)),
					Frame(title = "VFX", fitParent = true, Rows<3>(
						Label(text = "Quake", fitParent = true),
						TextField(
							fitParent = true, padding = 0_px,
							type = GUI::TextField::type::NOSWAP_ZSINT,
							swap_type = nswapLDEC,
							low = -1, high = 999999, val = local_ref.trigquaketime,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trigquaketime = val;
							}),
						IBTN("Sets the quake timer to the specified duration."
							" '-1' for 'none'."),
						//
						Label(text = "Wavy", fitParent = true),
						TextField(
							fitParent = true, padding = 0_px,
							type = GUI::TextField::type::NOSWAP_ZSINT,
							swap_type = nswapLDEC,
							low = -1, high = 999999, val = local_ref.trigwavytime,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trigwavytime = val;
							}),
						IBTN("Sets the wavy timer to the specified duration."
							" '-1' for 'none'.")
					))
				)),
				TabRef(name = "Level Based", Column(
					Rows<3>(
						Label(text = "Trig DMap Level", fitParent = true),
						TextField(
							fitParent = true,
							vPadding = 0_px,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -1, high = MAXLEVELS, val = local_ref.trigdmlevel,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.trigdmlevel = val;
							}),
						IBTN("The dmap level referenced by things on this tab."
							" If '-1', uses the current dmap's level.")
					),
					Frame(title = "Level Flags", vAlign = 0.0,
						Column(padding = 0_px,
							Rows<4>(
								Label(text = "Req Flags:", fitParent = true),
								req_litems_field = TextField(
									fitParent = true,
									vPadding = 0_px,
									type = GUI::TextField::type::INT_DECIMAL,
									low = 0, high = LI_ALL, val = local_ref.trig_levelitems,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.trig_levelitems = val;
									}),
								Button(
									width = 1.5_em, padding = 0_px, forceFitH = true,
									text = "P", hAlign = 1.0, onPressFunc = [&]()
									{
										int32_t flags = local_ref.trig_levelitems;
										auto const litem_names = GUI::ZCCheckListData::level_items();
										if(!call_checklist_dialog("Select 'Req Flags'",litem_names,flags))
											return;
										local_ref.trig_levelitems = flags;
										req_litems_field->setVal(local_ref.trig_levelitems);
									}
								),
								IBTN("See 'Require All', 'Require Not All', '->Set', and '->Unset' below."
									"\nUse the 'P' button to pick the flags for this value.")
							),
							Rows_Columns<2,2>(
								IBTN("The level flags set for 'Req Flags:' must ALL be on for this to trigger."
									"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work."),
								TRIGFLAG(TRIGFLAG_LITEM_COND,"Require All"),
								IBTN("The level flags set for 'Req Flags:' must NOT ALL (some is ok) be on for this to trigger."
									"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work."),
								TRIGFLAG(TRIGFLAG_LITEM_REVCOND,"Require Not All"),
								IBTN("The level flags set for 'Req Flags:' will be enabled when this trigger triggers."
									" If '->Unset' is also checked, the flags will be toggled instead."),
								TRIGFLAG(TRIGFLAG_LITEM_SET,"->Set"),
								IBTN("The level flags set for 'Req Flags:' will be disabled when this trigger triggers."
									" If '->Set' is also checked, the flags will be toggled instead."),
								TRIGFLAG(TRIGFLAG_LITEM_UNSET,"->Unset")
							)
						)
					),
					Frame(title = "LevelState Conditions",
						info = "These are 'Conditions'. They won't trigger the combo on their own, but they must apply for other triggers to work.",
						Rows<3>(
							Label(text = "Req States:", fitParent = true),
							Button(
								width = 1.5_em, padding = 0_px, forceFitH = true,
								text = "P", hAlign = 1.0, onPressFunc = [&]()
								{
									dword flags = local_ref.req_level_state;
									auto& lstates = GUI::ZCCheckListData::level_states();
									if(!call_checklist_dialog("Select 'Req States'",lstates,flags,8))
										return;
									local_ref.req_level_state = flags;
								}
							),
							IBTN("These LevelStates must be set (on the specified"
								" Trig DMap Level) for this trigger to activate."
								"\nUse the 'P' button to pick the flags for this value."),
							//
							Label(text = "Unreq States:", fitParent = true),
							Button(
								width = 1.5_em, padding = 0_px, forceFitH = true,
								text = "P", hAlign = 1.0, onPressFunc = [&]()
								{
									dword flags = local_ref.unreq_level_state;
									auto& lstates = GUI::ZCCheckListData::level_states();
									if(!call_checklist_dialog("Select 'Unreq States'",lstates,flags,8))
										return;
									local_ref.unreq_level_state = flags;
								}
							),
							IBTN("These LevelStates must NOT be set (on the specified"
								" Trig DMap Level) for this trigger to activate."
								"\nUse the 'P' button to pick the flags for this value.")
						)
					)
				))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(text = "Summarize",
					fitParent = true,
					onPressFunc = [&]()
					{
						string title = "Trigger Summary";
						if (!local_ref.label.empty())
							title += fmt::format(" '{}'", local_ref.label);
						InfoDialog(title, local_ref.summarize(parent_comboref), nullopt, nullptr, 0).show();
					}),
				warnbtn = Button(
					text = "Warnings",
					minwidth = 90_px,
					onClick = message::WARNINGS),
				Button(
					text = "Clear",
					minwidth = 90_px,
					onClick = message::CLEAR),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	cteff_tflag->setDisabled(!hasCTypeEffects(parent_comboref.type));
	ctcause_tflag->setDisabled(!hasCTypeCauses(parent_comboref.type));
	updateWarnings();
	
	l_minmax_trig->setText(local_ref.trigger_flags.get(TRIGFLAG_INVERTMINMAX)
		? maxstr : minstr);
	return window;
}

bool ComboTriggerDialog::apply_trigger()
{
	updateWarnings();
	if(warnings.size())
	{
		bool cancel = false;
		AlertDialog alert("Warnings",warnings,[&](bool ret,bool)
			{
				if(!ret) cancel = true;
			});
		alert.setSubtext("The following issues were found with this trigger:");
		alert.show();
		if(cancel)
			return false;
	}
	
	local_ref.force_ice_combo = force_ice_combo ? ice_combo : -1;
	if(!hasCTypeEffects(parent_comboref.type))
		local_ref.trigger_flags.set(TRIGFLAG_CMBTYPEFX, false);
	edited = true;
	parent_comboref.triggers[index] = local_ref;
	return true;
}

bool ComboTriggerDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::CLEAR:
		{
			bool doclear = false;
			AlertDialog("Are you sure?",
				"Clearing the trigger will reset all values",
				[&](bool ret,bool)
				{
					doclear = ret;
				}).show();
			if(doclear)
			{
				local_ref.clear();
				rerun_dlg = true;
				load_trigger();
				return true;
			}
			return false;
		}
		
		case message::WARNINGS:
		{
			updateWarnings();
			if(warnings.size())
				displayinfo("Warnings",warnings,"The following issues were found with this trigger:");
			return false;
		}
		case message::OK:
			return apply_trigger();
		case message::CANCEL:
			return true;
		case message::PLUSTRIGGER:
		{
			if(index == parent_comboref.triggers.size() - 1)
				return false;

			apply_trigger();
			index += 1;
			local_ref = parent_comboref.triggers[index];
			rerun_dlg = true;
			load_trigger();
			return true;
		}
		case message::MINUSTRIGGER:
		{
			if(index==0)
				return false;

			apply_trigger();
			index -= 1;
			local_ref = parent_comboref.triggers[index];
			rerun_dlg = true;
			load_trigger();
			return true;
		}
	}
	return false;
}

