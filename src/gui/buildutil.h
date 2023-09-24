#ifndef ZC_GUI_BUILDUTIL_H
#define ZC_GUI_BUILDUTIL_H

#include "builder.h"

class TabBuilder
{
public:
	void build(std::shared_ptr<GUI::TabPanel>& panel);
	std::shared_ptr<GUI::Widget>& operator[](std::string const& key);
	std::shared_ptr<GUI::Widget>& operator[](std::string&& key);
private:
	std::map<std::string, std::shared_ptr<GUI::Widget>> contents;
};
#endif

