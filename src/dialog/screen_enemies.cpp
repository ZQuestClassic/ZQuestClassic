#include "screen_enemies.h"
#include "info_lister.h"
#include "screen_data.h"
#include "enemypattern.h"
#include "gui/key.h"
#include "alert.h"
#include "info.h"
#include "base/zsys.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "zq/zq_class.h"
#include <fmt/format.h>
#include "zq/zquest.h"

extern bool saved;
extern char* guy_string[eMAXGUYS];

using std::string;

void call_screenenemies_dialog()
{
	ScreenEnemiesDialog().show();
}

ScreenEnemiesDialog::ScreenEnemiesDialog() :
	thescr(Map.CurrScr())
{
	memcpy(oldenemy, Map.CurrScr()->enemy, 10 * sizeof(word));
}

void ScreenEnemiesDialog::RebuildList()
{
	list_scr_enemies.clear();

	for (int q=0; q < 10; ++q)
	{
		char const* npcname = guy_string[thescr->enemy[q]];
		std::string name;
		if (thescr->enemy[q] <= 0)
			name = "(None)";
		else
			name = npcname;
		list_scr_enemies.add(name, thescr->enemy[q]);
	}
}

void ScreenEnemiesDialog::UpdatePreview()
{
	std::string copied_name = "(None)\n";
	int32_t enemyID = thescr->enemy[scr_enemies->getSelectedIndex()];
	if (unsigned(copied_enemy_id) < MAXGUYS)
	{
		guydata const& copied_enemy = guysbuf[copied_enemy_id];
		copied_name = fmt::format("{}", guy_string[copied_enemy_id]);
	}
	if (unsigned(enemyID) < MAXGUYS)
	{
		guydata const& enemy = guysbuf[enemyID];
		widgInfo->setText(fmt::format(
			"#{}\nTile: {}\nsTile: {}"
			"\neTile: {}\nHP: {}\nDamage: {}\nW. Damage: {}\nFamily: {}\nDrop: {}\nScript: {}\nW Script: {}"
			"\n\nCopied:\n{}",
			enemyID, enemy.tile, enemy.s_tile, enemy.e_tile, enemy.hp, enemy.dp,
			enemy.wdp, enemy.family, enemy.item_set, enemy.script, enemy.weaponscript, copied_name));
		if (enemyID > 0)
		{
			widgPrev->setDisabled(false);
			if (get_qr(qr_NEWENEMYTILES))
				widgPrev->setTile(enemy.e_tile + efrontfacingtile(enemyID));
			else
				widgPrev->setTile(enemy.tile + efrontfacingtile(enemyID));
			widgPrev->setCSet(enemy.cset & 0xF);
			widgPrev->setFrames(0);
			widgPrev->setSpeed(0);
			widgPrev->setDelay(0);
			widgPrev->setSkipX((enemy.SIZEflags & OVERRIDE_TILE_WIDTH)
				? enemy.txsz - 1 : 0);
			widgPrev->setSkipY((enemy.SIZEflags & OVERRIDE_TILE_HEIGHT)
				? enemy.tysz - 1 : 0);
		}
		else
		{
			widgPrev->setDisabled(true);
			widgPrev->setTile(0);
			widgPrev->setCSet(0);
			widgPrev->setFrames(0);
			widgPrev->setSpeed(0);
			widgPrev->setDelay(0);
			widgPrev->setSkipX(0);
			widgPrev->setSkipY(0);
		}
	}
	widgPrev->setVisible(true);
	widgPrev->setDoSized(true);
	widgPrev->overrideWidth(Size::pixels(48 + 4));
	widgPrev->overrideHeight(Size::pixels(48 + 4));
}

