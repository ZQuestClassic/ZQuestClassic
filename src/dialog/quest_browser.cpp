#include "dialog/quest_browser.h"
#include "dialog/quest_browser_data.h"
#include "dialog/tilesetwizard.h"
#include "base/util.h"
#include "base/version.h"
#include "core/qst.h"
#include "gui/builder.h"
#include "zalleg/files.h"
#include "zconfig.h"
#include "zq/zquest.h"
#include "zq/zq_files.h"
#include <fmt/format.h>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

using quest_browser::Entry;

enum { SORT_RECENT, SORT_EDITED, SORT_ZCVER };

static bool matches_filter(Entry const& e, std::string const& filter)
{
	if (filter.empty())
		return true;

	auto contains_ci = [](std::string const& haystack, std::string const& needle) {
		auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
			[](char a, char b) { return tolower((uint8_t)a) == tolower((uint8_t)b); });
		return it != haystack.end();
	};

	return contains_ci(e.title, filter) || contains_ci(e.author, filter)
		|| contains_ci(e.path, filter);
}

QuestBrowserDialog::QuestBrowserDialog()
{
	sortMode = std::clamp(zc_get_config(quest_browser::CFG_SECTION, "sort_mode", SORT_RECENT),
		(int32_t)SORT_RECENT, (int32_t)SORT_ZCVER);
	if (!quest_browser::gathered())
		quest_browser::gather();
}

void QuestBrowserDialog::refreshRows()
{
	auto const& entries = quest_browser::entries();

	std::vector<size_t> order;
	for (size_t i = 0; i < entries.size(); i++)
	{
		if (matches_filter(entries[i], filter))
			order.push_back(i);
	}

	std::stable_sort(order.begin(), order.end(), [&](size_t ai, size_t bi) {
		auto const& a = entries[ai];
		auto const& b = entries[bi];
		switch(sortMode)
		{
		case SORT_EDITED:
			return a.mtime > b.mtime;
		case SORT_ZCVER:
			return a.version_key > b.version_key;
		case SORT_RECENT:
		default:
			if (a.recent_rank != b.recent_rank)
				return a.recent_rank < b.recent_rank;
			return a.mtime > b.mtime;
		}
	});

	rowEntries = order;

	std::vector<GUI::QuestListRow> rows;
	rows.reserve(order.size());
	for (size_t i : order)
	{
		auto const& e = entries[i];
		GUI::QuestListRow row;
		row.path = e.path;
		if (e.meta_scanned && !e.meta_ok)
			row.title = "(unreadable quest)";
		else if (e.title.empty())
			row.title = e.meta_scanned ? quest_browser::display_name_for_path(e.path) : "...";
		else
			row.title = e.title;
		if (!e.author.empty())
			row.byline = fmt::format("By {}", e.author);
		// Drop build metadata ("+2026-06-18.local") - the date column
		// already says when, and full strings overflow the column.
		row.version = e.zc_version.substr(0, e.zc_version.find('+'));
		row.date = e.from_manifest ? e.date_str : quest_browser::format_date(e.mtime);
		row.icon_rgba = e.icon_rgba;
		rows.push_back(std::move(row));
	}

	questList->setEmptyText(filter.empty()
		? "No quests found - use \"Open Quest\" or \"Scan Folder\" to add some."
		: "No quests match filter.");

	// Keep the same quest selected when rows shift (background scans can
	// change sort order), without moving the scroll position.
	std::string sel_path;
	if (auto const* sel = questList->getSelectedRow())
		sel_path = sel->path;

	questList->setRows(std::move(rows));

	if (!sel_path.empty())
	{
		auto const& newRows = questList->getRows();
		for (size_t i = 0; i < newRows.size(); i++)
		{
			if (newRows[i].path == sel_path)
			{
				questList->setSelectedIndex((int32_t)i, false);
				break;
			}
		}
	}

	updatePathLabel();
}

