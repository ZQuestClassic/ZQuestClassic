#ifndef ZC_DIALOG_SCREEN_TRIGGERS_H_
#define ZC_DIALOG_SCREEN_TRIGGERS_H_

#include "gui/dialog.h"
#include "zq/gui/trigger_diagram.h"
#include <memory>

// Screen > Combo Triggers: a diagram of how the current screen's combo triggers link up.
void call_screen_triggers_dialog();

class ScreenTriggersDialog: public GUI::Dialog<ScreenTriggersDialog>
{
public:
	enum class message { OK };

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TriggerDiagram> diagram;
	bool expanded = false;

	// Rebuilds the diagram from the screen, after a combo was edited.
	void refresh();
};

#endif
