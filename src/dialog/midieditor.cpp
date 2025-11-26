#include "midieditor.h"
#include "alert.h"
#include "gui/builder.h"
#include "gui/key.h"
#include "info.h"
#include "zq/zquest.h"
#include "base/files.h"
#include <fmt/format.h>
#include "play_midi.h"

using std::string;
using std::to_string;

extern zctune* customtunes;
void mark_save_dirty();

bool call_midi_editor(int32_t index)
{
	MidiEditorDialog(index).show();
	return true;
}

MidiEditorDialog::MidiEditorDialog(zctune const& ref, int32_t index) :
	index(index), local_midiref(ref), midi_name(local_midiref.title)
{}

MidiEditorDialog::MidiEditorDialog(int32_t index) :
	MidiEditorDialog::MidiEditorDialog(customtunes[index], index)
{}

std::shared_ptr<GUI::Widget> MidiEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	zc_stop_midi();
	get_midi_info(local_midiref.data, &Midi_Info);
	timestring = timestr(Midi_Info.len_sec);
	window = Window(
		use_vsync = true,
		onTick = [&]() {lposition->setText(midi_pos >= 0 ? to_string(midi_pos) : "-1"); return ONTICK_CONTINUE;},
		title = "MIDI Specs",
		info = "Load MIDIs and edit their volumes and looping properties.",
		onClose = message::CANCEL,
		//shortcuts = {},
		Column(
			Row(
				vAlign = 1.0,
				spacing = 1_em,
				Button(
					text = "&Load",
					minwidth = 60_px,
					onClick = message::LOAD),
				Button(
					minwidth = 60_px,
					type = GUI::Button::type::ICON,
					icon = BTNICON_STOPSQUARE,
					onClick = message::STOP),
				Button(
					minwidth = 60_px,
					type = GUI::Button::type::ICON,
					icon = BTNICON_ARROW_RIGHT,
					onClick = message::PLAY),
				Button(
					minwidth = 60_px,
					type = GUI::Button::type::ICON,
					icon = BTNICON_ARROW_RIGHT2,
					onClick = message::FF),
				Button(
					minwidth = 60_px,
					type = GUI::Button::type::ICON,
					icon = BTNICON_ARROW_RIGHT3,
					onClick = message::FFF)
			),
			Rows<4>(vAlign = 1.0,
				Label(text = "MIDI:", rightPadding = 0_px, textAlign = 1.0),
				Label(text = (local_midiref.data) ? "Loaded" : "Invalid", rightPadding = 0_px, textAlign = 1.0, disabled = !(local_midiref.data)),
				Label(text = "Position:", rightPadding = 0_px, textAlign = 1.0),
				lposition = Label(text = "-1", fitParent = true, rightPadding = 0_px, textAlign = 1.0),
				//
				Label(text = "Time:", rightPadding = 0_px, textAlign = 1.0),
				Label(text = timestring, rightPadding = 0_px, textAlign = 1.0),
				Label(text = "Length:", rightPadding = 0_px, textAlign = 1.0),
				Label(text = to_string(Midi_Info.len_beats), rightPadding = 0_px, textAlign = 1.0),
				//
				Label(text = "Name:", rightPadding = 0_px, textAlign = 1.0),
				TextField(
					minwidth = 160_px,
					colSpan = 3,
					type = GUI::TextField::type::TEXT,
					maxLength = 35,
					fitParent = true,
					text = midi_name,
					onValChangedFunc = [&](GUI::TextField::type type, std::string_view text, int32_t)
					{
						midi_name = text;
					}
				),
				//
				Label(text = "Volume:", rightPadding = 0_px, textAlign = 1.0),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					maxLength = 4,
					val = local_midiref.volume,
					onValChangedFunc = [&](GUI::TextField::type type, std::string_view, int32_t val)
					{
						local_midiref.volume = val;
					}
				),
				Label(text = "Loop Start:", rightPadding = 0_px, textAlign = 1.0),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					maxLength = 4,
					val = local_midiref.loop_start,
					onValChangedFunc = [&](GUI::TextField::type type, std::string_view, int32_t val)
					{
						local_midiref.loop_start = val;
					}
				),
				//
				Label(text = "Start:", rightPadding = 0_px, textAlign = 1.0),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					maxLength = 4,
					val = local_midiref.start,
					maxLength = 4,
					onValChangedFunc = [&](GUI::TextField::type type, std::string_view, int32_t val)
					{
						local_midiref.start = val;
					}
				),
				Label(text = "Loop End:", rightPadding = 0_px, textAlign = 1.0),
				TextField(
					type = GUI::TextField::type::INT_DECIMAL,
					maxLength = 4,
					val = local_midiref.loop_end,
					onValChangedFunc = [&](GUI::TextField::type type, std::string_view, int32_t val)
					{
						local_midiref.loop_end = val;
					}
				),
				Checkbox(
					text = "Disable Saving",
					colSpan = 2,
					checked = (local_midiref.flags==1),
					onToggleFunc = [&](bool state)
					{
						local_midiref.flags = state ? 1 : 0;
					}
				),
				Checkbox(
					text = "Loops",
					colSpan = 2,
					checked = (local_midiref.loop==1),
					onToggleFunc = [&](bool state)
					{
						local_midiref.loop = state ? 1 : 0;
					}
				)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool MidiEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::LOAD:
		{
			if (prompt_for_existing_file_compat("Load tune", "mid;nsf", NULL, temppath, true))
			{
				zc_stop_midi();

				if (local_midiref.data != NULL && local_midiref.data != customtunes[index].data)
				{
					destroy_midi((MIDI*)local_midiref.data);
				}

				packfile_password("");

				if ((local_midiref.data = load_midi(temppath)) == NULL)
				{
					InfoDialog("Error!", "Error Loading Tune!").show();
				}
				else
				{
					char* t = get_filename(temppath);
					int32_t j;

					for (j = 0; j < 35 && t[j] != 0 && t[j] != '.'; j++)
					{
						local_midiref.title[j] = t[j];
					}

					local_midiref.title[j] = 0;
					midi_name = local_midiref.title;
				}

				get_midi_info((MIDI*)local_midiref.data, &Midi_Info);
				timestring = timestr(Midi_Info.len_sec);
				rerun_dlg = true;
				return true;
			}
		}
		break;
	case message::STOP:
		{
			zc_stop_midi();
		}
		break;
	case message::PLAY:
		{
			if (midi_pos > 0)
			{
				int32_t pos = midi_pos;
				zc_stop_midi();
				midi_loop_start = -1;
				midi_loop_end = -1;
				zc_play_midi((MIDI*)local_midiref.data, local_midiref.loop);
				zc_set_volume(-1, local_midiref.volume);
				midi_loop_start = local_midiref.loop_start;
				midi_loop_end = local_midiref.loop_end;

				if (midi_loop_end <= 0)
				{
					pos = zc_min(pos + 16, Midi_Info.len_beats);
				}
				else
				{
					pos = zc_min(pos + 16, midi_loop_end);
				}

				if (pos > 0)
				{
					zc_midi_seek(pos);
				}

				break;
			}
		}
		[[fallthrough]];
	case message::FF:
		{
			if (midi_pos > 0)
			{
				int32_t pos = midi_pos;
				zc_stop_midi();
				midi_loop_end = -1;
				midi_loop_start = -1;
				zc_play_midi((MIDI*)local_midiref.data, local_midiref.loop);
				zc_set_volume(-1, local_midiref.volume);
				midi_loop_end = local_midiref.loop_end;
				midi_loop_start = local_midiref.loop_start;

				if (midi_loop_end < 0)
				{
					pos = zc_min(pos + 64, Midi_Info.len_beats);
				}

				else
				{
					pos = zc_min(pos + 64, midi_loop_end);
				}

				if (pos > 0)
				{
					zc_midi_seek(pos);
				}

				break;
			}
		}
		[[fallthrough]];
	case message::FFF:
		{
			int32_t pos = midi_pos;
			zc_stop_midi();
			midi_loop_start = -1;
			midi_loop_end = -1;
			zc_play_midi((MIDI*)local_midiref.data, local_midiref.loop);
			zc_set_volume(-1, local_midiref.volume);
			zc_midi_seek(pos < 0 ? local_midiref.start : pos);
			midi_loop_start = local_midiref.loop_start;
			midi_loop_end = local_midiref.loop_end;
		}
		break;
	case message::OK:
		{
			strncpy(customtunes[index].title, midi_name.c_str(), 37);
			customtunes[index].volume = local_midiref.volume;
			customtunes[index].loop = local_midiref.loop;
			customtunes[index].start = local_midiref.start;
			customtunes[index].loop_start = local_midiref.loop_start;
			customtunes[index].loop_end = local_midiref.loop_end;
			customtunes[index].flags = local_midiref.flags;

			if (local_midiref.data != customtunes[index].data)
			{
				if (customtunes[index].data)
					destroy_midi(customtunes[index].data);
				customtunes[index].data = local_midiref.data;
			}

			mark_save_dirty();
		}
		[[fallthrough]];
	case message::CANCEL:
		{
			zc_stop_midi();
			return true;
		}
	}
	
	return false;
}