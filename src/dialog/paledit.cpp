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
static BITMAP* colbmp = nullptr;
bool call_paledit_dlg(char* namebuf, byte* cdata, PALETTE *pal, int32_t offset, int32_t index)
{
	if(!colbmp)
	{
		colbmp = create_bitmap_ex(8,192,156);
		clear_bitmap(colbmp);
		for(auto pos = 0; pos < 208; ++pos)
		{
			rectfill(colbmp, (pos%16)*12, (pos/16)*12, ((pos%16)*12)+12-1, ((pos/16)*12)+12-1, pos);
		}
	}
	DidCS9 = false;
	PalEditDialog(cdata, pal, namebuf, offset, index).show();
	return DidCS9;
}

PalEditDialog::PalEditDialog(byte* cdata, PALETTE* pal, char* namebuf, int32_t offset, int32_t index) :
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
	
	const Size panel_scale = 16_px;
	cls();
	bool interpfad = get_bit(quest_rules, qr_FADE);
	loadPal();
	ALLEGRO_FONT* lblf = get_zc_font_a5(font_lfont_l);
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
					Label(text = "0", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "1", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "2", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "3", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "4", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "5", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "6", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "7", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "8", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "9", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "A", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "B", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "C", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "D", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "E", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "F", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "2", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					frames[0] = PaletteFrame(colSpan = 16, rowSpan = 13,
						cdata = coldata, palette = palt, scale = panel_scale,
						count = 13, padding = 0_px, onUpdate = [&]()
						{
							loadPal();
						}
					),
					Label(text = "3", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "4", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "9", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "2", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "0" : "3", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "1" : "4", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "2" : "2", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "3" : "3", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "4" : "4", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "5" : "2", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "6" : "3", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = interpfad ? "7" : "4", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf)
				)),
				TabRef(name = " 2 ", Rows<17>(
					vAlign = 0.0,
					DummyWidget(padding = 0_px),
					Label(text = "0", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "1", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "2", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "3", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "4", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "5", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "6", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "7", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "8", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "9", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "A", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "B", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "C", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "D", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "E", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "F", width = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "1", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					frames[1] = PaletteFrame(colSpan = 16, rowSpan = 4,
						cdata = coldata+(13*48), palette = palt, scale = panel_scale,
						count = 4, padding = 0_px, onUpdate = [&]()
						{
							loadPal();
						}
					),
					Label(text = "5", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "7", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf),
					Label(text = "8", height = panel_scale, textAlign = 1, padding = 0_px, useFont_a5 = lblf)
				))
			),
			Rows<3>(
				Button(text = "Save",
					minwidth = 90_px, fitParent = true,
					onPressFunc = [&]()
					{
						if(getname("Save Palette (.png)","png",NULL,datapath,false))
						{
							if (paltab == 0)
							{
								char name[13];
								extract_name(temppath,name,FILENAME8_3);
								save_bitmap(temppath, colbmp, *palt);
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
					minwidth = 90_px, fitParent = true,
					onPressFunc = [&]()
					{
						undo_pal();
						loadPal();
					}
				),
				Button(text = "&Reset",
					minwidth = 90_px, fitParent = true,
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
				Row(colSpan = 3,
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
				Button(text = "&Edit",
					minwidth = 90_px, fitParent = true,
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
					minwidth = 90_px, fitParent = true,
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
					minwidth = 90_px, fitParent = true,
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
					minwidth = 90_px, fitParent = true,
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
					minwidth = 90_px, fitParent = true,
					onClick = message::CANCEL),
				Button(text = "Done",
					minwidth = 90_px, fitParent = true,
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