bool QuestBrowserDialog::pumpScan()
{
	auto& entries = quest_browser::entries();

	// Loading a quest's metadata means decoding (much of) the file, so only
	// scan what the user can currently see, +/- one row.
	int32_t start, end;
	questList->getVisibleRows(start, end);
	start = std::max(0, start - 1);
	end = std::min((int32_t)rowEntries.size(), end + 1);

	for (int32_t r = start; r < end; ++r)
	{
		auto& e = entries[rowEntries[r]];
		if (e.meta_scanned)
			continue;

		e.meta_ok = quest_browser::scan_meta(e);
		e.meta_scanned = true;
		quest_browser::mark_cache_dirty();
		refreshRows();
		return true;
	}

	for (int32_t r = start; r < end; ++r)
	{
		auto& e = entries[rowEntries[r]];
		if (!e.meta_ok || e.icon_scanned)
			continue;

		quest_browser::scan_icon(e);
		e.icon_scanned = true;
		quest_browser::mark_cache_dirty();
		refreshRows();
		return true;
	}

	quest_browser::persist();

	std::string new_version = quest_browser::check_for_update();
	if (!new_version.empty())
	{
		updateVersion = new_version;
		statusLabel->setText(fmt::format("Update available: {}", new_version));
		return true;
	}

	return false;
}

// Local dev builds tack ".local"/"+local" onto the last release tag; strip
// it so the link goes to that release's page.
static std::string release_page_url(std::string version)
{
	for (char const* suffix : {".local", "+local"})
	{
		if (version.ends_with(suffix))
			version.resize(version.size() - strlen(suffix));
	}
	return fmt::format("https://zquestclassic.com/releases/{}/", version);
}

bool QuestBrowserDialog::pickSelected()
{
	auto const* row = questList->getSelectedRow();
	if (!row)
		return false;

	chosen_path = row->path;
	res = result::OPEN_PATH;
	return true;
}

void QuestBrowserDialog::updatePathLabel()
{
	if (!pathLabel)
		return;

	std::string path;
	if (auto const* row = questList->getSelectedRow())
		path = row->path;
	// Keep the tail when truncating - it's the informative part.
	if (path.size() > pathLabelLen && pathLabelLen > 3)
		path = "..." + path.substr(path.size() - (pathLabelLen - 3));
	pathLabel->setText(fmt::format("{:<{}}", path, pathLabelLen));
}

std::shared_ptr<GUI::Widget> QuestBrowserDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	// Size the path label to roughly the list width.
	int32_t sw = screen ? screen->w : 640;
	pathLabelLen = std::max(40, (sw - 140) / 7);

	// The layout system packs row contents at their natural widths (grids
	// never stretch cells), so to left/right-justify groups within one line,
	// measure the groups and insert a spacer covering the leftover width.
	// If GUI::Grid ever learns a stretch primitive (a spacer that absorbs a
	// row's excess width), all of this measurement code - and the launcher's
	// space-padded-label equivalents - could be deleted.
	FONT* dlgfont = GUI_DEF_FONT;
	int32_t pad2 = 2 * DEFAULT_PADDING_INT;
	int32_t th = text_height(dlgfont);
	int32_t list_w = sw - 130; // keep in sync with QuestListView's preferred width
	auto button_w = [&](char const* s) {
		return 16 + gui_text_width(dlgfont, s) + pad2;
	};

	int32_t buttons_w = button_w("New Quest") + button_w("Open Quest...")
		+ button_w("Scan Folder...");
	int32_t filter_w = text_length(dlgfont, "Filter:") + pad2
		+ 120 + pad2 // the filter TextField's forced width
		+ text_length(dlgfont, "Sort:") + pad2
		+ 3 * th + text_length(dlgfont, "Recently Opened") + pad2;
	int32_t top_spacer = std::max(8, list_w - buttons_w - filter_w);

	int32_t checkbox_w = std::max(th + 4, 14) + 4
		+ gui_text_width(dlgfont, "Automatically open most recent quest") + 2 + pad2;
	// The checkbox sits on the button row's left side; spacers keep the
	// Load Quest button centered within the list width anyway.
	int32_t load_w = button_w("Load Quest");
	int32_t load_spacer_l = std::max(8, (list_w - load_w) / 2 - checkbox_w);
	int32_t load_spacer_r = std::max(8, (list_w - load_w) / 2);

	// The footer holds the update-available message (left, empty until a
	// background check finds one) and the current version (right edge).
	std::string version_text = fmt::format("v{}", getVersionString());
	int32_t version_w = text_length(dlgfont, version_text.c_str()) + pad2;
	int32_t status_w = std::max(8, list_w - version_w);

	static const GUI::ListData sortList
	{
		{ "Recently Opened", SORT_RECENT },
		{ "Last Edited", SORT_EDITED },
		{ "ZC Version", SORT_ZCVER }
	};

	std::shared_ptr<GUI::Window> window = Window(
		title = "Quests",
		onClose = message::QUIT,
		Column(
			Row(
				Button(text = "&New Quest", onClick = message::NEW_QUEST),
				Button(text = "&Open Quest...", onClick = message::OPEN_QUEST),
				Button(text = "Scan &Folder...", onClick = message::SCAN_FOLDER),
				Label(text = "", width = GUI::Size::pixels(top_spacer)),
				Label(text = "Filter:"),
				TextField(
					type = GUI::TextField::type::TEXT,
					maxLength = 64,
					width = 120_px,
					focused = true, // type right away to filter
					onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
					{
						filter = std::string(text);
						refreshRows();
					}),
				Label(text = "Sort:"),
				DropDownList(data = sortList,
					selectedValue = sortMode,
					onSelectFunc = [&](int32_t val)
					{
						sortMode = val;
						zc_set_config(quest_browser::CFG_SECTION, "sort_mode", val);
						refreshRows();
					})
			),
			questList = QuestListView(
				onDClick = message::PICK,
				onSelectFunc = [&](int32_t)
				{
					updatePathLabel();
				}
			),
			pathLabel = Label(text = std::string(pathLabelLen, ' '), hAlign = 0.0),
			Row(
				topMargin = 16_px,
				Checkbox(
					text = "Automatically open most recent quest",
					checked = OpenLastQuest != 0,
					onToggleFunc = [&](bool state)
					{
						OpenLastQuest = state ? 1 : 0;
						zc_set_config("zquest", "open_last_quest", OpenLastQuest);
					}),
				Label(text = "", width = GUI::Size::pixels(load_spacer_l)),
				Button(text = "&Load Quest", onClick = message::PICK),
				Label(text = "", width = GUI::Size::pixels(load_spacer_r))
			),
			Row(
				statusLabel = Label(text = "", maxLines = 1,
					width = GUI::Size::pixels(status_w),
					textColor = vc(14),
					onPressFunc = [&]()
					{
						if (!updateVersion.empty())
							util::open_web_link(release_page_url(updateVersion));
					}),
				Label(text = version_text,
					onPressFunc = []()
					{
						util::open_web_link(release_page_url(getVersionString()));
					})
			)
		)
	);

	questList->setOnIdleFunc([this]() { return pumpScan(); });
	refreshRows();
	return window;
}

