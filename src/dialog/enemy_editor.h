#ifndef ZC_DIALOG_ENEMYEDITOR_H
#define ZC_DIALOG_ENEMYEDITOR_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <gui/tileanim_frame.h>
#include <functional>
#include <string_view>
#include <map>

void call_enemy_editor(int32_t index);

class EnemyEditorDialog: public GUI::Dialog<EnemyEditorDialog>
{
public:
	enum class message { OK, CANCEL, ENEMYCLASS };
	

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	EnemyEditorDialog(guydata const& ref, char const* str, int32_t index);
	EnemyEditorDialog(int32_t index);
	void loadEnemyClass();
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TileFrame> animFrame;
	std::string enemyname;
	int32_t index;
	GUI::ListData list_enemies;
	guydata local_enemyref;
	friend void call_enemy_editor(int32_t index);
};

#endif
