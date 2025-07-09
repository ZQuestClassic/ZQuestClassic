#include "sfxdata.h"
#include "gui/key.h"
#include "info.h"
#include "alert.h"
#include "gui/builder.h"
#include "base/files.h"
#include "sfx.h"
#include "play_midi.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include "zc/zelda.h"
#include <zalleg/zalleg.h>
#include <fmt/format.h>

extern bool saved;
extern SAMPLE customsfxdata[WAV_COUNT];
extern SAMPLE templist[WAV_COUNT];
extern uint8_t customsfxflag[WAV_COUNT >> 3];
extern char* sfx_string[WAV_COUNT];

bool call_sfxdata_dialog(int32_t index)
{
	SFXDataDialog(index).show();
	return true;
}

void change_sfx(SAMPLE* sfx1, SAMPLE* sfx2);
SFXDataDialog::SFXDataDialog(int32_t index) :
	index(index)
{
	kill_sfx();
	zc_stop_midi();
	zc_set_volume(255, -1);
	customsfx = get_bit(customsfxflag, index - 1);
	change_sfx(&templist[index], &customsfxdata[index]);

	char name[36];
	strcpy(name, sfx_string[index]);
	sfxname = name;
}

std::shared_ptr<GUI::Widget> SFXDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	string titlebuf = fmt::format("SFX {}: {}", index, sfxname);
	window = Window(
		use_vsync = true,
		title = titlebuf,
		info = "Save and Load SFX Data for use in engine.",
		onClose = message::CANCEL,
		Column(
			Column(
				Row(
					Label(text = "Name", rightPadding = 0_px, textAlign = 1.0), 
					TextField(
						type = GUI::TextField::type::TEXT,
						maxLength = 36,
						fitParent = true,
						text = sfxname,
						onValChangedFunc = [&](GUI::TextField::type type, std::string_view text, int32_t)
						{
							sfxname = text;
						}
					)
				)
			),
			Column(
				Rows<2>(vAlign = 1.0, spacing = 1_em,
				
					Button(colSpan = 2,
						text = "Save",
						minwidth = 90_px,
						onClick = message::SAVE
					),
					Button(
						text = "Load",
						minwidth = 90_px,
						onClick = message::LOAD
					),
					Button(
						text = "Default",
						minwidth = 90_px,
						onClick = message::DEFAULT
					),
					Button(
						text = "Play",
						minwidth = 90_px,
						onClick = message::PLAY
					),
					Button(
						text = "Stop",
						minwidth = 90_px,
						onClick = message::STOP
					),
					Button(
						focused = true,
						text = "Ok",
						minwidth = 90_px,
						onClick = message::OK
					),
					Button(
						text = "Cancel",
						minwidth = 90_px,
						onClick = message::CANCEL
					)
				)
			)
		)
	);
	return window;
}

bool saveWAV(int32_t slot, const char* filename);
bool SFXDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::SAVE:
	{
		temppath[0] = 0;//memset(temppath, 0, sizeof(temppath));
		char tempname[36];
		strcpy(tempname, sfx_string[index]);
		//change spaces to dashes for f/s safety
		for (int32_t q = 0; q < 36; ++q)
		{
			if (tempname[q] == 32 || tempname[q] == 47 || tempname[q] == 92) //SPACE, Bslash, Fslash
				tempname[q] = 45; //becomes hyphen
		}

		tempname[35] = 0;

		strcpy(temppath, tempname);

		//save
		if (templist[index].data != NULL)
		{
			if (prompt_for_new_file_compat("Save .WAV file", "wav", NULL, temppath, true))
			{
				if (!saveWAV(index, temppath))
				{
					jwin_alert("Error!", "Could not write file", temppath, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
				}
				else
				{
					jwin_alert("Success!", "Saved WAV file", temppath, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
				}
			}
		}
		else
		{
			jwin_alert("Error!", "Cannot save an enpty slot!", NULL, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
		}
		break;
	}
	case message::LOAD:
		if (prompt_for_existing_file_compat("Open .WAV file", "wav", NULL, temppath, true))
		{
			SAMPLE* temp_sample;

			if ((temp_sample = load_wav(temppath)) == NULL)
			{
				jwin_alert("Error", "Could not open file", temppath, NULL, "OK", NULL, 13, 27, get_zc_font(font_lfont));
			}
			else
			{
				char sfxtitle[36];
				char* t = get_filename(temppath);
				int32_t j;

				for (j = 0; j < 35 && t[j] != 0 && t[j] != '.'; j++)
				{
					sfxtitle[j] = t[j];
				}

				sfxtitle[j] = 0;
				sfxname = sfxtitle;
				kill_sfx();
				change_sfx(&templist[index], temp_sample);
				destroy_sample(temp_sample);
				customsfx = 1;
				rerun_dlg = true;
				return true;
			}
		}
		break;
	case message::DEFAULT:
		kill_sfx();
		if (index < WAV_COUNT)
		{
			SAMPLE* temp_sample = (SAMPLE*)sfxdata[zc_min(index, Z35)].dat;
			change_sfx(&templist[index], temp_sample);
			customsfx = 1; //now count as custom sfx
			string name = fmt::format("s{}", index);
			if (index < Z35)
			{
				sfxname = old_sfx_string[index - 1];
			}
			else sfxname = name;
			rerun_dlg = true;
			return true;
		}
		break;
	case message::PLAY:
		kill_sfx();
		if (templist[index].data != NULL)
		{
			sfx(index, 128, false, true);
		}
		break;
	case message::STOP:
		kill_sfx();
		break;
	case message::OK:
		saved = false;
		kill_sfx();
		change_sfx(&customsfxdata[index], &templist[index]);
		set_bit(customsfxflag, index - 1, customsfx);
		strncpy(sfx_string[index], sfxname.c_str(), 36);
		[[fallthrough]];
	case message::CANCEL:
		kill_sfx();

		for (int32_t i = 1; i < WAV_COUNT; i++)
		{
			if (templist[i].data != NULL)
			{
				free(templist[i].data);
				templist[i].data = NULL;
			}
		}
		return true;
	}
	return false;
}