#include "slider.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include <cassert>
#include <utility>

using namespace GUI;

namespace GUI
{
	Slider::Slider() :
		message(-1), offset(0), max_offset(0)
	{
		
	}

	void Slider::setOffset(int32_t newoffset)
	{
		if (newoffset != offset)
			pendDraw();
		offset = newoffset;
		if (alDialog)
		{
			alDialog->d2 = newoffset - getMinOffset();
		}
	}
	int32_t Slider::getOffset() const
	{
		int32_t ret = offset;
		if (alDialog)
			ret = alDialog->d2 + getMinOffset();
		ret = vbound(ret, getMinOffset(), getMaxOffset());
		return ret;
	}

	void Slider::setMinOffset(int32_t newoffset)
	{
		if (newoffset != min_offset)
			pendDraw();
		int diff = min_offset - newoffset;
		min_offset = newoffset;
		if (alDialog)
		{
			alDialog->d1 = getMaxOffset() - newoffset;
			alDialog->d2 += diff;
		}
	}
	int32_t Slider::getMinOffset() const
	{
		return min_offset;
	}

	void Slider::setMaxOffset(int32_t newoffset)
	{
		if (newoffset != max_offset)
			pendDraw();
		max_offset = newoffset;
		if (alDialog)
		{
			alDialog->d1 = newoffset - getMinOffset();
		}
	}
	int32_t Slider::getMaxOffset() const
	{
		if (alDialog)
			return alDialog->d1;
		return max_offset;
	}

	void Slider::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void Slider::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void Slider::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<jwin_slider_proc>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			max_offset - min_offset, offset - min_offset, // d1, d2,
			NULL, NULL, NULL // dp, dp2, dp3
			});
	}

	void Slider::calculateSize()
	{
		setPreferredHeight(16_px);
		setPreferredWidth(160_px);
		Widget::calculateSize();
	}

	int32_t Slider::onEvent(int32_t event, MessageDispatcher& sendMessage)
	{
		assert(event == geCHANGE_VALUE);
		int ret = -1;
		if (onValChangedFunc)
		{
			onValChangedFunc(getOffset());
			pendDraw();
		}
		if (message >= 0)
			sendMessage(message, getOffset());
		return ret;
	}
}
