#include "subscr_lists_edit.h"
#include <gui/builder.h>
#include "alert.h"
#include <utility>
#include <sstream>
#include <fmt/format.h>
#include "new_subscr.h"
#include "zq/zq_subscr.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"

extern ZCSubscreen subscr_edit;
extern DIALOG *subscreen_dlg;

void delete_subscreen(size_t ind, byte ty);
void do_edit_subscr(size_t ind, byte ty);

static std::vector<ZCSubscreen>* subscr_vecs[sstMAX];
static const std::string _titles[sstMAX] = {"Active","Passive","Overlay"};
static const std::string _infos[sstMAX] = {
	"The subscreen that actively opens when you press 'Start'",
	"The subscreen visible at the top of the screen normally, which moves down when the active opens.",
	"Like the passive, but visible across the whole screen and does NOT move down for the active opening."
	};

void call_subscr_listedit_dlg()
{
	SubscrListEditDialog().show();
}

static size_t editsub_tabptr = 0;
std::shared_ptr<GUI::Widget> SubscrListEditDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::TabPanel> g;
	window = Window(
		title = "Edit Subscreens",
		onClose = message::DONE,
		Column(
			g = TabPanel(ptr = &editsub_tabptr),
			Rows<2>(
				Button(
					text = "Done",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::DONE,
					focused = true)
			)
		)
	);
	
	subscr_vecs[0] = &subscreens_active;
	subscr_vecs[1] = &subscreens_passive;
	subscr_vecs[2] = &subscreens_overlay;
	for(int q = 0; q < sstMAX; ++q)
	{
		auto* vec = subscr_vecs[q];
		subscr_lists[q] = GUI::ListData(vec->size(),
			[&,vec](size_t ind)
			{
				return fmt::format("{} ({:03d})",(*vec)[ind].name,ind);
			},
			[&](size_t ind){return int32_t(ind);});
		subscr_lists[q].add(fmt::format("<New {} Subscreen>",_titles[q]),vec->size());
		message edit_msg = message(int(message::EDIT0)+q),
			del_msg = message(int(message::DEL0)+q);
		g->add(TabRef(name = _titles[q],
				Column(
					INFOBTN_EX(_infos[q], maxheight = 1.5_em),
					List(data = subscr_lists[q],
						selectedValue = sel_inds[q],
						onDClick = edit_msg,
						onSelectFunc = [&,q](int32_t val)
						{
							sel_inds[q] = val;
						}),
					Row(
						Button(
							text = "Edit",
							minwidth = 90_px, hAlign = 1.0,
							onClick = edit_msg),
						Button(
							text = "Delete",
							minwidth = 90_px, hAlign = 0.0,
							onClick = del_msg)
					)
				)
			));
	}
	
	return window;
}

bool SubscrListEditDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	bool refresh = false;
	switch(msg.message)
	{
		case message::REFR_INFO:
			break;
		case message::EDIT0:
		case message::EDIT1:
		case message::EDIT2:
		{
			size_t cur_type = int(msg.message)-int(message::EDIT0);
			do_edit_subscr(sel_inds[cur_type],cur_type);
			refresh = true;
			break;
		}
		case message::DEL0:
		case message::DEL1:
		case message::DEL2:
		{
			size_t cur_type = int(msg.message)-int(message::DEL0);
			auto& vec = *subscr_vecs[cur_type];
			if(sel_inds[cur_type] >= vec.size())
				return false;
			bool run = false;
			AlertDialog(fmt::format("Delete {} Subscreen?",_titles[cur_type]),
				fmt::format("Are you sure you want to delete {} Subscreen {} '{}'?"
					" This cannot be undone!", _titles[cur_type], sel_inds[cur_type],
					vec[sel_inds[cur_type]].name),
				[&](bool ret,bool)
				{
					run = ret;
				}).show();
			if(run)
				delete_subscreen(sel_inds[cur_type],cur_type);
			refresh = true;
			break;
		}
		case message::REFRESH:
			refresh = true;
			break;
		case message::DONE:
			return true;
	}
	if(refresh)
	{
		rerun_dlg = true;
		return true;
	}
	return false;
}

