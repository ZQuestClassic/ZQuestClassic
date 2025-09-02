#include "edit_region.h"
#include "base/util.h"
#include "dialog/info.h"
#include "zq/zquest.h"
#include <gui/builder.h>
#include <fmt/format.h>
#include <base/qrs.h>
#include "zq/zq_misc.h"
#include "zq/render.h"
#include "zc_list_data.h"

// This is a snapshot of all the compat QRs as of Feb 17, 2025.
static std::vector<int> old_qrs = {
	// qr_OLD_STRING_EDITOR_MARGINS, // TODO QRHINT doesnt know about needing to do what onStrFix does
	// qr_STRING_FRAME_OLD_WIDTH_HEIGHT, // TODO QRHINT doesnt know about needing to do what onStrFix does
	qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES,
	qr_192b163_WARP,
	qr_210_WARPRETURN,
	qr_8WAY_SHOT_SFX_DEP,
	qr_ALLOW_EDITING_COMBO_0,
	qr_ALLTRIG_PERMSEC_NO_TEMP,
	qr_ANGULAR_REFLECT_BROKEN,
	qr_ANONE_NOANIM,
	qr_ARROWCLIP,
	qr_BITMAPOFFSETFIX,
	qr_BROKEN_ASKIP_Y_FRAMES,
	qr_BROKEN_ATTRIBUTE_31_32,
	qr_BROKEN_BIG_ENEMY_ANIMATION,
	qr_BROKEN_BOMB_AMMO_COSTS,
	qr_BROKEN_CONVEYORS,
	qr_BROKEN_DRAWSCREEN_FUNCTIONS,
	qr_BROKEN_FLAME_ARROW_REFLECTING,
	qr_BROKEN_GENERIC_PUSHBLOCK_LOCKING,
	qr_BROKEN_HORIZONTAL_WEAPON_ANIM,
	qr_BROKEN_INPUT_DOWN_STATE,
	qr_BROKEN_ITEM_CARRYING,
	qr_BROKEN_KEEPOLD_FLAG,
	qr_BROKEN_LIFTSWIM,
	qr_BROKEN_LIGHTBEAM_HITBOX,
	qr_BROKEN_MOVING_BOMBS,
	qr_BROKEN_OVERWORLD_MINIMAP,
	qr_BROKEN_PUSHBLOCK_FLAG_CLONING,
	qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS,
	qr_BROKEN_RAFT_SCROLL,
	qr_BROKEN_RING_POWER,
	qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION,
	qr_BROKEN_SIDEVIEW_SPRITE_JUMP,
	qr_BROKEN_SWORD_SPIN_TRIGGERS,
	qr_BROKEN_Z3_ANIMATION,
	qr_BROKENBOOKCOST,
	qr_BROKENHITBY,
	qr_BUGGED_LAYERED_FLAGS,
	qr_BUGGY_BUGGY_SLASH_TRIGGERS,
	qr_CANDLES_SHARED_LIMIT,
	qr_CHECKSCRIPTWEAPONOFFSCREENCLIP,
	qr_CONT_SWORD_TRIGGERS,
	qr_COPIED_SWIM_SPRITES,
	qr_CUSTOMWEAPON_IGNORE_COST,
	qr_DECO_2_YOFFSET,
	qr_DMAP_0_CONTINUE_BUG,
	qr_DUNGEONS_USE_CLASSIC_CHARTING,
	qr_ENEMIES_DONT_SCRIPT_FIRST_FRAME,
	qr_ENEMIES_SECRET_ONLY_16_31,
	qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY,
	qr_FAIRY_FLAG_COMPAT,
	qr_FFCPRELOAD_BUGGED_LOAD,
	qr_FIRE_LEVEL_TRIGGERS_ARENT_WEAPONS,
	qr_FLUCTUATING_ENEMY_JUMP,
	qr_GANON_CANT_SPAWN_ON_CONTINUE,
	qr_GANONINTRO,
	qr_GOHMA_UNDAMAGED_BUG,
	qr_GOTOLESSNOTEQUAL,
	qr_HARDCODED_BS_PATRA,
	qr_HARDCODED_ENEMY_ANIMS,
	qr_HARDCODED_FFC_BUSH_DROPS,
	qr_HARDCODED_LITEM_LTMS,
	qr_HOOKSHOTDOWNBUG,
	qr_ITEMPICKUPSETSBELOW,
	qr_LEEVERS_DONT_OBEY_STUN,
	qr_MANHANDLA_BLOCK_SFX,
	qr_MOVINGBLOCK_FAKE_SOLID,
	qr_NO_LANMOLA_RINGLEADER,
	qr_NO_LIFT_SPRITE,
	qr_NO_OVERWORLD_MAP_CHARTING,
	qr_NOFAIRYGUYFIRES,
	qr_NOSOLIDDAMAGECOMBOS,
	qr_OFFSCREENWEAPONS,
	qr_OLD_210_WATER,
	qr_OLD_BOMB_HITBOXES,
	qr_OLD_BRIDGE_COMBOS,
	qr_OLD_BUG_NET,
	qr_OLD_CHEST_COLLISION,
	qr_OLD_DMAP_INTRO_STRINGS,
	qr_OLD_DOORREPAIR,
	qr_OLD_DRAWOFFSET,
	qr_OLD_ENEMY_KNOCKBACK_COLLISION,
	qr_OLD_F6,
	qr_OLD_FAIRY_LIMIT,
	qr_OLD_FFC_FUNCTIONALITY,
	qr_OLD_FFC_SPEED_CAP,
	qr_OLD_FLAMETRAIL_DURATION,
	qr_OLD_GUY_HANDLING,
	qr_OLD_HALF_MAGIC,
	qr_OLD_ITEMDATA_SCRIPT_TIMING,
	qr_OLD_KEESE_Z_AXIS,
	qr_OLD_LADDER_ITEM_SIDEVIEW,
	qr_OLD_LANDING_SFX,
	qr_OLD_LENS_LAYEREFFECT,
	qr_OLD_LOCKBLOCK_COLLISION,
	qr_OLD_POTION_OR_HC,
	qr_OLD_SCRIPT_VOLUME,
	qr_OLD_SCRIPTED_KNOCKBACK,
	qr_OLD_SECRETMONEY,
	qr_OLD_SHALLOW_SFX,
	qr_OLD_SIDEVIEW_CEILING_COLLISON,
	qr_OLD_SIDEVIEW_LANDING_CODE,
	qr_OLD_SLASHNEXT_SECRETS,
	qr_OLD_TILE_INITIALIZATION,
	qr_OLD_WIZZROBE_SUBMERGING,
	qr_OLDCS2,
	qr_OLDHOOKSHOTGRAB,
	qr_OLDINFMAGIC,
	qr_OLDLENSORDER,
	qr_OLDSIDEVIEWSPIKES,
	qr_OLDSPRITEDRAWS,
	qr_PATRAS_USE_HARDCODED_OFFSETS,
	qr_PEAHATCLOCKVULN,
	qr_POLVIRE_NO_SHADOW,
	qr_REPLACEOPENDOORS,
	qr_SCREENSTATE_80s_BUG,
	qr_SCRIPT_FRIENDLY_ENEMY_TYPES,
	qr_SCROLLING_KILLS_CHARGE,
	qr_SCROLLWARP_NO_RESET_FRAME,
	qr_SHORTDGNWALK,
	qr_SPARKLES_INHERIT_PROPERTIES,
	qr_SPOTLIGHT_IGNR_SOLIDOBJ,
	qr_SPRITE_JUMP_IS_TRUNCATED,
	qr_STEPTEMP_SECRET_ONLY_16_31,
	qr_SUBSCR_BACKWARDS_ID_ORDER,
	qr_SUBSCR_OLD_SELECTOR,
	qr_TRIGGERSREPEAT,
	qr_WALKTHROUGHWALL_NO_DOORSTATE,
	qr_WARPS_RESTART_DMAPSCRIPT,
	qr_WIZZROBES_DONT_OBEY_STUN,
	qr_WRONG_BRANG_TRAIL_DIR,
};