std::shared_ptr<GUI::Widget> ScreenEnemiesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	window = Window(
		use_vsync = true,
		title = "Select Enemies",
		info = "Click the ten spaces to the left to select different enemies to spawn on this screen."
		" You can also access Enemy Flags and set the Spawning Pattern.",
		shortcuts = {
			E = message::PASTEFROMSCREEN,
			GUI::Key::F = message::FLAGS,
			P = message::PATTERN,
			K = message::OK,
			C = message::COPY,
			V = message::PASTE,
			Enter = message::EDIT,
			Del = message::CLEAR,
		},
		onClose = message::CANCEL,
		Row(
			Column(
				scr_enemies = List(fitParent = true, maxwidth = 400_px, vAlign = 0.0,
					data = list_scr_enemies,
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						selectedValue = scr_enemies->getSelectedValue();
						selectedIndex = scr_enemies->getSelectedIndex();
						UpdatePreview();
					},
					onDClick = message::EDIT
				),
				Row(hAlign = 1.0,vAlign = 1.0,topPadding = 0.5_em,spacing = 1_em,
					Button(
						text = Map.CanPaste() ? fmt::format("Past&e (from {}x{:02x})", (Map.CopyScr() >> 8) + 1, Map.CopyScr() & 255) : "Past&e from screen",
						colSpan=2, fitParent=true,
						minwidth = 90_px,
						onClick = message::OK,
						disabled = !Map.CanPaste()
					),
					Button(
						text = "&Flags",
						colSpan = 2, fitParent = true,
						minwidth = 90_px,
						onClick = message::FLAGS
					),
					Button(
						text = "&Pattern",
						colSpan = 2, fitParent = true,
						minwidth = 90_px,
						onClick = message::PATTERN
					)
				),
				Row(
					vAlign = 1.0,
					topPadding = 0.5_em,
					spacing = 1_em,
					Button(
						text = "O&K",
						fitParent = true,
						minwidth = 90_px,
						onClick = message::OK
					),
					Button(
						text = "Cancel",
						fitParent = true,
						minwidth = 90_px,
						onClick = message::CANCEL
					)
				)
			),
			Column(
				widgPrev = TileFrame(visible = false),
				widgInfo = Label(text = "")
			)
		)
	);	
	RebuildList();
	UpdatePreview();
	return window;
}

bool ScreenEnemiesDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	bool refresh = false;
	switch (msg.message)
	{
	case message::COPY:
		copied_enemy_id = thescr->enemy[scr_enemies->getSelectedIndex()];
		UpdatePreview();
		break;
	case message::PASTE:
		if (copied_enemy_id > 0 && copied_enemy_id < eMAXGUYS)
			thescr->enemy[scr_enemies->getSelectedIndex()] = copied_enemy_id;
		RebuildList();
		UpdatePreview();
		refresh = true;
		break;
	case message::EDIT:
	{
		int32_t current_enemy = thescr->enemy[scr_enemies->getSelectedIndex()];
		EnemyListerDialog(current_enemy, true).show();
		if (lister_sel_val != current_enemy)
		{
			thescr->enemy[scr_enemies->getSelectedIndex()] = lister_sel_val;
			RebuildList();
			UpdatePreview();
		}
		refresh = true;
		break;
	}
	case message::CLEAR:
		thescr->enemy[scr_enemies->getSelectedIndex()] = 0;
		RebuildList();
		UpdatePreview();
		refresh = true;
		break;
	case message::PASTEFROMSCREEN:
		Map.DoPasteScreenCommand(PasteCommandType::ScreenEnemies);
		RebuildList();
		UpdatePreview();
		refresh = true;
		break;
	case message::FLAGS:
		call_screendata_dialog(3);
		refresh = true;
		break;
	case message::PATTERN:
		call_enemypattern_dialog();
		refresh = true;
		break;
	case message::OK:
	{
		bool end = false;
		bool confirm = true;
		for (int32_t i = 0; i < 10; i++)
		{
			if (thescr->enemy[i] == 0)
				end = true;
			else if (end)
			{
				AlertDialog("Inactive Enemies!",
					"Enemies won't appear if they're preceded by '(None)' in the list! Continue?",
					[&](bool ret, bool)
					{
						confirm = ret;
					}).show();
			}
		}
		if (!confirm) rerun_dlg = true;
		return true;
	}
	case message::CANCEL:
		memcpy(thescr->enemy, oldenemy, 10 * sizeof(word));
		return true;
	}
	if(refresh) rerun_dlg = true;
	return refresh;
}
