#include "tilepicker.h"
#include "base/containers.h"
#include <gui/builder.h>
#include <utility>

bool call_tilepicker_dlg(string const& title, vector<vector<TilePickerData>>& data)
{
	bool ret = false;
	vector<vector<TilePickerData>> tmp_data = data;
	TilePickerDialog(title, ret, tmp_data).show();
	if (ret)
		data = tmp_data;
	return ret;
}

TilePickerDialog::TilePickerDialog(string const& title, bool& confirm, vector<vector<TilePickerData>>& data):
	d_title(title), confirm(confirm), data(data)
{}

std::shared_ptr<GUI::Widget> TilePickerDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	size_t num_rows = data.size();
	size_t num_cols = 0;
	for (auto const& vec : data)
		num_cols = zc_max(num_cols, vec.size());
	auto grid = GUI::Grid::rows(num_cols);
	
	for (size_t row = 0; row < num_rows; ++row)
	{
		auto& row_data = data[row];
		for (size_t col = 0; col < num_cols; ++col)
		{
			if (col >= row_data.size())
			{
				// dummy out the rest of the row
				grid->add(DummyWidget(colSpan = num_cols - col));
				break;
			}
			TilePickerData* d = &row_data[col];
			grid->add(SelTileSwatch(
				tile = d->tile,
				cset = d->cset,
				flip = d->flip,
				showFlip = true,
				showvals = false,
				onSelectFunc = [&, d](int32_t t, int32_t c, int32_t f,int32_t)
				{
					d->tile = t;
					d->cset = WRAP_CS(c);
					d->flip = f;
				}
			));
		}
	}
	
	window = Window(
		title = d_title,
		onClose = message::CANCEL,
		padding = 0_px,
		Column(padding = 0_px,
			grid,
			Row(
				spacing = 2_em,
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
	return window;
}

bool TilePickerDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			confirm = true;
			return true;
		case message::CANCEL:
			confirm = false;
			return true;
	}
	return false;
}

