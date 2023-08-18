#include "edit_dmap.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include "info.h"
#include <fmt/format.h>
#include <base/qrs.h>

static size_t editdmap_tab = 0;
static int32_t dmap_use_script_data = 2;
extern script_data* dmapscripts[NUMSCRIPTSDMAP];
#define DSCRDATA_NONE    0x00
#define DSCRDATA_ACTIVE  0x01
#define DSCRDATA_PASSIVE 0x02
#define DSCRDATA_ALL     0x03
void call_editdmap_dialog(int32_t slot)
{
	dmap_use_script_data = zc_get_config("zquest", "show_dmapscript_meta_type", DSCRDATA_ALL) & DSCRDATA_ALL;
	EditDMapDialog(slot).show();
}
void call_editdmap_dialog(size_t forceTab, int32_t slot)
{
	editdmap_tab = forceTab;
	call_editdmap_dialog(slot);
}

static const GUI::ListData ScriptDataList
{
	{ "None", DSCRDATA_NONE },
	{ "All", DSCRDATA_ALL },
	{ "Active", DSCRDATA_ACTIVE },
	{ "Passive", DSCRDATA_PASSIVE }
};

EditDMapDialog::EditDMapDialog(int32_t slot) :
	thedmap(&DMaps[slot]), local_dmap(DMaps[slot]), dmapslot(slot),
	list_maps(GUI::ListData::numbers(false, 1, map_count)),
	list_types(GUI::ZCListData::dmaptypes()),
	list_lpals(GUI::ZCListData::lpals()),
	list_strings(GUI::ZCListData::strings()),
	list_activesub(GUI::ZCListData::activesubscreens()),
	list_passivesub(GUI::ZCListData::passivesubscreens()),
	list_midis(GUI::ZCListData::midinames()),
	list_tracks(GUI::ListData::numbers(false, 1, 1)),
	list_disableditems(GUI::ZCListData::disableditems(local_dmap.disableditems)),
	list_items(GUI::ZCListData::items(false, false)),
	list_dmapscript(GUI::ZCListData::dmap_script())
{
	ZCMUSIC* tempdmapzcmusic = zcmusic_load_for_quest(local_dmap.tmusic, filepath);

	int32_t numtracks = 1;
	if (tempdmapzcmusic != NULL)
	{
		numtracks = zcmusic_get_tracks(tempdmapzcmusic);
		numtracks = (numtracks < 2) ? 1 : numtracks;
		list_tracks = GUI::ListData::numbers(false, 1, numtracks);

		zcmusic_unload_file(tempdmapzcmusic);
	}
}

// trims spaces at the end of a string
bool EditDMapDialog::disableEnhancedMusic()
{
	if (local_dmap.tmusic[0] == 0)
		return true;
	return false;
}

bool EditDMapDialog::disableMusicTracks()
{
	if (list_tracks.size() < 2)
		return true;
	return disableEnhancedMusic();
}

bool sm_dmap(int dmaptype)
{
	switch (dmaptype & dmfTYPE)
	{
		case dmOVERW:
			return false;
		default:
			return true;
	}
}

#define DMAP_CB(member, flag, cspan, txt, inf) \
INFOBTN(inf), \
Checkbox(checked = local_dmap.member&flag, \
	text = txt, fitParent = true, \
	colSpan = cspan, \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(local_dmap.member, flag, state); \
	})

#define DMAP_CB_SV(member, cspan, txt, inf) \
INFOBTN(inf), \
Checkbox(checked = local_dmap.sideview, \
	text = txt, fitParent = true, \
	colSpan = cspan, \
	onToggleFunc = [&](bool state) \
	{ \
		local_dmap.sideview = state; \
	})

std::shared_ptr<GUI::Widget> EditDMapDialog::DMAP_AC_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(padding = 0_px,
		l_ac_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_ac_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info", h_ac_initds[index]).show();
			}),
		tf_ac_initds[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_dmap.initD[index],
			onValChangedFunc = [&, index](GUI::TextField::type, std::string_view, int32_t val)
			{
				local_dmap.initD[index] = val;
			})
				);
}

