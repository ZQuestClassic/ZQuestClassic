#include "enemy_editor.h"
#include "info.h"
#include "alert.h"
#include "../zsys.h"
#include <gui/builder.h>

extern char *guy_string[];
extern guydata *guysbuf;
extern zcmodule moduledata;
extern bool saved;

void call_enemy_editor(int32_t index)
{
	EnemyEditorDialog(index).show();
}

EnemyEditorDialog::EnemyEditorDialog(guydata const& ref, char const* str, int32_t index):
	local_enemyref(ref), enemyname(str), index(index),
	list_enemies(GUI::ListData::enemyclass())
{}

EnemyEditorDialog::EnemyEditorDialog(int32_t index):
	EnemyEditorDialog(guysbuf[index], guy_string[index], index)
{}

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
					Rows<2>(padding = 0_px,
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
						Label(text = "Type:"),
						Row(
							height = sized(16_px, 21_px),
							DropDownList(data = list_enemies,
								fitParent = true, padding = 0_px,
								selectedValue = local_enemyref.family,
								onSelectionChanged = message::ENEMYCLASS
							),
							Button(width = 1.5_em, padding = 0_px, text = "?", hAlign = 1.0, onPressFunc = [&]()
							{
								/*InfoDialog(ZI.getEnemyClassName(local_enemyref.family),
									ZI.getEnemyClassHelp(local_enemyref.family)
								).show();*/
							})
						)
					)
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
			return true;

		case message::CANCEL:
		default:
			return true;
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