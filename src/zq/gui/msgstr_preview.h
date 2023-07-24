#ifndef ZC_GUI_MSGSTR_PREVIEW_H
#define ZC_GUI_MSGSTR_PREVIEW_H

#include "base/zdefs.h"
#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <string>

namespace GUI
{

class MsgPreview: public Widget
{
public:
	MsgPreview();
	
	void setText(std::string newText);
	void setIndex(int32_t ind);
	void setData(MsgStr const* data);
	
	std::string const& getText()
	{
		return text;
	}
	
	int32_t getIndex() const
	{
		return index;
	}
	
	MsgStr const* getData() const
	{
		return str_data;
	}
private:
	std::string text;
	DialogRef alDialog;
	MsgStr const* str_data;
	int32_t index;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