std::shared_ptr<GUI::Widget> EditDMapDialog::DMAP_SS_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(padding = 0_px,
		l_ss_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_ss_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info", h_ss_initds[index]).show();
			}),
		tf_ss_initds[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_dmap.sub_initD[index],
			onValChangedFunc = [&, index](GUI::TextField::type, std::string_view, int32_t val)
			{
				local_dmap.sub_initD[index] = val;
			})
				);
}

std::shared_ptr<GUI::Widget> EditDMapDialog::DMAP_MAP_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(padding = 0_px,
		l_map_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_map_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info", h_map_initds[index]).show();
			}),
		tf_map_initds[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_dmap.onmap_initD[index],
			onValChangedFunc = [&, index](GUI::TextField::type, std::string_view, int32_t val)
			{
				local_dmap.onmap_initD[index] = val;
			})
				);
}

void EditDMapDialog::refreshDMapStrings()
{
	//string_switch->switchTo(get_qr(qr_OLD_DMAP_INTRO_STRINGS) ? 0 : 1);
}

std::shared_ptr<GUI::Widget> EditDMapDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	char titlebuf[256];
	sprintf(titlebuf, "Edit DMap (%d)", dmapslot);
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		Column(
			Row(hAlign = 0.0,
				Label(text = "Name:", hAlign = 0.0, colSpan = 2),
				TextField(
					fitParent = true,
					type = GUI::TextField::type::TEXT,
					maxLength = 20,
					text = local_dmap.name,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
					{
						std::string str;
						str.assign(text);
						strncpy(local_dmap.name, str.c_str(), 20);
						local_dmap.name[20] = 0;
					}
				)
			),
			TabPanel(
				ptr = &editdmap_tab,
				TabRef(name = "Mechanics", Column(
					Rows<6>(
						Label(text = "Map:"),
						DropDownList(data = list_maps,
							fitParent = true,
							selectedValue = local_dmap.map + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.map = val - 1;
								dmap_mmap->setCurMap(val);
							}),
						Label(text = "Type:"),
						DropDownList(data = list_types,
							fitParent = true,
							selectedValue = local_dmap.type & dmfTYPE,
							onSelectFunc = [&](int32_t val)
							{
								bool wassmol = sm_dmap(local_dmap.type);
								local_dmap.type = (local_dmap.type & ~dmfTYPE) | (val & dmfTYPE);
								bool smol = sm_dmap(local_dmap.type);
								if (smol != wassmol)
								{
									dmap_mmap->setSmallDMap(smol);
									dmap_grid->setSmallDMap(smol);
									dmap_slider->setDisabled(!smol);
									if (smol)
									{
										local_dmap.xoff = dmap_slider->getOffset();
										dmap_mmap->setOffset(local_dmap.xoff);
									}
									else
									{
										local_dmap.xoff = 0;
										dmap_mmap->setOffset(0);
									}
								}
							}),
						Label(text = "Level:"),
						TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 511, val = local_dmap.level,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								local_dmap.level = val;
							})
					),
					Rows<2>(
						dmap_mmap = DMapMinimap(
							rightMargin = 6_px,
							curMap = local_dmap.map + 1,
							smallDMap = sm_dmap(local_dmap.type),
							offset = local_dmap.xoff
						),
						dmap_grid = DMapMapGrid(
							leftMargin = 6_px,
							mapGridPtr = &local_dmap.grid[0],
							continueScreen = local_dmap.cont, compassScreen = local_dmap.compass,
							smallDMap = sm_dmap(local_dmap.type),
							onUpdate = [&](byte* byteptr, byte compassScreen, byte continueScreen)
							{
								if (compassScreen != local_dmap.compass)
								{
									local_dmap.compass = compassScreen;
									compass_field->setVal(compassScreen);
								}
								if (continueScreen != local_dmap.cont)
								{
									local_dmap.cont = continueScreen;
									continue_field->setVal(continueScreen);
								}
							})
					),
					Rows<4>(
						dmap_slider = Slider(
							colSpan = 2, fitParent = true, rightMargin = 64_px,
							disabled = !sm_dmap(local_dmap.type),
							offset = local_dmap.xoff,
							minOffset = -7, maxOffset = 15,
							onValChangedFunc = [&](int32_t offset)
							{
								local_dmap.xoff = offset;
								dmap_mmap->setOffset(offset);
							}),
						Label(text = "Compass: 0x", hAlign = 1.0, rightPadding = 0_px),
						compass_field = TextField(
							hAlign = 0.0,
							fitParent = true, minwidth = 2_em,
							type = GUI::TextField::type::INT_HEX,
							low = 0x00, high = 0x7F, val = local_dmap.compass,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								dmap_grid->setCompassScreen(val);
								local_dmap.compass = val;
							}),
						Checkbox(checked = local_dmap.type & dmfCONTINUE,
							colSpan = 2, text = "Continue here",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_dmap.type, dmfCONTINUE, state);
							}),
						Label(text = "Continue: 0x", hAlign = 1.0, rightPadding = 0_px),
						continue_field = TextField(
							hAlign = 0.0,
							fitParent = true, minwidth = 2_em,
							type = GUI::TextField::type::INT_HEX,
							low = 0, high = 127, val = local_dmap.cont,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								dmap_grid->setContinueScreen(val);
								local_dmap.cont = val;
							}),
						DummyWidget(colSpan = 2),
						Label(text = "Mirror DMap:", hAlign = 1.0),
						TextField(
							hAlign = 0.0,
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -1, high = 511, val = local_dmap.mirrorDMap,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								local_dmap.mirrorDMap = val;
							})
					)
				)),
				TabRef(name = "Appearance", Column(
					Rows<2>(
						Label(text = "Level Palette:"),
						DropDownList(data = list_lpals,
							fitParent = true,
							selectedValue = local_dmap.color + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.color = val - 1;
							})
					),
					string_switch = Switcher(
						Rows<2>(
							Label(text = "DMap Title"),
							Label(text = "DMap Intro"),
							TextField(
								width = 86_px, height = 22_px,
								maxLength = 20, type = GUI::TextField::type::TEXT_LEGACY,
								text = local_dmap.title,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									std::string str;
									str.assign(text);
									strncpy(local_dmap.title, str.c_str(), 20);
									local_dmap.title[20] = 0;
								}),
							TextField(
								width = 198_px, height = 30_px,
								maxLength = 72, type = GUI::TextField::type::TEXT_LEGACY,
								text = local_dmap.intro,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									std::string str;
									str.assign(text);
									strncpy(local_dmap.intro, str.c_str(), 72);
									local_dmap.intro[72] = 0;
								})
						),
						Rows<2>(
							Label(text = "DMap Title:"),
							TextField(
								fitParent = true, 
								type = GUI::TextField::type::TEXT,
								text = local_dmap.title,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									//TODO
								}),
							Label(text = "DMap Intro:"),
							DropDownList(data = list_strings,
								fitParent = true,
								width = 256_px,
								selectedValue = 1,
								onSelectFunc = [&](int32_t val)
								{
									//TODO
								})
						)
					),
					Rows<2>(
						framed = true, frameText = "Subscreens",
						Label(text = "Active:"),
						DropDownList(data = list_activesub,
							fitParent = true,
							selectedValue = local_dmap.active_subscreen + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.active_subscreen = val - 1;
							}),
						Label(text = "Passive:"),
						DropDownList(data = list_passivesub,
							fitParent = true,
							selectedValue = local_dmap.passive_subscreen + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.passive_subscreen = val - 1;
							})
					)
				)),
				TabRef(name = "Music", Column(
					Rows<2>(
						Label(text = "Midi:"),
						DropDownList(data = list_midis,
							fitParent = true,
							selectedValue = local_dmap.midi + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.midi = val - 1;
							})
					),
					Column(
						framed = true, frameText = "Enhanced Music",
						Rows<2>(
							tmusic_field = TextField(
								colSpan = 2,
								fitParent = true,
								type = GUI::TextField::type::TEXT,
								read_only = true, disabled = disableEnhancedMusic(),
								text = local_dmap.tmusic,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									
								}),
							Label(text = "Track:"),
							tmusic_track_list = DropDownList(data = list_tracks,
								fitParent = true,
								selectedValue = local_dmap.tmusictrack + 1,
								disabled = disableMusicTracks(),
								onSelectFunc = [&](int32_t val)
								{
									local_dmap.tmusictrack = val - 1;
								})
						),
						Rows<2>(
							Rows<2>(framed = true, frameText = "Loop Points",
								Label(text = "Start:", hAlign = 1.0),
								tmusic_start_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::FIXED_DECIMAL,
									disabled = disableEnhancedMusic(),
									low = 0, high = 2147479999,
									val = local_dmap.tmusic_loop_start,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_dmap.tmusic_loop_start = val;
									}),
								Label(text = "End:", hAlign = 1.0),
								tmusic_end_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::FIXED_DECIMAL,
									disabled = disableEnhancedMusic(),
									val = local_dmap.tmusic_loop_end,
									low = 0, high = 2147479999,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_dmap.tmusic_loop_end = val;
									})
							),
							Rows<2>(framed = true, frameText = "Crossfades",
								Label(text = "In:", hAlign = 1.0),
								tmusic_xfadein_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::INT_DECIMAL,
									disabled = disableEnhancedMusic(),
									val = local_dmap.tmusic_xfade_in,
									low = 0, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_dmap.tmusic_xfade_in = val;
									}),
								Label(text = "Out:", hAlign = 1.0),
								tmusic_xfadeout_field = TextField(
									fitParent = true, hAlign = 0.0,
									type = GUI::TextField::type::INT_DECIMAL,
									disabled = disableEnhancedMusic(),
									val = local_dmap.tmusic_xfade_out,
									low = 0, high = 65535,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_dmap.tmusic_xfade_out = val;
									})
							)
						),
						Rows<2>(
							Button(text = "Load",
								onPressFunc = [&]()
								{
									if (getname("Load DMap Music", (char*)zcmusic_types, NULL, tmusicpath, false))
									{
										strcpy(tmusicpath, temppath);
										char* tmfname = get_filename(tmusicpath);

										if (strlen(tmfname) > 55)
										{
											jwin_alert("Error", "Filename too long", "(>55 characters", NULL, "O&K", NULL, 'k', 0, get_zc_font(font_lfont));
											temppath[0] = 0;
										}
										else
										{
											ZCMUSIC* tempdmapzcmusic = zcmusic_load_for_quest(tmfname, filepath);

											int32_t numtracks = 1;
											if (tempdmapzcmusic != NULL)
											{
												numtracks = zcmusic_get_tracks(tempdmapzcmusic);
												numtracks = (numtracks < 2) ? 1 : numtracks;
												list_tracks = GUI::ListData::numbers(false, 1, numtracks);
												tmusic_track_list->setSelectedValue(1);
												
												std::string str;
												str.assign(tempdmapzcmusic->filename);
												strncpy(local_dmap.tmusic, str.c_str(), 56);
												local_dmap.tmusic[55] = 0;

												zcmusic_unload_file(tempdmapzcmusic);
											}

											tmusic_field->setText(local_dmap.tmusic);
											tmusic_track_list->setDisabled(disableMusicTracks());
											tmusic_start_field->setDisabled(disableEnhancedMusic());
											tmusic_end_field->setDisabled(disableEnhancedMusic());
											tmusic_xfadein_field->setDisabled(disableEnhancedMusic());
											tmusic_xfadeout_field->setDisabled(disableEnhancedMusic());
										}
									}
								}),
							Button(text = "Clear",
								onPressFunc = [&]()
								{
									memset(local_dmap.tmusic, 0, 56);
									tmusic_field->setText("");
									tmusic_track_list->setDisabled(true);
									tmusic_start_field->setDisabled(true);
									tmusic_end_field->setDisabled(true);
									tmusic_xfadein_field->setDisabled(true);
									tmusic_xfadeout_field->setDisabled(true);
								})
						)
					)
				)),
				TabRef(name = "Maps", Column(
					TabPanel(
						ptr = &editdmap_tab,
						TabRef(name = "Without Map",
							Rows<2>(
								Label(text = "Minimap"),
								Label(text = "Large"),
								Column(
									SelTileSwatch(
										tile = local_dmap.minimap_1_tile,
										cset = local_dmap.minimap_1_cset,
										tilewid = 5, tilehei = 3,
										showvals = false,
										onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
										{
											local_dmap.minimap_1_tile = t;
											cset = local_dmap.minimap_1_cset = c;
										}),
									Label(text = "\n")
								),
								SelTileSwatch(
									tile = local_dmap.largemap_1_tile,
									cset = local_dmap.largemap_1_cset,
									tilewid = 9, tilehei = 5,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_dmap.largemap_1_tile = t;
										local_dmap.largemap_1_cset = c;
									})
							)
						),
						TabRef(name = "With Map",
							Rows<2>(
								Label(text = "Minimap"),
								Label(text = "Large"),
								Column(
									SelTileSwatch(
										tile = local_dmap.minimap_2_tile,
										cset = local_dmap.minimap_2_cset,
										tilewid = 5, tilehei = 3,
										showvals = false,
										onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
										{
											local_dmap.minimap_2_tile = t;
											local_dmap.minimap_2_cset = c;
										}),
									Label(text = "Setting this tile disables\nthe classic NES minimap.")
								),
								SelTileSwatch(
									tile = local_dmap.largemap_2_tile,
									cset = local_dmap.largemap_2_cset,
									tilewid = 9, tilehei = 5,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_dmap.largemap_2_tile = t;
										local_dmap.largemap_2_cset = c;
									})
							)
						)
					)
				)),
				TabRef(name = "Flags", Column(
					TabPanel(
						TabRef(name = "Mechanical", Column(hAlign = 0.0, vAlign = 0.0,
							Label(text = "These flags have to do with game mechanics.", hAlign = 0.0, vAlign = 0.0),
							Rows<2>(hAlign = 0.0, vAlign = 0.0,
								DMAP_CB(flags, dmfWHIRLWIND, 1, "Allow Whistle Whirlwinds", "Check to enable warping whirlwinds spawned by the whistle."),
								DMAP_CB(flags, dmfWHIRLWINDRET, 1, "Whistle Whirlwinds Return Player To Start", "If checked, whirlwinds will take the player back to their continue point. Otherwise it will use the warp ring specified by the item."),
								DMAP_CB(flags, dmfALWAYSMSG, 1, "Always Display Intro String", "If checked, the DMap's intro string will play every time the player enters."),
								DMAP_CB(flags, dmfVIEWMAP, 1, "View Overworld Map By Pressing Map", "If checked, spacebar will open a map view of the entire map. "),
								DMAP_CB(flags, dmfDMAPMAP, 1, "...But Only Show Screens Marked In Minimap", "Combined with the previous flag, this will mask out any screens not checked in the grid in the 'Mechanics' tab."),
								DMAP_CB_SV(flags, 1, "Sideview", "If checked, the default state of the 'Toggle Sideview Gravity' screen flag will be inverted. All screens will default to sideview and the screen flag turns it off."),
								DMAP_CB(flags, dmfBUNNYIFNOPEARL, 1, "Become Bunny With No Pearl", "If checked, the player will be transformed on this DMap if not carrying a Pearl item. See the 'Pearls' itemclass for more details."),
								DMAP_CB(flags, dmfMIRRORCONTINUE, 1, "Mirror Continues Instead Of Warping", "If checked, using the mirror on this DMap will return the player to the entrance, as if they used 'Divine Escape' or F6 based on the item's flags.")
								)
						)),
						TabRef(name = "Visual", Column(hAlign = 0.0, vAlign = 0.0,
							Label(text = "These flags are for purely visual effects.", hAlign = 0.0, vAlign = 0.0),
							Rows<2>(hAlign = 0.0, vAlign = 0.0,
								DMAP_CB(flags, dmfWAVY, 1, "Underwater Wave Effect", "Draws a wavy effect over the whole screen."),
								DMAP_CB(flags, dmfMINIMAPCOLORFIX, 1, "Use Minimap Foreground Color 2", "If checked, the NES and Interior minimap squares will use 'Minimap Foreground 2' from misc colors."),
								DMAP_CB(flags, dmfLAYER3BG, 1, "Layer 3 Is Background On All Screens", "If checked, the default state of the 'Toggle Layer 3 is Background' screen flag will be inverted. All screens will default to background and the screen flag turns it off."),
								DMAP_CB(flags, dmfLAYER2BG, 1, "Layer 2 Is Background On All Screens", "If checked, the default state of the 'Toggle Layer 2 is Background' screen flag will be inverted. All screens will default to background and the screen flag turns it off."),
								DMAP_CB(flags, dmfNOCOMPASS, 1, "Don't Display Compass Marker In Minimap", "If checked, no compass marker will appear on the subscreen minimap.")
							)
						)),
						TabRef(name = "NES", Column(hAlign = 0.0, vAlign = 0.0,
							Label(text = "These flags relate to screen 80 and 81.", hAlign = 0.0, vAlign = 0.0),
							Rows<2>(hAlign = 0.0, vAlign = 0.0,
								DMAP_CB(flags, dmfCAVES, 1, "Use Caves Instead Of Item Cellars", "if checked, all Cave/Item cellars will use the cave palette and behavior rather than the item cellar one."),
								DMAP_CB(flags, dmf3STAIR, 1, "Allow 3-Stair Warp Rooms", "Enables 3-Stair Warps on Screen 81."),
								DMAP_CB(flags, dmfGUYCAVES, 1, "Special Rooms And Guys Are In Caves Only", "Makes it so Guys (shopkeepers, hints, ect) can only appear on Screen 80."),
								DMAP_CB(flags, dmfNEWCELLARENEMIES, 1, "Use Enemy List For Cellar Enemies", "If checked, the enemy list for passageways and item cellars will use that screen's enemy list. Othwise, it will only spawn the first four and default to Keese if they're not placed.")
							)
						)),
						TabRef(name = "Script", Column(hAlign = 0.0, vAlign = 0.0,
							Label(text = "These flags are for use with scripts. They have no inherent behavior.", hAlign = 0.0, vAlign = 0.0),
							Rows<2>(hAlign = 0.0, vAlign = 0.0,
								DMAP_CB(flags, dmfSCRIPT1, 1, "Script 1", "This is used for scripts and has no inherent effect."),
								DMAP_CB(flags, dmfSCRIPT2, 1, "Script 2", "This is used for scripts and has no inherent effect."),
								DMAP_CB(flags, dmfSCRIPT3, 1, "Script 3", "This is used for scripts and has no inherent effect."),
								DMAP_CB(flags, dmfSCRIPT4, 1, "Script 4", "This is used for scripts and has no inherent effect."),
								DMAP_CB(flags, dmfSCRIPT5, 1, "Script 5", "This is used for scripts and has no inherent effect.")
							)
						))
					)
				)),
				TabRef(name = "Disable", Column(
					Rows<5>(
						disabled_list = List(minheight = 300_px, colSpan = 2,
							data = list_disableditems, isABC = true,
							selectedIndex = 0),
						Column(
							Button(text = "->",
								onPressFunc = [&]()
								{
									int32_t val = disabled_list->getSelectedValue();
									if (val >= 0)
									{
										local_dmap.disableditems[val] &= ~1;
										list_disableditems = GUI::ZCListData::disableditems(local_dmap.disableditems);
										disabled_list->setListData(list_disableditems);
										if (disabled_list->getSelectedIndex() >= list_disableditems.size())
										{
											disabled_list->setSelectedIndex(list_disableditems.size()-1, false);
										}
									}
								}),
							Button(text = "<-",
								onPressFunc = [&]()
								{
									int32_t val = item_list->getSelectedValue();
									if (val >= 0)
									{
										local_dmap.disableditems[val] |= 1;
										list_disableditems = GUI::ZCListData::disableditems(local_dmap.disableditems);
									}
								})
						),
						item_list = List(minheight = 300_px, colSpan = 2,
							data = list_items, isABC = true,
							focused = true,
							selectedIndex = 0)
					)
				)),
				TabRef(name = "Scripts", Column(
					TabPanel(
						TabRef(name = "Active", Column(
							Rows<2>(
								Column(
									DMAP_AC_INITD(0),
									DMAP_AC_INITD(1),
									DMAP_AC_INITD(2),
									DMAP_AC_INITD(3),
									DMAP_AC_INITD(4),
									DMAP_AC_INITD(5),
									DMAP_AC_INITD(6),
									DMAP_AC_INITD(7)
								),
								Column(padding = 0_px, fitParent = true,
									Rows<2>(vAlign = 0.0,
										SCRIPT_LIST_PROC("Script:", list_dmapscript, local_dmap.script, refreshScripts)
									)
								)
							)
						)),
						TabRef(name = "Subscreen", Column(
							Rows<2>(
								Column(
									DMAP_SS_INITD(0),
									DMAP_SS_INITD(1),
									DMAP_SS_INITD(2),
									DMAP_SS_INITD(3),
									DMAP_SS_INITD(4),
									DMAP_SS_INITD(5),
									DMAP_SS_INITD(6),
									DMAP_SS_INITD(7)
								),
								Column(padding = 0_px, fitParent = true,
									Rows<2>(vAlign = 0.0,
										SCRIPT_LIST_PROC("Active Subscreen Script:", list_dmapscript, local_dmap.active_sub_script, refreshScripts),
										SCRIPT_LIST_PROC("Passive Subscreen Script:", list_dmapscript, local_dmap.passive_sub_script, refreshScripts)
									),
									Row(hAlign = 1.0,
										Label(text = "Script Info:"),
										DropDownList(
											maxwidth = 10_em,
											data = ScriptDataList,
											selectedValue = dmap_use_script_data,
											onSelectFunc = [&](int32_t val)
											{
												dmap_use_script_data = val;
												zc_set_config("zquest", "show_dmapscript_meta_type", val);
												refreshScripts();
											})
									)
								)
							)
						)),
						TabRef(name = "Map", Column(
							Rows<2>(
								Column(
									DMAP_MAP_INITD(0),
									DMAP_MAP_INITD(1),
									DMAP_MAP_INITD(2),
									DMAP_MAP_INITD(3),
									DMAP_MAP_INITD(4),
									DMAP_MAP_INITD(5),
									DMAP_MAP_INITD(6),
									DMAP_MAP_INITD(7)
								),
								Column(padding = 0_px, fitParent = true,
									Rows<2>(vAlign = 0.0,
										SCRIPT_LIST_PROC("OnMap Script:", list_dmapscript, local_dmap.onmap_script, refreshScripts)
									)
								)
							)
						))
					)
				))
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
	refreshScripts();
	refreshDMapStrings();
	return window;
}

