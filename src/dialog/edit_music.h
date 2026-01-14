#ifndef ZC_DIALOG_EDITDMAPDLG_H_
#define ZC_DIALOG_EDITDMAPDLG_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/drop_down_list.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <array>
#include "advanced_music.h"

void call_edit_music_dialog(size_t idx);

class EditMusicDialog : public GUI::Dialog<EditMusicDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	EditMusicDialog(size_t idx);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bool disableEnhancedMusic(bool disableontracker = false);
	bool disableMusicTracks();
	void silenceMusicPreview();
	void musicPreview(bool previewloop = false);
	void midiPreview();
	void update_pause_btn();
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> field;
	std::shared_ptr<GUI::DropDownList> track_list;
	std::shared_ptr<GUI::TextField> start_field;
	std::shared_ptr<GUI::TextField> end_field;
	std::shared_ptr<GUI::TextField> xfadein_field;
	std::shared_ptr<GUI::TextField> xfadeout_field;
	std::shared_ptr<GUI::Label> progress_lbl;
	std::shared_ptr<GUI::Button> previewstop_btn;
	
	int32_t musicpreview_saved = 0;
	bool is_playing = false;

	size_t music_slot;
	AdvancedMusic local_music;

	GUI::ListData list_midis, list_tracks;
};

#endif
