#include "edit_music.h"
#include <gui/builder.h>
#include "base/process_management.h"
#include "base/files.h"
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include "zc/zc_sys.h"
#include "play_midi.h"
#include "info.h"
#include "subscr.h"
#include <fmt/format.h>
#include <base/qrs.h>
#include "music_playback.h"

extern int32_t midi_volume;
void call_edit_music_dialog(size_t idx)
{
	if (idx > quest_music.size()) return;
	EditMusicDialog(idx).show();
}

EditMusicDialog::EditMusicDialog(size_t idx) :
	music_slot(idx), local_music(idx < quest_music.size() ? quest_music[idx] : AdvancedMusic()),
	list_midis(GUI::ZCListData::midinames(true, true)),
	list_tracks(GUI::ListData::numbers(false, 1, 1))
{
	ZCMUSIC* temp_music = zcmusic_load_for_quest(local_music.enhanced.path.c_str(), filepath).first;

	int32_t numtracks = 1;
	if (temp_music != NULL)
	{
		numtracks = zcmusic_get_tracks(temp_music);
		numtracks = (numtracks < 2) ? 1 : numtracks;
		list_tracks = GUI::ListData::numbers(false, 1, numtracks);

		zcmusic_unload_file(temp_music);
	}
}

bool EditMusicDialog::disableEnhancedMusic(bool disableontracker)
{
	if (local_music.enhanced.is_empty())
		return true;

	ZCMUSIC* temp_music = zcmusic_load_for_quest(local_music.enhanced.path.c_str(), filepath).first;

	if (temp_music != NULL)
	{
		if (disableontracker && !(temp_music->type == ZCMF_MP3 || temp_music->type == ZCMF_OGG || temp_music->type == ZCMF_DUH))
			return true;
	}
	else
		return true;

	return false;
}

bool EditMusicDialog::disableMusicTracks()
{
	if (list_tracks.size() < 2)
		return true;
	return disableEnhancedMusic();
}

void EditMusicDialog::update_pause_btn()
{
	bool dis = false;
	if (is_playing)
	{
		if (zcmusic)
			previewstop_btn->setText("Pause");
		else previewstop_btn->setText("Stop");
	}		
	else if (musicpreview_saved)
	{
		previewstop_btn->setText("Resume");
	}
	else
	{
		dis = true;
		if (previewstop_btn->getText() == "Resume")
			previewstop_btn->setText("Pause");
	}
	previewstop_btn->setDisabled(dis);
}
void EditMusicDialog::silenceMusicPreview()
{
	is_playing = false;
	update_pause_btn();
	
	zc_stop_midi();

	if (zcmusic != NULL)
	{
		zcmusic_stop(zcmusic);
		zcmusic_unload_file(zcmusic);
		zcmusic = NULL;
		zcmixer->newtrack = NULL;
	}
}

void EditMusicDialog::midiPreview()
{
	silenceMusicPreview();
	if (local_music.midi > 0)
	{
		zc_play_midi((MIDI*)customtunes[local_music.midi - 1].data, true);
		is_playing = true;
		update_pause_btn();
	}
	else if (local_music.midi < 0)
		displayinfo("Can't Preview", "This internal MIDI cannot be previewed in the editor at this time.");
}
void EditMusicDialog::musicPreview(bool previewloop)
{
	silenceMusicPreview();

	if (!local_music.enhanced.is_empty())
	{
		if(play_enh_music_crossfade(local_music.enhanced.path.c_str(), filepath, local_music.enhanced.track, midi_volume, (previewloop || musicpreview_saved) ? 0 : local_music.enhanced.xfade_in, local_music.enhanced.xfade_out))
		{
			if (previewloop)
			{
				int32_t startpos = zc_max(local_music.enhanced.loop_end - 10000, 0);
				zcmusic_set_curpos(zcmusic, startpos);
			}
			if (musicpreview_saved)
				zcmusic_set_curpos(zcmusic, musicpreview_saved);
			musicpreview_saved = 0;
			zcmusic_set_loop(zcmusic, (local_music.enhanced.loop_start / 10000.0), (local_music.enhanced.loop_end / 10000.0));
			is_playing = true;
			update_pause_btn();
		}
	}
}

