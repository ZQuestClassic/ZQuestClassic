#ifndef ZC_DIALOG_COMPILEZSCRIPT_H
#define ZC_DIALOG_COMPILEZSCRIPT_H

#include <gui/dialog.h>
#include "gui/radioset.h"
#include "gui/label.h"
#include "gui/window.h"
#include <functional>
#include <string_view>

int32_t onCompileScript();
bool do_compile_and_slots(int assign_mode, bool delay);

class CompileZScriptDialog: public GUI::Dialog<CompileZScriptDialog>
{
public:
	enum class message { REFR_INFO, LOAD, EXPORT, EDIT, DOCS, STD_ZH, COMPILE, CANCEL, SETTINGS, SLOTS };

	CompileZScriptDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
	void updateLabels();
	
private:
	std::shared_ptr<GUI::Label> labels[2];
	std::shared_ptr<GUI::Window> window;
};

#endif
