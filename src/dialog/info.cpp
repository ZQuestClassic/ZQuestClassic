#include "info.h"
#include <gui/builder.h>
#include <utility>
#include "base/qrs.h"
#include <fmt/ranges.h>

#ifdef IS_EDITOR
#include "quest_rules.h"
#include "zq/zq_files.h"

extern const GUI::ListData ruletemplatesList;
#endif

void displayinfo(string const& title, string const& text, optional<string> subtext)
{
	if (is_headless() || is_ci())
	{
		al_trace("[info] %s\n", fmt::format("{} - {}\n{}", title, text, subtext.value_or("")).c_str());
		return;
	}

	InfoDialog(title,text,subtext).show();
}

void displayinfo(string const& title, vector<string> const& lines, optional<string> subtext)
{
	if (is_headless() || is_ci())
	{
		al_trace("[info] %s\n", fmt::format("{} - {}\n{}", title, fmt::join(lines, "\n"), subtext.value_or("")).c_str());
		return;
	}

	InfoDialog(title,lines,subtext).show();
}

InfoDialog::InfoDialog(string const& title, string const& text,
	optional<string> subtext, byte* dest_qrs, int text_align):
	d_title(title),
	d_text(text), d_subtext(subtext),
	dest_qrs(dest_qrs),
	_text_align(text_align)
{
	postinit();
}

InfoDialog::InfoDialog(string const& title, vector<string> const& lines,
	optional<string> subtext, byte* dest_qrs, int text_align):
	d_title(title),
	d_text(), d_subtext(subtext),
	dest_qrs(dest_qrs),
	_text_align(text_align)
{
	size_t size = 0;

	for(auto& line: lines)
		size += line.size();
	size += lines.size()-1;
	d_text.reserve(size);

	auto remaining = lines.size();
	for(auto& line: lines)
	{
		d_text += line;
		--remaining;
		if(remaining > 0)
			d_text += '\n';
	}
	postinit();
}

static byte* next_dest_qr = nullptr;
void InfoDialog::postinit()
{
	old_dest_qrs = next_dest_qr;
	while(true)
	{
		size_t pos = d_text.find_first_of("$");
		if(pos == string::npos)
			break;
		size_t nextpos = d_text.find_first_of("$",pos+1);
		if(nextpos == string::npos)
			break;
		string sub = d_text.substr(pos+1,nextpos-pos-1);
		d_text.erase(pos,nextpos-pos+1);
		#ifdef IS_EDITOR
		dword special_type = 0; //qr by default
		if(sub[0] == '#') //Special type id given
		{
			auto nexthash = sub.find_first_of("#",1);
			if(nexthash == string::npos || nexthash == 1)
				continue; //invalid
			if(sub.find_first_not_of("0123456789",1) != nexthash)
				continue; //invalid
			special_type = atoi(sub.substr(1,nexthash).c_str());
			sub.erase(0,nexthash+1);
		}
		bool running = true;
		while(running)
		{
			size_t commapos = sub.find_first_of(",");
			string sub2;
			if(commapos == string::npos)
			{
				running = false;
				sub2 = sub;
			}
			else
			{
				sub2 = sub.substr(0,commapos);
				sub.erase(0,commapos+1);
			}
			if(sub2.size() < 1 || sub2.find_first_not_of("0123456789") != string::npos)
				continue; //invalid
			int val = atoi(sub2.c_str());
			
			switch(special_type)
			{
				case INFOHINT_T_QR:
					qrs.insert(val);
					break;
				case INFOHINT_T_RULETMPL:
					ruleTemplates.insert(val);
					break;
			}
		}
		#endif
	}
	
	if(qrs.size() || ruleTemplates.size())
	{
		if(!dest_qrs)
			dest_qrs = next_dest_qr ? next_dest_qr : quest_rules;
		next_dest_qr = local_qrs;
		memcpy(local_qrs, dest_qrs, sizeof(local_qrs));
	}
}

