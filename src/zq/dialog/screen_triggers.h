#ifndef ZC_DIALOG_SCREEN_TRIGGERS_H_
#define ZC_DIALOG_SCREEN_TRIGGERS_H_

#include "gui/dialog.h"
#include "zq/gui/trigger_diagram.h"
#include <memory>
#include <optional>
#include <vector>

// Screen > Combo Triggers: a diagram of how the current screen's combo triggers link up.
void call_screen_triggers_dialog();

class ScreenTriggersDialog: public GUI::Dialog<ScreenTriggersDialog>
{
public:
	enum class message { OK };

	ScreenTriggersDialog();
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TriggerDiagram> diagram;
	bool expanded = false;
	// The screen the diagram is of. Starts as the editor's; in a scrolling region, clicking
	// another of its screens in the picture moves to it.
	int screen;
	// In a scrolling region, the cards come from all of its screens rather than just `screen`.
	// The checkbox saves it in the config; a hub's note turns it on for this dialog only.
	bool whole_region;
	std::vector<trigger_graph_screen> region;
	// The graph's hubs, to find one again when the dialog is rebuilt.
	std::vector<trigger_graph::hub> hubs;
	// When the dialog is rebuilt for another screen, or the whole region, rather than to expand
	// or shrink.
	bool new_graph = false;
	// The hub to show only what links to, once the dialog is rebuilt.
	std::optional<trigger_graph::hub> show_hub;

	// Rebuilds the diagram from the screen, after a combo was edited.
	void refresh();
};

#endif
