#include "weap_data_editor.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zinfo.h"
#include <fmt/format.h>

extern script_data *lwpnscripts[NUMSCRIPTWEAPONS];
extern script_data *ewpnscripts[NUMSCRIPTWEAPONS];

static std::optional<weapon_data> copied_data;

void call_weap_data_editor(weapon_data& weap, bool lweapon, bool togglable)
{
	WeaponDataDialog(weap, lweapon, togglable).show();
}

WeaponDataDialog::WeaponDataDialog(weapon_data& ref, bool lweapon, bool togglable):
	local_ref(ref), dest_ref(ref), is_lw(lweapon), togglable(togglable),
	list_sprites(GUI::ZCListData::miscsprites()),
	list_weaptype(GUI::ZCListData::weaptypes()),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_light_shapes(GUI::ZCListData::light_shapes()),
	list_deftypes(GUI::ZCListData::deftypes()),
	list_lwpnscripts(GUI::ZCListData::lweapon_script()),
	list_ewpnscripts(GUI::ZCListData::eweapon_script())
{}

//{ Macros

#define ATTR_LAB_WID 12_em

std::shared_ptr<GUI::Widget> WeaponDataDialog::MoveFlag(move_flags index, string const& str)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Checkbox(
		text = str,
		checked = local_ref.moveflags & index,
		fitParent = true,
		onToggleFunc = [&, index](bool state)
		{
			SETFLAG(local_ref.moveflags, index, state);
		}
	);
}

std::shared_ptr<GUI::Widget> WeaponDataDialog::ScriptField(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Row(padding = 0_px,
		l_initds[index] = Label(minwidth = ATTR_LAB_WID, hAlign = 1.0, textAlign = 2),
		ib_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info", h_initd[index]).show();
			}),
		tf_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			val = local_ref.initd[index],
			onValChangedFunc = [&, index](GUI::TextField::type, std::string_view, int32_t val)
			{
				local_ref.initd[index] = val;
			})
	);
}

#define NUM_FIELD(member,_min,_max) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = local_ref.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_ref.member = val; \
	})

//}

