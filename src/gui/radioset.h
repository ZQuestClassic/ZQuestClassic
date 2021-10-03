#ifndef ZC_GUI_RADIOSET_H
#define ZC_GUI_RADIOSET_H

#include "widget.h"
#include "radio.h"
#include "grid.h"
#include "list_data.h"
#include <vector>
#include "dialog_ref.h"

namespace GUI
{

/* A container that displays any one of multiple widgets. Used to dynamically
 * change the layout, e.g. to switch between a TextField and a DropDownList
 * when some selection is changed.
 */
class RadioSet: public Widget
{
public:
	RadioSet();
	
	/* Sets whether the radio is checked or not. */
	void setChecked(size_t value);

	/* Returns true if the radio is checked. */
	size_t getChecked();

	void setProcSet(int newProcSet);
	
	int getProcSet() const {return procset;}

	void loadList(GUI::ListData radiolist);

	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int>(m);
	}
private:
	std::vector<std::shared_ptr<Radio>> children;
	std::shared_ptr<Grid> content;
	int message;
	size_t checked;
	int procset;

	/* Sets or unsets D_HIDDEN for each of the child's DIALOGs. */
	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;
};

}

#endif
