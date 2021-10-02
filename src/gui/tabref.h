#ifndef ZC_GUI_TABREF_H
#define ZC_GUI_TABREF_H

#include "widget.h"
#include <vector>

namespace GUI
{

/* A container that stores a single tab's worth of info
 * Stores a name string and a content widget
 */
class TabRef: public Widget
{
public:
	TabRef();

	/* Sets the widget that will appear in the window. */
	void setContent(std::shared_ptr<Widget> newContent) noexcept;
	
	std::string const& getName() const {return tabname;}
	void setName(std::string name) {tabname = name;}

private:
	std::shared_ptr<Widget> content;
	std::string tabname;

	/* Sets or unsets D_HIDDEN for each of the child's DIALOGs. */
	void applyVisibility(bool visible) override;
	void calculateSize() override;
	void arrange(int contX, int contY, int contW, int contH) override;
	void realize(DialogRunner& runner) override;
	void drawTab();
	friend class TabPanel;
	friend class QRPanel;
};

}

#endif
