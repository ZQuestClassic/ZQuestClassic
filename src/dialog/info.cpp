#include "info.h"
#include <gui/builder.h>
#include <utility>
#include "core/qrs.h"
#include <fmt/ranges.h>
#include "zc_list_data.h"

#ifdef IS_EDITOR
#include "quest_rules.h"
#include "zq/zq_files.h"
#endif

InfoDialog::InfoDialog(string const& title, string const& text):
	d_title(title),
	d_text(text), d_subtext(nullopt),
	d_text_align(1),
	_has_run_postinit(false), d_dest_qrs(nullptr)
{}

InfoDialog::InfoDialog(string const& title, vector<string> const& lines):
	d_title(title),
	d_text(""), d_subtext(nullopt),
	d_text_align(1),
	_has_run_postinit(false), d_dest_qrs(nullptr)
{
	d_text = fmt::format("{}", fmt::join(lines, "\n"));
}

InfoDialog& InfoDialog::set_subtext(optional<string> subtext)
{
	d_subtext = subtext;
	return *this;
}
InfoDialog& InfoDialog::set_dest_qrs(byte* dest_qrs)
{
	d_dest_qrs = dest_qrs;
	return *this;
}
InfoDialog& InfoDialog::set_text_align(int text_align)
{
	d_text_align = text_align;
	return *this;
}

#define HINT_QRS (hint_data.hints[INFOHINT_T_QR])
#define HINT_TEMPLATES (hint_data.hints[INFOHINT_T_RULETMPL])

static byte* next_dest_qr = nullptr;
void InfoDialog::postinit()
{
	_has_run_postinit = true;
	old_dest_qrs = next_dest_qr;
	
	if (get_app_id() == App::zquest)
	{
		hint_data = parse_hint_text(d_text);
	}
	else
	{
		erase_hint_text(d_text);
		hint_data = {};
	}
	
	if (!(HINT_QRS.empty() && HINT_TEMPLATES.empty()))
	{
		if(!d_dest_qrs)
			d_dest_qrs = next_dest_qr ? next_dest_qr : quest_rules;
		next_dest_qr = local_qrs;
		memcpy(local_qrs, d_dest_qrs, sizeof(local_qrs));
	}
}

std::shared_ptr<GUI::Widget> InfoDialog::view()
{
	if (is_headless() || is_ci())
	{
		al_trace("[info] %s\n", fmt::format("{} - {}\n{}", d_title, d_text, d_subtext.value_or("")).c_str());
		return nullptr;
	}
	if (!_has_run_postinit)
		postinit();
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> gr;
	std::shared_ptr<GUI::Grid> closeRow;
	bool add_grid = false, addok = false;
	#ifdef IS_EDITOR
	auto& qrs = HINT_QRS;
	auto& ruleTemplates = HINT_TEMPLATES;
	add_grid = addok = qrs.size() || ruleTemplates.size();
	if (add_grid)
		gr = Row(padding = 0_px);
	if (!qrs.empty())
	{
		GUI::ListData tosearch = (combinedQRList()+combinedZSRList()).filter(
			[&](GUI::ListItem& itm)
			{
				if(qrs.contains(itm.value))
					return true;
				return false;
			});
		if (tosearch.size())
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
	if (!ruleTemplates.empty())
	{
		std::shared_ptr<GUI::Grid> cboxes = Rows<2>();
		cboxes->add(Label(colSpan = 2, text = "Note: Selecting a rule template"
			"\nwill write to numerous QRs."));
		int cnt = 0;
		auto const& list_rule_templates = GUI::ZCListData::rule_templates_list();
		for (size_t q = 0; q < list_rule_templates.size(); ++q)
		{
			if (!ruleTemplates.contains(q))
				continue;
			string infostr = list_rule_templates.getInfo(q);
			cboxes->add(infostr.size() ? INFOBTN(infostr) : DINFOBTN());
			cboxes->add(Checkbox(
					hAlign = 0.0, checked = false,
					text = list_rule_templates.getText(q),
					onToggleFunc = [=, this](bool state)
					{
						on_templates[q] = state;
					}
				));
			++cnt;
		}
		if (cnt)
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
	
	std::ostringstream oss;
	oss << d_text;
	for (auto& [txt, url] : hint_data.urls)
		oss << fmt::format("\n[{}]({})", txt, url);
	
	window->setCopyText(oss.str());
	
	if (add_grid)
		main_col->add(gr);
	if (!hint_data.urls.empty())
	{
		auto url_gr = Rows<4>(padding = 0_px);
		for (auto& [txt, url] : hint_data.urls)
		{
			url_gr->add(Button(text = txt,
				height = 2_em,
				onPressFunc = [&, url]()
				{
					util::open_web_link(url);
				}
			));
		}
		main_col->add(url_gr);
	}
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
	double hal = d_text_align / 2.0;
	if(d_subtext)
		col->add(Label(noHLine = true, hPadding = 2_em,
			maxwidth = maxw, hAlign = hal, textAlign = d_text_align, text = *d_subtext));
	std::shared_ptr<GUI::Label> main_label =
		Label(noHLine = true, hPadding = 2_em,
			maxwidth = maxw, hAlign = hal, textAlign = d_text_align, text = d_text);
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
			if(d_dest_qrs)
			{
				if (!HINT_TEMPLATES.empty())
				{
					for(int q = 0; q < sz_ruletemplate; ++q)
					{
						if(on_templates[q])
							applyRuleTemplateWithConfirmation(q,local_qrs);
					}
				}
				memcpy(d_dest_qrs, local_qrs, sizeof(local_qrs));
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

