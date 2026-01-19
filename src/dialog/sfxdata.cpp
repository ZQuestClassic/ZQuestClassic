#include "sfxdata.h"
#include "gui/key.h"
#include "info.h"
#include "gui/builder.h"
#include "base/files.h"
#include "sfx.h"
#include "play_midi.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include "zc/zelda.h"
#include <zalleg/zalleg.h>
#include <fmt/format.h>

void mark_save_dirty();

bool call_sfxdata_dialog(int32_t index)
{
	if (unsigned(index-1) > quest_sounds.size())
		return false;
	SFXDataDialog(index).show();
	return true;
}

SFXDataDialog::SFXDataDialog(int32_t index) : index(index),
	local_ref(ZCSFX())
{
	if (unsigned(index-1) < quest_sounds.size())
		local_ref = quest_sounds[index-1];
	else
		index = quest_sounds.size()+1;
	
	// Don't know why these are here. . . maybe to clean up if tunes were playing?
	kill_sfx();
	zc_stop_midi();
	zc_set_volume(255, -1);
}

std::shared_ptr<GUI::Widget> SFXDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	shared_ptr<GUI::Grid> left_grid = Rows<4>(vAlign = 1.0);
	shared_ptr<GUI::Widget> bottom_widg;
	if (sound_was_installed)
	{
		bottom_widg = DummyWidget();
		// Default doesn't REQUIRE sound, but feels weird without save/load/play options
		left_grid->add(Button(fitParent = true, minwidth = 90_px,
			text = "Default", onClick = message::DEFAULT));
		left_grid->add(INFOBTN("Clears the SFX to default."));
		left_grid->add(DummyWidget(colSpan = 2));
		// save/load requires sound
		left_grid->add(Button(fitParent = true, disabled = local_ref.is_invalid(),
			text = "Save", minwidth = 90_px, onClick = message::SAVE));
		left_grid->add(INFOBTN("Save the SFX as a '.wav' / '.ogg' file."
			"\nCurrently, converting from '.wav' to '.ogg' is not supported."));
		left_grid->add(Button(fitParent = true, minwidth = 90_px,
			text = "Load", onClick = message::LOAD));
		left_grid->add(INFOBTN("Load an SFX from a '.wav' / '.ogg' file."
			"\n'.ogg' format is recommended, as it is much smaller, and thus loads faster / takes up less space in the quest file."));
		// playing sound requires sound
		left_grid->add(Button(fitParent = true, disabled = local_ref.is_invalid(),
			text = "Play", minwidth = 90_px, onClick = message::PLAY));
		left_grid->add(INFOBTN("Play a preview of the sound (at '128' volume)."));
		left_grid->add(btn_stop = Button(colSpan = 2, fitParent = true, disabled = true,
			text = "Stop", minwidth = 90_px, onClick = message::STOP));
		//
		left_grid->add(Button(colSpan = 2, fitParent = true, focused = true,
			text = "Ok", minwidth = 90_px, onClick = message::OK));
		left_grid->add(Button(colSpan = 2, fitParent = true, minwidth = 90_px,
			text = "Cancel", onClick = message::CANCEL));
	}
	else
	{
		left_grid->add(Label(fitParent = true,
			text = "Sound cannot be saved/loaded/played\nwhile 'nosound' setting is enabled."
			"\nRe-launch the editor with\nsound enabled to modify sounds."));
		bottom_widg = Row(
			Button(colSpan = 2, fitParent = true, focused = true,
				text = "Ok", minwidth = 90_px, onClick = message::OK),
			Button(colSpan = 2, fitParent = true, minwidth = 90_px,
				text = "Cancel", onClick = message::CANCEL)
		);
	}
	
	string sfx_info = local_ref.get_sound_info();
	string titlebuf = fmt::format("SFX {}: {}", index, local_ref.sfx_name);
	window = Window(
		use_vsync = true,
		onTick = [&]()
		{
			if (local_ref.is_allocated())
			{
				local_ref.cleanup();
				if (!local_ref.is_allocated())
					btn_stop->setDisabled(true);
			}
			return ONTICK_CONTINUE;
		},
		title = titlebuf,
		info = "Save and Load SFX Data for use in engine.",
		onClose = message::CANCEL,
		Column(
			Column(
				Row(
					Label(text = "Name", rightPadding = 0_px, textAlign = 1.0), 
					TextField(
						type = GUI::TextField::type::TEXT,
						width = 300_px, height = 4_px+(3*(1_em+2_px)),
						maxLength = 255,
						fitParent = true,
						text = local_ref.sfx_name,
						onValChangedFunc = [&](GUI::TextField::type type, std::string_view text, int32_t)
						{
							local_ref.sfx_name = text;
						}
					)
				)
			),
			Row(
				left_grid,
				Label(minwidth = 6_em, fitParent = true, text = sfx_info)
			),
			bottom_widg
		)
	);
	return window;
}

ZCSFX make_default_sfx(size_t index);
bool SFXDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::SAVE:
		{
			temppath[0] = 0;
			string tempname = local_ref.sfx_name;
			//change spaces to dashes for f/s safety
			for (size_t q = 0; q < tempname.size(); ++q)
			{
				if (tempname[q] == ' ' || tempname[q] == '\\' || tempname[q] == '/')
					tempname[q] = '-';
			}

			strcpy(temppath, tempname.c_str());

			//save
			if (local_ref.is_invalid())
				displayinfo("Error!", "Cannot save an empty slot!");
			else
			{
				switch (local_ref.get_sample_type())
				{
					default:
					case SMPL_WAV:
						strcat(temppath, ".wav");
						break;
					case SMPL_OGG:
						strcat(temppath, ".ogg");
						break;
				}
				if (prompt_for_new_file_compat("Save SFX file", "wav;ogg", NULL, temppath, true))
				{
					try
					{
						local_ref.save_sound(temppath);
						displayinfo("Success!", fmt::format("Saved SFX file\n{}", temppath));
					}
					catch (zcsfx_io_exception &e)
					{
						displayinfo("Error", e.what());
					}
				}
			}
			
			break;
		}
		case message::LOAD:
		{
			if (prompt_for_existing_file_compat("Open SFX file", "wav;ogg", NULL, temppath, true))
			{
				try
				{
					local_ref.load_file(temppath);
					rerun_dlg = true;
					return true;
				}
				catch (zcsfx_io_exception &e)
				{
					displayinfo("Error", e.what());
				}
				return false;
			}
			break;
		}
		case message::DEFAULT:
			if (!alert_confirm("Reset to default?", "Reset this sound to default settings?"))
				return false;
			local_ref.stop();
			local_ref = make_default_sfx(index);
			rerun_dlg = true;
			return true;
		case message::PLAY:
			local_ref.play(128, false);
			btn_stop->setDisabled(false);
			break;
		case message::STOP:
			local_ref.stop();
			btn_stop->setDisabled(true);
			break;
		case message::OK:
			mark_save_dirty();
			local_ref.stop();
			if (index == quest_sounds.size() + 1)
				quest_sounds.emplace_back(std::move(local_ref));
			else quest_sounds[index-1] = std::move(local_ref);
			return true;
		case message::CANCEL:
			local_ref.cleanup_memory();
			return true;
	}
	return false;
}