static size_t wpndata_tabs[1] = {0};
std::shared_ptr<GUI::Widget> WeaponDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::Grid> script_col = Column(vAlign = 0.0);
	if(togglable)
		script_col->add(Button(text = "Toggle Type", disabled = !togglable,
			onPressFunc = [&]()
			{
				is_lw = !is_lw;
				refresh_dlg();
			}));
	script_col->add(Row(
		padding = 0_px,
		SCRIPT_LIST_PROC("Script:", is_lw ? list_lwpnscripts : list_ewpnscripts, local_ref.script, refresh_script)
	));
	window = Window(
		use_vsync = true,
		title = "Weapon Data Editor",
		onClose = message::CANCEL,
		Column(
			TabPanel(
				ptr = &wpndata_tabs[0],
				TabRef(name = "Flags", Row(
					Column(
						Frame(title = "MoveFlags",
							info = "Movement Flags that apply to the weapon."
								" Some weapon types (ex. Sword, Wand, Hammer) will ignore this setting.",
							Column(fitParent = true,
								MoveFlag(move_obeys_grav, "Obeys Gravity"),
								MoveFlag(move_can_pitfall, "Can Fall Into Pitfalls"),
								MoveFlag(move_can_waterdrown, "Can Drown In Liquid")
							)
						),
						Frame(title = "Unlockable Flags",
							info = "The weapon will not be blockable by the specified means.",
							Column(
								Checkbox(text = "Block Defense Types",
									checked = local_ref.unblockable & WPNUNB_BLOCK,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.unblockable, WPNUNB_BLOCK, state);
									}
								),
								Checkbox(text = "Ignore Defense Types",
									checked = local_ref.unblockable & WPNUNB_IGNR,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.unblockable, WPNUNB_IGNR, state);
									}
								),
								Checkbox(text = "Shield Block flags",
									checked = local_ref.unblockable & WPNUNB_SHLD,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.unblockable, WPNUNB_SHLD, state);
									}
								),
								Checkbox(text = "Shield Reflect flags",
									checked = local_ref.unblockable & WPNUNB_REFL,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.unblockable, WPNUNB_REFL, state);
									}
								)
							)
						)
					),
					Column(
						Frame(title = "Misc Flags",
							Rows_Columns<2,5>(
								Checkbox(text = "Pick Up Items",
									checked = local_ref.wflags & WFLAG_PICKUP_ITEMS,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_PICKUP_ITEMS, state);
									}
								),
								INFOBTN("The weapon will collect items it touches, similar to how Melee weapons do."),
								//
								Checkbox(text = "Break On Landing",
									checked = local_ref.wflags & WFLAG_BREAK_WHEN_LANDING,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_BREAK_WHEN_LANDING, state);
									}
								),
								INFOBTN("The weapon will die when it lands after being airborne."),
								//
								Checkbox(text = "Break On Solid",
									checked = local_ref.wflags & WFLAG_BREAK_ON_SOLID,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_BREAK_ON_SOLID, state);
									}
								),
								INFOBTN("The weapon will die when it collides with a solid."),
								//
								Checkbox(text = "Stop On Landing",
									checked = local_ref.wflags & WFLAG_STOP_WHEN_LANDING,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_STOP_WHEN_LANDING, state);
									}
								),
								INFOBTN("The weapon will stop moving when it lands after being airborne."),
								//
								Checkbox(text = "Stop On Solid",
									checked = local_ref.wflags & WFLAG_STOP_WHEN_HIT_SOLID,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_STOP_WHEN_HIT_SOLID, state);
									}
								),
								INFOBTN("The weapon will stop moving when it collides with a solid."),
								//
								Checkbox(text = "Ignited: Any",
									checked = local_ref.wflags & WFLAG_BURN_ANYFIRE,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_BURN_ANYFIRE, state);
									}
								),
								INFOBTN("The weapon is ignited aflame with 'Any' fire."),
								//
								Checkbox(text = "Ignited: Strong",
									checked = local_ref.wflags & WFLAG_BURN_STRONGFIRE,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_BURN_STRONGFIRE, state);
									}
								),
								INFOBTN("The weapon is ignited aflame with 'Strong' fire."),
								//
								Checkbox(text = "Ignited: Magic",
									checked = local_ref.wflags & WFLAG_BURN_MAGICFIRE,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_BURN_MAGICFIRE, state);
									}
								),
								INFOBTN("The weapon is ignited aflame with 'Magic' fire."),
								//
								Checkbox(text = "Ignited: Divine",
									checked = local_ref.wflags & WFLAG_BURN_DIVINEFIRE,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_BURN_DIVINEFIRE, state);
									}
								),
								INFOBTN("The weapon is ignited aflame with 'Divine' fire."),
								//
								//WFLAG_UPDATE_IGNITE_SPRITE is in the Burn/Glow tab
								//
								Checkbox(text = "No Collision When Still",
									checked = local_ref.wflags & WFLAG_NO_COLL_WHEN_STILL,
									fitParent = true,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.wflags, WFLAG_NO_COLL_WHEN_STILL, state);
									}
								),
								INFOBTN("The weapon will not hit any entities while it's step is 0.")
							)
						)
					)
				)),
				TabRef(name = "Burn/Glow", Column(
					Row(
						INFOBTN("With this checked, the weapon will use the appropriate"
							" burning sprite INSTEAD of its' normal sprite."
							"\nAdditionally, the weapon will use the specified light radius for each burning state."),
						burn_box = Checkbox(
							checked = (local_ref.wflags & WFLAG_UPDATE_IGNITE_SPRITE),
							text = "Use Burning Sprites",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.wflags,WFLAG_UPDATE_IGNITE_SPRITE,state);
								if(state)
									SETFLAG(local_ref.flags, wdata_glow_rad, false);
								refresh_burnglow();
							}
						),
						INFOBTN("With this checked, the weapon will use the specified glow radius."
							" This overrides 'Use Burning Sprites'."),
						glow_box = Checkbox(
							checked = (local_ref.flags & wdata_glow_rad),
							text = "Use Glow",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.flags,wdata_glow_rad,state);
								if(state)
									SETFLAG(local_ref.wflags, WFLAG_UPDATE_IGNITE_SPRITE, false);
								refresh_burnglow();
							}
						)
					),
					burn_grid = Rows<4>(
						_d, Label(text = "Sprite"), glow_label = Label(text = "Light Radius"), _d,
						//
						Label(text = "No Fire:", hAlign = 1.0),
						DropDownList(
							data = list_sprites,
							selectedValue = local_ref.burnsprs[WPNSPR_BASE],
							onSelectFunc = [&](int32_t val)
							{
								local_ref.burnsprs[WPNSPR_BASE] = val;
							}),
						burn_field_base = NUM_FIELD(light_rads[WPNSPR_BASE], 0, 255),
						INFOBTN("Settings used for the weapon when not on fire"),
						//
						Label(text = "Normal Fire:", hAlign = 1.0),
						DropDownList(
							data = list_sprites,
							selectedValue = local_ref.burnsprs[WPNSPR_IGNITE_ANY],
							onSelectFunc = [&](int32_t val)
							{
								local_ref.burnsprs[WPNSPR_IGNITE_ANY] = val;
							}),
						NUM_FIELD(light_rads[WPNSPR_IGNITE_ANY], 0, 255),
						INFOBTN("Settings used for the weapon when on 'Normal' fire"),
						//
						Label(text = "Strong Fire:", hAlign = 1.0),
						DropDownList(
							data = list_sprites,
							selectedValue = local_ref.burnsprs[WPNSPR_IGNITE_STRONG],
							onSelectFunc = [&](int32_t val)
							{
								local_ref.burnsprs[WPNSPR_IGNITE_STRONG] = val;
							}),
						NUM_FIELD(light_rads[WPNSPR_IGNITE_STRONG], 0, 255),
						INFOBTN("Settings used for the weapon when on 'Strong' fire"),
						//
						Label(text = "Magic Fire:", hAlign = 1.0),
						DropDownList(
							data = list_sprites,
							selectedValue = local_ref.burnsprs[WPNSPR_IGNITE_MAGIC],
							onSelectFunc = [&](int32_t val)
							{
								local_ref.burnsprs[WPNSPR_IGNITE_MAGIC] = val;
							}),
						NUM_FIELD(light_rads[WPNSPR_IGNITE_MAGIC], 0, 255),
						INFOBTN("Settings used for the weapon when on 'Magic' fire"),
						//
						Label(text = "Divine Fire:", hAlign = 1.0),
						DropDownList(
							data = list_sprites,
							selectedValue = local_ref.burnsprs[WPNSPR_IGNITE_DIVINE],
							onSelectFunc = [&](int32_t val)
							{
								local_ref.burnsprs[WPNSPR_IGNITE_DIVINE] = val;
							}),
						NUM_FIELD(light_rads[WPNSPR_IGNITE_DIVINE], 0, 255),
						INFOBTN("Settings used for the weapon when on 'Divine' fire")
					),
					Row(
						Label(text = "Glow Shape:"),
						DropDownList(data = list_light_shapes,
							selectedValue = local_ref.glow_shape,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.glow_shape = val;
							})
					)
				)),
				TabRef(name = "Size", Row(
					Rows_Columns<3, 5>(
						Label(textAlign = 2, width = 6_em, text = "HitXOffset:"),
						TextField(fitParent = true,
							val = local_ref.hxofs,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -214748, high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.hxofs = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_HIT_X_OFFSET),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_HIT_X_OFFSET,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "HitYOffset:"),
						TextField(fitParent = true,
							val = local_ref.hyofs,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -214748, high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.hyofs = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_HIT_Y_OFFSET),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_HIT_Y_OFFSET,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "HitWidth:"),
						TextField(fitParent = true,
							val = local_ref.hxsz,
							type = GUI::TextField::type::INT_DECIMAL,
							high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.hxsz = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_HIT_WIDTH),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_HIT_WIDTH,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "HitHeight:"),
						TextField(fitParent = true,
							val = local_ref.hysz,
							type = GUI::TextField::type::INT_DECIMAL,
							high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.hysz = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_HIT_HEIGHT),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_HIT_HEIGHT,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "HitZHeight:"),
						TextField(fitParent = true,
							val = local_ref.hzsz,
							type = GUI::TextField::type::INT_DECIMAL,
							high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.hzsz = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_HIT_Z_HEIGHT),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_HIT_Z_HEIGHT,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "TileWidth:"),
						TextField(fitParent = true,
							val = local_ref.tilew,
							type = GUI::TextField::type::INT_DECIMAL,
							high = 32,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.tilew = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_TILE_WIDTH),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_TILE_WIDTH,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "TileHeight:"),
						TextField(fitParent = true,
							val = local_ref.tileh,
							type = GUI::TextField::type::INT_DECIMAL,
							high = 32,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.tileh = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_TILE_HEIGHT),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_TILE_HEIGHT,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "DrawXOffset:"),
						TextField(fitParent = true,
							val = local_ref.xofs,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -214748, high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.xofs = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_DRAW_X_OFFSET),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_DRAW_X_OFFSET,state);
							}
						),
						Label(textAlign = 2, width = 6_em, text = "DrawYOffset:"),
						TextField(fitParent = true,
							val = local_ref.yofs,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -214748, high = 214748,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.yofs = val;
							}
						),
						Checkbox(
							hAlign = 0.0,
							checked = (local_ref.override_flags & OVERRIDE_DRAW_Y_OFFSET),
							text = "Enabled",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.override_flags,OVERRIDE_DRAW_Y_OFFSET,state);
							}
						)
					)
				)),
				TabRef(name = "Misc", Column(
					Row(
						Rows_Columns<3, 3>(
							Label(text = "Lift Level:", hAlign = 1.0),
							NUM_FIELD(lift_level, 0, 255),
							INFOBTN("If 0, the weapon is not liftable. Otherwise, liftable using Lift Gloves of at least this level."
								"\nPresently, only LWeapons are able to be lifted."),
							Label(text = "Lift Time:", hAlign = 1.0),
							NUM_FIELD(lift_time, 0, 255),
							INFOBTN("The time, in frames, to lift the weapon above the Hero's head."),
							Label(text = "Lift Height:", hAlign = 1.0),
							TextField(
								fitParent = true, vPadding = 0_px,
								maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
								places = 4,
								val = local_ref.lift_height.getZLong(),
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.lift_height = zslongToFix(val);
								}),
							INFOBTN("The Z height above the Hero's head to lift the weapon."),
							//
							Checkbox(text = "Set Step Speed:",
								checked = local_ref.flags & wdata_set_step,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.flags, wdata_set_step, state);
									step_tf->setDisabled(!state);
								}
							),
							step_tf = TextField(
								fitParent = true, vPadding = 0_px,
								maxLength = 11, type = GUI::TextField::type::FIXED_DECIMAL,
								places = 4,
								val = local_ref.step.getZLong() * 100,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.step = zslongToFix(val / 100);
								}),
							INFOBTN("Movement speed. 100 step speed is 1 pixel per frame."
								"\nNot all weapon types will obey this."),
							Label(text = "Timeout:", hAlign = 1.0),
							NUM_FIELD(timeout, 0, 214748),
							INFOBTN("If >0, the weapon dies automatically after this many frames."),
							Label(text = "Pierce Count:", hAlign = 1.0),
							NUM_FIELD(pierce_count, -1, 32767),
							INFOBTN("The weapon can hit this many entities (enemies+player) before"
								" dying. If 0, the weapon pierces infinitely. If -1, uses the weapon"
								" type's default piercing behaviors."
								"\nNote: Some weapon types may ignore this (ex. melee weapons)."
								" Bomb type weapons don't use this, but their blast will.")
						)
					),
					Rows<3>(
						Label(textAlign = 2, text = "Imitate Weapon Type:"),
						INFOBTN("If set, the weapon will behave in some ways as the selected type (ex."
							" the weapon will count as this type for Combo Triggers, enemy defenses, etc)."
							" Does not change the actual type / behavior of the weapon (ex. a Boomerang set"
							" to imitate a Sword still flies out and returns)"),
						DropDownList(
							fitParent = true,
							data = list_weaptype,
							selectedValue = local_ref.imitate_weapon,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.imitate_weapon = val;
							}
						),
						Label(textAlign = 2, text = "Default Defense:"),
						INFOBTN("If this weapon would apply a '(None)' defense on hitting"
							" a target, the Default Defense will be applied instead."),
						DropDownList(
							fitParent = true,
							data = list_deftypes,
							selectedValue = local_ref.default_defense,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.default_defense = val;
							}
						)
					)
				)),
				TabRef(name = "Script", Row(
					Column(
						ScriptField(0),
						ScriptField(1),
						ScriptField(2),
						ScriptField(3),
						ScriptField(4),
						ScriptField(5),
						ScriptField(6),
						ScriptField(7)
					),
					script_col
				))
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
					onClick = message::CANCEL),
				Button(
					text = "Copy",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						if(!copied_data)
							refresh_dlg();
						copied_data = local_ref;
					}),
				Button(
					text = "Paste",
					minwidth = 90_px,
					disabled = !copied_data,
					onPressFunc = [&]()
					{
						local_ref = *copied_data;
						refresh_dlg();
					})
			)
		)
	);
	
	refresh_burnglow();
	refresh_script();
	step_tf->setDisabled(!(local_ref.flags & wdata_set_step));
	
	return window;
}

