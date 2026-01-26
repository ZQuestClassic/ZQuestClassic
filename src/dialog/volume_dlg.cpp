#include "common.h"
#include "volume_dlg.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zc/ffscript.h"
#include "base/qrs.h"

extern char sfx_sect[];
extern int32_t midi_volume, digi_volume, sfx_volume, emusic_volume, pan_style;
static int old_midi_volume, old_sfx_volume, old_emusic_volume, old_pan_style;
static bool has_applied_volume = false;

static void apply_volume()
{
	master_volume(digi_volume,midi_volume);
	if (zcmusic)
		zcmusic_set_volume(zcmusic, emusic_volume);
	
	adjust_all_sfx_vol();
	has_applied_volume = true;
}

static void save_volume() // write out volume globals to configs
{
	zc_set_config(sfx_sect, "midi", midi_volume);
	zc_set_config(sfx_sect, "sfx", sfx_volume);
	zc_set_config(sfx_sect, "emusic", emusic_volume);
	zc_set_config(sfx_sect, "pan", pan_style);
}

static void cache_volume() // cache the volumes before dialog opens
{
	old_midi_volume = midi_volume;
	old_sfx_volume = sfx_volume;
	old_emusic_volume = emusic_volume;
	old_pan_style = pan_style;
	has_applied_volume = false;
}

static void revert_volume() // revert to cached volumes
{
	midi_volume = old_midi_volume;
	sfx_volume = old_sfx_volume;
	emusic_volume = old_emusic_volume;
	pan_style = old_pan_style;
	if (has_applied_volume)
		apply_volume();
}

void call_volume_dialog()
{
	if (get_qr(qr_OLD_SCRIPT_VOLUME))
	{
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
			master_volume(-1, ((int32_t)FFCore.usr_midi_volume));
		if (FFCore.coreflags & FFCORE_SCRIPTED_DIGI_VOLUME)
			master_volume((int32_t)(FFCore.usr_digi_volume), 1);
		if (FFCore.coreflags & FFCORE_SCRIPTED_MUSIC_VOLUME)
			emusic_volume = (int32_t)FFCore.usr_music_volume;
		if (FFCore.coreflags & FFCORE_SCRIPTED_SFX_VOLUME)
			sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
		pan_style = (int32_t)FFCore.usr_panstyle;
	
	cache_volume();
	pan_style = vbound(pan_style, 0, 3);
	VolumeDialog().show();
}

VolumeDialog::VolumeDialog() {}

static const GUI::ListData list_pan_types =
{
	{ "MONO", 0 },
	{ "1/2", 1 },
	{ "3/4", 2 },
	{ "FULL", 3 },
};
std::shared_ptr<GUI::Widget> VolumeDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	shared_ptr<GUI::Grid> vols = Column();
	static const string vol_names[] = {"MIDI:", "Enhanced Music:", "SFX:"};
	static int* const vol_vals[] = {&midi_volume, &emusic_volume, &sfx_volume};
	for (size_t q = 0; q < 3; ++q)
	{
		int* v = vol_vals[q];
		vols->add(Row(padding = 0_px,
			Label(text = vol_names[q], textAlign = 2, hAlign = 1.0),
			vol_labels[q] = Label(text = to_string(*v), textAlign = 0, width = 3_em)
		));
		vols->add(Slider(
			max_value = 255, value = *v,
			fitParent = true,
			minwidth = 350_px, height = 32_px,
			handle_width = 12_px, bar_width = 5_px,
			onValChangedFunc = [&, v, q](int val)
			{
				*v = val;
				vol_labels[q]->setText(to_string(val));
			}));
	}
	
	window = Window(
		title = "Sound Settings",
		onClose = message::CANCEL,
		Column(
			Column(
				Frame(title = "Volume",
					vols
				),
				Row(
					Label(text = "Pan Style:"),
					DropDownList(data = list_pan_types,
						fitParent = true,
						selectedValue = pan_style,
						onSelectFunc = [&](int32_t val)
						{
							pan_style = val;
						})
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

bool VolumeDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			apply_volume();
			save_volume();
			return true;
		}

		case message::CANCEL:
			revert_volume();
			return true;
	}
	return false;
}
