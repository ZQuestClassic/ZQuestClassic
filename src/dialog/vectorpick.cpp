#include "vectorpick.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "gui/use_size.h"

static bool badmax;
void call_edit_vector(std::vector<int32_t>& vec, bool zsint, size_t min, size_t max)
{
	if((badmax = max > 1000))
	{
		max = 1000;
	}
	VectorPickDialog(vec, zsint, min, max).show();
}

VectorPickDialog::VectorPickDialog(std::vector<int32_t>& vec, bool zsint, size_t min, size_t max)
	: local_vec(vec), dest_vec(vec), min(min), max(max), zsint(zsint)
{
	local_vec.clear();
	local_vec.assign(vec.begin(),vec.end());
}

static int32_t scroll_pos1 = 0;
std::shared_ptr<GUI::Widget> VectorPickDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Grid> wingrid, sgrid;
	
	sgrid = GUI::Internal::makeRows(sized(2,4));
	
	window = Window(
		title = "Vector Editor",
		minwidth = 30_em,
		info = "Select a list of numbers.",
		onClose = message::CANCEL,
		Column(
			wingrid = Column(padding=0_px),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
	
	if(min)
	{
		if(local_vec.size() < min)
			local_vec.resize(min);
	}
	if(max)
	{
		if(local_vec.size() > max)
			local_vec.resize(max);
	}
	
	bool atmin = (min && local_vec.size() <= min) || local_vec.empty();
	bool atmax = max && local_vec.size() >= max;
	
	if(min || max)
	{
		char buf[256];
		if(min == max)
		{
			sprintf(buf, "Requires exactly %zd value%s.", min, min>1 ? "s" : "");
		}
		else if(min && max)
		{
			sprintf(buf, "Requires between %zd and %zd values.", min, max);
		}
		else
		{
			auto v = min ? min : max;
			sprintf(buf, "Requires at %s %zd value%s.", min ? "least" : "most", v, v>1 ? "s" : "");
		}
		wingrid->add(Label(text = buf));
	}
	
	wingrid->add(Row(padding = 0_px,
		Button(
			text = "Empty",
			onClick = message::RELOAD,
			disabled = atmin,
			onPressFunc = [&]()
			{
				if(min)
					local_vec.resize(min);
				else local_vec.clear();
			}
		),
		Button(
			text = "Fill",
			onClick = message::RELOAD,
			disabled = atmax || !max, 
			onPressFunc = [&]()
			{
				local_vec.resize(max);
			}
		)
	));
	
	for(size_t ind = 0; ind < local_vec.size(); ++ind)
	{
		std::shared_ptr<GUI::Grid> row = Row(framed = true);
		row->add(Label(text = "["+std::to_string(ind)+"]:", rightPadding = 0_px));
		if(zsint)
		{
			row->add(TextField(
				maxwidth = 8_em, hPadding = 0_px,
				type = GUI::TextField::type::SWAP_ZSINT2,
				val = local_vec[ind],
				onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_vec[ind] = val;
				}));
		}
		else
		{
			row->add(TextField(
				maxwidth = 8_em, hPadding = 0_px,
				type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
				val = local_vec[ind]*10000,
				onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_vec[ind] = val/10000;
				}));
		}
		if(!atmin)
		{
			row->add(Button(
					text = "-", forceFitH = true,
					hPadding = 0_px,
					onClick = message::RELOAD,
					onPressFunc = [&,ind]()
					{
						auto it = local_vec.begin();
						for(size_t q = 0; q < ind; ++q)
							++it;
						local_vec.erase(it);
					}
				));
		}
		sgrid->add(row);
	}
	if(!atmax)
	{
		sgrid->add(Row(
			padding = 0_px,
			Button(text = "Add Value", hPadding = 0_px,
				onClick = message::RELOAD,
				onPressFunc = [&]()
				{
					local_vec.push_back(0);
				})
		));
	}
	wingrid->add(ScrollingPane(
		ptr = &scroll_pos1,
		targHeight = sized(150_px,300_px),
		sgrid));
	return window;
}

bool VectorPickDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			rerun_dlg = true;
			return true;
		case message::OK:
			dest_vec.clear();
			dest_vec.assign(local_vec.begin(),local_vec.end());
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}