bool QuestBrowserDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
	case message::NEW_QUEST:
		res = result::NEW_QUEST;
		return true;

	case message::OPEN_QUEST:
	{
		if (char* name = get_qst_name(nullptr))
		{
			std::string path = quest_browser::normalize_path(name);
			if (fs::path(path).extension() == ".qst")
				quest_browser::add_imported_path(path);
			chosen_path = path;
			res = result::OPEN_PATH;
			return true;
		}
		return false;
	}

	case message::SCAN_FOLDER:
	{
		auto folder = prompt_for_existing_folder("Scan Folder for Quests", "");
		if (folder)
		{
			quest_browser::add_imported_path(*folder);
			quest_browser::gather();
			refreshRows();
		}
		return false;
	}

	case message::PICK:
		return pickSelected();

	case message::QUIT:
		res = result::QUIT;
		return true;

	default:
		return false;
	}
}

// Returns true once a quest is loaded (picked, or created via the wizard).
static bool run_quest_browser()
{
	// Re-gather so a reopened browser sees fresh recent-quest ranks and any
	// newly imported files.
	quest_browser::persist();
	quest_browser::invalidate();

	while (true)
	{
		QuestBrowserDialog dlg;
		dlg.show();

		switch(dlg.res)
		{
		case QuestBrowserDialog::result::NEW_QUEST:
			if (call_tileset_wizard())
			{
				// A fresh quest must not inherit the previous quest's
				// filename, or a quick Save would overwrite it.
				filepath[0] = temppath[0] = 0;
				first_save = false;
				quest_browser::persist();
				return true;
			}
			break; // wizard cancelled; back to the browser

		case QuestBrowserDialog::result::OPEN_PATH:
			if (open_quest(dlg.chosen_path.c_str()) == qe_OK)
			{
				quest_browser::persist();
				return true;
			}
			break; // load failed or password prompt cancelled; back to the browser

		case QuestBrowserDialog::result::QUIT:
		default:
			quest_browser::persist();
			return false;
		}
	}
}

bool quest_browser_open()
{
	return run_quest_browser();
}
