#ifndef ZC_GUI_SLIDER_H_
#define ZC_GUI_SLIDER_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"

namespace GUI
{

	class Slider : public Widget
	{
	public:
		Slider();

		void setValue(int32_t new_value);
		int32_t getValue() const;

		void setMinValue(int32_t new_value);
		int32_t getMinValue() const;

		void setMaxValue(int32_t new_value);
		int32_t getMaxValue() const;

		void setStep(int32_t new_step);
		int32_t getStep() const;

		void setHandleWidth(Size new_handle_width);
		int32_t getHandleWidth() const;
		
		void setBarWidth(Size new_bar_width);
		int32_t getBarWidth() const;

		int32_t getTotalRange() const
		{
			return getMaxValue() - getMinValue();
		}

		template<typename T>
		RequireMessage<T> onValueChanged(T m)
		{
			message = static_cast<int32_t>(m);
		}

		void setOnValChanged(std::function<void(int32_t)> newfunc)
		{
			onValChangedFunc = std::move(newfunc);
		}

	private:
		int32_t message;
		std::function<void(int32_t)> onValChangedFunc;

		int32_t value, min_value, max_value, step;
		Size handle_width, bar_width;

		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;
		void calculateSize() override;

		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	};

}

#endif
