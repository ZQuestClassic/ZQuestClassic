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
#include <optional>

namespace fs = std::filesystem;

using quest_browser::Entry;

enum { SORT_RECENT, SORT_EDITED, SORT_ZCVER };

// The filter's best fuzzy score against the entry's title, author, and
// filename, or empty if none of them match.
static std::optional<int> filter_score(Entry const& e, std::string const& filter)
{
	// Only the filename, not the whole path: a fuzzy match needs the letters in order, not
	// adjacent, and a long path (shared by every entry) has many short patterns in it.
	std::string const filename = fs::path(e.path).filename().string();
	std::optional<int> best;
	for (std::string const* field : {&e.title, &e.author, &filename})
	{
		auto score = util::fuzzy_match_score(filter, *field);
		if (score && (!best || *score > *best))
			best = score;
	}
	return best;
}

QuestBrowserDialog::QuestBrowserDialog()
{
	sortMode = std::clamp(zc_get_config(quest_browser::CFG_SECTION, "sort_mode", SORT_RECENT),
		(int32_t)SORT_RECENT, (int32_t)SORT_ZCVER);
	if (!quest_browser::gathered())
		quest_browser::gather();
	quest_browser::start_update_check();
}

void QuestBrowserDialog::refreshRows()
{
	auto const& entries = quest_browser::entries();

	std::vector<size_t> order;
	std::vector<int> scores(entries.size());
	for (size_t i = 0; i < entries.size(); i++)
	{
		if (auto score = filter_score(entries[i], filter))
		{
			order.push_back(i);
			scores[i] = *score;
		}
	}

	std::stable_sort(order.begin(), order.end(), [&](size_t ai, size_t bi) {
		// While filtering, the best matches come first; the sort mode
		// only breaks ties.
		if (!filter.empty() && scores[ai] != scores[bi])
			return scores[ai] > scores[bi];
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
		? "No quests found - use \"Browse for File\" or \"Scan Folder\" to add some."
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

	updateSelection();
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

	if (!quest_browser::update_checked())
	{
		if (!quest_browser::poll_update_check())
			return false; // still waiting on the network; keep the dialog responsive
		std::string new_version = quest_browser::known_update();
		if (!new_version.empty())
		{
			if (updateSlot)
				showUpdate(new_version);
			else
			{
				// The footer was laid out without room for the label (the
				// check hadn't run yet). Rebuild the view; it now knows.
				if (auto const* sel = questList->getSelectedRow())
					rerunSelectPath = sel->path;
				rerunScroll = questList->getScrollIndex();
				refresh_dlg();
			}
			return true;
		}
	}

	return false;
}

void QuestBrowserDialog::showUpdate(std::string const& version)
{
	updateVersion = version;
	updateLabel->setText("Update available");
	updateLabel->setTooltip(fmt::format("New version available: {} (click to open release notes)", version));
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

void QuestBrowserDialog::updateSelection()
{
	if (!pathLabel)
		return;

	auto const* row = questList->getSelectedRow();
	std::string path = row ? row->path : "";
	// Keep the tail when truncating - it's the informative part.
	if (path.size() > pathLabelLen && pathLabelLen > 3)
		path = "..." + path.substr(path.size() - (pathLabelLen - 3));
	pathLabel->setText(fmt::format("{:<{}}", path, pathLabelLen));

	if (openButton)
		openButton->setDisabled(!row);
}

std::shared_ptr<GUI::Widget> QuestBrowserDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	// Size from the logical screen, not `screen`, which is null during a
	// refresh_dlg() rebuild.
	int32_t sw = zq_screen_w;
	// The list sets the dialog's width; every other row aligns to its edges.
	int32_t list_w = sw - 110;
	// Path label capacity, in characters, roughly the list's width.
	pathLabelLen = std::max(40, (list_w - 20) / 7);

	// Grids never stretch cells, so a row that should span the list's width
	// gets a spacer sized to the leftover: measure the row's other widgets,
	// then insert a blank label covering the difference. Rows get no padding
	// of their own so their first and last widgets line up with the list.
	// If GUI::Grid ever learns a stretch primitive, this measurement (and
	// the launcher's space-padded-label equivalent) could be deleted.
	int32_t row_w = list_w + 2 * DEFAULT_PADDING_INT; // the list's total width
	auto spacer_for = [&](std::vector<std::shared_ptr<GUI::Widget>> const& widgets)
	{
		int32_t used = 0;
		for (auto const& w : widgets)
		{
			w->calculateSize();
			used += w->getTotalWidth();
		}
		return Label(text = "", hPadding = 0_px,
			width = GUI::Size::pixels(std::max(0, row_w - used)));
	};

	static const GUI::ListData sortList
	{
		{ "Recently Opened", SORT_RECENT },
		{ "Last Edited", SORT_EDITED },
		{ "ZC Version", SORT_ZCVER }
	};

	// Top row: actions that open a quest on the left, filter and sort on
	// the right. Keep "open" out of the other button names so the only
	// button called Open is the one that opens the selection.
	std::shared_ptr<GUI::Widget> newBtn = Button(text = "&New Quest",
		onClick = message::NEW_QUEST,
		tooltip = "Create a new quest from a tileset");
	std::shared_ptr<GUI::Widget> browseBtn = Button(text = "&Browse for File...",
		onClick = message::BROWSE_FILE,
		tooltip = "Pick a quest file to open.\nIt is also added to this list.");
	std::shared_ptr<GUI::Widget> filterLabel = Label(text = "Filter:");
	std::shared_ptr<GUI::Widget> filterField = TextField(
		type = GUI::TextField::type::TEXT,
		text = filter, // survives a refresh_dlg() rebuild
		maxLength = 64,
		width = 120_px,
		focused = true, // type right away to filter
		tooltip = "Filter by title, author, or filename.\nLetters only need to appear in order;\nthe best matches sort first.",
		onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
		{
			filter = std::string(text);
			refreshRows();
		});
	std::shared_ptr<GUI::Widget> sortLabel = Label(text = "Sort:");
	std::shared_ptr<GUI::Widget> sortDrop = DropDownList(data = sortList,
		selectedValue = sortMode,
		onSelectFunc = [&](int32_t val)
		{
			sortMode = val;
			zc_set_config(quest_browser::CFG_SECTION, "sort_mode", val);
			refreshRows();
		});

	// Open (the default button) sits at the right edge; Scan Folder, which
	// only manages the list, sits at the left as a secondary action.
	std::shared_ptr<GUI::Widget> scanBtn = Button(text = "Scan &Folder...",
		onClick = message::SCAN_FOLDER,
		tooltip = "Add every quest in a folder to this list");
	openButton = Button(text = "&Open", minwidth = 70_px,
		onClick = message::PICK,
		tooltip = "Open the selected quest\n(or press Enter / double-click it)");

	// The footer holds the auto-open checkbox (left) and, at the right
	// edge, the current version followed by the update notice once the
	// background check finds one.
	std::shared_ptr<GUI::Widget> autoOpen = Checkbox(
		text = "Automatically open most recent quest",
		checked = OpenLastQuest != 0,
		// The checkbox draws its box 2px left of its origin; nudge it over
		// so the box lines up with the list's edge.
		leftPadding = GUI::Size::pixels(DEFAULT_PADDING_INT + 2),
		tooltip = "On startup, skip this window and open\nthe last quest you had open",
		onToggleFunc = [&](bool state)
		{
			OpenLastQuest = state ? 1 : 0;
			zc_set_config("zquest", "open_last_quest", OpenLastQuest);
		});
	std::string version_text = fmt::format("v{}", getVersionString());
	std::shared_ptr<GUI::Widget> versionLabel = Label(text = version_text,
		tooltip = "Open this version's release notes",
		onPressFunc = []()
		{
			util::open_web_link(release_page_url(getVersionString()));
		});
	// "Update available" sits right of the version link once the update
	// check finds something. The row is laid out once, so it only gets a
	// cell when the check has already found an update; if the check (which
	// runs in the background from the first show) finishes later and finds
	// one, pumpScan rebuilds the view via refresh_dlg().
	updateVersion = quest_browser::known_update();
	updateSlot = !updateVersion.empty();
	updateLabel = Label(text = "Update available", maxLines = 1,
		textColor = vc(14),
		onPressFunc = [&]()
		{
			if (!updateVersion.empty())
				util::open_web_link(release_page_url(updateVersion));
		});
	if (updateSlot)
		showUpdate(updateVersion);
	else
	{
		updateLabel->overrideWidth(0_px);
		updateLabel->setHPadding(0_px);
		updateLabel->setText("");
	}

	std::shared_ptr<GUI::Window> window = Window(
		title = "Quests",
		onClose = message::QUIT,
		onEnter = message::PICK, // Open is the default button
		Column(
			Row(
				hPadding = 0_px,
				newBtn,
				browseBtn,
				spacer_for({newBtn, browseBtn, filterLabel, filterField, sortLabel, sortDrop}),
				filterLabel,
				filterField,
				sortLabel,
				sortDrop
			),
			questList = QuestListView(
				width = GUI::Size::pixels(list_w),
				onDClick = message::PICK,
				onSelectFunc = [&](int32_t)
				{
					updateSelection();
				}
			),
			pathLabel = Label(text = std::string(pathLabelLen, ' '), hAlign = 0.0),
			Row(
				hPadding = 0_px,
				topMargin = 16_px,
				scanBtn,
				spacer_for({scanBtn, openButton}),
				openButton
			),
			Row(
				hPadding = 0_px,
				autoOpen,
				spacer_for({autoOpen, versionLabel, updateLabel}),
				versionLabel,
				updateLabel
			)
		)
	);

	questList->setOnIdleFunc([this]() { return pumpScan(); });
	refreshRows();

	// After a refresh_dlg() rebuild, put the list back where it was.
	if (!rerunSelectPath.empty())
	{
		auto const& rows = questList->getRows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			if (rows[i].path == rerunSelectPath)
			{
				questList->setSelectedIndex((int32_t)i, false);
				break;
			}
		}
		questList->setScrollIndex(rerunScroll);
		rerunSelectPath.clear();
		updateSelection();
	}
	return window;
}

bool QuestBrowserDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
	case message::NEW_QUEST:
		res = result::NEW_QUEST;
		return true;

	case message::BROWSE_FILE:
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
