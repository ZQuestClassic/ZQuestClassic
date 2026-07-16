#ifndef ZC_GUI_MSGSTR_PREVIEW_H_
#define ZC_GUI_MSGSTR_PREVIEW_H_

#include "core/msgstr.h"
#include "core/msg_scroll.h"
#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <string>

namespace GUI
{

class MsgPreview: public Widget
{
public:
	MsgPreview();
	virtual ~MsgPreview();
	
	void setIndex(int32_t ind);
	void setData(MsgStr const* data);
	
	int32_t getIndex() const
	{
		return index;
	}
	
	MsgStr const* getData() const
	{
		return str_data;
	}
private:
	DialogRef alDialog;
	MsgStr const* str_data;
	int32_t index;
	BITMAP *bg_buf, *fg_buf;
	
	MsgScrollState scroll;
	int fg_bmp_height;
	int16_t msg_margins[4];

	int tmp_scroll;
	bool is_tmp_scrolling;

	void update_string();
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH) override;
	void realize(DialogRunner& runner) override;
	friend int32_t d_newmsg_preview_proc(int32_t msg,DIALOG *d,int32_t);
};

}

#endif
