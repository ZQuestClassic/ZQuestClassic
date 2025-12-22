#pragma once

#include "base/headers.h"
#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/grid.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <initializer_list>
#include "dialog/externs.h"

class TextPromptDialog: public GUI::Dialog<TextPromptDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	TextPromptDialog(string const& title, string const& body, string& the_text, std::function<bool(string const&, string&)> validate);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
	
protected:
	std::shared_ptr<GUI::Window> window;
	string d_title, d_text, error_str;
	string local_str;
	string& dest_str;
	std::function<bool(string const&, string&)> validate;
	
};

