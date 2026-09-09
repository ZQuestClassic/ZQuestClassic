#ifndef ZC_DIALOG_QUEST_BROWSER_H_
#define ZC_DIALOG_QUEST_BROWSER_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/window.h>
#include <zq/gui/quest_list_view.h>
#include <memory>
#include <string>

// Shows the quest browser and runs it until the editor has a quest loaded
// (returns true) or the user closes it (returns false).
bool quest_browser_open();

// Records that a quest file was opened or saved with the editor fully loaded,
// so the browser's metadata cache can be cheaply refreshed for this entry.
void quest_browser_record_loaded_quest(char const* path);

class QuestBrowserDialog: public GUI::Dialog<QuestBrowserDialog>
{
public:
	enum class message
	{
		NEW_QUEST, BROWSE_FILE, SCAN_FOLDER, PICK, QUIT
	};
	enum class result { QUIT, NEW_QUEST, OPEN_PATH };

	QuestBrowserDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

	// The browser holds no work; closing it should never warn.
	bool hasUnsavedChanges() const override { return false; }

	result res = result::QUIT;
	std::string chosen_path;

private:
	std::shared_ptr<GUI::QuestListView> questList;
	std::shared_ptr<GUI::Label> updateLabel;
	std::shared_ptr<GUI::Label> pathLabel;
	std::shared_ptr<GUI::Button> openButton;
	std::string filter;
	// Version from the update check; empty until one is found. Clicking the
	// "Update available" label opens this version's release page.
	std::string updateVersion;
	// Whether the footer was laid out with room for the update label.
	bool updateSlot = false;
	// Selection and scroll to restore after refresh_dlg() rebuilds the view.
	std::string rerunSelectPath;
	int32_t rerunScroll = 0;
	int32_t sortMode;
	size_t pathLabelLen = 80;
	// Maps list row index -> index into the entry table.
	std::vector<size_t> rowEntries;

	void refreshRows();
	// Shows the selected row's path below the list, and enables the Open
	// button only while a row is selected.
	void updateSelection();
	// Fills in the "Update available" label for the given newer version.
	void showUpdate(std::string const& version);
	// One unit of scan work per call; returns true if row data changed.
	// This prevents the main thread from getting blocked while we load
	// data about quests to show in the list.
	bool pumpScan();
	bool pickSelected();
};

#endif
