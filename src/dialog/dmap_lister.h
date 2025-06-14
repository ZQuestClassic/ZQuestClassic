#ifndef ZC_DIALOG_DMAPLISTER_H_
#define ZC_DIALOG_DMAPLISTER_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/list.h>
#include <gui/list_data.h>
#include <gui/window.h>
#include <zq/gui/dmap_frame.h>
#include <base/dmap.h>

void call_dmaplisterdialog();

class DMapListerDialog : public GUI::Dialog<DMapListerDialog>
{
public:
	enum class message {
		REFR_INFO, EDIT, DONE, CLEAR, COPY, PASTE
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	DMapListerDialog();

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::DMapFrame> dmapframe;
	std::shared_ptr<GUI::List> dmaplist;
	std::shared_ptr<GUI::Label> l_map, l_level, l_copied;

	dmap old_dmap[512];
	int32_t copied;
	GUI::ListData list_dmaps;	
	bool alphabetized = false;

	friend void call_dmaplisterdialog();
};


#endif