// To reduce the amount of old features that need to be upgraded for region support, we suggest
// disabling these QRs.
static std::vector<int> get_problematic_qrs()
{
	std::vector<int> result;

	for (auto qr : old_qrs)
	{
		if (get_bit(quest_rules, qr))
			result.push_back(qr);
	}

	return result;
}

void call_edit_map_settings(int32_t slot)
{
	pause_dlg_tint(true);
	zq_set_screen_never_freeze(true);
	
	EditMapSettingsDialog(slot).show();
	
	pause_dlg_tint(false);
	zq_set_screen_never_freeze(false);
	Map.refresh_color();
}

static bool validate_regions(const regions_data& data)
{
	auto region_ids = data.get_all_region_ids();
	std::vector<region_description> result;
	return get_all_region_descriptions(result, region_ids);
}

EditMapSettingsDialog::EditMapSettingsDialog(int32_t slot) :
	mapslot(slot), the_regions_data(&Regions[slot]), local_regions_data(Regions[slot]),
	region_valid(false),
	list_lpals(GUI::ZCListData::lpals())
{
	for (int q = 0; q < MAPSCRS; ++q)
	{
		if (Map.Scr(q)->is_valid())
		{
			region_valid = true;
			break;
		}
	}
	local_info = map_infos[mapslot];
}

#define BTN_REGIONIDX(index, indexstr) \
region_checks[index] = Checkbox(checked = region_grid->getCurrentRegionIndex() == index, \
	text = indexstr, maxheight = 16_px, \
	onToggleFunc = [&](bool state) \
	{ \
		region_grid->setCurrentRegionIndex(index); \
		refreshRegionGrid(); \
	})

