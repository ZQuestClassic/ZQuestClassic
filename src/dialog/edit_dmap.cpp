#include "edit_dmap.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include <fmt/format.h>
#include <base/qrs.h>

static size_t editdmap_tab = 0;
void call_editdmap_dialog(int32_t slot)
{
	EditDMapDialog(slot).show();
}
void call_editdmap_dialog(size_t forceTab, int32_t slot)
{
	editdmap_tab = forceTab;
	call_editdmap_dialog(slot);
}

EditDMapDialog::EditDMapDialog(int32_t slot) :
	thedmap(&DMaps[slot]), local_dmap(DMaps[slot]), dmapslot(slot),
	list_maps(GUI::ListData::numbers(false, 1, map_count)),
	list_types(GUI::ZCListData::dmaptypes()),
	list_lpals(GUI::ZCListData::lpals()),
	list_strings(GUI::ZCListData::strings()),
	list_activesub(GUI::ZCListData::activesubscreens()),
	list_passivesub(GUI::ZCListData::passivesubscreens()),
	list_midis(GUI::ZCListData::midinames()),
	list_tracks(GUI::ListData::numbers(false, 1, 1))
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
	zprint2("listtracksize %d\n", list_tracks.size());
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
									zprint2("Title: %s\n", text);
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
										zprint2("tmfname %s\n", tmfname);

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
												zprint2("Loaded music %s %d\n", tempdmapzcmusic->filename, numtracks);

												std::string str;
												str.assign(tempdmapzcmusic->filename);
												strncpy(local_dmap.tmusic, str.c_str(), 56);
												local_dmap.name[56] = 0;

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
									topMargin = 0_px,
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
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Disable", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Scripts", Column(
					Row(
						Label(text = "Placeholder")
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
	refreshDMapStrings();
	return window;
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
