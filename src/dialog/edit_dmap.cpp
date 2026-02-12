#include "edit_dmap.h"
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

static size_t editdmap_tab = 0;
static size_t editdmap_tab2 = 0;
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
	dmapslot(slot), thedmap(&DMaps[slot]), local_dmap(DMaps[slot]),
	list_maps(GUI::ListData::numbers(false, 1, map_count)),
	list_types(GUI::ZCListData::dmaptypes()),
	list_activesub(GUI::ZCListData::subscreens(sstACTIVE, true)),
	list_passivesub(GUI::ZCListData::subscreens(sstPASSIVE, true)),
	list_overlaysub(GUI::ZCListData::subscreens(sstOVERLAY, true)),
	list_mapsub(GUI::ZCListData::subscreens(sstMAP, true, true)),
	list_strings(GUI::ZCListData::strings()),
	list_lpals(GUI::ZCListData::lpals()),
	list_disabled_items(GUI::ZCListData::disabled_items(local_dmap.disabled_items)),
	list_items(GUI::ZCListData::items(false, false)),
	list_dmapscript(GUI::ZCListData::dmap_script()),
	list_music(GUI::ZCListData::music_names(true, false))
{}

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
				onSwitch = [&](size_t, size_t)
				{
					refreshGridSquares();
				},
				TabRef(name = "Mechanics", TabPanel(
					TabRef(name = "1", Column(
						Rows<4>(
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
											local_dmap.xoff = dmap_slider->getValue();
											dmap_mmap->setOffset(local_dmap.xoff);
										}
										else
										{
											local_dmap.xoff = 0;
											dmap_mmap->setOffset(0);
										}
										refreshGridSquares();
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
								}),
							Label(text = "Floor:"),
							TextField(
								fitParent = true, minwidth = 3_em,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 0, high = 255, val = local_dmap.floor,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
								{
									local_dmap.floor = val;
								})
						),
						Rows<4>(
							dmap_mmap = DMapMinimap(
								colSpan = 2,
								rightMargin = 6_px,
								curMap = local_dmap.map + 1,
								smallDMap = sm_dmap(local_dmap.type),
								offset = local_dmap.xoff
							),
							dmap_grid = DMapMapGrid(
								colSpan = 2,
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
								}),
							dmap_slider = Slider(
								colSpan = 2, fitParent = true, 
								disabled = !sm_dmap(local_dmap.type),
								maxheight = 16_px,
								min_value = -7, max_value = 15,
								value = local_dmap.xoff,
								onValChangedFunc = [&](int32_t offset)
								{
									local_dmap.xoff = offset;
									dmap_mmap->setOffset(offset);
								}),
							Row(colSpan = 2,
								Label(text = "Mirror DMap:", hAlign = 1.0),
								TextField(
									hAlign = 0.0,
									minwidth = 3_em,
									type = GUI::TextField::type::INT_DECIMAL,
									low = -1, high = 511, val = local_dmap.mirrorDMap,
									onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
									{
										local_dmap.mirrorDMap = val;
									}),
								INFOBTN("If '> -1', the Mirror will warp you to the specified dmap from this dmap.")
							),
							Row(colSpan = 2, hAlign = 0.0,
								INFOBTN("Sets the player's continue point to the continue screen when entering this DMap in most circumstances."),
								Checkbox(checked = local_dmap.type & dmfCONTINUE,
									text = "Continue here",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_dmap.type, dmfCONTINUE, state);
										refreshGridSquares();
									})
							),
							continue_frame = Frame(colSpan = 2, style = GUI::Frame::style::GREEN,
								Row(padding = 0_px,
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
									INFOBTN("The screen where the player spawns in the map in some circumstances.\nCtrl + Click to place.")
								)
							),
							Row(colSpan = 2, hAlign = 0.0,
								INFOBTN("If checked, no compass marker will appear on the subscreen minimap."),
								Checkbox(checked = local_dmap.flags & dmfNOCOMPASS,
									text = "No Compass",
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_dmap.flags, dmfNOCOMPASS, state);
										refreshGridSquares();
									})
							),
							compass_frame = Frame(colSpan = 2, style = GUI::Frame::style::RED,
								Row(padding = 0_px,
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
									INFOBTN("The screen where the compass marker appears on the map. \nAlt + Click to place.")
								)
							)
						)
					)),
					TabRef(name = "2", Column(
						Frame(title = "Gravity",
							Column(
								Rows<2>(padding = 0_px,
									INFOBTN("The 'Gravity' and 'Terminal Velocity' set here will apply for this dmap."),
									Checkbox(checked = local_dmap.flags & dmfCUSTOM_GRAVITY,
										text = "Customize Gravity", fitParent = true,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_dmap.flags, dmfCUSTOM_GRAVITY, state);
											for (int q = 0; q < 2; ++q)
												grav_tf[q]->setDisabled(!state);
										})
								),
								Rows<2>(padding = 0_px,
									Label(text = "Gravity:", hAlign = 1.0),
									grav_tf[0] = TextField(maxLength = 11,
										type = GUI::TextField::type::NOSWAP_ZSINT,
										swap_type = nswapDEC, val = local_dmap.dmap_gravity.getZLong(),
										disabled = !(local_dmap.flags & dmfCUSTOM_GRAVITY),
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_dmap.dmap_gravity = zslongToFix(val);
										}),
									Label(text = "Terminal Velocity:", hAlign = 1.0),
									grav_tf[1] = TextField(maxLength = 11,
										type = GUI::TextField::type::NOSWAP_ZSINT,
										swap_type = nswapDEC, val = local_dmap.dmap_terminal_v.getZLong(),
										disabled = !(local_dmap.flags & dmfCUSTOM_GRAVITY),
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_dmap.dmap_terminal_v = zslongToFix(val);
										})
								)
							)
						)
					))
				)),
				TabRef(name = "Appearance", Column(
					Rows<4>(
						Label(text = "Level Palette:"),
						DropDownList(data = list_lpals,
							fitParent = true,
							selectedValue = local_dmap.color,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.color = val;
							}),
						DummyWidget(),
						DummyWidget(),
						//
						Label(text = "Music:"),
						DropDownList(data = list_music,
							fitParent = true,
							selectedValue = local_dmap.music,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.music = val;
							}),
						INFOBTN("The music to play on this dmap. Screen-specific music takes priority."),
						Button(text = "Edit Music",
							forceFitH = true,
							onPressFunc = [&]()
							{
								call_music_dialog(local_dmap.music);
								list_music = GUI::ZCListData::music_names(true, false);
								refresh_dlg();
							}
						)
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
									local_dmap.title = text;
									truncate_dmap_title(local_dmap.title);
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
								maxLength = 8192, type = GUI::TextField::type::TEXT,
								text = local_dmap.title,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									local_dmap.title = text;
								}),
							Label(text = "DMap Intro:"),
							DropDownList(data = list_strings,
								fitParent = true,
								width = 256_px,
								selectedValue = local_dmap.intro_string_id,
								onSelectFunc = [&](int32_t val)
								{
									local_dmap.intro_string_id = val;
								})
						)
					),
					Rows<3>(
						framed = true, frameText = "Subscreens",
						Label(text = "Active:"),
						DropDownList(data = list_activesub,
							fitParent = true, hAlign = 1.0,
							selectedValue = local_dmap.active_subscreen,
							disabled = list_activesub.invalid(),
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.active_subscreen = val;
							}),
						INFOBTN("The active subscreen to use on this dmap. This is "
							" the menu that appears when you press 'Start'."),
						Label(text = "Passive:"),
						DropDownList(data = list_passivesub,
							fitParent = true, hAlign = 1.0,
							selectedValue = local_dmap.passive_subscreen,
							disabled = list_passivesub.invalid(),
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.passive_subscreen = val;
							}),
						INFOBTN("The passive subscreen to use on this dmap. This is "
							" the menu that draws at the top of the screen."),
						Label(text = "Overlay:"),
						DropDownList(data = list_overlaysub,
							fitParent = true, hAlign = 1.0,
							selectedValue = local_dmap.overlay_subscreen,
							disabled = list_overlaysub.invalid(),
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.overlay_subscreen = val;
							}),
						INFOBTN("The overlay subscreen to use on this dmap. This is "
							" the menu that draws OVER the entire screen."),
						Label(text = "Map:"),
						DropDownList(data = list_mapsub,
							fitParent = true, hAlign = 1.0,
							selectedValue = local_dmap.map_subscreen,
							disabled = list_mapsub.invalid(),
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.map_subscreen = val;
							}),
						INFOBTN("The map subscreen to use on this dmap. If not '(None)',"
							" will replace the standard large map when pressing the Map button.")
					)
				)),
				TabRef(name = "Maps", Column(
					TabPanel(
						ptr = &editdmap_tab2,
						TabRef(name = "Without Map",
							Columns<3>(
								Label(text = "Minimap"),
								SelTileSwatch(
									tile = local_dmap.minimap_tile[0],
									cset = local_dmap.minimap_cset[0],
									tilewid = 5, tilehei = 3,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_dmap.minimap_tile[0] = t;
										cset = local_dmap.minimap_cset[0] = c;
									}),
								DummyWidget(),
									Label(text = "Large"),
									SelTileSwatch(
										rowSpan = 2,
									tile = local_dmap.largemap_tile[0],
									cset = local_dmap.largemap_cset[0],
									tilewid = 9, tilehei = 5,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_dmap.largemap_tile[0] = t;
										local_dmap.largemap_cset[0] = c;
									})
							)
						),
						TabRef(name = "With Map",
							Columns<3>(
								Label(text = "Minimap"),
								SelTileSwatch(
									tile = local_dmap.minimap_tile[1],
									cset = local_dmap.minimap_cset[1],
									tilewid = 5, tilehei = 3,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_dmap.minimap_tile[1] = t;
										local_dmap.minimap_cset[1] = c;
									}),
								Label(text = "Setting this tile disables\nthe classic NES minimap.", vAlign = 0.0),
								Label(text = "Large"),
								SelTileSwatch(
									rowSpan = 2,
									tile = local_dmap.largemap_tile[1],
									cset = local_dmap.largemap_cset[1],
									tilewid = 9, tilehei = 5,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_dmap.largemap_tile[1] = t;
										local_dmap.largemap_cset[1] = c;
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
								DMAP_CB(flags, dmfWHIRLWINDRET, 1, "Whistle Whirlwinds Return Hero To Start", "If checked, whirlwinds will take the Hero back to their continue point. Otherwise it will use the warp ring specified by the item."),
								DMAP_CB(flags, dmfALWAYSMSG, 1, "Always Display Intro String", "If checked, the DMap's intro string will play every time the Hero enters."),
								DMAP_CB(flags, dmfVIEWMAP, 1, "View Overworld Map By Pressing Map", "If checked, spacebar will open a map view of the entire map. "),
								DMAP_CB(flags, dmfDMAPMAP, 1, "...But Only Show Screens Marked In Minimap", "Combined with the previous flag, this will mask out any screens not checked in the grid in the 'Mechanics' tab."),
								DMAP_CB_SV(flags, 1, "Sideview", "If checked, the default state of the 'Toggle Sideview Gravity' screen flag will be inverted. All screens will default to sideview and the screen flag turns it off."),
								DMAP_CB(flags, dmfBUNNYIFNOPEARL, 1, "Become Bunny With No Pearl", "If checked, the Hero will be transformed on this DMap if not carrying a Pearl item. See the 'Pearls' itemclass for more details."),
								DMAP_CB(flags, dmfMIRRORCONTINUE, 1, "Mirror Continues Instead Of Warping", "If checked, using the mirror on this DMap will return the Hero to the entrance, as if they used 'Divine Escape' or F6 based on the item's flags.")
								)
						)),
						TabRef(name = "Visual", Column(hAlign = 0.0, vAlign = 0.0,
							Label(text = "These flags are for purely visual effects.", hAlign = 0.0, vAlign = 0.0),
							Rows<2>(hAlign = 0.0, vAlign = 0.0,
								DMAP_CB(flags, dmfWAVY, 1, "Underwater Wave Effect", "Draws a wavy effect over the whole screen."),
								DMAP_CB(flags, dmfMINIMAPCOLORFIX, 1, "Use Minimap Foreground Color 2", "If checked, the NES and Interior minimap squares will use 'Minimap Foreground 2' from misc colors."),
								DMAP_CB(flags, dmfLAYER3BG, 1, "Layer 3 Is Background On All Screens", "If checked, the default state of the 'Toggle Layer 3 is Background' screen flag will be inverted. All screens will default to background and the screen flag turns it off."),
								DMAP_CB(flags, dmfLAYER2BG, 1, "Layer 2 Is Background On All Screens", "If checked, the default state of the 'Toggle Layer 2 is Background' screen flag will be inverted. All screens will default to background and the screen flag turns it off."),
								DMAP_CB(flags, dmfEXTENDEDVIEWPORT, 1, "Extended Viewport", "If checked, the viewport extends into the part of the screen normally occupied by the passive subscreen. \nMake your passive subscreen background transparent when using this feature.")
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
							data = list_disabled_items, isABC = true,
							selectedIndex = 0),
						Column(
							Button(text = "->",
								onPressFunc = [&]()
								{
									int32_t val = disabled_list->getSelectedValue();
									if (valid_item_id(val))
									{
										local_dmap.disabled_items.set(val, false);
										list_disabled_items = GUI::ZCListData::disabled_items(local_dmap.disabled_items);
										disabled_list->setListData(list_disabled_items);
										if (disabled_list->getSelectedIndex() >= list_disabled_items.size())
										{
											disabled_list->setSelectedIndex(list_disabled_items.size()-1, false);
										}
									}
								}),
							Button(text = "<-",
								onPressFunc = [&]()
								{
									int32_t val = item_list->getSelectedValue();
									if (valid_item_id(val))
									{
										local_dmap.disabled_items.set(val, true);
										list_disabled_items = GUI::ZCListData::disabled_items(local_dmap.disabled_items);
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
									Rows<3>(vAlign = 0.0,
										SCRIPT_LIST_PROC("Active:", list_dmapscript, local_dmap.active_sub_script, refreshScripts),
										INFOBTN("If a script is assigned to this slot, it will run when you press 'Start'."
											" This script runs INSTEAD of the engine's Active Subscreen opening,"
											" and all action is frozen including other scripts until this script exits."),
										SCRIPT_LIST_PROC("Passive:", list_dmapscript, local_dmap.passive_sub_script, refreshScripts),
										INFOBTN("Runs at timings consistent with the engine passive subscreen."
											" Useful for drawing custom UI draws.")
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
									Rows<3>(vAlign = 0.0,
										SCRIPT_LIST_PROC("On Map:", list_dmapscript, local_dmap.onmap_script, refreshScripts),
										INFOBTN("If a script is assigned to this slot, it will run when you press 'Map'."
											" This script runs INSTEAD of the engine's Map opening,"
											" and all action is frozen including other scripts until this script exits.")
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
	refreshGridSquares();
	refreshDMapStrings();
	refreshScripts();
	return window;
}

void EditDMapDialog::refreshGridSquares()
{
	bool showcont = true; 
	bool showcomp = !(local_dmap.flags & dmfNOCOMPASS) && (local_dmap.type & dmfTYPE) != dmOVERW;
	dmap_grid->setShowSquares(showcont, showcomp);
	continue_frame->setStyle(showcont ? GUI::Frame::style::GREEN : GUI::Frame::style::INVIS);
	compass_frame->setStyle(showcomp ? GUI::Frame::style::RED : GUI::Frame::style::INVIS);
	pendDraw();
}

void EditDMapDialog::refreshDMapStrings()
{
	string_switch->switchTo(get_qr(qr_OLD_DMAP_INTRO_STRINGS) ? 0 : 1);
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
		mark_save_dirty();
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
	return false;
}
