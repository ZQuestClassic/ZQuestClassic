#include "enemy_editor.h"
#include "info.h"
#include "alert.h"
#include "../zsys.h"
#include "../zquest.h"
#include "../colors.h"
#include <gui/builder.h>

extern char *guy_string[];
extern guydata *guysbuf;
extern zcmodule moduledata;
extern bool saved;
extern void load_cset(RGB *pal,int32_t cset_index,int32_t dataset);

void call_enemy_editor(int32_t index)
{
	EnemyEditorDialog(index).show();
}

EnemyEditorDialog::EnemyEditorDialog(guydata const& ref, char const* str, int32_t index):
	local_enemyref(ref), enemyname(str), index(index),
	list_enemies(GUI::ListData::enemyclass()),
	list_weapons(GUI::ListData::enemyweapons()),
	list_anim(GUI::ListData::enemyanim()),
	list_itemsets(GUI::ListData::itemsets())
{
	get_palette(oldpal);
}

EnemyEditorDialog::EnemyEditorDialog(int32_t index):
	EnemyEditorDialog(guysbuf[index], guy_string[index], index)
{}

static size_t enmtabs[4] = {0, 0, 0, 0};

//{ Macros

#define ACTION_FIELD_WID 2_em
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em

#define WH_FIELD(txt, mem) \
Label(textAlign=2,padding = 0_px,text=txt), \
TextField( \
	padding = 0_px, forceFitW = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	high = 20, val = local_enemyref.mem, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_enemyref.mem = val; \
	} \
)

