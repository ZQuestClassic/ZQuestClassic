#include "numpick.h"
#include "common.h"
#include "info.h"
#include <gui/builder.h>
#include "gui/use_size.h"
#include <fmt/format.h>

static const size_t pglimit = 50;

template<typename Sz>
VectorPickDialog<Sz>::VectorPickDialog(bounded_vec<Sz,int32_t>& vec, bool zsint)
	: local_vec(vec), dest_vec(vec), zsint(zsint), pg(0)
{}

static int32_t scroll_pos_vec1 = 0;
template<typename Sz>
std::shared_ptr<GUI::Widget> VectorPickDialog<Sz>::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	local_vec.normalize();
	
	std::shared_ptr<GUI::Grid> wingrid, sgrid, cmdrow;
	
	sgrid = GUI::Internal::makeRows(4);
	
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
					text = "Empty",
					onClick = message::RELOAD,
					disabled = local_vec.inner_empty(),
					onPressFunc = [&]()
					{
						local_vec.clear();
					}
				),
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	wingrid->add(Label(text = fmt::format("Requires at most {} value{}.", local_vec.size(), local_vec.size()>1 ? "s" : "")));
	for(size_t ind = pg*pglimit; ind < local_vec.size() && ind < (pg+1)*pglimit; ++ind)
	{
		std::shared_ptr<GUI::Grid> row = Row(framed = true, fitParent = true);
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
				val = local_vec[ind],
				onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_vec[ind] = val;
				}));
		}
		row->add(Button(
				text = "-", forceFitH = true,
				hPadding = 0_px,
				disabled = local_vec[ind] != local_vec.defval(),
				onClick = message::RELOAD,
				onPressFunc = [&,ind]()
				{
					local_vec[ind] = local_vec.defval();
				}
			));
		sgrid->add(row);
	}
	wingrid->add(Row(
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_LEFT2,
				disabled = !pg,
				onPressFunc = [&]()
				{
					if(pg > 0)
					{
						pg = 0;
						this->refresh_dlg();
					}
				}),
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_LEFT,
				disabled = !pg,
				onPressFunc = [&]()
				{
					if(pg > 0)
					{
						--pg;
						this->refresh_dlg();
					}
				}),
			Label(text = fmt::format("Page {} ({}-{})",pg,pg*pglimit,((pg+1)*pglimit)-1),
				minwidth = 8_em, textAlign = 1),
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_RIGHT,
				disabled = (local_vec.size() <= (pg+1)*pglimit),
				onPressFunc = [&]()
				{
					if(local_vec.size() > (pg+1)*pglimit)
					{
						++pg;
						this->refresh_dlg();
					}
				}),
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_RIGHT2,
				disabled = (local_vec.size() <= (pg+1)*pglimit),
				onPressFunc = [&]()
				{
					if(local_vec.size() > (pg+1)*pglimit)
					{
						while(local_vec.size() > (pg+1)*pglimit)
							++pg;
						this->refresh_dlg();
					}
				})
		));
	wingrid->add(Frame(sgrid));
	return window;
}

template<typename Sz>
bool VectorPickDialog<Sz>::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			this->rerun_dlg = true;
			return true;
		case message::OK:
			dest_vec = local_vec;
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

template<typename Sz>
MapPickDialog<Sz>::MapPickDialog(bounded_map<Sz,int32_t>& mp, bool zsint)
	: local_map(mp), dest_map(mp), zsint(zsint), pg(0)
{}

