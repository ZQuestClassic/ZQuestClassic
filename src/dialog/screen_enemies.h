#ifndef ZC_DIALOG_SCREEN_ENEMIES_H_
#define ZC_DIALOG_SCREEN_ENEMIES_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/list.h>
#include <gui/list_data.h>
#include <gui/window.h>
#include <zq/gui/tileanim_frame.h>
#include <initializer_list>
#include <string>


void call_screenenemies_dialog();

class ScreenEnemiesDialog : public GUI::Dialog<ScreenEnemiesDialog>
{
public:
	enum class message
	{
		REFR_INFO, COPY, PASTE, EDIT, CLEAR, PASTEFROMSCREEN, FLAGS, PATTERN, OK, CANCEL
	};

	void RebuildList();
	void UpdatePreview();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	ScreenEnemiesDialog();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::List> scr_enemies;
	std::shared_ptr<GUI::Label> widgInfo;
	std::shared_ptr<GUI::TileFrame> widgPrev;

	int32_t copied_enemy_id=0;
	int32_t last_enemy;
	mapscr* thescr;
	bool focus_list;
	word oldenemy[10];
	GUI::ListData list_scr_enemies, list_patterns;
	

	friend void call_screenenemies_dialog();
};
#endif