#define ENM_ATTRIBUTE(ind) \
l_attributes[ind] = Label(minwidth = ATTR_LAB_WID, textAlign = 2), \
ib_attributes[ind] = Button(forceFitH = true, text = "?", \
	disabled = true, \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Attribute Info",h_attribute[ind]).show(); \
	}), \
TextField( \
	fitParent = true, minwidth = 8_em, \
	type = GUI::TextField::type::SWAP_ZSINT, \
	val = local_enemyref.guymisc[ind], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_enemyref.guymisc[ind] = val; \
	})

//}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	char titlebuf[256];
	sprintf(titlebuf, "Enemy Editor (%d): %s", index, enemyname.c_str());
	if(is_large)
	{
		window = Window(
			use_vsync = true,
			title = titlebuf,
			info = "Edit enemies.",
			onEnter = message::OK,
			onClose = message::CANCEL,
			Column(
				Row(
					Rows<3>(padding = 0_px,
						Label(text = "Name:"),
						TextField(
							fitParent = true,
							maxLength = 63,
							text = enemyname,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
							{
								enemyname = str;
								char buf[256];
								sprintf(buf, "Enemy Editor (%d): %s", index, enemyname.c_str());
								window->setTitle(buf);
							}
						),
						DummyWidget(),
						Label(text = "Type:"),
						DropDownList(data = list_enemies,
							fitParent = true, padding = 0_px,
							selectedValue = local_enemyref.family,
							onSelectionChanged = message::ENEMYCLASS
						),
						Button(width = 1.5_em, forceFitH = true, padding = 0_px, text = "?", hAlign = 1.0, onPressFunc = [&]()
						{
							/*InfoDialog(ZI.getEnemyClassName(local_enemyref.family),
								ZI.getEnemyClassHelp(local_enemyref.family)
							).show();*/
						})
					)
				),
				TabPanel(
					ptr = &enmtabs[0],
					TabRef(name = "Basic", TabPanel(
						ptr = &enmtabs[1],
						TabRef(name = "Data", Column(
							Row(
								Column(
									Rows<7>(
										DummyWidget(),
										DummyWidget(),
										Label(textAlign=1,text="Old"),
										DummyWidget(),
										Label(textAlign=1,text="Spec."),
										DummyWidget(),
										Label(textAlign=1,text="New"),
										Button(text = "?", height = 24_lpx,
										onPressFunc = [&]()
										{
											InfoDialog("Tile info",
												"The tiles used by the enemy. Which tile is used and when depends on the circumstances.\n"
												"The 'Old' tile is only used if 'Use New Enemy Tiles' is disabled, mainly used for more NES-styled animation.\n"
												"The 'New' tile is used if 'Use New Enemy Tiles' is enabled, used for more modern animation.\n"
												"The 'Spec.' tile is used in special circumstances, such as broken shield tiles for walking enemies, Gleeok neck tiles, etc.").show();
										}),
										DummyWidget(),
										oldtile = SelTileSwatch(
											tile = local_enemyref.tile,
											cset = ((local_enemyref.cset & 0x0F) == 14 ? 13 : (local_enemyref.cset & 0x0F)),
											fakecs14 = true,
											showvals = false,
											padding = 0_px,
											onSelectFunc = [&](int32_t t, int32_t c)
											{
												local_enemyref.tile = t;
												updateCSet(c);
												c = (c == 13 ? 14 : c);
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= c&0x0F;
											}
										),
										DummyWidget(),
										specialtile = SelTileSwatch(
											tile = local_enemyref.s_tile,
											cset = ((local_enemyref.cset & 0x0F) == 14 ? 13 : (local_enemyref.cset & 0x0F)),
											fakecs14 = true,
											showvals = false,
											padding = 0_px,
											onSelectFunc = [&](int32_t t, int32_t c)
											{
												local_enemyref.s_tile = t;
												updateCSet(c);
												c = (c == 13 ? 14 : c);
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= c&0x0F;
											}
										),
										DummyWidget(),
										newtile = SelTileSwatch(
											tile = local_enemyref.e_tile,
											cset = ((local_enemyref.cset & 0x0F) == 14 ? 13 : (local_enemyref.cset & 0x0F)),
											fakecs14 = true,
											showvals = false,
											padding = 0_px,
											onSelectFunc = [&](int32_t t, int32_t c)
											{
												local_enemyref.e_tile = t;
												updateCSet(c);
												c = (c == 13 ? 14 : c);
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= c&0x0F;
											}
										),
										Button(text = "?", rowSpan = 2, height = 24_lpx, topPadding = 10_lpx,
										onPressFunc = [&]()
										{
											InfoDialog("Width and Height",
												"The width and height of the protected tile block for Old, Special, and New animation, respectively.\n"
												"The protected tile block will give a warning whenever you attempt to copy over or delete the block.\n"
												"Max width is 20, max height is 20.").show();
										}),
										WH_FIELD("W:", width),
										WH_FIELD("W:", s_width),
										WH_FIELD("W:", e_width),
										WH_FIELD("H:", height),
										WH_FIELD("H:", s_height),
										WH_FIELD("H:", e_height)
									)
								),
								Column(
									Rows<4>(
										Label(text="Health:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 32767,
											val = local_enemyref.hp,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.hp = val;
											}
										),
										Label(text="Random Rate:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 16,
											val = local_enemyref.rate,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.rate = val;
											}
										),
										Label(text="Cnt. Damage:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 32767,
											val = local_enemyref.dp,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.dp = val;
											}
										),
										Label(text="Halt Rate:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 16,
											val = local_enemyref.hrate,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.hrate = val;
											}
										),
										Label(text="Wpn. Damage:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 32767,
											val = local_enemyref.wdp,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.wdp = val;
											}
										),
										Label(text="Homing Factor:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -255,
											high = 255,
											val = local_enemyref.homing,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.homing = val;
											}
										),
										Label(text="Hunger:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -4,
											high = 4,
											val = local_enemyref.grumble,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.grumble = val;
											}
										),
										Label(text="Step Speed:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 1000,
											val = local_enemyref.step,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.step = val;
											}
										)
									)
								)
							),
							Row
							(
								Columns<4>
								(
									Label(text="Weapon:", hAlign = 1.0),
									Label(text="Item Dropset:", hAlign = 1.0),
									Label(text="Old Animation:", hAlign = 1.0),
									Label(text="New Animation:", hAlign = 1.0),
									DropDownList(
										fitParent = true,
										data = list_weapons,
										selectedValue = local_enemyref.weapon,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.weapon = val;
										}
									),
									DropDownList(
										fitParent = true,
										data = list_itemsets,
										selectedValue = local_enemyref.item_set,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.item_set = val;
										}
									),
									DropDownList(
										fitParent = true,
										disabled = get_bit(quest_rules, qr_NEWENEMYTILES),
										data = list_anim,
										selectedValue = local_enemyref.anim,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.anim = val;
										}
									),
									DropDownList(
										fitParent = true,
										disabled = !get_bit(quest_rules, qr_NEWENEMYTILES),
										data = list_anim,
										selectedValue = local_enemyref.e_anim,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.e_anim = val;
										}
									),
									palbox = Checkbox(
										text = "Use Boss Pal CSet", leftPadding = 8_px, hAlign = 0.0,
										checked = ((local_enemyref.cset & 0x0F) == 14),
										colSpan = 2,
										boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
										onToggleFunc = [&](bool state)
										{
											if (state)
											{
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= 14&0x0F;
												updateCSet(13);
												paltext->setDisabled(false);
											}
											else
											{
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= 8&0x0F;
												updateCSet(8);
												paltext->setDisabled(true);
											}
										}
									),
									Label(text="Boss Palette CSet:", leftPadding = 8_px, hAlign = 1.0),
									Label(text="Old Frame Rate:", leftPadding = 8_px, hAlign = 1.0),
									Label(text="New Frame Rate:", leftPadding = 8_px, hAlign = 1.0),
									paltext = TextField(
										fitParent = true,
										disabled = (!((local_enemyref.cset & 0x0F) == 14)),
										type = GUI::TextField::type::INT_DECIMAL,
										low = -1,
										high = 29,
										val = local_enemyref.bosspal,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_enemyref.bosspal = val;
											if(val>-1)
											{
												load_cset(RAMpal,csBOSS-1,pSprite(val));
												set_palette(RAMpal);
											}
											else 
											{
												load_cset(RAMpal,csBOSS-1,csBOSS);
												set_palette(RAMpal);
											}
										}
									),
									TextField(
										fitParent = true,
										type = GUI::TextField::type::INT_DECIMAL,
										disabled = get_bit(quest_rules, qr_NEWENEMYTILES),
										high = 256,
										val = local_enemyref.frate,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_enemyref.frate = val;
										}
									),
									TextField(
										fitParent = true,
										type = GUI::TextField::type::INT_DECIMAL,
										disabled = (!get_bit(quest_rules, qr_NEWENEMYTILES)),
										high = 256,
										val = local_enemyref.e_frate,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_enemyref.e_frate = val;
										}
									)
								)
							)
						)),
						TabRef(name = "Attributes", DummyWidget()
						
						)
					))
				),		
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
	}
	
	if (local_enemyref.cset == 14 && local_enemyref.bosspal>-1)
	{
		load_cset(RAMpal,csBOSS-1,pSprite(local_enemyref.bosspal));
		updateCSet(13);
	}
	else 
	{
		load_cset(RAMpal,csBOSS-1,csBOSS);
	}
	set_palette(RAMpal);
	loadEnemyClass();
	return window;
}

