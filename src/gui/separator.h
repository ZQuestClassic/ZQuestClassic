#pragma once

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include <string>
#include <utility>

namespace GUI
{

class Separator: public Widget
{
public:
	Separator(bool vertical = false);

	void setColor1(byte c);
	void setColor2(byte c);
	byte getColor1() const;
	byte getColor2() const;
	
private:
	DialogRef alDialog;
	bool vertical;
	byte c1, c2;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	
	friend int32_t separator_proc(int32_t msg, DIALOG* d, int32_t);
};

}
