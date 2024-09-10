#ifndef ZC_DIALOG_ENEMY_PATTERN_H_
#define ZC_DIALOG_ENEMY_PATTERN_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <gui/list.h>

void call_enemypattern_dialog();

class EnemyPatternDialog : public GUI::Dialog<EnemyPatternDialog>
{
public:
    enum class message
    {
        REFR_INFO, OK, CANCEL
    };

    std::shared_ptr<GUI::Widget> view() override;
    bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
    EnemyPatternDialog(mapscr* scr);

    std::shared_ptr<GUI::Window> window;
    std::shared_ptr<GUI::List> scr_pattern;

    int32_t last_pattern;
    mapscr* thescr;
    GUI::ListData list_patterns;

    friend void call_enemypattern_dialog();
};

#endif