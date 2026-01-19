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
	if (!index) return false;
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
	
	kill_sfx();
	zc_stop_midi();
	zc_set_volume(255, -1);
}

std::shared_ptr<GUI::Widget> SFXDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	string sfx_info;
	
	if (local_ref.is_invalid())
		sfx_info = "Empty SFX";
	else
	{
		std::ostringstream oss;
		oss << "Type: ";
		switch (local_ref.type)
		{
			case SMPL_WAV:
				oss << ".wav";
				break;
			case SMPL_OGG:
				oss << ".ogg";
				break;
		}
		oss << "\nLen: " << local_ref.get_len();
		oss << "\nFreq: " << local_ref.get_frequency();
		oss << "\nDuration: " << (zfix(int(local_ref.get_len())) / int(local_ref.get_frequency())).str();
		auto channels = local_ref.get_chan_conf();
		oss << "\nChannels: " << int(channels >> 4);
		if (channels & 0xF)
			oss << "." << int(channels & 0xF);
		auto depth = local_ref.get_depth();
		oss << "\nDepth: ";
		if (depth & ALLEGRO_AUDIO_DEPTH_UNSIGNED)
			oss << "U";
		switch (depth&0x7)
		{
			case ALLEGRO_AUDIO_DEPTH_INT8:
				oss << "INT8";
				break;
			case ALLEGRO_AUDIO_DEPTH_INT16:
				oss << "INT16";
				break;
			case ALLEGRO_AUDIO_DEPTH_INT24:
				oss << "INT24";
				break;
			case ALLEGRO_AUDIO_DEPTH_FLOAT32:
				oss << "FLOAT32";
				break;
		}
		sfx_info = oss.str();
	}
	
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
				Rows<4>(vAlign = 1.0,
					Button(fitParent = true,
						text = "Default",
						minwidth = 90_px,
						onClick = message::DEFAULT
					),
					INFOBTN("Clears the SFX to default."),
					DummyWidget(colSpan = 2),
					Button(fitParent = true,
						disabled = local_ref.is_invalid(),
						text = "Save",
						minwidth = 90_px,
						onClick = message::SAVE
					),
					INFOBTN("Save the SFX as a '.wav' file."),
					Button(fitParent = true,
						text = "Load",
						minwidth = 90_px,
						onClick = message::LOAD
					),
					INFOBTN("Load an SFX from a '.wav' / '.ogg' file."),
					Button(fitParent = true,
						disabled = local_ref.is_invalid(),
						text = "Play",
						minwidth = 90_px,
						onClick = message::PLAY
					),
					INFOBTN("Play a preview of the sound (at '128' volume)."),
					btn_stop = Button(colSpan = 2, fitParent = true,
						disabled = true,
						text = "Stop",
						minwidth = 90_px,
						onClick = message::STOP
					),
					Button(colSpan = 2, fitParent = true,
						focused = true,
						text = "Ok",
						minwidth = 90_px,
						onClick = message::OK
					),
					Button(colSpan = 2, fitParent = true,
						text = "Cancel",
						minwidth = 90_px,
						onClick = message::CANCEL
					)
				),
				Label(minwidth = 6_em, fitParent = true, text = sfx_info)
			)
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
			if (local_ref.sample)
			{
				static const string ext = "wav"; // only wav saving is currently supported
				switch (local_ref.type)
				{
					default:
					case SMPL_WAV:
						strcat(temppath, ".wav");
						break;
					// case SMPL_OGG:
						// strcat(temppath, ".ogg");
						// break;
				}
				if (prompt_for_new_file_compat("Save SFX file", ext, NULL, temppath, true))
				{
					if (!al_save_sample(temppath, local_ref.sample))
						displayinfo("Error!", fmt::format("Could not write file\n{}", temppath));
					else
						displayinfo("Success!", fmt::format("Saved SFX file\n{}", temppath));
				}
			}
			else displayinfo("Error!", "Cannot save an empty slot!");
			break;
		}
		case message::LOAD:
		{
			if (prompt_for_existing_file_compat("Open SFX file", "wav;ogg", NULL, temppath, true))
			{
				ALLEGRO_SAMPLE* temp_sample;

				if ((temp_sample = al_load_sample(temppath)) == NULL)
					displayinfo("Error", fmt::format("Could not open file '{}'", temppath));
				else
				{
					string t = get_filename(temppath);
					if (t.ends_with(".wav"))
						local_ref.type = SMPL_WAV;
					else if (t.ends_with(".ogg"))
						local_ref.type = SMPL_OGG;
					else ASSERT(false);
					
					local_ref.sfx_name = t.substr(0, t.find_first_of("."));
					local_ref.clear_sample();
					local_ref.sample = temp_sample;
					
					rerun_dlg = true;
					return true;
				}
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
			local_ref.clear_sample();
			return true;
	}
	return false;
}