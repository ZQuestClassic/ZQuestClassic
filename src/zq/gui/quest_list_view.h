#ifndef ZC_GUI_QUEST_LIST_VIEW_H_
#define ZC_GUI_QUEST_LIST_VIEW_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <functional>
#include <string>
#include <vector>

class RenderTreeItem;
struct ALLEGRO_BITMAP;

int32_t new_quest_list_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{

struct QuestListRow
{
	// Absolute path. Not drawn in the row; identifies the quest, and the
	// browser shows the selected row's path below the list.
	std::string path;
	std::string title;
	// Second line of the cell, preformatted (e.g. "By Author").
	std::string byline;
	std::string version; // ZC version the quest was made in
	std::string date;    // last edited
	// 16x16 RGBA pixels (1024 bytes), row-major. Empty = default icon.
	std::vector<uint8_t> icon_rgba;
};

// Scrollable listing of quests: one owner-drawn row per quest with an icon and several text
// columns. Selection/scroll live in the DIALOG's d1/d2 like jwin_list_proc.
class QuestListView: public Widget
{
public:
	QuestListView();
	~QuestListView();

	void setRows(std::vector<QuestListRow> rows);
	// Message shown centered when there are no rows.
	void setEmptyText(std::string text)
	{
		emptyText = std::move(text);
	}
	std::vector<QuestListRow> const& getRows() const { return rows; }

	int32_t getSelectedIndex() const;
	void setSelectedIndex(int32_t index, bool scroll_into_view = true);
	// Returns nullptr if the list is empty.
	QuestListRow const* getSelectedRow() const;

	// Called every MSG_IDLE while the dialog is up. Return true if row data was changed and the
	// list should redraw.
	void setOnIdleFunc(std::function<bool()> newFunc)
	{
		onIdleFunc = std::move(newFunc);
	}

	void setOnSelectFunc(GUI::function<void(int32_t)> newFunc)
	{
		onSelectFunc = std::move(newFunc);
	}
	void setOnDClickFunc(GUI::function<void(int32_t)> newFunc)
	{
		onDClickFunc = std::move(newFunc);
	}

	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}
	template<typename T>
	RequireMessage<T> onDClick(T m)
	{
		msg_d = static_cast<int32_t>(m);
	}

	// The range of rows currently on screen: [start, end). Used to
	// prioritize (and limit) background metadata scanning.
	void getVisibleRows(int32_t& start, int32_t& end) const;

	// Used by new_quest_list_proc.
	ALLEGRO_BITMAP* getIconBitmap(int32_t index);
	bool runIdle();
	int32_t rowHeight() const;
	int32_t visibleRowCount() const;
	FONT* titleFont() const;
	FONT* subFont() const;
	std::string const& getEmptyText() const { return emptyText; }
	RenderTreeItem* rti;

private:
	std::string emptyText = "No quests found.";
	std::vector<QuestListRow> rows;
	// Parallel to `rows`; created lazily during draw, owned by this widget.
	std::vector<ALLEGRO_BITMAP*> iconBitmaps;
	int32_t selectedIndex;
	int32_t message, msg_d;
	std::function<bool()> onIdleFunc;
	GUI::function<void(int32_t)> onSelectFunc;
	GUI::function<void(int32_t)> onDClickFunc;
	DialogRef alDialog;

	void destroyIconBitmaps();

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
