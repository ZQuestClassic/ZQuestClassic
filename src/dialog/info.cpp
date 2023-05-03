#include "info.h"
#include <gui/builder.h>
#include <utility>

#ifdef IS_ZQUEST
#include "quest_rules.h"
#endif

extern byte quest_rules[QUESTRULES_NEW_SIZE];

void displayinfo(std::string title, std::string text)
{
	InfoDialog(title,text).show();
}

InfoDialog::InfoDialog(std::string title, std::string text):
	dlgTitle(title),
	dlgText(text)
{
	postinit();
}

InfoDialog::InfoDialog(std::string title, std::vector<std::string_view> lines):
	dlgTitle(title),
	dlgText()
{
	size_t size = 0;

	for(auto& line: lines)
		size += line.size();
	size += lines.size()-1;
	dlgText.reserve(size);

	auto remaining = lines.size();
	for(auto& line: lines)
	{
		dlgText += line;
		--remaining;
		if(remaining > 0)
			dlgText += '\n';
	}
	postinit();
}

void InfoDialog::postinit()
{
	while(true)
	{
		size_t pos = dlgText.find_first_of("$");
		if(pos == std::string::npos)
			break;
		size_t nextpos = dlgText.find_first_of("$",pos+1);
		if(nextpos == std::string::npos)
			break;
		std::string sub = dlgText.substr(pos+1,nextpos-pos-1);
		dlgText.erase(pos,nextpos-pos+1);
		#ifdef IS_ZQUEST
		bool running = true;
		while(running)
		{
			size_t commapos = sub.find_first_of(",");
			std::string sub2;
			if(commapos == std::string::npos)
			{
				running = false;
				sub2 = sub;
			}
			else
			{
				sub2 = sub.substr(0,commapos);
				sub.erase(0,commapos+1);
			}
			if(sub2.size() < 1 || sub2.find_first_not_of("0123456789") != std::string::npos)
				continue; //invalid
			int val = atoi(sub2.c_str());
			qrs.insert(val);
		}
		#endif
	}
	
	if(qrs.size())
	{
		memcpy(local_qrs, quest_rules, sizeof(local_qrs));
	}
}

std::shared_ptr<GUI::Widget> InfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Widget> widg;
	std::shared_ptr<GUI::Grid> closeRow;
	
	#ifdef IS_ZQUEST
	if(qrs.size())
	{
		GUI::ListData tosearch = combinedQRList()+combinedZSRList();
		widg = Frame(title = "Related QRs",
			QRPanel(
				padding = 3_px,
				onToggle = message::TOGGLE_QR,
				initializer = local_qrs,
				count = 0,
				data = tosearch.filter(
					[&](GUI::ListItem& itm)
					{
						if(qrs.contains(itm.value))
							return true;
						return false;
					})
			)
		);
		closeRow = Row(padding = 0_px,
			Button(
				text = "OK",
				topPadding = 0.5_em,
				onClick = message::OK,
				focused = true),
			Button(
				text = "Cancel",
				topPadding = 0.5_em,
				onClick = message::CANCEL)
		);
	}
	else
	#endif
	{
		widg = DummyWidget(padding=0_px);
		closeRow = Row(padding = 0_px,
			Button(
				text = "&Close",
				topPadding = 0.5_em,
				onClick = message::OK,
				focused = true)
		);
	}

	return Window(
		title = std::move(dlgTitle),
		onClose = 0,
		hPadding = 0_px, 
		Column(
			hPadding = 0_px, 
			Label(noHLine = true,
				hPadding = 2_em,
				maxLines = 20,
				maxwidth = Size::pixels(zq_screen_w)-12_px-5_em,
				textAlign = 1,
				text = std::move(dlgText)),
			widg,
			closeRow
		)
	);
}

bool InfoDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::TOGGLE_QR:
			toggle_bit(local_qrs, msg.argument);
			return false;
		case message::OK:
			if(qrs.size())
				memcpy(quest_rules, local_qrs, sizeof(local_qrs));
		[[fallthrough]];
		case message::CANCEL:
			return true;
	}
	return false;
}