void WeaponDataDialog::refresh_burnglow()
{
	bool burn = (local_ref.wflags & WFLAG_UPDATE_IGNITE_SPRITE);
	bool glow = (local_ref.flags & wdata_glow_rad);
	
	for(auto [idx, child] : burn_grid->get_children())
		child->setDisabled(!burn);
	glow_label->setDisabled(!burn && !glow);
	burn_field_base->setDisabled(!burn && !glow);
	burn_box->setChecked(burn);
	glow_box->setChecked(glow);
}

void WeaponDataDialog::refresh_script()
{
	int32_t sw_initd[8];
	for (auto q = 0; q < 8; ++q)
	{
		l_initd[q] = "InitD[" + to_string(q) + "]:";
		h_initd[q].clear();
		sw_initd[q] = -1;
	}
	if (local_ref.script)
	{
		if(auto scriptdata = (is_lw ? lwpnscripts : ewpnscripts)[local_ref.script])
		{
			zasm_meta const& meta = scriptdata->meta;
			for (auto q = 0; q < 8; ++q)
			{
				if (unsigned(meta.initd_type[q]) < nswapMAX)
					sw_initd[q] = meta.initd_type[q];
				if (meta.initd[q].size())
					l_initd[q] = meta.initd[q];
				if (meta.initd_help[q].size())
					h_initd[q] = meta.initd_help[q];
			}
		}
	}
	else
	{
		for (auto q = 0; q < 8; ++q)
			sw_initd[q] = nswapDEC;
	}
	for (auto q = 0; q < 8; ++q)
	{
		ib_initds[q]->setDisabled(h_initd[q].empty());
		l_initds[q]->setText(l_initd[q]);
		if (sw_initd[q] > -1)
			tf_initd[q]->setSwapType(sw_initd[q]);
	}
}

bool WeaponDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			dest_ref = local_ref;
			return true;

		case message::CANCEL:
			return true;
	}
	return false;
}

