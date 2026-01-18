#include "zc_midi_dlg.h"
#include "common.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zc/zelda.h"
#include "midi.h"
#include "base/files.h"
#include <fmt/format.h>
#include "info.h"

extern int paused_midi_pos;
extern byte midi_suspended;

void call_zc_midi_dlg()
{
	bool do_pause_midi = midi_pos >= 0 && currmidi;
	auto restore_midi = currmidi;
	if(do_pause_midi)
	{
		paused_midi_pos = midi_pos;
		stop_midi();
		midi_suspended = midissuspHALTED;
	}
	ZCMidiDlg().show();
	if(do_pause_midi)
	{
		// TODO: this probably doesn't resume midis nicely when scrolling (or in some other inner-gameloop).
		midi_suspended = midissuspRESUME;
		currmidi = restore_midi;
		midi_pos = paused_midi_pos;
	}
}

ZCMidiDlg::ZCMidiDlg()
{
	list_midis.clear();
	bool found_tune = false;
	for(uint q = 0; q < MAXCUSTOMMIDIS; ++q)
	{
		uint idx = q + ZC_MIDI_COUNT;
		if(tunes[idx].data)
		{
			if (!found_tune)
			{
				tune = idx;
				found_tune = true;
			}
			list_midis.add(fmt::format("{} ({:03})", tunes[idx].song_title, q+1), idx);
		}
	}
}

void ZCMidiDlg::refresh_status()
{
	static char zmi_text_buffer[4096] = {0};
	if (unsigned(tune >= MAXMIDIS))
	{
		description->setText("");
		save_btn->setDisabled(true);
	}
	else
	{
		save_btn->setDisabled(tunes[tune].flags&tfDISABLESAVE);
		
		midi_info zmi;
		get_midi_info(tunes[tune].data, &zmi);
		get_midi_text(tunes[tune].data, &zmi, zmi_text_buffer);
		description->setText(zmi_text_buffer);
	}
}

static int scroll_pos;
std::shared_ptr<GUI::Widget> ZCMidiDlg::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	window = Window(
		title = "MIDI Info",
		onClose = message::OK,
		Column(
			Column(
				DropDownList(data = list_midis,
					fitParent = true, maxwidth = 30_em,
					selectedValue = tune,
					onSelectFunc = [&](int32_t val)
					{
						tune = val;
						refresh_status();
						refresh_dlg();
					}),
				ScrollingPane(ptr_y = &scroll_pos, height = 150_px,
					description = Label(width = 300_px, maxwidth = 300_px, maxLines = 500, vAlign = 0.0, hAlign = 0.0, hPadding = 10_px)
				)
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "Listen",
					minwidth = 90_px,
					onClick = message::LISTEN),
				save_btn = Button(
					text = "Save",
					minwidth = 90_px,
					onClick = message::SAVE),
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK)
			)
		)
	);
	refresh_status();
	return window;
}

bool ZCMidiDlg::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::LISTEN:
			jukebox(tune);
			listening = true;
			return false;
		case message::SAVE:
		{
			static EXT_LIST list[] =
			{
				{ (char *)"MIDI files (*.mid)", (char *)"mid" },
				{ NULL,								  NULL }
			};
			if (auto fname = prompt_for_new_file(fmt::format("Save MIDI: {}", tunes[tune].song_title),
				"", list, "tune.mid"))
			{
				if(exists(fname->c_str()))
				{
					if (!alert_confirm("Overwrite?", fmt::format("File '{}' already exists;"
						" overwrite it?", *fname)))
						return false;
				}
				if (save_midi(fname->c_str(), tunes[tune].data) != 0)
					InfoDialog("Error", fmt::format("Error saving MIDI '{}'", *fname)).show();
			}
			return false;
		}
		case message::OK:
			if (listening)
				music_stop();
			return true;
	}
	return false;
}
