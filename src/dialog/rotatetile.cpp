#include "rotatetile.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include <fmt/format.h>

RotateTileDialog::RotateTileDialog(int32_t* w, int32_t* h, zfix* r) :
	swid(*w), shei(*h), tr(*r), trot(r)
{}

std::shared_ptr<GUI::Widget> RotateTileDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Rotate Tile",
		info = "Rotates a tile block by an arbitrary rotation in degrees. This can degrade sprite quality and is not reversible with additional rotations.",
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Label(text = fmt::format("Source Size: {}x{}", swid, shei), colSpan = 2),
				Label(text = "Rotation (Degrees):"),
				TextField(
					type = GUI::TextField::type::NOSWAP_ZSINT,
					swap_type = nswapDEC,
					low = -3600000, high = 3600000, val = tr.getZLong(), focused = true,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
					{
						tr = zslongToFix(val);
					})
			),
			Row(
				topPadding = 0.5_em,
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
}

bool RotateTileDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::OK:
		*trot = tr;
		return true;
	case message::CANCEL:
		return true;
	}
	return false;
}
