#include "text_prompt.h"
#include <gui/builder.h>
#include <utility>

static bool ret = false;
optional<string> call_get_text(string const& title, string const& body, string def_text, std::function<bool(string const&, string&)> validate)
{
	ret = false;
	TextPromptDialog(title, body, def_text, validate).show();
	if (ret)
		return def_text;
	return nullopt;
}

TextPromptDialog::TextPromptDialog(string const& title, string const& body,
	string& the_text, std::function<bool(string const&, string&)> validate) :
	d_title(title), d_text(body), error_str(),
	local_str(the_text), dest_str(the_text),
	validate(validate)
{}

std::shared_ptr<GUI::Widget> TextPromptDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Grid> grid = Column();
	if (!d_text.empty())
		grid->add(Label(text = d_text));
	grid->add(TextField(
		type = GUI::TextField::type::TEXT,
		maxLength = 128,
		text = local_str,
		onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
		{
			local_str = sv;
		}));
	if (!error_str.empty())
		grid->add(Label(text = error_str));
	window = Window(
		title = d_title,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		Column(
			grid,
			Row(padding = 0_px,
				Button(
					text = "OK",
					topPadding = 0.5_em,
					onClick = message::OK,
					focused = true),
				Button(
					text = "Cancel",
					topPadding = 0.5_em,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool TextPromptDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			if (validate)
			{
				error_str = "";
				if (!validate(local_str, error_str))
				{
					// invalid string, reload dialog with error message
					rerun_dlg = true;
					return true;
				}
			}
			ret = true;
			return true;
		case message::CANCEL:
			ret = false;
			return true;
	}
	return false;
}

