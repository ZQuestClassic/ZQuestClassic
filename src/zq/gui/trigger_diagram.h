#ifndef ZQ_GUI_TRIGGER_DIAGRAM_H_
#define ZQ_GUI_TRIGGER_DIAGRAM_H_

#include "core/trigger_graph.h"
#include "gui/dialog_ref.h"
#include "gui/widget.h"
#include <functional>
#include <string>
#include <vector>

class RenderTreeItem;
struct ALLEGRO_BITMAP;

int32_t trigger_diagram_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{

// Draws a screen's trigger_graph the way the combo triggers guide draws its diagrams, next to
// a picture of the screen and a list of the combos. Each combo is a card listing its triggers'
// causes (purple), conditions (amber) and effects (teal); combo changes are arrows between
// cards, and the copycats, trigger groups, states and secrets that link triggers are boxes on
// the left. Hovering a card or a row of the list outlines where its combo is placed, hovering
// the screen picks out the cards placed there, clicking a row or the screen scrolls to the
// card, and clicking a card calls the click function with its combo and the trigger clicked.
class TriggerDiagram: public Widget
{
public:
	enum class LineStyle { solid, dashed, dotted };

	struct Rect
	{
		float x, y, w, h;
		bool contains(float px, float py) const
		{
			return px >= x && py >= y && px < x + w && py < y + h;
		}
	};

	// What the dialog gives the widget for each card of the graph.
	struct CardInfo
	{
		// A 16x16 picture of the combo. The widget takes ownership.
		ALLEGRO_BITMAP* icon;
		std::string title, subtitle;
		// Where the combo is placed, in screen pixels.
		std::vector<Rect> placements;
		// The CSet it's pictured in, for "Use Combo for Drawing".
		int cset = 0;
	};

	// Expanded, the diagram fills the program window, without the screen picture and list.
	explicit TriggerDiagram(bool expanded = false);
	~TriggerDiagram();

	// `screen_bmp` is a 256x176 picture of the screen. The widget takes ownership of it and
	// of the icons.
	void setGraph(trigger_graph graph, std::vector<CardInfo> cards, ALLEGRO_BITMAP* screen_bmp);
	// `trigger` is the index of the trigger clicked in the card, or -1 for the card's title.
	void setOnClickCard(std::function<void(int cid, int trigger)> func)
	{
		onClickCard = std::move(func);
	}
	float getScrollX() const { return scrollX; }
	float getScrollY() const { return scrollY; }
	void setScroll(float x, float y)
	{
		scrollX = x;
		scrollY = y;
	}

private:
	enum class ChipKind { plain, cause, condition, effect };
	struct Chip
	{
		Rect rect;
		ChipKind kind;
		std::vector<std::string> lines;
		bool keeps_firing;
	};
	struct CardLayout
	{
		Rect rect;
		std::vector<Chip> chips;
		// Small arrows between a trigger's causes, conditions and effects.
		std::vector<std::pair<float, float>> arrows;
		std::vector<float> dividers;
		std::vector<std::pair<float, std::vector<std::string>>> labels;
		// Where each trigger starts and ends, relative to the card's top.
		std::vector<std::pair<float, float>> triggerSpans;
	};
	struct Path
	{
		std::vector<std::pair<float, float>> points;
		LineStyle style;
		bool arrow_start, arrow_end;
		int card, hub;
	};

	trigger_graph graph;
	std::vector<CardInfo> cards;
	ALLEGRO_BITMAP* screenBmp = nullptr;

	std::vector<CardLayout> cardLayouts;
	std::vector<Rect> hubRects;
	std::vector<std::vector<std::string>> hubLines;
	std::vector<Path> paths;
	float contentW = 0, contentH = 0;
	// The list's rows: each card, and whether it's indented under the card before it, being
	// later in a chain of combo changes.
	std::vector<std::pair<size_t, bool>> listRows;
	float listScroll = 0;
	// The card the last click on the screen picture went to.
	int previewFocus = -1;
	// The card last scrolled to from the list or the screen picture, which stays highlighted.
	int focusCard = -1;
	// With "Show Only What Links Here", the hub whose cards and lines are shown; the rest fade.
	int filterHub = -1;

	const bool expanded;
	float scrollX = 0, scrollY = 0;
	// mouse_w when last checked, to see the horizontal wheel turn.
	int lastMouseW = 0;
	int hoverCard = -1, hoverHub = -1, hoverPos = -1;
	std::function<void(int cid, int trigger)> onClickCard;
	int clickedCid = -1, clickedTrigger = -1;

	DialogRef alDialog;
	RenderTreeItem* rti = nullptr;

	void freeBitmaps();
	void layout();
	// Where the widget is: its DIALOG once realized.
	Rect bounds() const;
	Rect previewRect() const;
	Rect viewRect() const;
	Rect listRect() const;
	// The card of the list row at a point, or -1.
	int listCardAt(int mx, int my) const;
	// A card placed at a point of the screen picture, or -1. Each call at a spot with several
	// returns the next of them.
	int previewCardAt(int mx, int my);
	void clampScroll();
	void scrollToCard(size_t card);
	// The card or hub under the mouse, in the diagram's coordinates.
	void updateHover(int mx, int my);
	bool cardHighlighted(int card) const;
	// Whether a card is shown in full with the filter (filterHub).
	bool linksFilter(int card) const;
	// The trigger of a card at a height in the diagram, or -1 for the card's title.
	int triggerAt(int card, float diagram_y) const;
	// Pops the right-click menu. Returns true if an "Edit" item was picked, with clickedCid
	// and clickedTrigger set for the click event.
	bool contextMenu(int mx, int my);
	void copyJson() const;
	void copyTrigger(int card, int trigger) const;
	void savePng();
	// The whole diagram and its legend, as a PNG.
	void exportPng(std::string const& path);
	void render();
	// The cards, hubs and lines, with the diagram's origin at (ox, oy). Cards outside `clip`
	// are skipped.
	void drawDiagram(float ox, float oy, Rect clip);
	void markDirty();

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	friend int32_t (::trigger_diagram_proc)(int32_t msg, DIALOG* d, int32_t c);
};

}

#endif
