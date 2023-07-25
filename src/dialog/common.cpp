#include "common.h"
#include "zq/zq_misc.h"
#include "base/msgstr.h"
#include "base/misctypes.h"
#include "zq/zquest.h"
#include <fmt/format.h>
#include <algorithm>
#include <array>
#include <vector>

GUI::ListData getItemListData(bool includeNone)
{
	std::vector<GUI::ListItem> listItems;
	listItems.reserve(MAXITEMS+(includeNone ? 1 : 0));

	if(includeNone)
		listItems.emplace_back("(None)", -1);
	for(int32_t i = 0; i < MAXITEMS; ++i)
		listItems.emplace_back(item_string[i], i);

	auto sortBegin = listItems.begin();
	if(includeNone)
		++sortBegin;
	std::sort(sortBegin, listItems.end(),
		[](const auto& a, const auto& b)
		{
			return a.text<b.text;
		});

	return GUI::ListData(std::move(listItems));
}

GUI::ListData getStringListData()
{
	std::vector<size_t> msgMap(msg_count, 0);
	for(size_t i = 0; i < msg_count; ++i)
	{
		auto& msg = MsgStrings[i];
		msgMap[msg.listpos] = i;
	}

	return GUI::ListData(msg_count,
		[&msgMap](size_t index)
		{
			return fmt::format("{}: {}", msgMap[index], MsgStrings[msgMap[index]].s);
		},
		[&msgMap](size_t index)
		{
			return (int32_t)msgMap[index];
		});
}

GUI::ListData getShopListData()
{
	return GUI::ListData(256,
		[](size_t index)
		{
			return fmt::format("{}: {}", index, QMisc.shop[index].name);
		},
		[](size_t index)
		{
			return (int32_t)index;
		});
}

GUI::ListData getBShopListData()
{
	return GUI::ListData(256,
		[](size_t index)
		{
			return fmt::format("{}: {}", index, QMisc.bottle_shop_types[index].name);
		},
		[](size_t index)
		{
			return (int32_t)index;
		});
}

GUI::ListData getInfoShopListData()
{
	return GUI::ListData(256,
		[](size_t index)
		{
			return fmt::format("{}: {}", index, QMisc.info[index].name);
		},
		[](size_t index)
		{
			// TODO: ListData should probably deal with size_t.
			return (int32_t)index;
		});
}

