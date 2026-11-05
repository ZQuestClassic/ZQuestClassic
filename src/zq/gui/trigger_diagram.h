#ifndef ZQ_GUI_TRIGGER_DIAGRAM_H_
#define ZQ_GUI_TRIGGER_DIAGRAM_H_

#include "core/trigger_graph.h"
#include "gui/dialog_ref.h"
#include "gui/widget.h"
#include <array>
#include <functional>
#include <string>
#include <vector>

class RenderTreeItem;
struct ALLEGRO_BITMAP;

int32_t trigger_diagram_proc(int32_t msg, DIALOG* d, int32_t c);

namespace GUI
{

// Draws a screen's trigger_graph the way the combo triggers guide draws its diagrams, next to
// a picture of the screen (or its scrolling region) and a list of the combos. Each combo is a
// card listing its triggers' causes (purple), conditions (amber) and effects (teal); combo
// changes are arrows between cards, and the copycats, trigger groups, states and secrets that
// link triggers are boxes on the left. Hovering a card or a row of the list outlines where its
// combo is placed, hovering the picture picks out the cards placed there, clicking a row or the
// picture scrolls to the card, and clicking a card calls the click function with its combo and
// the trigger clicked.
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
		// Where the combo is placed, in the picture's pixels.
		std::vector<Rect> placements;
		// The CSet it's pictured in, for "Use Combo for Drawing".
		int cset = 0;
	};

	// The screens the graph is of, side by side as they are on the map.
	struct Picture
	{
		// The widget takes ownership.
		ALLEGRO_BITMAP* bmp = nullptr;
		// The screen at its top-left, and how many screens across and down it is.
		int origin = 0, cols = 1, rows = 1;
		// The dialog's screen, outlined when there's more than one.
		int screen = 0;
		// Only `screen`'s combos are cards: the other screens are dimmed, and clicking one calls
		// the pick screen function.
		bool single = true;
	};

	// The picture's zoom, and the point of it at the top-left of its box. A zoom of 0 is unset.
	struct PictureView
	{
		float zoom = 0, x = 0, y = 0;
	};

	// Expanded, the diagram fills the program window, without the picture and list.
	explicit TriggerDiagram(bool expanded = false);
	~TriggerDiagram();

	// The widget takes ownership of the picture and the icons.
	void setGraph(trigger_graph graph, std::vector<CardInfo> cards, Picture picture);
	// `trigger` is the index of the trigger clicked in the card, or -1 for the card's title.
	void setOnClickCard(std::function<void(int cid, int trigger)> func)
	{
		onClickCard = std::move(func);
	}
	void setOnPickScreen(std::function<void(int screen)> func)
	{
		onPickScreen = std::move(func);
	}
	// What the diagram says when the graph has no cards, a line each.
	void setEmptyText(std::vector<std::string> lines)
	{
		emptyText = std::move(lines);
		markDirty();
	}
	// For a hub's note about the combos on the region's other screens (trigger_graph::elsewhere).
	void setOnShowElsewhere(std::function<void(size_t hub)> func)
	{
		onShowElsewhere = std::move(func);
	}
	// Fades everything but `hub` and what links to it, as "Show Only What Links Here" does.
	void showOnlyHub(int hub)
	{
		filterHub = hub;
		markDirty();
	}
	PictureView getPictureView() const { return pictureView; }
	void setPictureView(PictureView view);
	// Pans the picture to the dialog's screen, keeping the zoom.
	void showPictureScreen();
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
	Picture picture;
	PictureView pictureView;

	std::vector<CardLayout> cardLayouts;
	std::vector<Rect> hubRects;
	std::vector<std::vector<std::string>> hubLines;
	// Each hub's note about the region's other screens, and where it is in the hub's box.
	std::vector<std::vector<std::string>> hubNotes;
	std::vector<Rect> hubNoteRects;
	std::vector<Path> paths;
	std::vector<std::string> emptyText;
	float contentW = 0, contentH = 0;
	// The list's rows: each card, and whether it's indented under the card before it, being
	// later in a chain of combo changes.
	std::vector<std::pair<size_t, bool>> listRows;
	float listScroll = 0;
	// The card the last click on the picture went to.
	int previewFocus = -1;
	// The card last scrolled to from the list or the picture, which stays highlighted.
	int focusCard = -1;
	// With "Show Only What Links Here", the hub whose cards and lines are shown; the rest fade.
	int filterHub = -1;

	const bool expanded;
	float scrollX = 0, scrollY = 0;
	// mouse_w when last checked, to see the horizontal wheel turn.
	int lastMouseW = 0;
	int hoverCard = -1, hoverHub = -1;
	// The point of the picture under the mouse, or -1.
	int hoverPicX = -1, hoverPicY = -1;
	// Whether the mouse is over the picture's box, for the hint about zooming and panning.
	bool hoverPreview = false;
	std::function<void(int cid, int trigger)> onClickCard;
	std::function<void(int screen)> onPickScreen;
	std::function<void(size_t hub)> onShowElsewhere;
	// What the next geCLICK event calls, as the proc can't rebuild the dialog it's in.
	enum class Action { none, edit, pick_screen, show_elsewhere };
	Action action = Action::none;
	int clickedCid = -1, clickedTrigger = -1, pickedScreen = -1, elsewhereHub = -1;

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
	// The point of the picture at a point of the widget.
	std::pair<float, float> toPicture(float mx, float my) const;
	// The screen of the picture at a point of the widget, or -1.
	int pictureScreenAt(int mx, int my) const;
	// The picture's width and height, in its own pixels.
	std::pair<float, float> pictureSize() const;
	// The zoom that fits the whole picture in its box.
	float minPictureZoom() const;
	// Whether some of the picture is out of view past each side of its box: left, top, right
	// and bottom.
	std::array<bool, 4> pictureOverflow() const;
	// The small map of the whole picture in the corner of its box, shown while some of the
	// picture is out of view. Empty when there's none.
	Rect mapRect() const;
	// Pans the picture to centre the point of the map under the mouse, until the button is
	// released.
	void dragMap();
	void clampPicture();
	void zoomPicture(float factor, int mx, int my);
	// Pans the picture to show where a card is placed, if it's out of view.
	void showPicturePlacement(size_t card);
	// A card placed at a point of the picture, or -1. Each call at a spot with several returns
	// the next of them.
	int previewCardAt(int mx, int my);
	// The hub whose note about the region's other screens is at a point, or -1.
	int hubNoteAt(int mx, int my) const;
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
