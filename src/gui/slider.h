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

		void setOffset(int32_t newoffset);
		int32_t getOffset() const;

		void setMinOffset(int32_t newoffset);
		int32_t getMinOffset() const;

		void setMaxOffset(int32_t newoffset);
		int32_t getMaxOffset() const;

		int32_t getTotalRange() const
		{
			return getMaxOffset() - getMinOffset();
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

		int32_t offset, min_offset, max_offset;

		DialogRef alDialog;

		void applyVisibility(bool visible) override;
		void applyDisabled(bool dis) override;
		void realize(DialogRunner& runner) override;
		void calculateSize() override;

		int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	};

}

#endif
