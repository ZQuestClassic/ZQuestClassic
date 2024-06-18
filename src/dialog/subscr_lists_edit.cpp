#include "subscr_lists_edit.h"
#include <gui/builder.h>
#include "alert.h"
#include <utility>
#include <sstream>
#include <fmt/format.h>
#include "base/files.h"
#include "new_subscr.h"
#include "zq/zq_subscr.h"
#include "zq/zq_misc.h"
#include "zq/zq_files.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"

extern ZCSubscreen subscr_edit;
extern DIALOG *subscreen_dlg;
extern bool saved;

void delete_subscreen(size_t ind, byte ty);
void do_edit_subscr(size_t ind, byte ty);

static std::vector<ZCSubscreen>* subscr_vecs[sstMAX];

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
		subscr_lists[q].add(fmt::format("<New {} Subscreen>",subscr_names[q]),vec->size());
		message edit_msg = message(int(message::EDIT0)+q),
			del_msg = message(int(message::DEL0)+q);
		g->add(TabRef(name = subscr_names[q],
				Column(
					INFOBTN_EX(subscr_infos[q], maxheight = 1.5_em),
					List(data = subscr_lists[q],
						selectedValue = sel_inds[q],
						onDClick = edit_msg,
						onRClick = message::REFRESH,
						onSelectFunc = [&,q](int32_t val)
						{
							sel_inds[q] = val;
						},
						onRClickFunc = [&,q](int32_t val, int32_t mx, int32_t my)
						{
							sel_inds[q] = val;
							forceDraw();
							rclick_menu(q,mx,my);
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
void SubscrListEditDialog::rclick_menu(size_t cur_type, int mx, int my)
{
	auto& ci = copy_inds[cur_type];
	auto& si = sel_inds[cur_type];
	auto& vec = *subscr_vecs[cur_type];
	if(ci >= vec.size()) //bad copy index
		ci = -1;
	bool newslot = si>=vec.size();
	NewMenu rcmenu
	{
		{ "&Copy", [&](){ci = si;}, nullopt, newslot },
		{ "Paste", "&v", [&]()
			{
				if(ci==si) return;
				bool run = true;
				if(!newslot)
					AlertDialog(fmt::format("Overwrite {} Subscreen?",subscr_names[cur_type]),
						fmt::format("Are you sure you want to overwrite {0} Subscreen {1} '{2}'"
							" with {0} Subscreen {3} '{4}'? This cannot be undone!", subscr_names[cur_type], si,
							vec[si].name, ci, vec[ci].name),
						[&](bool ret,bool)
						{
							run = ret;
						}).show();
				if(run)
				{
					if(newslot)
						vec.emplace_back(vec[ci]); //copy constructor
					else vec[si] = vec[ci];
					saved=false;
				}
			}, nullopt, ci<0 },
		{ "&Save", [&]()
			{
				if(!prompt_for_new_file_compat("Export Subscreen (.sub)","sub",NULL,datapath,false))
					return;
				save_subscreen(temppath, vec[si]);
			}, nullopt, newslot },
		{ "&Load", [&]()
			{
				if(!prompt_for_existing_file_compat("Import Subscreen (.sub)","sub",NULL,datapath,false))
					return;
				ZCSubscreen tmp = ZCSubscreen();
				tmp.sub_type = cur_type;
				if(load_subscreen(temppath, tmp))
				{
					if(newslot)
						vec.emplace_back(tmp); //copy constructor
					else vec[si] = tmp;
					saved=false;
				}
			} },
	};
	rcmenu.pop(mx, my);
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
			AlertDialog(fmt::format("Delete {} Subscreen?",subscr_names[cur_type]),
				fmt::format("Are you sure you want to delete {} Subscreen {} '{}'?"
					" This cannot be undone!", subscr_names[cur_type], sel_inds[cur_type],
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

