#include "tilesetwizard.h"
#include "quest_rules.h"
#include "headerdlg.h"
#include "pickruleset.h"
#include "pickruletemplate.h"
#include "info.h"
#include "qst.h"
#include "zq/zq_class.h"
#include "zq/zq_files.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include <gui/builder.h>

extern BITMAP* zqdialog_bg_bmp;
void update_map_count(word newmapcount);

void popup_bugfix_dlg(const char* cfg); //zq_class.cpp

static int wizard_state = 0;
enum
{
	wizstate_preload,
	wizstate_loaded,
	wizstate_max
};
void call_tileset_wizard()
{
	wizard_state = wizstate_preload;
	TilesetWizard().show();
}
void call_tileset_wizard_existing()
{
	wizard_state = wizstate_loaded;
	TilesetWizard().show();
}

TilesetWizard::TilesetWizard() : tsetflags(TILESET_CLEARHEADER)
{}

#define TILESET_FLAG(bit, txt, inf) \
INFOBTN(inf), \
Checkbox( \
	text = txt, hAlign = 0.0, \
	checked = tsetflags & bit, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(tsetflags, bit, state); \
	} \
)
std::shared_ptr<GUI::Widget> TilesetWizard::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Grid> g;
	window = Window(
		title = "Tileset Wizard",
		onClose = message::CANCEL,
		g = Column()
	);
	switch(wizard_state)
	{
		default:
			displayinfo("ERROR", "Bad WizardState!");
			wizard_state = wizstate_preload;
			[[fallthrough]];
		case wizstate_preload:
		{
			g->add(Rows<2>(
					TILESET_FLAG(TILESET_CLEARMAPS, "Clear Maps",
						"If checked, all maps/screens preset in the tileset will be cleared."),
					TILESET_FLAG(TILESET_CLEARSCRIPTS, "Clear Scripts",
						"If checked, any and all scripts in the tileset will be removed."),
					TILESET_FLAG(TILESET_CLEARHEADER, "Clear Header",
						"If checked, the header (title, password, author, etc) will be cleared.")
				));
			g->add(Row(
					vAlign = 1.0,
					spacing = 2_em,
					Button(
						text = "Load",
						minwidth = 90_px,
						onClick = message::OK),
					Button(
						text = "Cancel",
						minwidth = 90_px,
						onClick = message::CANCEL)
				));
			break;
		}
		case wizstate_loaded:
		{
			g->add(Row(
					Rows<2>(
						Button(text = "Header",
							fitParent = true,
							onPressFunc = [&]()
							{
								call_header_dlg();
							}),
						INFOBTN("Modify the quest's title, author, version info, etc"),
						Button(text = "Rulesets",
							fitParent = true,
							onPressFunc = [&]()
							{
								call_ruleset_dlg();
							}),
						INFOBTN("Choosing a ruleset wipes all quest rules to a default set of values."),
						Button(text = "Rule Templates",
							fitParent = true,
							onPressFunc = [&]()
							{
								call_ruletemplate_dlg();
							}),
						INFOBTN("Choosing a rule template forcibly sets a chunk of quest rules to particular presets."),
						Button(text = "Quest Rules",
							fitParent = true,
							onPressFunc = [&]()
							{
								auto dlg = QRDialog(quest_rules, 21, [&](byte* qrs)
									{
										memcpy(quest_rules, qrs, QR_SZ);
										unpack_qrs();
									});
								dlg.searchmode = true;
								dlg.show();
							}),
						INFOBTN("View and search ALL the quest rules individually.")
					),
					Rows<3>(
						Label(text = "Map Count", hAlign = 1.0),
						TextField(
							type = GUI::TextField::type::INT_DECIMAL,
							low = 1, high = 255,
							val = new_map_count,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								new_map_count = val;
							}
						),
						INFOBTN("The number of maps in the quest. Editable in 'Quest->Options' later.")
					)
				));
			g->add(Row(
					vAlign = 1.0,
					spacing = 2_em,
					Button(
						text = "OK",
						minwidth = 90_px,
						onClick = message::OK)
				));
			break;
		}
	}
	return window;
}

bool TilesetWizard::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::CANCEL:
			switch(wizard_state)
			{
				case wizstate_preload:
					return true;
			}
			[[fallthrough]];
		case message::OK:
		{
			switch(wizard_state)
			{
				case wizstate_preload:
				{
					if(char* fname = get_qst_name("./tilesets"))
					{
						if(qe_OK == load_tileset(fname, tsetflags))
						{
							// Advance to next state
							wizard_state = wizstate_loaded;
							rerun_dlg = true;
							auto* tmp = screen;
							screen = zqdialog_bg_bmp;
							refresh(rALL);
							screen = tmp;
							new_map_count = map_count;
							return true;
						}
						else return true; //Fail; exit dialog
					}
					else return false; //Cancel; return to prior dialog
				}
				case wizstate_loaded:
				{
					update_map_count(new_map_count);
					wizard_state = wizstate_preload;
					if(zc_get_config("zquest","auto_loadtileset_bugfixes",0))
						applyRuleTemplate(ruletemplateFixCompat);
					else
						popup_bugfix_dlg("dsa_compatrule3");
					rerun_dlg = false;
					return true;
				}
			}
			return true;
		}
	}
	return false;
}
