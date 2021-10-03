#include "script_rules.h"
#include <gui/builder.h>
#include "gui/size.h"
#include "../jwin.h"
#include "../zquest.h"
#include "../zsys.h"

using GUI::Size;
using GUI::operator ""_em;
using GUI::operator ""_px;
using GUI::operator ""_lpx;
using GUI::operator ""_spx;

static const GUI::ListData scriptRulesList
{
	{ "Item Scripts Continue To Run", qr_ITEMSCRIPTSKEEPRUNNING },
	{ "Clear InitD[] on Script Change", qr_CLEARINITDONSCRIPTCHANGE },
	{ "Hero OnDeath script runs AFTER engine death animation", qr_ONDEATH_RUNS_AFTER_DEATH_ANIM },
	{ "Passive Subscreen Script runs during Active Subscreen Script", qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT },
	{ "DMap Active Script runs during Active Subscreen Script", qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT },
	{ "Combos Run Scripts on Layer 0", qr_COMBOSCRIPTS_LAYER_0 },
	{ "Combos Run Scripts on Layer 1", qr_COMBOSCRIPTS_LAYER_1 },
	{ "Combos Run Scripts on Layer 2", qr_COMBOSCRIPTS_LAYER_2 },
	{ "Combos Run Scripts on Layer 3", qr_COMBOSCRIPTS_LAYER_3 },
	{ "Combos Run Scripts on Layer 4", qr_COMBOSCRIPTS_LAYER_4 },
	{ "Combos Run Scripts on Layer 5", qr_COMBOSCRIPTS_LAYER_5 },
	{ "Combos Run Scripts on Layer 6", qr_COMBOSCRIPTS_LAYER_6 },
	{ "Use Old Global Init and SaveLoad Timing", qr_OLD_INIT_SCRIPT_TIMING },
	{ "Passive Subscreen Script runs during wipes/refills", qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN }
};

static const GUI::ListData instructionRulesList
{
	{ "No Item Script Waitdraw()", qr_NOITEMWAITDRAW },
	{ "No FFC Waitdraw()", qr_NOFFCWAITDRAW },
	{ "Old eweapon->Parent", qr_OLDEWPNPARENT },
	{ "Old Args for CreateBitmap() and bitmap->Create()", qr_OLDCREATEBITMAP_ARGS },
	{ "Print Script Metadata on Traces", qr_TRACESCRIPTIDS },
	{ "Writing to INPUT Overrides Drunk State", qr_FIXDRUNKINPUTS },
	{ "Don't Allow Setting Action to Rafting", qr_DISALLOW_SETTING_RAFTING },
	{ "Writing npc->Defense[NPCD_SCRIPT] Sets All Script Defences", qr_250WRITEEDEFSCRIPT },
	{ "Writing npc->Weapon Sets its Weapon Sprite", qr_SETENEMYWEAPONSPRITESONWPNCHANGE },
	{ "Broken DrawInteger and DrawCharacter Scaling", qr_BROKENCHARINTDRAWING },
	{ "npc->Weapon Uses Sprite 246-255 for EW_CUSTOM*", qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES },
	{ "All bitmap-> and FileSystem-> paths relative to quest 'Files' folder", qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE },
	{ "Don't allow overwriting hopping action", qr_NO_OVERWRITING_HOPPING },
	{ "Sprite->Step uses new, precise values", qr_STEP_IS_FLOAT },
	{ "Old printf() args", qr_OLD_PRINTF_ARGS },
	{ "Writing Screen->EntryX, EntryY Resets Spawn Points", qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS },
	{ "Log on Loading Invalid UID", qr_LOG_INVALID_UID_LOAD },
	{ "Broken Combodata->InitD[]", qr_COMBODATA_INITD_MULT_TENK },
	{ "Script writes to Hero->Step don't carry over", qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER }
};

static const GUI::ListData objectRulesList
{
	{ "Sprite Coordinates are Float", qr_SPRITEXY_IS_FLOAT },
	{ "Weapons Have Shadows", qr_WEAPONSHADOWS },
	{ "Items Have Shadows", qr_ITEMSHADOWS },
	{ "Weapons Live One Extra Frame With WDS_DEAD", qr_WEAPONS_EXTRA_FRAME }
};

static const GUI::ListData drawingRulesList
{
	{ "Scripts Draw When Stepping Forward In Dungeons", qr_SCRIPTSRUNINLINKSTEPFORWARD },
	{ "Scripts Draw During Warps", qr_SCRIPTDRAWSINWARPS }
};

static const GUI::ListData bugfixRulesList
{
	{ "Fix Scripts Running During Scrolling", qr_FIXSCRIPTSDURINGSCROLLING },
	{ "Game->Misc[] is not *10000", qr_OLDQUESTMISC },
	{ "Always Deallocate Arrays", qr_ALWAYS_DEALLOCATE_ARRAYS },
	{ "Don't Deallocate Init/SaveLoad Local Arrays", qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS }
};

ScriptRulesDialog::ScriptRulesDialog(byte const* qrs, size_t qrs_per_tab, std::function<void(byte*)> setQRs):
	setQRs(setQRs), qrs_per_tab(qrs_per_tab)
{
	memcpy(local_qrs, qrs, QR_SZ);
}

std::shared_ptr<GUI::Widget> ScriptRulesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	return Window(
		title = "Script Rules",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			TabPanel(
				maxwidth = sized(308_px, 800_px),
				TabRef(
					name = "Script",
					QRPanel(
						margins = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = scriptRulesList
					)
				),
				TabRef(
					name = "Instructions",
					QRPanel(
						margins = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = instructionRulesList
					)
				),
				TabRef(
					name = "Object",
					QRPanel(
						margins = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = objectRulesList
					)
				),
				TabRef(
					name = "Drawing",
					QRPanel(
						margins = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = drawingRulesList
					)
				),
				TabRef(
					name = "Bugfix",
					QRPanel(
						margins = 2_spx,
						onToggle = message::TOGGLE_QR,
						initializer = local_qrs,
						count = qrs_per_tab,
						data = bugfixRulesList
					)
				)
			),
			Row(
				topMargin = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					onClick = message::OK),
				Button(
					text = "Cancel",
					onClick = message::CANCEL)
			)
		)
	);
}

bool ScriptRulesDialog::handleMessage(message msg, GUI::MessageArg messageArg)
{
	switch(msg)
	{
		case message::TOGGLE_QR:
			toggle_bit(local_qrs, messageArg);
			return false;
		case message::OK:
			setQRs(local_qrs);
			[[fallthrough]];
		case message::CANCEL:
		default:
			return true;
	}
}
