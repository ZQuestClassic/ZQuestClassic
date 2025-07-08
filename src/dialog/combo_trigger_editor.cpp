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
	parent_comboref(parentdlg.local_comboref), index(index)
{}

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
		checked = (local_ref.triggerflags[index/32] & (1<<(index%32))),
		fitParent = true, colSpan = cspan,
		onToggleFunc = [&, index](bool state)
		{
			SETFLAG(local_ref.triggerflags[index/32],(1<<(index%32)),state);
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
	if(trig.triggerflags[0] & (
		combotriggerSWORD|combotriggerSWORDBEAM|combotriggerBRANG|combotriggerBOMB|combotriggerSBOMB|
		combotriggerLITBOMB|combotriggerLITSBOMB|combotriggerARROW|combotriggerFIRE|combotriggerWHISTLE|
		combotriggerBAIT|combotriggerWAND|combotriggerMAGIC|combotriggerWIND|combotriggerREFMAGIC|
		combotriggerREFFIREBALL|combotriggerREFROCK|combotriggerHAMMER|combotriggerEWFIREBALL)) return true;
	if(trig.triggerflags[1] & (
		combotriggerHOOKSHOT|combotriggerSPARKLE|combotriggerBYRNA|combotriggerREFBEAM|combotriggerSTOMP|
		combotriggerSCRIPT01|combotriggerSCRIPT02|combotriggerSCRIPT03|combotriggerSCRIPT04|combotriggerSCRIPT05|
		combotriggerSCRIPT06|combotriggerSCRIPT07|combotriggerSCRIPT08|combotriggerSCRIPT09|combotriggerSCRIPT10|
		combotriggerEWARROW|combotriggerEWBRANG|combotriggerEWSWORD|combotriggerEWROCK))
		return true;
	if(trig.triggerflags[2] & (
		combotriggerEWSCRIPT01|combotriggerEWSCRIPT02|combotriggerEWSCRIPT03|combotriggerEWSCRIPT04|combotriggerEWSCRIPT05|
		combotriggerEWSCRIPT06|combotriggerEWSCRIPT07|combotriggerEWSCRIPT08|combotriggerEWSCRIPT09|combotriggerEWSCRIPT10|
		combotriggerEWMAGIC|combotriggerEWBBLAST|combotriggerEWSBBLAST|combotriggerEWLITBOMB|combotriggerEWLITSBOMB|
		combotriggerEWFIRETRAIL|combotriggerEWFLAME|combotriggerEWWIND|combotriggerEWFLAME2|
		combotriggerTHROWN|combotriggerQUAKESTUN|combotriggerSQUAKESTUN|combotriggerANYFIRE|
		combotriggerSTRONGFIRE|combotriggerMAGICFIRE|combotriggerDIVINEFIRE)) return true;
	if(trig.triggerflags[3] & (combotriggerLWREFARROW|combotriggerLWREFFIRE|combotriggerLWREFFIRE2)) return true;
	return false;
}
static bool has_trigger_cause(combo_trigger const& trig)
{
	if(trig.triggerbtn && (trig.triggerflags[0] & (combotriggerBTN_TOP|combotriggerBTN_BOTTOM|
		combotriggerBTN_LEFT|combotriggerBTN_RIGHT))) return true;
	if(trig.trigtimer) return true;
	if(trig.triggerflags[0] & (
		combotriggerSWORD|combotriggerSWORDBEAM|combotriggerBRANG|combotriggerBOMB|combotriggerSBOMB|
		combotriggerLITBOMB|combotriggerLITSBOMB|combotriggerARROW|combotriggerFIRE|combotriggerWHISTLE|
		combotriggerBAIT|combotriggerWAND|combotriggerMAGIC|combotriggerWIND|combotriggerREFMAGIC|
		combotriggerREFFIREBALL|combotriggerREFROCK|combotriggerHAMMER|combotriggerSTEP|combotriggerSTEPSENS|
		combotriggerSHUTTER|combotriggerEWFIREBALL)) return true;
	if(trig.triggerflags[1] & (
		combotriggerHOOKSHOT|combotriggerSPARKLE|combotriggerBYRNA|combotriggerREFBEAM|combotriggerSTOMP|
		combotriggerSCRIPT01|combotriggerSCRIPT02|combotriggerSCRIPT03|combotriggerSCRIPT04|combotriggerSCRIPT05|
		combotriggerSCRIPT06|combotriggerSCRIPT07|combotriggerSCRIPT08|combotriggerSCRIPT09|combotriggerSCRIPT10|
		combotriggerAUTOMATIC|combotriggerLIGHTON|combotriggerLIGHTOFF|combotriggerPUSH|combotriggerLENSON|
		combotriggerLENSOFF|combotriggerEWARROW|combotriggerEWBRANG|combotriggerEWSWORD|combotriggerEWROCK))
		return true;
	if(trig.triggerflags[2] & (
		combotriggerEWSCRIPT01|combotriggerEWSCRIPT02|combotriggerEWSCRIPT03|combotriggerEWSCRIPT04|combotriggerEWSCRIPT05|
		combotriggerEWSCRIPT06|combotriggerEWSCRIPT07|combotriggerEWSCRIPT08|combotriggerEWSCRIPT09|combotriggerEWSCRIPT10|
		combotriggerEWMAGIC|combotriggerEWBBLAST|combotriggerEWSBBLAST|combotriggerEWLITBOMB|combotriggerEWLITSBOMB|
		combotriggerEWFIRETRAIL|combotriggerEWFLAME|combotriggerEWWIND|combotriggerEWFLAME2|combotriggerENEMIESKILLED|
		combotriggerSECRETSTR|combotriggerTHROWN|combotriggerQUAKESTUN|combotriggerSQUAKESTUN|combotriggerANYFIRE|
		combotriggerSTRONGFIRE|combotriggerMAGICFIRE|combotriggerDIVINEFIRE)) return true;
	if(trig.triggerflags[3] & (
		combotriggerTRIGLEVELSTATE|combotriggerTRIGGLOBALSTATE|combotriggerTGROUP_LESS|combotriggerTGROUP_GREATER|
		combotriggerPUSHEDTRIG|combotriggerDIVETRIG|combotriggerDIVESENSTRIG|combotriggerLWREFARROW|combotriggerLWREFFIRE|
		combotriggerLWREFFIRE2)) return true;
	if(trig.triggerflags[4] & (combotriggerSCREENLOAD|combotriggerPLAYERLANDHERE|
		combotriggerPLAYERLANDANYWHERE|combotriggerCMBTYPECAUSES)) return true;
	if(trig.exstate != -1 && !(trig.triggerflags[4] & combotriggerUNSETEXSTATE)) return true;
	if(trig.exdoor_dir != -1 && !(trig.triggerflags[4] & combotriggerUNSETEXDOOR)) return true;
	if(trig.trigcopycat) return true;
	return false;
}
static bool has_trigger_effect(combo_trigger const& trig)
{
	if(trig.triggerflags[0] & (combotriggerRESETANIM|combotriggerCMBTYPEFX|combotriggerKILLWPN)) return true;
	if(trig.triggerflags[1] & (combotriggerCONSUMEITEM|combotriggerCOUNTEREAT|combotriggerSECRETS)) return true;
	if(trig.triggerflags[3] & (combotriggerLEVELSTATE|combotriggerGLOBALSTATE|combotriggerKILLENEMIES|combotriggerCLEARENEMIES|
		combotriggerCLEARLWEAPONS|combotriggerCLEAREWEAPONS|combotriggerIGNITE_ANYFIRE|combotriggerIGNITE_STRONGFIRE|
		combotriggerIGNITE_MAGICFIRE|combotriggerIGNITE_DIVINEFIRE|combotriggerTOGGLEDARK|combotriggerLITEM_SET|
		combotriggerLITEM_UNSET|combotriggerTINT_CLEAR)) return true;
	if(trig.triggerflags[4] & (combotriggerSETPLAYER_X_ABS|combotriggerSETPLAYER_X_REL_CMB|combotriggerSETPLAYER_Y_ABS|
		combotriggerSETPLAYER_Y_REL_CMB|combotriggerSETPLAYER_Z_ABS)) return true;
	if(trig.dest_player_x || trig.dest_player_y || trig.dest_player_z) return true;
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
	bool tgroup = local_ref.triggerflags[3] & combotriggerTGROUP_CONTRIB;
	if(!tgroup || (cause != effect))
	{
		if(!cause)
			warnings.emplace_back("Trigger has no 'Cause', and will never be triggered!");
		if(!effect)
			warnings.emplace_back("Trigger has no 'Effect', and will do nothing!");
	}
	if((local_ref.triggerflags[0] & (combotriggerKILLWPN)) || (local_ref.triggerflags[3] & (combotriggerIGNITE_ANYFIRE|
		combotriggerIGNITE_STRONGFIRE|combotriggerIGNITE_MAGICFIRE|combotriggerIGNITE_DIVINEFIRE|combotriggerSEPARATEWEAPON)))
	{
		if(!has_weapon_cause(local_ref))
			warnings.emplace_back("Trigger has weapon-specific 'Effect'(s), but no weapon-based causes!");
	}
	if((local_ref.triggerflags[4] & combotriggerUNSETEXSTATE) && local_ref.exstate < 0)
		warnings.emplace_back("Can't unset ExState -1!");
	if((local_ref.triggerflags[4] & combotriggerUNSETEXDOOR) && local_ref.exdoor_dir < 0)
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
										checked = (local_ref.triggerflags[0] & (combotriggerINVERTMINMAX)),
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.triggerflags[0],(combotriggerINVERTMINMAX),state);
											l_minmax_trig->setText(state ? maxstr : minstr);
										})
								),
								Rows<4>(
									TRIGFLAG(0,"Sword"),
									TRIGFLAG(1,"Sword Beam"),
									TRIGFLAG(2,"Boomerang"),
									TRIGFLAG(3,"Bomb Boom"),
									TRIGFLAG(4,"Super Bomb Boom"),
									TRIGFLAG(5,"Placed Bomb"),
									TRIGFLAG(6,"Placed Super Bomb"),
									TRIGFLAG(7,"Arrow"),
									TRIGFLAG(8,"Fire"),
									TRIGFLAG(9,"Whistle"),
									TRIGFLAG(10,"Bait"),
									TRIGFLAG(11,"Wand"),
									TRIGFLAG(12,"Magic"),
									TRIGFLAG(13,"Wind"),
									TRIGFLAG(14,"Refl. Magic"),
									TRIGFLAG(15,"Refl. Fireball"),
									TRIGFLAG(16,"Refl. Rock"),
									TRIGFLAG(115,"Refl. Arrow"),
									TRIGFLAG(116,"Refl. Fire"),
									TRIGFLAG(117,"Refl. Fire 2"),
									TRIGFLAG(17,"Hammer"),
									TRIGFLAG(32,"Hookshot"),
									TRIGFLAG(33,"Sparkle"),
									TRIGFLAG(34,"Byrna"),
									TRIGFLAG(35,"Refl. Beam"),
									TRIGFLAG(36,"Stomp"),
									TRIGFLAG(89, "Thrown"),
									TRIGFLAG(90, "Quake Hammer"),
									TRIGFLAG(91, "S. Quake Hammer"),
									TRIGFLAG(37,"Custom Weapon 1"),
									TRIGFLAG(38,"Custom Weapon 2"),
									TRIGFLAG(39,"Custom Weapon 3"),
									TRIGFLAG(40,"Custom Weapon 4"),
									TRIGFLAG(41,"Custom Weapon 5"),
									TRIGFLAG(42,"Custom Weapon 6"),
									TRIGFLAG(43,"Custom Weapon 7"),
									TRIGFLAG(44,"Custom Weapon 8"),
									TRIGFLAG(45,"Custom Weapon 9"),
									TRIGFLAG(46,"Custom Weapon 10")
								)
							)
						)
					)),
					TabRef(name = "EWeapons", Row(
						Frame(title = "EW Types", info = "Triggered by any eweapon matching these types",
							Column(
								Rows<4>(
									TRIGFLAG(31,"Fireball"),
									TRIGFLAG(60,"Arrow"),
									TRIGFLAG(61,"Boomerang"),
									TRIGFLAG(62,"Sword"),
									TRIGFLAG(63,"Rock"),
									TRIGFLAG(74,"Magic"),
									TRIGFLAG(75,"Bomb Blast"),
									TRIGFLAG(76,"SBomb Blast"),
									TRIGFLAG(77,"Lit Bomb"),
									TRIGFLAG(78,"Lit SBomb"),
									TRIGFLAG(79,"Fire Trail"),
									TRIGFLAG(80,"Flame"),
									TRIGFLAG(81,"Wind"),
									TRIGFLAG(82,"Flame 2"),
									TRIGFLAG(64,"Custom Weapon 1"),
									TRIGFLAG(65,"Custom Weapon 2"),
									TRIGFLAG(66,"Custom Weapon 3"),
									TRIGFLAG(67,"Custom Weapon 4"),
									TRIGFLAG(68,"Custom Weapon 5"),
									TRIGFLAG(69,"Custom Weapon 6"),
									TRIGFLAG(70,"Custom Weapon 7"),
									TRIGFLAG(71,"Custom Weapon 8"),
									TRIGFLAG(72,"Custom Weapon 9"),
									TRIGFLAG(73,"Custom Weapon 10")
								)
							)
						)
					)),
					TabRef(name = "Other", Row(
						Frame(title = "Burning", info = "Triggered by weapons (lweapon or eweapon) burning with particular levels of fire"
							+ QRHINT({qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS}),
							Rows<2>(
								IBTN("Triggered by weapons burning with 'Normal' type fire.\nNote: Many engine fire sources will *always* count as 'Normal Fire', regardless of other settings."),
								TRIGFLAG(92, "Normal Fire"),
								IBTN("Triggered by weapons burning with 'Strong' type fire"),
								TRIGFLAG(93, "Strong Fire"),
								IBTN("Triggered by weapons burning with 'Magic' type fire"),
								TRIGFLAG(94, "Magic Fire"),
								IBTN("Triggered by weapons burning with 'Divine' type fire"),
								TRIGFLAG(95, "Divine Fire")
							)
						),
						Frame(
							Rows<2>(
								IBTN("Only weapons with Z=0 will count for this trigger. (No effect in sideview)"),
								TRIGFLAG(126,"Only Grounded Weapons")
							)
						)
					)),
					TabRef(name = "Weapon Effects", Column(
						Frame(fitParent = true, title = "Weapon Effects",
							info = "Trigger effects related to the weapon that triggered this trigger. These"
								" effects will not occur if the combo is triggered by a non-weapon trigger.",
							Rows<2>(hAlign = 0.0,
								IBTN("Destroy the triggering weapon"),
								TRIGFLAG(30, "Kill Triggering Weapon"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Normal Fire' triggers."),
								TRIGFLAG(104, "Ignite Weapon (Normal)"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Strong Fire' triggers."),
								TRIGFLAG(105, "Ignite Weapon (Strong)"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Magic Fire' triggers."),
								TRIGFLAG(106, "Ignite Weapon (Magic)"),
								IBTN("Light the triggering weapon on fire, making it trigger 'Divine Fire' triggers."),
								TRIGFLAG(107, "Ignite Weapon (Divine)")
							)
						),
						Frame(vPadding = 0_px, fitParent = true,
							Rows<2>(hAlign = 0.0,
								IBTN("If triggered by a weapon, only the effects from the 'Weapon Effects'"
									" section will occur- all other flags will only apply to non-weapon triggers."),
								TRIGFLAG(108,"Weapon Separate Triggers")
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
										static const vector<CheckListInfo> button_names =
										{
											{ "A" },
											{ "B" },
											{ "L" },
											{ "R" },
											{ "Ex1" },
											{ "Ex2" },
											{ "Ex3" },
											{ "Ex4" },
										};
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
								TRIGFLAG(20,"Btn: Top"),
								TRIGFLAG(21,"Btn: Bottom"),
								TRIGFLAG(22,"Btn: Left"),
								TRIGFLAG(23,"Btn: Right")
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
							TRIGFLAG(25,"Step->"),
							IBTN("Triggers when stepped on by even a pixel"),
							TRIGFLAG(26,"Step-> (Sensitive)"),
							//
							IBTN("Triggered when hit by a Light Beam matching the 'LightBeam' value"),
							TRIGFLAG(55,"Light On->"),
							IBTN("Triggered when NOT hit by a Light Beam matching the 'LightBeam' value"),
							TRIGFLAG(56,"Light Off->"),
							//
							IBTN("Triggered when a " + std::string(ZI.getItemClassName(itype_lens))
								+ " with 'Triggers Lens Trigflag' checked is activated."),
							TRIGFLAG(58,"Lens On->"),
							IBTN("Triggered when a " + std::string(ZI.getItemClassName(itype_lens))
								+ " with 'Triggers Lens Trigflag' checked is NOT activated."),
							TRIGFLAG(59,"Lens Off->"),
							//
							IBTN("Triggered when the Hero pushes against the combo"),
							TRIGFLAG(57,"Push->"),
							IBTN("Triggered when the combo is pushed as a pushblock (after it settles into the new position)."),
							TRIGFLAG(112,"Pushed->"),
							//
							IBTN("Triggers when the Hero dives on this combo"),
							TRIGFLAG(113, "Dive->"),
							IBTN("Triggers when the Hero dives on this combo (more sensitive hitbox)"),
							TRIGFLAG(114, "Dive-> (Sensitive)"),
							//
							IBTN("Triggers when screen/region loads, after levelstates and exstates are applied"),
							TRIGFLAG(128,"Triggers when screen loads"),
							IBTN("Triggers every frame automatically"),
							TRIGFLAG(47,"Always Triggered"),
							IBTN("Triggers when all enemies are defeated"),
							TRIGFLAG(87, "Enemies->"),
							IBTN("Triggers when room shutters would open"),
							TRIGFLAG(27,"Shutter->"),
							IBTN("Triggered when the player lands on this combo (after the general tab 'Landing' SFX plays)"),
							TRIGFLAG(137, "Land Here->"),
							IBTN("Triggered when the player lands from the air (anywhere on the screen)"),
							TRIGFLAG(138, "Land Anywhere->"),
							IBTN("Triggers when screen secrets trigger"),
							TRIGFLAG(88, "Secrets->"),
							IBTN("Triggers when the combo's inherent type-based *cause* occurs"
								" (Ex. the Player falling down a Pitfall or drowning in Liquid)."
								" Not available for all combo types; will be greyed out when unavailable."),
							ctcause_tflag = TRIGFLAG(144, "ComboType Causes->")
						)
					)
				)),
				TabRef(name = "Effects", Column(padding = 0_px,
					Frame(title = "Move Player",
						Row(
							Rows<3>(padding = 0_px,
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
								IBTN("Move the Player's Z by this much")
							),
							Rows<4>(padding = 0_px,
								IBTN("Set the Player's 'X' to the 'Dest X' value exactly."
									" No collision checks are performed; the player simply teleports."),
								TRIGFLAG(139, "Set X (Abs)"),
								IBTN("Set the Player's 'X' to the 'Dest X' value relative to this combo."
									" For FFCs, relative to the center of the hitbox."
									" No collision checks are performed; the player simply teleports."),
								TRIGFLAG(140, "Set X (Rel)"),
								IBTN("Set the Player's 'Y' to the 'Dest Y' value exactly."),
								TRIGFLAG(141, "Set Y (Abs)"),
								IBTN("Set the Player's 'Y' to the 'Dest Y' value relative to this combo."
									" For FFCs, relative to the center of the hitbox."
									" No collision checks are performed; the player simply teleports."),
								TRIGFLAG(142, "Set Y (Rel)"),
								IBTN("Set the Player's 'Z' to the 'Dest Z' value exactly."),
								TRIGFLAG(143, "Set Z (Abs)")
							)
						)
					),
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
									"\nEx. '1' causes '->Next CSet', '-1' causes '->Prev CSet'."),
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
							Rows_Columns<2,4>(framed = true, hAlign = 1.0,
								IBTN("Triggering the combo will trigger screen secrets. Will be permanent,"
									" unless 'Temporary Secrets' screen data flag is checked."),
								TRIGFLAG(48,"Triggers Secrets"),
								IBTN("Triggering the combo toggles the screen's \"darkness\". This resets upon leaving the screen."),
								TRIGFLAG(118,"Toggle Darkness"),
								IBTN("After triggering, the combo animation is reset. If the combo has changed"
									" (by any trigger effect), the new combo is the one that resets."),
								TRIGFLAG(18,"Reset Anim"),
								IBTN("Triggering the combo will cause its inherent type-based effects to occur."
									" Ex. Triggering a 'Signpost' displays its' string, triggering a chest opens it."
									" Not available for all combo types; will be greyed out when unavailable."),
								cteff_tflag = TRIGFLAG(28,"->ComboType Effects"),
								//
								IBTN("Kill all enemies on screen (same as 'kill all enemies' item)"),
								TRIGFLAG(100, "Kill Enemies"),
								IBTN("Delete all enemies on screen."),
								TRIGFLAG(101, "Clear Enemies"),
								IBTN("Delete all LWeapons on screen."),
								TRIGFLAG(102, "Clear LWeapons"),
								IBTN("Delete all EWeapons on screen."),
								TRIGFLAG(103, "Clear EWeapons")
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
							Frame(title = "Status Effects", hAlign = 0.0,
								Rows_Columns<3, 3>(padding = 0_px,
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
								TRIGFLAG(147, "Req X >=", 1, true),
								IBTN("The combo will only trigger if the Hero's X value is >= 'Player X'."),
								TRIGFLAG(148, "Req X <=", 1, true),
								IBTN("The combo will only trigger if the Hero's X value is <= 'Player X'."),
								TRIGFLAG(149, "...Relative", 1, true),
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
								TRIGFLAG(150, "Req Y >=", 1, true),
								IBTN("The combo will only trigger if the Hero's Y value is >= 'Player Y'."),
								TRIGFLAG(151, "Req Y <=", 1, true),
								IBTN("The combo will only trigger if the Hero's Y value is <= 'Player Y'."),
								TRIGFLAG(152, "...Relative", 1, true),
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
								TRIGFLAG(133, "Invert Player Z Req", 1, true),
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
								TRIGFLAG(145, "Req Jump >=", 1, true),
								IBTN("The combo will only trigger if the Hero's Jump value is >= 'Player Jump'."),
								TRIGFLAG(146, "Req Jump <=", 1, true),
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
											static const vector<CheckListInfo> dir_names =
											{
												{ "Up" },
												{ "Down" },
												{ "Left" },
												{ "Right" },
											};
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
								TRIGFLAG(119, "Req. Darkness"),
								IBTN("Can only trigger if the room is NOT darkened."),
								TRIGFLAG(120, "Req. No Darkness"),
								IBTN("'Proximity:' requires the Hero to be far away, instead of close"),
								TRIGFLAG(19,"Invert Proximity Req"),
								IBTN("Can only trigger if the player is standing on the ground. Handles 'standing' in sideview as well."),
								TRIGFLAG(131, "Req. Player Standing"),
								IBTN("Can only trigger if the player is NOT standing on the ground. Handles 'standing' in sideview as well."),
								TRIGFLAG(132, "Req. Player Not Standing")
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
								TRIGFLAG(51,"Require >="),
								IBTN("Only trigger if the specified counter has less than the specified amount."
									"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work."),
								TRIGFLAG(52,"Require <"),
								IBTN("The Counter Amount is a percentage of the max."),
								TRIGFLAG(135,"...Is Percent"),
								IBTN(fmt::format("The Counter Amount will be discounted based on the Hero's current '{}' item.", ZI.getItemClassName(itype_wealthmedal))),
								TRIGFLAG(127,"Apply Discount"),
								IBTN("If the counter has the specified amount, consume it."
									" Negative amount will add to the counter."),
								TRIGFLAG(53,"Consume Amount"),
								IBTN("The 'Consume Amount' will be drained/granted gradually, instead of at once."),
								TRIGFLAG(134,"...Gradual"),
								IBTN("The 'Consume Amount' will occur even if the combo does not meet its' *counter based* trigger conditions."),
								TRIGFLAG(54,"Consume w/o trig")
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
									TRIGFLAG(49,"Invert Item Req"),
									IBTN("'Req Item:' must NOT be owned to trigger"),
									TRIGFLAG(50,"Consume Item Req"),
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
							TRIGFLAG(83, "Spawns Special Item",3),
							IBTN("The item spawned by the combo will automatically be collected by the Hero."),
							TRIGFLAG(86, "Spawned Item auto-collects",3),
							IBTN("The combo's 'ExState' will be set when the spawned item is picked up, rather than when it is triggered."),
							TRIGFLAG(84, "Trigger ExState after item pickup",3),
							IBTN("The combo's 'ExState' will be set when the spawned enemy is defeated, rather than when it is triggered."),
							TRIGFLAG(85, "Trigger ExState after enemy kill",3),
							IBTN("The combo's 'ExState' will be unset instead of set when triggered."
								" (Does not necessarily revert combos that were changed by the ExState on it's own)"),
							TRIGFLAG(129, "Clear ExState on trigger",3),
							IBTN("The combo's 'ExDoor' will be unset instead of set when triggered."
								" (Does not necessarily revert combos that were changed by the ExDoor on it's own)"),
							TRIGFLAG(130, "Clear ExDoor on trigger",3),
							IBTN("This combo is triggered when the level-based switch state specified as 'LevelState' is toggled."),
							TRIGFLAG(96, "LevelState->"),
							IBTN("When triggered, toggles the level-based switch state specified as 'LevelState'."),
							TRIGFLAG(97, "->LevelState"),
							IBTN("This combo is triggered when the globalswitch state specified as 'GlobalState' is toggled."),
							TRIGFLAG(98, "GlobalState->"),
							IBTN("When triggered, toggles the global switch state specified as 'GlobalState'."
								"\nIf 'GlobalState Timer' is >0, resets the timer of the state to the specified value instead of toggling it."),
							TRIGFLAG(99, "->GlobalState"),
							IBTN("This combo contributes to its Trigger Group."),
							TRIGFLAG(109, "Contributes To TrigGroup",3),
							IBTN("When the number of combos that contribute to this trigger's Trigger Group is LESS than the Trigger Group Val, trigger this trigger."),
							TRIGFLAG(110, "TrigGroup Less->"),
							IBTN("When the number of combos that contribute to this trigger's Trigger Group is GREATER than the Trigger Group Val, trigger this trigger."),
							TRIGFLAG(111, "TrigGroup Greater->"),
							IBTN("The 'Copycat' will not cause this combo to trigger; it will only be used to trigger other combos when this combo triggers."),
							TRIGFLAG(136, "Copycat doesn't trigger this")
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
										static const vector<CheckListInfo> gstates =
										{
											{ "0" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" }, { "7" },
											{ "8" }, { "9" }, { "10" }, { "11" }, { "12" }, { "13" }, { "14" }, { "15" },
											{ "16" }, { "17" }, { "18" }, { "19" }, { "20" }, { "21" }, { "22" }, { "23" },
											{ "24" }, { "25" }, { "26" }, { "27" }, { "28" }, { "29" }, { "30" }, { "31" },
											{ "32" }, { "33" }, { "34" }, { "35" }, { "36" }, { "37" }, { "38" }, { "39" },
											{ "40" }, { "41" }, { "42" }, { "43" }, { "44" }, { "45" }, { "46" }, { "47" },
											{ "48" }, { "49" }, { "50" }, { "51" }, { "52" }, { "53" }, { "54" }, { "55" },
											{ "56" }, { "57" }, { "58" }, { "59" }, { "60" }, { "61" }, { "62" }, { "63" },
											{ "64" }, { "65" }, { "66" }, { "67" }, { "68" }, { "69" }, { "70" }, { "71" },
											{ "72" }, { "73" }, { "74" }, { "75" }, { "76" }, { "77" }, { "78" }, { "79" },
											{ "80" }, { "81" }, { "82" }, { "83" }, { "84" }, { "85" }, { "86" }, { "87" },
											{ "88" }, { "89" }, { "90" }, { "91" }, { "92" }, { "93" }, { "94" }, { "95" },
											{ "96" }, { "97" }, { "98" }, { "99" }, { "100" }, { "101" }, { "102" }, { "103" },
											{ "104" }, { "105" }, { "106" }, { "107" }, { "108" }, { "109" }, { "110" }, { "111" },
											{ "112" }, { "113" }, { "114" }, { "115" }, { "116" }, { "117" }, { "118" }, { "119" },
											{ "120" }, { "121" }, { "122" }, { "123" }, { "124" }, { "125" }, { "126" }, { "127" },
											{ "128" }, { "129" }, { "130" }, { "131" }, { "132" }, { "133" }, { "134" }, { "135" },
											{ "136" }, { "137" }, { "138" }, { "139" }, { "140" }, { "141" }, { "142" }, { "143" },
											{ "144" }, { "145" }, { "146" }, { "147" }, { "148" }, { "149" }, { "150" }, { "151" },
											{ "152" }, { "153" }, { "154" }, { "155" }, { "156" }, { "157" }, { "158" }, { "159" },
											{ "160" }, { "161" }, { "162" }, { "163" }, { "164" }, { "165" }, { "166" }, { "167" },
											{ "168" }, { "169" }, { "170" }, { "171" }, { "172" }, { "173" }, { "174" }, { "175" },
											{ "176" }, { "177" }, { "178" }, { "179" }, { "180" }, { "181" }, { "182" }, { "183" },
											{ "184" }, { "185" }, { "186" }, { "187" }, { "188" }, { "189" }, { "190" }, { "191" },
											{ "192" }, { "193" }, { "194" }, { "195" }, { "196" }, { "197" }, { "198" }, { "199" },
											{ "200" }, { "201" }, { "202" }, { "203" }, { "204" }, { "205" }, { "206" }, { "207" },
											{ "208" }, { "209" }, { "210" }, { "211" }, { "212" }, { "213" }, { "214" }, { "215" },
											{ "216" }, { "217" }, { "218" }, { "219" }, { "220" }, { "221" }, { "222" }, { "223" },
											{ "224" }, { "225" }, { "226" }, { "227" }, { "228" }, { "229" }, { "230" }, { "231" },
											{ "232" }, { "233" }, { "234" }, { "235" }, { "236" }, { "237" }, { "238" }, { "239" },
											{ "240" }, { "241" }, { "242" }, { "243" }, { "244" }, { "245" }, { "246" }, { "247" },
											{ "248" }, { "249" }, { "250" }, { "251" }, { "252" }, { "253" }, { "254" }, { "255" }
										};
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
										static const vector<CheckListInfo> gstates =
										{
											{ "0" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" }, { "7" },
											{ "8" }, { "9" }, { "10" }, { "11" }, { "12" }, { "13" }, { "14" }, { "15" },
											{ "16" }, { "17" }, { "18" }, { "19" }, { "20" }, { "21" }, { "22" }, { "23" },
											{ "24" }, { "25" }, { "26" }, { "27" }, { "28" }, { "29" }, { "30" }, { "31" },
											{ "32" }, { "33" }, { "34" }, { "35" }, { "36" }, { "37" }, { "38" }, { "39" },
											{ "40" }, { "41" }, { "42" }, { "43" }, { "44" }, { "45" }, { "46" }, { "47" },
											{ "48" }, { "49" }, { "50" }, { "51" }, { "52" }, { "53" }, { "54" }, { "55" },
											{ "56" }, { "57" }, { "58" }, { "59" }, { "60" }, { "61" }, { "62" }, { "63" },
											{ "64" }, { "65" }, { "66" }, { "67" }, { "68" }, { "69" }, { "70" }, { "71" },
											{ "72" }, { "73" }, { "74" }, { "75" }, { "76" }, { "77" }, { "78" }, { "79" },
											{ "80" }, { "81" }, { "82" }, { "83" }, { "84" }, { "85" }, { "86" }, { "87" },
											{ "88" }, { "89" }, { "90" }, { "91" }, { "92" }, { "93" }, { "94" }, { "95" },
											{ "96" }, { "97" }, { "98" }, { "99" }, { "100" }, { "101" }, { "102" }, { "103" },
											{ "104" }, { "105" }, { "106" }, { "107" }, { "108" }, { "109" }, { "110" }, { "111" },
											{ "112" }, { "113" }, { "114" }, { "115" }, { "116" }, { "117" }, { "118" }, { "119" },
											{ "120" }, { "121" }, { "122" }, { "123" }, { "124" }, { "125" }, { "126" }, { "127" },
											{ "128" }, { "129" }, { "130" }, { "131" }, { "132" }, { "133" }, { "134" }, { "135" },
											{ "136" }, { "137" }, { "138" }, { "139" }, { "140" }, { "141" }, { "142" }, { "143" },
											{ "144" }, { "145" }, { "146" }, { "147" }, { "148" }, { "149" }, { "150" }, { "151" },
											{ "152" }, { "153" }, { "154" }, { "155" }, { "156" }, { "157" }, { "158" }, { "159" },
											{ "160" }, { "161" }, { "162" }, { "163" }, { "164" }, { "165" }, { "166" }, { "167" },
											{ "168" }, { "169" }, { "170" }, { "171" }, { "172" }, { "173" }, { "174" }, { "175" },
											{ "176" }, { "177" }, { "178" }, { "179" }, { "180" }, { "181" }, { "182" }, { "183" },
											{ "184" }, { "185" }, { "186" }, { "187" }, { "188" }, { "189" }, { "190" }, { "191" },
											{ "192" }, { "193" }, { "194" }, { "195" }, { "196" }, { "197" }, { "198" }, { "199" },
											{ "200" }, { "201" }, { "202" }, { "203" }, { "204" }, { "205" }, { "206" }, { "207" },
											{ "208" }, { "209" }, { "210" }, { "211" }, { "212" }, { "213" }, { "214" }, { "215" },
											{ "216" }, { "217" }, { "218" }, { "219" }, { "220" }, { "221" }, { "222" }, { "223" },
											{ "224" }, { "225" }, { "226" }, { "227" }, { "228" }, { "229" }, { "230" }, { "231" },
											{ "232" }, { "233" }, { "234" }, { "235" }, { "236" }, { "237" }, { "238" }, { "239" },
											{ "240" }, { "241" }, { "242" }, { "243" }, { "244" }, { "245" }, { "246" }, { "247" },
											{ "248" }, { "249" }, { "250" }, { "251" }, { "252" }, { "253" }, { "254" }, { "255" }
										};
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
							TRIGFLAG(125,"->ClearTint",1,true),
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
									low = 0, high = 255, val = local_ref.trig_levelitems,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.trig_levelitems = val;
									}),
								Button(
									width = 1.5_em, padding = 0_px, forceFitH = true,
									text = "P", hAlign = 1.0, onPressFunc = [&]()
									{
										int32_t flags = local_ref.trig_levelitems;
										static const vector<CheckListInfo> litem_names =
										{
											{ "McGuffin", "The Hero has the McGuffin for the 'Trig DMap Level'" },
											{ "Map", "The Hero has the Map for the 'Trig DMap Level'" },
											{ "Compass", "The Hero has the Compass for the 'Trig DMap Level'" },
											{ "Boss Killed", "The Hero has cleared the 'Dungeon Boss' room for the 'Trig DMap Level'" },
											{ "Boss Key", "The Hero has the Boss Key for the 'Trig DMap Level'" },
											{ "Custom 1", "The Hero has the Custom 1 state for the 'Trig DMap Level'" },
											{ "Custom 2", "The Hero has the Custom 2 state for the 'Trig DMap Level'" },
											{ "Custom 3", "The Hero has the Custom 3 state for the 'Trig DMap Level'" },
										};
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
								TRIGFLAG(121,"Require All"),
								IBTN("The level flags set for 'Req Flags:' must NOT ALL (some is ok) be on for this to trigger."
									"\n\nThis is a 'Condition'. It won't trigger the combo on its own, but it must apply for other triggers to work."),
								TRIGFLAG(122,"Require Not All"),
								IBTN("The level flags set for 'Req Flags:' will be enabled when this trigger triggers."
									" If '->Unset' is also checked, the flags will be toggled instead."),
								TRIGFLAG(123,"->Set"),
								IBTN("The level flags set for 'Req Flags:' will be disabled when this trigger triggers."
									" If '->Set' is also checked, the flags will be toggled instead."),
								TRIGFLAG(124,"->Unset")
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
									static const vector<CheckListInfo> lstates =
									{
										{ "0" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" }, { "7" },
										{ "8" }, { "9" }, { "10" }, { "11" }, { "12" }, { "13" }, { "14" }, { "15" },
										{ "16" }, { "17" }, { "18" }, { "19" }, { "20" }, { "21" }, { "22" }, { "23" },
										{ "24" }, { "25" }, { "26" }, { "27" }, { "28" }, { "29" }, { "30" }, { "31" }
									};
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
									static const vector<CheckListInfo> lstates =
									{
										{ "0" }, { "1" }, { "2" }, { "3" }, { "4" }, { "5" }, { "6" }, { "7" },
										{ "8" }, { "9" }, { "10" }, { "11" }, { "12" }, { "13" }, { "14" }, { "15" },
										{ "16" }, { "17" }, { "18" }, { "19" }, { "20" }, { "21" }, { "22" }, { "23" },
										{ "24" }, { "25" }, { "26" }, { "27" }, { "28" }, { "29" }, { "30" }, { "31" }
									};
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
	
	l_minmax_trig->setText((local_ref.triggerflags[0] & (combotriggerINVERTMINMAX))
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
	
	if(!hasCTypeEffects(parent_comboref.type))
		local_ref.triggerflags[0] &= ~combotriggerCMBTYPEFX;
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
			return true;
		}
	}
	return false;
}