void EditDMapDialog::refreshScripts()
{
	std::string ac_initds[8];
	std::string ss_initds[8];
	std::string map_initds[8];
	int32_t ty_ac_initds[8];
	int32_t ty_ss_initds[8];
	int32_t ty_map_initds[8];
	for (auto q = 0; q < 8; ++q)
	{
		ac_initds[q] = "InitD[" + std::to_string(q) + "]";
		h_ac_initds[q].clear();
		ty_ac_initds[q] = -1;
		ss_initds[q] = "InitD[" + std::to_string(q) + "]";
		h_ss_initds[q].clear();
		ty_ss_initds[q] = -1;
		map_initds[q] = "InitD[" + std::to_string(q) + "]";
		h_map_initds[q].clear();
		ty_map_initds[q] = -1;
	}
	bool did_dmap_scr = false;
	auto iscd = dmap_use_script_data;
	if (!iscd) iscd = DSCRDATA_ALL;
	if (local_dmap.script)
	{
		zasm_meta const& meta = dmapscripts[local_dmap.script]->meta;
		for (auto q = 0; q < 8; ++q)
		{
			if (meta.initd[q].size())
				ac_initds[q] = meta.initd[q];
			if (meta.initd_help[q].size())
				h_ac_initds[q] = meta.initd_help[q];
			if (meta.initd_type[q] > -1)
				ty_ac_initds[q] = meta.initd_type[q];
		}
	}
	else
	{
		for (auto q = 0; q < 8; ++q)
			ty_ac_initds[q] = nswapDEC;
	}
	if (local_dmap.active_sub_script && (iscd & DSCRDATA_ACTIVE))
	{
		did_dmap_scr = true;
		zasm_meta const& meta = dmapscripts[local_dmap.active_sub_script]->meta;
		for (auto q = 0; q < 8; ++q)
		{
			if (meta.initd[q].size())
				ss_initds[q] = meta.initd[q];
			if (meta.initd_help[q].size())
				h_ss_initds[q] = meta.initd_help[q];
			if (meta.initd_type[q] > -1)
				ty_ss_initds[q] = meta.initd_type[q];
		}
	}
	if (local_dmap.passive_sub_script && (iscd & DSCRDATA_PASSIVE))
	{
		did_dmap_scr = true;
		zasm_meta const& meta = dmapscripts[local_dmap.passive_sub_script]->meta;
		for (auto q = 0; q < 8; ++q)
		{
			if (meta.initd[q].size())
				ss_initds[q] = meta.initd[q];
			if (meta.initd_help[q].size())
				h_ss_initds[q] = meta.initd_help[q];
			if (meta.initd_type[q] > -1)
				ty_ss_initds[q] = meta.initd_type[q];
		}
	}
	if (!did_dmap_scr)
	{
		for (auto q = 0; q < 8; ++q)
			ty_ss_initds[q] = nswapDEC;
	}
	if (local_dmap.onmap_script)
	{
		zasm_meta const& meta = dmapscripts[local_dmap.onmap_script]->meta;
		for (auto q = 0; q < 8; ++q)
		{
			if (meta.initd[q].size())
				map_initds[q] = meta.initd[q];
			if (meta.initd_help[q].size())
				h_map_initds[q] = meta.initd_help[q];
			if (meta.initd_type[q] > -1)
				ty_map_initds[q] = meta.initd_type[q];
		}
	}
	else
	{
		for (auto q = 0; q < 8; ++q)
			ty_map_initds[q] = nswapDEC;
	}
	for (auto q = 0; q < 8; ++q)
	{
		if (ty_ac_initds[q] > -1)
			tf_ac_initds[q]->setSwapType(ty_ac_initds[q]);
		if (ty_ss_initds[q] > -1)
			tf_ss_initds[q]->setSwapType(ty_ss_initds[q]);
		if (ty_map_initds[q] > -1)
			tf_map_initds[q]->setSwapType(ty_map_initds[q]);
		l_ac_initds[q]->setText(ac_initds[q]);
		l_ss_initds[q]->setText(ss_initds[q]);
		l_map_initds[q]->setText(map_initds[q]);
		ib_ac_initds[q]->setDisabled(h_ac_initds[q].empty());
		ib_ss_initds[q]->setDisabled(h_ss_initds[q].empty());
		ib_map_initds[q]->setDisabled(h_map_initds[q].empty());
	}
}

bool EditDMapDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::REFR_INFO:
		break;
	case message::OK:
		*thedmap = local_dmap;
		saved = false;
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
	return false;
}
