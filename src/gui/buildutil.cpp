#include "buildutil.h"

void TabBuilder::build(std::shared_ptr<GUI::TabPanel>& panel)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if(contents.empty()) return;
	
	for(auto& ref : contents)
		panel->add(TabRef(name = std::move(ref.first), std::move(ref.second)));
	contents.clear();
}
std::shared_ptr<GUI::Widget>& TabBuilder::operator[](std::string const& key)
{
	return contents[key];
}
std::shared_ptr<GUI::Widget>& TabBuilder::operator[](std::string&& key)
{
	return contents[key];
}

