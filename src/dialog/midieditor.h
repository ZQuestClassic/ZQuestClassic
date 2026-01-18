#ifndef ZC_DIALOG_MIDI_EDITOR_H_
#define ZC_DIALOG_MIDI_EDITOR_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/window.h>
#include <gui/text_field.h>
#include <string_view>

bool call_midi_editor(int32_t index);

class MidiEditorDialog : public GUI::Dialog<MidiEditorDialog>
{
public:
	enum class message {
		REFR_INFO, LOAD, STOP, PLAY, FF, FFF, OK, CANCEL
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	MidiEditorDialog(zctune const& ref, int32_t index);
	MidiEditorDialog(int32_t index);
	zctune local_midiref;
	int32_t index;
	string timestring;
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> lposition;
	friend bool call_midi_editor(int32_t index);
};
#endif