#define DEF_LAYER(lyr) \
Label(text = fmt::format("{}:", lyr), hAlign = 1.0), \
TextField( \
	fitParent = true, \
	type = GUI::TextField::type::NOSWAP_ZSINT, \
	swap_type = nswapLDEC, minwidth = 3_em, \
	low = 0, high = map_count, val = local_info.autolayers[lyr-1], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_info.autolayers[lyr-1] = val; \
	})

static size_t tabpos = 0;
std::shared_ptr<GUI::Widget> EditMapSettingsDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	auto problematic_qrs = get_problematic_qrs();

	char titlebuf[256];
	sprintf(titlebuf, "Edit Map Settings (%d)", mapslot + 1);
	std::shared_ptr<GUI::Grid> reg_col;
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		use_vsync = true,
		Column(
			TabPanel(ptr = &tabpos,
				TabRef(name = "Regions", reg_col = Column()),
				TabRef(name = "Defaults",
					Column(
						Frame(title = "AutoLayer Maps",
							Rows_Columns<2, 3>(
								DEF_LAYER(1),
								DEF_LAYER(2),
								DEF_LAYER(3),
								DEF_LAYER(4),
								DEF_LAYER(5),
								DEF_LAYER(6)
							)
						),
						Frame(
							Rows<2>(
								Label(text = "Screen Palette:"),
								DropDownList(data = list_lpals,
									fitParent = true,
									selectedValue = local_info.autopalette,
									onSelectFunc = [&](int32_t val)
									{
										local_info.autopalette = val;
										
										// Preview the palette in the background
										loadlvlpal(val);
										rebuild_trans_table();
										refresh(rALL);
									})
							)
						)
					)
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
	
	std::shared_ptr<GUI::Button> z3_guide_btn = Button(
		text = "Z3 User Guide",
		onPressFunc = [&]()
		{
			util::open_web_link("https://docs.zquestclassic.com/tutorials/z3_user_guide");
		});
	if (region_valid)
	{
		reg_col->add(Frame(info = "Screens with the same Region ID define a single Region\n"
			"  of screens connected via free-scrolling\n\n"
			"Regions must be rectangular\n\n"
			"For more on Regions, see the Z3 User Guide",
			RegionGrid(topPadding = DEFAULT_PADDING+1.5_em,
				focused = true,
				localRegionsData = &local_regions_data,
				onUpdate = [&]()
				{
					refreshRegionGrid();
				}))
			);
		reg_col->add(Frame(title = "Select Region ID:",
			Row(
				BTN_REGIONIDX(0, "0 (no scrolling)"),
				BTN_REGIONIDX(1, "1"),
				BTN_REGIONIDX(2, "2"),
				BTN_REGIONIDX(3, "3"),
				BTN_REGIONIDX(4, "4"),
				BTN_REGIONIDX(5, "5"),
				BTN_REGIONIDX(6, "6"),
				BTN_REGIONIDX(7, "7"),
				BTN_REGIONIDX(8, "8"),
				BTN_REGIONIDX(9, "9")
			)
		));
		reg_col->add(Row(
				z3_guide_btn,
				Button(text = "Problematic QRs",
					onClick = message::QRS,
					disabled = problematic_qrs.empty())
			));
	}
	else
	{
		reg_col->add(Label(text = "There must be at least one valid screen in a map to configure regions."));
		reg_col->add(z3_guide_btn);
	}
	
	return window;
}

void EditMapSettingsDialog::refreshRegionGrid()
{
	int32_t idx = region_grid->getCurrentRegionIndex();
	for (int32_t i = 0; i < 10; ++i)
	{
		region_checks[i]->setChecked(i == idx);
	}
}

bool EditMapSettingsDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::QRS:
		{
			InfoDialog("Problematic QRs",
				"Compat QRs use less tested portions of the ZC engine, and so may create issues\n"
				"when enabled while using the Regions feature.\n"
				"You should disable as many of these as you can." + QRHINT(get_problematic_qrs())
			).show();
			break;
		}
		case message::OK:
		{
			if (!validate_regions(local_regions_data))
			{
				InfoDialog("Bad regions", "Scrolling regions must be rectangular").show();
				return false;
			}

			if (region_valid)
			{
				*the_regions_data = local_regions_data;
				Map.regions_mark_dirty();
			}
			auto& mapinf = map_infos[mapslot];
			bool dirty_screen_defaults = mapinf != local_info;

			if (dirty_screen_defaults) // reset the defaults of invalid screens
			{
				mapinf = local_info;
				for (auto q = 0; q < MAPSCRS; ++q)
					if (!(Map.Scr(q)->is_valid()))
						Map.clearscr(q);
			}
			if (region_valid || dirty_screen_defaults)
				saved = false;
			return true;
		}
		case message::CANCEL:
			return true;
		case message::REFR_INFO:
			return false;
	}
	return false;
}
