#ifndef ZC_GUI_RADIO_H
#define ZC_GUI_RADIO_H

#include "widget.h"
#include "dialog_ref.h"

namespace GUI
{

class Radio: public Widget
{
public:
	Radio();

	/* Sets the text to appear next to the radio. */
	void setText(std::string newText);

	/* Sets whether the radio is checked or not. */
	void setChecked(bool value);

	/* Returns true if the radio is checked. */
	bool getChecked();

	void setProcSet(int32_t newProcSet);
	
	int32_t getProcSet() const {return procset;}
	
	void setIndex(size_t newIndex);
	
	size_t getIndex() const {return index;}

	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int32_t>(m);
	}
protected:
	int32_t message;
private:
	bool checked;
	std::string text;
	int32_t procset;
	size_t index;
	DialogRef alDialog;

	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont_a5(ALLEGRO_FONT* newFont) override;
};

}

#endif
