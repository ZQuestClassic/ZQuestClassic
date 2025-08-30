#include "tilesetwizard.h"
#include "quest_rules.h"
#include "headerdlg.h"
#include "pickruleset.h"
#include "pickruletemplate.h"
#include "info.h"
#include "base/qst.h"
#include "zq/zq_class.h"
#include "zq/zq_files.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include <gui/builder.h>
#include <optional>

extern BITMAP* zqdialog_bg_bmp;
void update_map_count(word newmapcount);

static const GUI::ListData tilesetList
{
	{ "Cambria (recommended)", cambria,
		"Cambria is a modern tileset with retro aesthetics." },
	{ "Classic", classic,
		"Classic is a minimalist tileset." },
	// TODO: we don't have a good way to load a blank quest.
	// { "Blank", blank,
	// 	"No graphics, sfx, combos ... a totally blank slate." },
	{ "Choose from disk", file,
		"Choose a tileset from disk." }
};

static int wizard_state = 0;
enum
{
	wizstate_preload,
	wizstate_loaded,
	wizstate_max
};
bool call_tileset_wizard()
{
	wizard_state = wizstate_preload;
	TilesetWizard wizard;
	wizard.show();
	return wizard.success();
}

TilesetWizard::TilesetWizard() : tileset_choice(cambria), tsetflags(TILESET_CLEARHEADER|TILESET_BUGFIX|TILESET_SCR_BUGFIX)
{}

#define TILESET_FLAG(bit, txt) \
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
			window->setTitle("New Quest > Choose a tileset");

			g->add(Row(
				Label(text = "To start a new quest, first select a tileset")
			));

			g->add(Row(
				hPadding = 8_px,
				this->tilesetChoice = RadioSet(
					hPadding = 6_px,
					set = 0,
					onToggle = message::CHOOSE_TILESET,
					checked = tileset_choice,
					data = tilesetList
				),
				this->tilesetInfo = Label(noHLine = true,
					fitParent = true,
					framed = true,
					maxLines = 4,
					margins = 0_px,
					padding = 9_px,
					height = 100_px,
					text = tilesetList.findInfo(tileset_choice)
				)
			));

			g->add(Rows<1>(
				TILESET_FLAG(TILESET_BUGFIX, "Apply bug fixes"),
				TILESET_FLAG(TILESET_SCR_BUGFIX, "Apply script bug fixes"),
				TILESET_FLAG(TILESET_CLEARMAPS, "Clear tileset maps"),
				TILESET_FLAG(TILESET_CLEARSCRIPTS, "Clear tileset scripts")
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
		case wizstate_loaded:
		{
			window->setTitle("New Quest > Configure");

			g->add(Row(
				Label(text = "Configure your quest (this can be done later)")
			));

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
					)
				));
			
			g->add(Row(
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

bool TilesetWizard::loadTileset(std::string path)
{
	if (qe_OK == load_tileset(path.c_str(), tsetflags))
	{
		// Advance to next state
		wizard_state = wizstate_loaded;
		rerun_dlg = true;
		new_map_count = map_count;
		return true;
	}

	return false;
}

static std::optional<zquestheader> load_qst_header(std::string path)
{
	zquestheader tempheader{};

	byte skip_flags[4];
	for (int i=0; i<skip_max; ++i)
		set_bit(skip_flags,i,1);
	set_bit(skip_flags,skip_header,0);

	int ret = loadquest(path.c_str(), &tempheader, &QMisc, customtunes, false, skip_flags);
	if (ret)
		return std::nullopt;

	return tempheader;
}

bool TilesetWizard::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::CANCEL:
			return true;

		case message::CHOOSE_TILESET:
		{
			file_path = "";
			tileset_choice = (tileset_option)tilesetChoice->getChecked();
			tilesetInfo->setText(tilesetList.findInfo(tileset_choice));

			if (tileset_choice == file)
			{
				if (auto fname = get_qst_name("./tilesets"))
				{
					if (auto header = load_qst_header(fname))
					{
						file_path = fname;
						tilesetInfo->setText(fmt::format("File: {}\nTileset: {}\nAuthor: {}",
							util::get_filename(fname), header->title, header->author));
					}
				}

				if (file_path.empty())
				{
					tileset_choice = cambria;
					tilesetChoice->setChecked(tileset_choice);
					tilesetInfo->setText(tilesetList.findInfo(tileset_choice));
				}
			}

			return false;
		}

		case message::OK:
		{
			if (wizard_state == wizstate_preload)
			{
				if (tileset_choice == blank)
				{
					// TODO: how?
					rerun_dlg = false;
					return true;
				}

				if (tileset_choice == cambria)
					file_path = "tilesets/cambria.qst";
				else if (tileset_choice == classic)
					file_path = "modules/classic/default.qst";

				if (!file_path.empty())
					loadTileset(file_path);

				return true;
			}
			else if (wizard_state == wizstate_loaded)
			{
				update_map_count(new_map_count);
				rerun_dlg = false;

				return true;
			}

			return false;
		}
	}

	return false;
}

bool TilesetWizard::success() const
{
	return wizard_state == wizstate_loaded;
}
