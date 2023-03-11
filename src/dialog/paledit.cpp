#include "paledit.h"
#include "info.h"
#include "base/zsys.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "zquest.h"
#include "../gui/use_size.h"
#include "zq_misc.h"

extern int32_t cset_count, cset_first;
extern RGB** gUndoPal;
extern PALETTE pal;
void undo_pal();

static bool DidCS9 = false;

bool call_paledit_dlg(char* namebuf, byte* cdata, PALETTE *pal, int32_t offset, int32_t index)
{
	BITMAP* tmp = create_bitmap_ex(8,192_px,156_px);
	clear_bitmap(tmp);
	for(auto pos = 0; pos < 208; ++pos)
	{
		rectfill(tmp, (pos%16)*12_px, (pos/16)*12_px, ((pos%16)*12_px)+12_px-1_px, ((pos/16)*12_px)+(12_px-1), pos);
	}
	DidCS9 = false;
	PalEditDialog(tmp, cdata, pal, namebuf, offset, index).show();
	destroy_bitmap(tmp);
	return DidCS9;
}

PalEditDialog::PalEditDialog(BITMAP* bmp, byte* cdata, PALETTE* pal, char* namebuf, int32_t offset, int32_t index) : bmp(bmp),
	namebuf(namebuf), coldata(cdata), palt(pal), offset(offset), index(index)
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
		onClose = message::CANCEL,
		use_vsync = true,
		onTick = [&]()
		{
			cyclebutton->setDisabled(interpfad?(index>=256):(paltab == 1));
			updatePal();
			return false;
		},
		Column(
			tabpan = TabPanel(
				ptr = &paltab,
				onSwitch = [&](size_t,size_t)
				{
					loadPal();
				},
				TabRef(name = " 1 ", Rows<17>(
					vAlign = 0.0,
					DummyWidget(padding = 0_px),
					Label(text = "0", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "1", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "2", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "3", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "4", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "5", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "6", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "7", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "8", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "9", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "A", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "B", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "C", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "D", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "E", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "F", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "2", height = 12_px, textAlign = 1, padding = 0_px),
					frames[0] = PaletteFrame(colSpan = 16, rowSpan = 13,
						bitmap = bmp, cdata = coldata, palette = palt,
						count = 13, padding = 0_px, onUpdate = [&]()
						{
							loadPal();
						}
					),
					Label(text = "3", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "4", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "9", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "2", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "0" : "3", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "1" : "4", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "2" : "2", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "3" : "3", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "4" : "4", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "5" : "2", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "6" : "3", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = interpfad ? "7" : "4", height = 12_px, textAlign = 1, padding = 0_px)
				)),
				TabRef(name = " 2 ", Rows<17>(
					vAlign = 0.0,
					DummyWidget(padding = 0_px),
					Label(text = "0", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "1", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "2", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "3", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "4", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "5", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "6", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "7", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "8", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "9", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "A", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "B", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "C", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "D", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "E", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "F", width = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "1", height = 12_px, textAlign = 1, padding = 0_px),
					frames[1] = PaletteFrame(colSpan = 16, rowSpan = 4,
						bitmap = bmp, cdata = coldata+(13*48), palette = palt,
						count = 4, padding = 0_px, onUpdate = [&]()
						{
							loadPal();
						}
					),
					Label(text = "5", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "7", height = 12_px, textAlign = 1, padding = 0_px),
					Label(text = "8", height = 12_px, textAlign = 1, padding = 0_px)
				))
			),
			Rows<4>(
				Button(text = "Save",
					minwidth = 90_px,
					colSpan = 2,
					onPressFunc = [&]()
					{
						if(getname("Save Palette (.png)","png",NULL,datapath,false))
						{
							if (paltab == 0)
							{
								char name[13];
								extract_name(temppath,name,FILENAME8_3);
								save_bitmap(temppath, bmp, *palt);
							}
							else
							{
								BITMAP* tmp2 = create_bitmap_ex(8,192_px,48_px);
								clear_bitmap(tmp2);
								for(auto pos = 0; pos < 64; ++pos)
								{
									rectfill(tmp2, (pos%16)*12_px, (pos/16)*12_px, ((pos%16)*12_px)+12_px-1_px, ((pos/16)*12_px)+(12_px-1), pos);
								}
								char name[13];
								extract_name(temppath,name,FILENAME8_3);
								save_bitmap(temppath, tmp2, *palt);
								destroy_bitmap(tmp2);
							}
						}
					}
				),
				Button(text = "&Undo",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						undo_pal();
						loadPal();
					}
				),
				Button(text = "&Reset",
					minwidth = 90_px,
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
				TextField(
					type = GUI::TextField::type::TEXT,
					text = std::string(namebuf), 
					maxLength = 16, 
					colSpan = 3, 
					fitParent = true,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view text,int32_t)
					{
						std::string foo;
						foo.assign(text);
						strncpy(palnames[index], foo.c_str(), 16);
					}
				)
			),
			Rows<3>(
				topPadding = 0.5_em,
				vAlign = 1.0,
				Button(text = "&Edit",
					minwidth = 90_px,
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
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						int32_t val = tabpan->getCurrentIndex()?13:0;
						val += frames[tabpan->getCurrentIndex()]->getSelection();
						if(grab_dataset(offset + val))
						{
							memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
						}
						cls();
						pendDraw();
						loadPal();
					}
				),
				cyclebutton = Button(text = (get_bit(quest_rules,qr_FADE))?"Cycle":"Auto Dark",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						if(!get_bit(quest_rules,qr_FADE))
						{
							calc_dark(offset);
							memcpy(*gUndoPal, pal, sizeof(RGB)*16*cset_count);
							cls();
							pendDraw();
							loadPal();
						}
						else
						{
							if ( index < 256 ) //don't display cycle data for palettes 256 through 511. They don't have valid cycle data. 
								edit_cycles(index);
							else jwin_alert("Notice","Palettes above 0xFF do not have Palette Cycles",NULL,NULL,"O&K",NULL,'k',0,lfont);
						}
					}
				),
				Button(text = "&Preview in CS 9",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						int32_t val = tabpan->getCurrentIndex()?13:0;
						val += frames[tabpan->getCurrentIndex()]->getSelection();
						load_cset(RAMpal,9,offset + val);
						InfoDialog("CSet 9 Preview","The selected cset has been temporarily copied to ZQuest's CSet 9."
							" \nThis feature is useful if you want to preview or edit how a tile would look in the selected CSet."
							" \nThis effect is temporary and will be reverted when you load a different palette,"
							" such as by changing screens in ZQuest to a screen with a different palette.").show();
						set_pal();
						DidCS9 = true;
					},
					onClick = message::OK
				),
				Button(text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL),
				Button(text = "Done",
					minwidth = 90_px,
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