static int32_t scroll_pos_map1 = 0;
template<typename Sz>
std::shared_ptr<GUI::Widget> MapPickDialog<Sz>::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	if(local_map.empty()) return nullptr;
	
	std::shared_ptr<GUI::Grid> wingrid, sgrid, cmdrow;
	
	sgrid = GUI::Internal::makeRows(4);
	
	window = Window(
		title = "Map Editor",
		minwidth = 30_em,
		info = "Select a set of numbers.",
		onClose = message::CANCEL,
		Column(
			wingrid = Column(padding = 0_px),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "Empty",
					disabled = local_map.inner_empty(),
					onPressFunc = [&]()
					{
						local_map.clear();
						this->refresh_dlg();
					}),
				Button(
					text = "Fill",
					disabled = local_map.capacity() == local_map.size(),
					onPressFunc = [&]()
					{
						for(Sz q = 0; q < local_map.size(); ++q)
							local_map[q] = local_map.defval();
						this->refresh_dlg();
					}),
				Button(text = "Add Value", hPadding = 0_px,
					onPressFunc = [&]()
					{
						for(Sz q = 0; q < local_map.size(); ++q)
						{
							if(!local_map.contains(q))
							{
								if(auto v = call_get_num("Add at what index?", q, local_map.size()-1, 0))
								{
									local_map[*v] = local_map.defval();
									this->refresh_dlg();
								}
								break;
							}
						}
					}),
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	size_t onpg = 0, skipcnt = 0;
	for(auto [k,v] : local_map.inner())
	{
		if(onpg > pg) break;
		bool skip = onpg != pg;
		if(++skipcnt == pglimit)
		{
			skipcnt = 0;
			++onpg;
		}
		if(skip)
			continue;
		std::shared_ptr<GUI::Grid> row = Row(framed = true, fitParent = true);
		row->add(Label(text = "["+std::to_string(k)+"]:", rightPadding = 0_px));
		if(zsint)
		{
			row->add(TextField(
				maxwidth = 8_em, hPadding = 0_px,
				type = GUI::TextField::type::SWAP_ZSINT2,
				val = local_map[k],
				onValChangedFunc = [&, k=k](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_map[k] = val;
				}));
		}
		else
		{
			row->add(TextField(
				maxwidth = 8_em, hPadding = 0_px,
				type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
				val = local_map[k],
				onValChangedFunc = [&, k=k](GUI::TextField::type,std::string_view,int32_t val)
				{
					local_map[k] = val;
				}));
		}
		row->add(Button(
				text = "-", forceFitH = true,
				hPadding = 0_px,
				onClick = message::RELOAD,
				onPressFunc = [&, k=k]()
				{
					local_map.erase(k);
				}
			));
		sgrid->add(row);
	}
	if (local_map.inner_empty()) sgrid->add(DummyWidget()); //prevent crash on empty map
	auto curpg_start = pg*pglimit;
	auto curpg_end = zc_min(local_map.size()-1,((pg+1)*pglimit)-1);
	wingrid->add(Row(
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_LEFT2,
				disabled = !pg,
				onPressFunc = [&]()
				{
					if(pg > 0)
					{
						pg = 0;
						this->refresh_dlg();
					}
				}),
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_LEFT,
				disabled = !pg,
				onPressFunc = [&]()
				{
					if(pg > 0)
					{
						--pg;
						this->refresh_dlg();
					}
				}),
			Label(text = fmt::format("Page {} ({}-{})",pg,curpg_start,curpg_end),
				minwidth = 8_em, textAlign = 1),
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_RIGHT,
				disabled = (local_map.capacity() <= (pg+1)*pglimit),
				onPressFunc = [&]()
				{
					if(local_map.capacity() > (pg+1)*pglimit)
					{
						++pg;
						this->refresh_dlg();
					}
				}),
			Button(type = GUI::Button::type::ICON, icon = BTNICON_ARROW_RIGHT2,
				disabled = (local_map.capacity() <= (pg+1)*pglimit),
				onPressFunc = [&]()
				{
					if(local_map.capacity() > (pg+1)*pglimit)
					{
						while(local_map.capacity() > (pg+1)*pglimit)
							++pg;
						this->refresh_dlg();
					}
				})
		));
	wingrid->add(Frame(sgrid));
	return window;
}

template<typename Sz>
bool MapPickDialog<Sz>::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			this->rerun_dlg = true;
			return true;
		case message::OK:
			dest_map = local_map;
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}


NumPickDialog::NumPickDialog(string const& lbl,optional<string> inf,
	optional<int32_t>& retv, int32_t snum, bool zsint, int32_t vmax, int32_t vmin)
	: retv(retv), labeltext(lbl), local_val(snum), zsint(zsint), min(vmin), max(vmax)
{
	retv = nullopt;
	infostr = inf;
}

std::shared_ptr<GUI::Widget> NumPickDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::TextField> tf;
	
	if(zsint)
	{
		tf = TextField(
			focused = true,
			width = 8_em, hPadding = 0_px,
			type = GUI::TextField::type::SWAP_ZSINT2,
			low = min, high = max, val = local_val,
			onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_val = val;
			});
	}
	else
	{
		tf = TextField(
			focused = true,
			width = 8_em, hPadding = 0_px,
			type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
			low = min, high = max, val = local_val,
			onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_val = val;
			});
	}
	std::shared_ptr<GUI::Widget> w, inflbl;
	if(max < min)
		w = DummyWidget(padding = 0_px);
	else w = Label(text = fmt::format("({} <= x <= {})",min,max));
	if(infostr)
		inflbl = Label(text = *infostr);
	else inflbl = DummyWidget(padding = 0_px);
	window = Window(
		title = "Number Picker",
		minwidth = 30_em,
		onClose = message::CANCEL,
		onEnter = message::OK,
		Column(
			Label(text = labeltext),
			w, tf,
			inflbl,
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	return window;
}

bool NumPickDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			rerun_dlg = true;
			return true;
		case message::OK:
			if(max > min)
				local_val = vbound(local_val,max,min);
			retv = local_val;
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

optional<int32_t> call_get_num(std::string const& lbl, int32_t dv, int32_t max, int32_t min)
{
	optional<int32_t> ret = nullopt;
	NumPickDialog(lbl,nullopt,ret,dv,false,max,min).show();
	return ret;
}
optional<int32_t> call_get_num(std::string const& lbl, string const& inf, int32_t dv, int32_t max, int32_t min)
{
	optional<int32_t> ret = nullopt;
	NumPickDialog(lbl,inf,ret,dv,false,max,min).show();
	return ret;
}
optional<zfix> call_get_zfix(std::string const& lbl, zfix dv, zfix max, zfix min)
{
	optional<int32_t> ret = nullopt;
	NumPickDialog(lbl,nullopt,ret,dv.getZLong(),true,max.getZLong(),min.getZLong()).show();
	if(ret)
		return zslongToFix(*ret);
	return ret;
}
optional<zfix> call_get_zfix(std::string const& lbl, string const& inf, zfix dv, zfix max, zfix min)
{
	optional<int32_t> ret = nullopt;
	NumPickDialog(lbl,inf,ret,dv.getZLong(),true,max.getZLong(),min.getZLong()).show();
	if(ret)
		return zslongToFix(*ret);
	return ret;
}

template class VectorPickDialog<byte>;
template class VectorPickDialog<word>;
template class VectorPickDialog<dword>;
template class MapPickDialog<byte>;
template class MapPickDialog<word>;
template class MapPickDialog<dword>;

