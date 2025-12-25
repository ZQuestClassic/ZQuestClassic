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
		message(-1), value(0), min_value(0), max_value(0), step(1),
		handle_width(7_px), bar_width(3_px)
	{}
	
	static int calc_d1(int max_value, int min_value, int step)
	{
		if ((max_value - min_value) % step == 0)
			return (max_value - min_value) / step;
		return (zfix(max_value - min_value) / step).getRound();
	}
	
	static int calc_d2(int value, int min_value, int step)
	{
		if ((value - min_value) % step == 0)
			return (value - min_value) / step;
		return (zfix(value - min_value) / step).getRound();
	}

	void Slider::setValue(int32_t new_value)
	{
		new_value = vbound(new_value, min_value, max_value);
		if (new_value != value)
			pendDraw();
		if (alDialog)
			alDialog->d2 = calc_d2(new_value, min_value, step);
		value = new_value;
	}
	int32_t Slider::getValue() const
	{
		int32_t ret = value;
		if (alDialog)
			ret = (alDialog->d2 + min_value) * step;
		ret = vbound(ret, min_value, max_value);
		return ret;
	}

	void Slider::setMinValue(int32_t new_value)
	{
		if (new_value != min_value)
			pendDraw();
		if (alDialog)
		{
			alDialog->d1 = calc_d1(max_value, new_value, step);
			alDialog->d2 = calc_d2(getValue(), new_value, step);
		}
		min_value = new_value;
	}
	int32_t Slider::getMinValue() const
	{
		return min_value;
	}

	void Slider::setMaxValue(int32_t new_value)
	{
		if (new_value != max_value)
			pendDraw();
		if (alDialog)
			alDialog->d1 = calc_d1(new_value, min_value, step);
		max_value = new_value;
	}
	int32_t Slider::getMaxValue() const
	{
		return max_value;
	}

	void Slider::setStep(int32_t new_step)
	{
		if (new_step < 1) new_step = 1; // avoid div by 0
		if (new_step != step)
			pendDraw();
		if (alDialog)
		{
			alDialog->d1 = calc_d1(max_value, min_value, new_step);
			alDialog->d2 = calc_d2(getValue(), min_value, new_step);
		}
		step = new_step;
	}
	int32_t Slider::getStep() const
	{
		return step;
	}

	void Slider::setHandleWidth(Size new_handle_width)
	{
		handle_width = new_handle_width;
		if (alDialog)
			alDialog->fg = new_handle_width;
	}
	int32_t Slider::getHandleWidth() const
	{
		return handle_width;
	}

	void Slider::setBarWidth(Size new_bar_width)
	{
		bar_width = new_bar_width;
		if (alDialog)
			alDialog->fg = new_bar_width;
	}
	int32_t Slider::getBarWidth() const
	{
		return bar_width;
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
			handle_width, bar_width,
			0,
			getFlags(),
			calc_d1(max_value, min_value, step), calc_d2(value, min_value, step), // d1, d2,
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
			onValChangedFunc(getValue());
			pendDraw();
		}
		if (message >= 0)
			sendMessage(message, getValue());
		return ret;
	}
}