std::shared_ptr<GUI::Widget> EditMusicDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	window = Window(
		title = fmt::format("Edit Music ({}) '{}'", music_slot+1, local_music.name),
		onClose = message::CANCEL,
		use_vsync = true,
		onTick = [&]()
		{
			zcmixer_update(zcmixer, midi_volume, 1000000, false);
			if (zcmusic)
			{
				int32_t pos = zcmusic_get_curpos(zcmusic);
				if (pos > 0)
					progress_lbl->setText(fmt::format("{:.4f}s", pos / 10000.0));
				else
					progress_lbl->setText("");
			}
			return ONTICK_CONTINUE;
		},
		Column(
			Row(hAlign = 0.0,
				Label(text = "Name:", hAlign = 0.0, colSpan = 2),
				TextField(
					fitParent = true,
					type = GUI::TextField::type::TEXT,
					maxLength = 512, maxwidth = 30_em,
					text = local_music.name,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
					{
						local_music.name.assign(text);
						window->setTitle(fmt::format("Edit Music ({}) '{}'", music_slot+1, local_music.name));
					}
				)
			),
			Column(
				Rows<3>(
					Label(text = "MIDI:"),
					DropDownList(data = list_midis,
						fitParent = true,
						selectedValue = local_music.midi,
						onSelectFunc = [&](int32_t val)
						{
							local_music.midi = val;
						}),
					Button(text = "Preview",
						forceFitH = true,
						onPressFunc = [&]()
						{
							midiPreview();
						})
				),
				Frame(title = "Enhanced Music",
					Column(
						Rows<2>(
							field = TextField(
								colSpan = 2,
								fitParent = true,
								type = GUI::TextField::type::TEXT,
								read_only = true, disabled = disableEnhancedMusic(),
								text = local_music.enhanced.path),
							Label(text = "Track:"),
							track_list = DropDownList(data = list_tracks,
								fitParent = true,
								selectedValue = local_music.enhanced.track + 1,
								disabled = disableMusicTracks(),
								onSelectFunc = [&](int32_t val)
								{
									local_music.enhanced.track = val - 1;
								})
						),
						Rows<2>(
							Rows<2>(framed = true, frameText = "Loop Points",
								Label(text = "Start:", hAlign = 1.0),
								start_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::FIXED_DECIMAL,
									disabled = disableEnhancedMusic(true),
									low = 0, high = 2147483647,
									val = local_music.enhanced.loop_start,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_music.enhanced.loop_start = val;
									}),
								Label(text = "End:", hAlign = 1.0),
								end_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::FIXED_DECIMAL,
									disabled = disableEnhancedMusic(true),
									val = local_music.enhanced.loop_end,
									low = 0, high = 2147483647,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_music.enhanced.loop_end = val;
									})
							),
							Rows<2>(framed = true, frameText = "Crossfades",
								Label(text = "In:", hAlign = 1.0),
								xfadein_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::INT_DECIMAL,
									disabled = disableEnhancedMusic(),
									val = local_music.enhanced.xfade_in,
									low = 0, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_music.enhanced.xfade_in = val;
									}),
								Label(text = "Out:", hAlign = 1.0),
								xfadeout_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::INT_DECIMAL,
									disabled = disableEnhancedMusic(),
									val = local_music.enhanced.xfade_out,
									low = 0, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_music.enhanced.xfade_out = val;
									})
							)
						),
						progress_lbl = Label(text = "", hAlign = 0.0),
						Rows<3>(
							Button(text = "Preview",
								maxheight = 24_px,
								disabled = disableEnhancedMusic(),
								onPressFunc = [&]()
								{
									musicpreview_saved = 0;
									musicPreview();
								}),
							Button(text = "Preview Loop",
								maxheight = 24_px,
								disabled = disableEnhancedMusic(true),
								onPressFunc = [&]()
								{
									musicpreview_saved = 0;
									musicPreview(true);
								})
						),
						Rows<2>(
							Button(text = "Load",
								maxheight = 24_px,
								onPressFunc = [&]()
								{
									zc_stop_midi();

									if (zcmusic != NULL)
									{
										zcmusic_stop(zcmusic);
										zcmusic_unload_file(zcmusic);
										zcmusic = NULL;
										zcmixer->newtrack = NULL;
									}

									if (prompt_for_existing_file_compat("Load Enhanced Music", (char*)zcmusic_types, NULL, tmusicpath, false))
									{
										strcpy(tmusicpath, temppath);
										char* tmfname = get_filename(tmusicpath);

										if (strlen(tmfname) > 256)
										{
											displayinfo("Error", "Filename too long (>256 characters)");
											temppath[0] = 0;
										}
										else
										{
											auto [music, err] = zcmusic_load_for_quest(tmfname, filepath);

											int32_t numtracks = 1;
											if (music)
											{
												numtracks = zcmusic_get_tracks(music);
												numtracks = (numtracks < 2) ? 1 : numtracks;
												list_tracks = GUI::ListData::numbers(false, 1, numtracks);
												track_list->setSelectedValue(1);
												
												local_music.enhanced.path.assign(music->filename);
												local_music.enhanced.track = 0;

												zcmusic_unload_file(music);
											}
											else
											{
												string s = "";
												switch(err)
												{
													case ZCM_E_NO_AUDIO:
														s = "Allegro's audio driver is not initialized.";
														break;
													case ZCM_E_NOT_FOUND:
														s =  "File not found."
															"\nEnhanced music files must be saved in the same folder as the quest, the ZC program folder,"
															" or in a \"music\" or \"questname_music\" subfolder of the two.";
														break;
													case ZCM_E_ERROR:
														s =  "Error loading file.";
														break;
												}
												InfoDialog("Error", s).show();
											}

											refresh_dlg();
										}
									}
								}),
							Button(text = "Clear",
								maxheight = 24_px,
								onPressFunc = [&]()
								{
									zc_stop_midi();

									if (zcmusic != NULL)
									{
										zcmusic_stop(zcmusic);
										zcmusic_unload_file(zcmusic);
										zcmusic = NULL;
										zcmixer->newtrack = NULL;
									}

									local_music.enhanced.clear();
									refresh_dlg();
								})
						)
					)
				),
				Row(
					previewstop_btn = Button(text = "Pause",
						maxheight = 24_px,
						disabled = !(musicpreview_saved || is_playing),
						onPressFunc = [&]()
						{
							if(musicpreview_saved)
							{
								musicPreview();
							}
							else
							{
								if (zcmusic)
									musicpreview_saved = zcmusic_get_curpos(zcmusic);
								silenceMusicPreview();
							}
						})
				)
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
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

bool EditMusicDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::REFR_INFO:
			break;
		case message::OK:
			if (music_slot >= quest_music.size())
				quest_music.emplace_back(local_music);
			else quest_music[music_slot] = local_music;
			quest_music[music_slot].id = music_slot + 1;
			mark_save_dirty();
			[[fallthrough]];
		case message::CANCEL:
		default:
			silenceMusicPreview();
			return true;
	}
	return false;
}
