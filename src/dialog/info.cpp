#include "info.h"
#include <gui/builder.h>
#include <utility>

InfoDialog::InfoDialog(std::string title, std::string text):
	dlgTitle(std::move(title)),
	dlgText(std::move(text))
{}

InfoDialog::InfoDialog(std::string title, std::vector<std::string_view> lines):
	dlgTitle(std::move(title)),
	dlgText("")
{
	size_t size=0;

	for(auto& line: lines)
		size+=line.size();
	size+=lines.size()-1;
	dlgText.reserve(size);

	auto remaining=lines.size();
	for(auto& line: lines)
	{
		dlgText+=line;
		remaining--;
		if(remaining>0)
			dlgText+='\n';
	}
}

std::shared_ptr<GUI::Widget> InfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title=std::move(dlgTitle),
		onClose=0,
		Column(
			Label(
				width=20_em,
				hPadding=2_em,
				maxLines=10,
				text=std::move(dlgText)),
			Button(
				text="&Close",
				vPadding=1_em,
				vAlign=1.0,
				onClick=0,
				focused=true)
		)
	);
}

bool InfoDialog::handleMessage(message)
{
	return true;
}
