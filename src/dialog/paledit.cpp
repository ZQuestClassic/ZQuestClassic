#include "paledit.h"
#include "../zsys.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "../gui/use_size.h"
#include "../zq_misc.h"

extern int32_t cset_count, cset_first;
extern RGB** gUndoPal;
extern PALETTE pal;
void undo_pal();

void call_paledit_dlg(char* namebuf, byte* cdata, PALETTE *pal, int32_t offset)
{
	BITMAP* tmp = create_bitmap_ex(8,128_spx,128_spx);
	clear_bitmap(tmp);
	for(auto pos = 0; pos < 256; ++pos)
	{
		rectfill(tmp, (pos%16)*8_spx, (pos/16)*8_spx, ((pos%16)*8_spx)+8_spx-1_px, ((pos/16)*8_spx)+(8_spx-1), pos);
	}
	PalEditDialog(tmp, cdata, pal, namebuf, offset).show();
	destroy_bitmap(tmp);
}

PalEditDialog::PalEditDialog(BITMAP* bmp, byte* cdata, PALETTE* pal, char* namebuf, int32_t offset) : bmp(bmp),
	namebuf(namebuf), coldata(cdata), palt(pal), offset(offset)
{
	for(auto i = 0; i < pdLEVEL; ++i)
	{
		load_cset(undo,i,i+offset);
	}
	memcpy(undo1, undo, sizeof(undo));
}


void PalEditDialog::updatePal()
{
	(*palt)[dvc(0)]=(*palt)[zc_oldrand()%14+dvc(1)];
	set_palette_range(*palt,dvc(0),dvc(0),false);
}

static size_t paltab = 0;
void PalEditDialog::loadPal()
{
	cset_count = (paltab ? 4 : 13);
	cset_first = (paltab ? 13 : 0) + offset;
	static RGB* ptr = NULL;
	ptr = &(undo1[(paltab ? 13 : 0) * 16]);
	gUndoPal = &ptr;
	for(int32_t i=0; i<cset_count; i++)
	{
		load_cset(*palt,i,i+cset_first);
	}
	for(int32_t i=240; i<256; i++)
	{
		(*palt)[i] = RAMpal[i];
	}
	scare_mouse();
	set_palette(*palt);
	unscare_mouse();
	pendDraw();
}
void cls()
{
	clear_to_color(screen,0xE0);
}

std::shared_ptr<GUI::Widget> PalEditDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	cls();
	bool interpfad = get_bit(quest_rules, qr_FADE);
	loadPal();
	
	return Window(
		title = "Palette Editor",
		onEnter = message::OK,
		onClose = message::CANCEL,
		use_vsync = true,
		onTick = [&]()
		{
			updatePal();
		},
		Column(
			tabpan = TabPanel(
				ptr = &paltab,
				onSwitch = [&](size_t tab)
				{
					loadPal();
				},
				TabRef(name = " 1 ", Rows<17>(
					vAlign = 0.0,
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
					frames[0] = PaletteFrame(colSpan = 16, rowSpan = 13,
						bitmap = bmp, cdata = coldata, palette = palt,
						count = 13, padding = 0_px, onUpdate = [&]()
						{
							loadPal();
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
					vAlign = 0.0,
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
					frames[1] = PaletteFrame(colSpan = 16, rowSpan = 4,
						bitmap = bmp, cdata = coldata+(13*48), palette = palt,
						count = 4, padding = 0_px, onUpdate = [&]()
						{
							loadPal();
						}
					),
					Label(text = "5", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "7", height = 8_spx, textAlign = 1, padding = 0_px),
					Label(text = "8", height = 8_spx, textAlign = 1, padding = 0_px)
				))
			),
			Rows<4>(
				Button(text = "Save",
					minwidth = 90_lpx,
					colSpan = 2,
					onPressFunc = [&]()
					{
						
					},
					onClick = message::OK
				),
				Button(text = "&Undo",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						undo_pal();
						loadPal();
					}
				),
				Button(text = "&Reset",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
						int32_t i = (paltab ? 13 : 0) * 16;
						int32_t end_i = i + (16*cset_count);
						for(; i < end_i; ++i)
						{
							coldata[(i*3)+0] = undo[i].r;
							coldata[(i*3)+1] = undo[i].g;
							coldata[(i*3)+2] = undo[i].b;
						}
						loadPal();
					}
				),
				Label(text = "Name:"),
				TextField(text = namebuf, maxLength = 16, colSpan = 3, fitParent = true)
			),
			Rows<3>(
				topPadding = 0.5_em,
				vAlign = 1.0,
				Button(text = "&Edit",
					minwidth = 90_lpx,
					onPressFunc = [&]()
					{
						int32_t val = tabpan->getCurrentIndex()?13:0;
						val += frames[tabpan->getCurrentIndex()]->getSelection();
						if(edit_dataset(offset + val))
						{
							memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
						}
						cls();
						pendDraw();
						loadPal();
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
					onPressFunc = [&]()
					{
						
					},
					onClick = message::OK
				),
				Button(text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL),
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
		case message::CANCEL:
			for(auto i = 0; i < pdLEVEL*16; ++i)
			{
				coldata[(i*3)+0] = undo[i].r;
				coldata[(i*3)+1] = undo[i].g;
				coldata[(i*3)+2] = undo[i].b;
			}
			return true;
		case message::OK:
		default:
			return true;
	}
}