std::shared_ptr<GUI::Widget> InfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> gr;
	std::shared_ptr<GUI::Grid> closeRow;
	bool add_grid = false, addok = false;
	#ifdef IS_EDITOR
	add_grid = addok = qrs.size() || ruleTemplates.size();
	if(add_grid)
		gr = Row(padding = 0_px);
	if(qrs.size())
	{
		GUI::ListData tosearch = (combinedQRList()+combinedZSRList()).filter(
			[&](GUI::ListItem& itm)
			{
				if(qrs.contains(itm.value))
					return true;
				return false;
			});
		if(tosearch.size())
		{
			gr->add(Frame(title = "Related QRs",
				QRPanel(
					padding = 3_px,
					onToggle = message::TOGGLE_QR,
					onCloseInfo = message::REFR_INFO,
					qr_ptr = local_qrs,
					count = 0,
					data = tosearch
				)
			));
		}
	}
	if(ruleTemplates.size())
	{
		std::shared_ptr<GUI::Grid> cboxes = Rows<2>();
		cboxes->add(Label(colSpan = 2, text = "Note: Selecting a rule template"
			"\nwill write to numerous QRs."));
		int cnt = 0;
		for(size_t q = 0; q < ruletemplatesList.size(); ++q)
		{
			if(!ruleTemplates.contains(q))
				continue;
			string infostr = ruletemplatesList.getInfo(q);
			cboxes->add(infostr.size() ? INFOBTN(infostr) : DINFOBTN());
			cboxes->add(Checkbox(
					hAlign = 0.0, checked = false,
					text = ruletemplatesList.getText(q),
					onToggleFunc = [=](bool state)
					{
						on_templates[q] = state;
					}
				));
			++cnt;
		}
		if(cnt)
			gr->add(Frame(title = "Related Rule Templates",cboxes));
	}
	#endif
	if(addok)
	{
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
	{
		closeRow = Row(padding = 0_px,
			Button(
				text = "&Close",
				topPadding = 0.5_em,
				onClick = message::CANCEL,
				focused = true)
		);
	}
	
	std::shared_ptr<GUI::Grid> main_col;
	window = Window(
		title = d_title,
		onClose = message::CANCEL,
		hPadding = 0_px, 
		main_col = Column(
			hPadding = 0_px,
			build_text()
		)
	);
	if(add_grid)
		main_col->add(gr);
	main_col->add(closeRow);
	return window;
}

std::shared_ptr<GUI::Grid> InfoDialog::build_text()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> col = Column(padding = 0_px);
	Size maxw = Size::pixels(zq_screen_w)-12_px-5_em;
	Size maxh = (DEFAULT_PADDING*20)+20_em;
	double hal = _text_align / 2.0;
	if(d_subtext)
		col->add(Label(noHLine = true, hPadding = 2_em,
			maxwidth = maxw, hAlign = hal, textAlign = _text_align, text = *d_subtext));
	std::shared_ptr<GUI::Label> main_label =
		Label(noHLine = true, hPadding = 2_em,
			maxwidth = maxw, hAlign = hal, textAlign = _text_align, text = d_text);
	main_label->calculateSize();
	if(main_label->getHeight() > maxh)
	{
		col->add(ScrollingPane(
			targHeight = maxh,
			main_label));
	}
	else
	{
		col->add(main_label);
	}
	return col;
}

bool InfoDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::TOGGLE_QR:
			toggle_bit(local_qrs, msg.argument);
			return false;
		case message::OK:
			#ifdef IS_EDITOR
			if(dest_qrs)
			{
				if(ruleTemplates.size())
				{
					for(int q = 0; q < sz_ruletemplate; ++q)
					{
						if(on_templates[q])
							applyRuleTemplate(q,local_qrs);
					}
				}
				memcpy(dest_qrs, local_qrs, sizeof(local_qrs));
			}
			#endif
		[[fallthrough]];
		case message::CANCEL:
			next_dest_qr = old_dest_qrs;
			return true;
		case message::REFR_INFO:
			rerun_dlg = true;
			return true;
	}
	return false;
}

