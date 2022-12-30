#ifndef ZC_DIALOG_VECTORPICK_H
#define ZC_DIALOG_VECTORPICK_H

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/window.h>
#include <functional>

void call_edit_vector(std::vector<int32_t>& vec, bool zsint = false,
	size_t min = 0, size_t max = 0);

class VectorPickDialog: public GUI::Dialog<VectorPickDialog>
{
public:
	enum class message
	{
		OK, CANCEL, RELOAD
	};

	VectorPickDialog(std::vector<int32_t>& vec, bool zsint, size_t min, size_t max);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::vector<int32_t> local_vec;
	std::vector<int32_t>& dest_vec;
	size_t min,max;
	bool zsint;
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Button> pastebtn;
};

#endif
