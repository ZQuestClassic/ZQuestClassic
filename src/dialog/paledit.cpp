#include "paledit.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "../gui/use_size.h"

void call_paledit_dlg(char* namebuf, byte* cdata, PALETTE *pal)
{
	BITMAP* tmp = create_bitmap_ex(8,128_spx,128_spx);
	clear_bitmap(tmp);
	for(auto pos = 0; pos < 256; ++pos)
	{
		rectfill(tmp, (pos%16)*8_spx, (pos/16)*8_spx, ((pos%16)*8_spx)+8_spx-1_px, ((pos/16)*8_spx)+(8_spx-1), pos);
	}
	PalEditDialog(tmp, cdata, pal, namebuf).show();
	destroy_bitmap(tmp);
}

PalEditDialog::PalEditDialog(BITMAP* bmp, byte* cdata, PALETTE* pal, char* namebuf) : bmp(bmp),
	namebuf(namebuf), coldata(cdata), palt(pal)
{}

static size_t paltab = 0;
std::shared_ptr<GUI::Widget> PalEditDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	bool interpfad = false;
	
	return Window(
		title = "Palette Editor",
		onEnter = message::OK,
		onClose = message::OK,
		Column(
			TabPanel(
				ptr = &paltab,
				onSwitch = [&](size_t tab)
				{
					//tab==0,1
				},
				TabRef(name = " 1 ", Rows<17>(
					DummyWidget(padding = 0_px),
					Label(text = "0", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "1", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "2", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "3", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "4", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "5", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "6", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "7", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "8", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "9", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "A", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "B", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "C", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "D", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "E", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "F", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "2", height = 8_spx, textAlign = 1, padding = 0_px),
					PaletteFrame(colSpan = 16, rowSpan = 13,
						bitmap = bmp, cdata = coldata, palette = palt,
						count = 13, padding = 0_px, onUpdate = []()
						{
							
						}
					),
					Label(text = "3", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "4", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "9", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "2", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "0" : "3", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "1" : "4", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "2" : "2", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "3" : "3", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "4" : "4", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "5" : "2", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "6" : "3", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "7" : "4", height = 8_spx, textAlign = 1, padding = 0_px)
				)),
				TabRef(name = " 2 ", Rows<17>(
					DummyWidget(padding = 0_px),
					Label(text = "0", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "1", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "2", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "3", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "4", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "5", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "6", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "7", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "8", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "9", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "A", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "B", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "C", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "D", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "E", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "F", width = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "1", height = 8_spx, textAlign = 1, padding = 0_px),
					PaletteFrame(colSpan = 16, rowSpan = 4,
						bitmap = bmp, cdata = coldata+(13*48), palette = palt,
						count = 4, padding = 0_px, onUpdate = []()
						{
							
						}
					),
					Label(text = "5", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "7", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "8", height = 8_spx, textAlign = 1, padding = 0_px)
				))
			),
			Rows<3>(
				Button(text = "Save",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						
					},
					onClick = message::OK
				),
				Button(text = "&Undo",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						
					}
				),
				DummyWidget(),
				Label(text = "Name:"),
				TextField(text = namebuf, maxLength = 16, colSpan = 2)
			),
			Rows<3>(
				topPadding = 0.5_em,
				vAlign = 1.0,
				Button(text = "&Edit",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						
					}
				),
				Button(text = "&Grab",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						
					}
				),
				Button(text = "Cycle",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						
					}
				),
				Button(text = "&Load to CS 9",
					minwidth = 90_lpx,
					colSpan = 2, fitParent = true,
					onPressFunc = [&]()
					{
						
					},
					onClick = message::OK
				),
				Button(text = "Done",
					minwidth = 90_lpx,
					onClick = message::OK)
			)
		)
	);
}

bool PalEditDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		default:
			return true;
	}
}