bool EnemyEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::ENEMYCLASS:
		{
			local_enemyref.family = int32_t(msg.argument);
			loadEnemyClass();
			return false;
		}

		case message::OK:
			saved = false;
			guysbuf[index] = local_enemyref;
			strcpy(guy_string[index], enemyname.c_str());
			[[fallthrough]];

		case message::CANCEL:
		default:
			memcpy(RAMpal, oldpal, sizeof(oldpal));
			set_palette(RAMpal);
			return true;
	}
}

void EnemyEditorDialog::updateCSet(int32_t cset)
{
	oldtile->setCSet(cset);
	specialtile->setCSet(cset);
	newtile->setCSet(cset);
	if (cset != 13) 
	{
		palbox->setChecked(false);
		paltext->setDisabled(true);
	}
}

void EnemyEditorDialog::loadEnemyClass()
{
	/*
	ItemNameInfo inf;
	loadinfo(&inf, local_itemref);
	
	#define __SET(obj, mem) \
	l_##obj->setText(inf.mem.size() ? inf.mem : defInfo.mem); \
	h_##obj = (inf.h_##mem.size() ? inf.h_##mem : ""); \
	if(ib_##obj) \
		ib_##obj->setDisabled(h_##obj.empty());
	
	__SET(power, power);
	
	__SET(attribs[0], misc[0]);
	__SET(attribs[1], misc[1]);
	__SET(attribs[2], misc[2]);
	__SET(attribs[3], misc[3]);
	__SET(attribs[4], misc[4]);
	__SET(attribs[5], misc[5]);
	__SET(attribs[6], misc[6]);
	__SET(attribs[7], misc[7]);
	__SET(attribs[8], misc[8]);
	__SET(attribs[9], misc[9]);
	
	__SET(flags[0], flag[0]);
	__SET(flags[1], flag[1]);
	__SET(flags[2], flag[2]);
	__SET(flags[3], flag[3]);
	__SET(flags[4], flag[4]);
	__SET(flags[5], flag[5]);
	__SET(flags[6], flag[6]);
	__SET(flags[7], flag[7]);
	__SET(flags[8], flag[8]);
	__SET(flags[9], flag[9]);
	__SET(flags[10], flag[10]);
	__SET(flags[11], flag[11]);
	__SET(flags[12], flag[12]);
	__SET(flags[13], flag[13]);
	__SET(flags[14], flag[14]);
	__SET(flags[15], flag[15]);
	
	__SET(sfx[0], actionsnd[0]);
	__SET(sfx[1], actionsnd[1]);
	
	__SET(spr[0], wpn[0]);
	__SET(spr[1], wpn[1]);
	__SET(spr[2], wpn[2]);
	__SET(spr[3], wpn[3]);
	__SET(spr[4], wpn[4]);
	__SET(spr[5], wpn[5]);
	__SET(spr[6], wpn[6]);
	__SET(spr[7], wpn[7]);
	__SET(spr[8], wpn[8]);
	__SET(spr[9], wpn[9]);
	#undef __SET
	*/
}