#include "enemy_editor.h"
#include "info.h"
#include "alert.h"
#include "../zsys.h"
#include "../zquest.h"
#include "../colors.h"
#include <gui/builder.h>

extern char *guy_string[];
extern guydata *guysbuf;
extern zcmodule moduledata;
extern bool saved;
extern void load_cset(RGB *pal,int32_t cset_index,int32_t dataset);

void call_enemy_editor(int32_t index)
{
	EnemyEditorDialog(index).show();
}

EnemyNameInfoGUI defInfo =
{
	-1,
	{
		"Misc Attribute 1:", "Misc Attribute 2:", "Misc Attribute 3:", "Misc Attribute 4", 
		"Misc Attribute 5:", "Misc Attribute 6:","Misc Attribute 7:", "Misc Attribute 8:", 
		"Misc Attribute 9:","Misc Attribute 10:", "Misc Attribute 11:", "Misc Attribute 12:", 
		"Misc Attribute 13:", "Misc Attribute 14:", "Misc Attribute 15:", "Misc Attribute 16:", 
		"Misc Attribute 17:", "Misc Attribute 18:", "Misc Attribute 19:", "Misc Attribute 20:", 
		"Misc Attribute 21:", "Misc Attribute 22:", "Misc Attribute 23:", "Misc Attribute 24:", 
		"Misc Attribute 25:", "Misc Attribute 26:", "Misc Attribute 27:", "Misc Attribute 28:", 
		"Misc Attribute 29:", "Misc Attribute 30:", "Misc Attribute 31:", "Misc Attribute 32:"
		
	}
};

void loadinfo(EnemyNameInfoGUI * inf, guydata const& ref)
{
	inf->clear();
	inf->efam = ref.family;
	#define _SET(mem, str, helpstr) \
	do{ \
		inf->mem = str; \
		inf->h_##mem = helpstr; \
	}while(false)
	#define FLAG(val) (ref.flags & ITEM_FLAG##val)
	switch(ref.family)
	{
		case eePATRA:
		{
			_SET(attributes[0], "Outer Eyes:", "The amount of eyes spawned in for the outer ring. \nEach eye is an equal distance apart from each other when spawned.");
			_SET(attributes[1], "Inner Eyes:", "The amount of eyes spawned in for the inner ring. \nThese eyes are invincible until the entire outer ring is killed. \nEach eye is an equal distance apart from each other when spawned.");
			_SET(attributes[2], "Outer Eye HP:", "The amount of health eyes in the outer ring spawn with.");
			_SET(attributes[3], "Eye Pattern:", "The movement pattern of eyes. This pattern triggers every X full rotations of the eyes, with X being equal to Pattern Cooldown + Warning Spins."
				"\nOval Pattern will cause the eyes to squish vertically, giving off the illusion of them rotating around the main eye in the third dimension."
				"\nBig Circle Pattern will cause the eyes to expand or retract to their respective expand radius.");
			_SET(attributes[4], "Shooters:", "Which eyes shoot projectiles."
				"\nCenter Eye means the main eye will shoot projectiles."
				"\nInner Eyes means the inner ring will shoot projectiles."
				"\nInner + Center means both will shoot.");
			_SET(attributes[5], "Pattern Cooldown:", "How many full rotations the eyes need to do before either Warning Spins happen (if Warning Spins is greater than zero),"
			"\nor the Eye Pattern happens (if Warning Spins is not greater than zero)");
			_SET(attributes[6], "Pattern Cycles:", "How many cycles the Eye Pattern lasts for. For Oval, this is how many times the eyes squish. For Big Circle, this is the number of expansions.");
			_SET(attributes[7], "Outer Eye Tile Offset:", "The offset of the tile the outer eyes use from the main eye."
				"\nThis does not affect the tiles the inner eyes use, as the tile they use depends on the quest rule 'Hardcoded Patra Inner Eye offsets'."
				"\nIf enabled, inner eyes will either use an offset of 120 if the center eye is a shooter, and an offset of 40 otherwise."
				"\nIf disabled, inner eyes won't use an offset, and will instead just use the enemy's Special Tile as their tile.");
			_SET(attributes[8], "Eye CSet:", "The CSet both the inner and outer eyes use.");
			_SET(attributes[9], "Type:", "What type of Patra this is."
				"\nThis affects default tile width and height of the main eye, default ring sizes, expanded sizes, and the time in frames it takes for eyes to make a full rotation."
				"\nNES Patras take 84 frames to make a full rotation, have a default tilewidth and tileheight of 1, and have a default hitbox of 16*16 (centered)."
				"\nBS Patras take 90 frames to make a full rotation, have a default tilewidth and tileheight of 2, and have a default hitbox of 32*16 with a Y draw offset of -8."
				"\nFor default ring sizes and expanded sizes, please see those respective attributes help text.");
			_SET(attributes[17], "Shot Chance:", "1/N chance of shooting every frame, where N is Shot Chance. If 0, uses default of 1 in 128 chance. If -1, it will instead fire in the middle of pattern cycles.");
			_SET(attributes[18], "Shot Cooldown:", "Minimum time in frames between shots.");
			_SET(attributes[19], "Inner Eye Firing Pattern:", "The firing pattern the inner eyes use."
				"\nRandom (any): the default Patra 3 movement from older versions of ZC. Each eye individually has a chance to fire separate from each other,"
				" usually leading to bullet spam or unfair patterns."
				"\nRandom (single): the main eye randomly picks one of it's inner eyes to fire, potentially preventing multiple eyes from firing at the same time."
				"\nBarrage: each eye in the ring fires one after the other, going around the ring in order until all the eyes have fired once."
				"\nRing: all eyes in the ring fire at the same time, using the same angle for all of them to create a ring pattern aimed at the player."
				"\nStream: A single eye is chosen and fires 8 consecutive shots at the player, with 12 frames between each shot.");		
			_SET(attributes[20], "Warning Spins:", "Number of full rotations inserted between the Pattern Cooldown and the Pattern Cycle,"
				" with these rotations moving twice as fast as normal rotations. This is intended to allow the enemy to give a warning"
				" before expanding, with this warning being that the eyes rotate faster.");
			_SET(attributes[21], "Stays Still:", "Allows you to have the Patra stay still during certain actions.");
			_SET(attributes[22], "Outer Ring Loss Speed Boost:", "Allows you to give the patra a speed boost after it loses all of it's outer ring,"
				" measured the same way as step speed (1/100ths of a pixel per frame). \nCan be negative to give it a speed loss.");
			_SET(attributes[23], "Inner Ring Loss Speed Boost:", "Allows you to give the patra a speed boost after it loses all of it's outer ring,"
				" measured the same way as step speed (1/100ths of a pixel per frame). \nCan be negative to give it a speed loss."
				" Stacks with Outer Ring Loss Speed Boost.");
			_SET(attributes[24], "Can Fire:", "Allows you to have the Patra not fire depending on how many eyes it has left."
				"\nAlways: Can fire regardless of eye count."
				"\nWhen Vulnerable: Will only fire if it is vulnerable (inner eyes will only fire if no outer eyes remain, center eye only fires if no inner or outer eyes remain)."
				"\nWhen Invulnerable: Will only fire if it is invulnerbale (inner eyes will only fire if outer eyes remain, center eye only fires if inner eyes or outer eyes remain)."
				"\nWhen Behind 1 Layer: Will only fire if exactly 1 layer of protection remains. For inner eyes, this is the same as 'When Invulnerable'. For center eye, it will only fire if inner eyes remain but no outer eyes remain.");
			_SET(attributes[25], "Can Expand:", "Allows you to have the Patra not expand depending on how many eyes it has left."
				"\nAlways: Can expand regardless of eye count."
				"\nWith Outer Ring: Will only expand if there are still outer eyes remaining."
				"\nWithout Outer Ring: Will only expand if there are no outer eyes remaining.");
			_SET(attributes[26], "Inner Eye HP:", "How much health Inner Eyes have. Used to be hardcoded at 24 HP per eye.");
			_SET(attributes[27], "Center Eye Firing Pattern:", "The shot pattern the Center Eye uses when firing."
				"\n1 Shot: Fires 1 Shot."
				"\n1 Shot (Fast): Fires 1 Shot at double speed."
				"\n3 Shots: Fires 3 Shots, spread out similar to an aquamentus."
				"\n3 Shots (Fast): Fires 3 Shots at double speed, spread out similar to an aquamentus."
				"\n5 Shots: Fires 5 Shots, spread out similar to the final phase of the aquamentus in BS level 7."
				"\n5 Shots (Fast): Fires 5 Shots at double speed, spread out similar to the final phase of the aquamentus in BS level 7."
				"\n4 Shots (Cardinal): Fires 4 Shots in the cardinal directions, with each shot's direction being up, down, left, and right."
				"\n4 Shots (Diagonal): Fires 4 Shots in the diagonal directions, with each shot's direction being upleft, upright, downleft, and downright."
				"\n4 Shots (Random Choice): Picks either 4 Shots (Cardinal) or 4 Shots (Diagonal) at random."
				"\n8 Shots: Fires a shot in all 8 directions."
				"\nBreath: Fires multiple shots for a duration lasting between 50 and 100 frames, with 1 shot every 4 frames with angle variations; similar to walking enemy's breath shot type."
				"\nStream: Fires 8 shots over a duration of time, 1 shot every 12 frames.");
			_SET(attributes[28], "Outer Eye Radius:", "The radius of the outer eyes when not expanding. If left at 0, will use default values,"
				" which depends on whether it's big circle or oval patra and whether it's a BS Patra. Default values are as follows:"
				"\nBS Big Circle: 30."
				"\nBS Oval: 45."
				"\nNES Big Circle: 28."
				"\nNES Oval: 42.");
			_SET(attributes[29], "Inner Eye Radius:", "The radius of the inner eyes when not expanding. If left at 0, will use default values,"
				" which depends on whether it's big circle or oval patra and whether it's a BS Patra. Default values are as follows:"
				"\nBS Big Circle: 15."
				"\nBS Oval: 22."
				"\nNES Big Circle: 14."
				"\nNES Oval: 21.");
			_SET(attributes[30], "Outer Eye Expand Radius:", "The radius of the outer eyes when expanding. If left at 0, will use default values,"
				" which depends on whether it's big circle or oval patra and whether it's a BS Patra. Default values are as follows:"
				"\nBS Big Circle: 60."
				"\nBS Oval: 22."
				"\nNES Big Circle: 56."
				"\nNES Oval: 21.");
			_SET(attributes[31], "Inner Eye Expand Radius:", "The radius of the inner eyes when expanding. If left at 0, will use default values,"
				" which depends on whether it's big circle or oval patra and whether it's a BS Patra. Default values are as follows:"
				"\nBS Big Circle: 30."
				"\nBS Oval: 11."
				"\nNES Big Circle: 28."
				"\nNES Oval: 10.");
		}
	}
	#undef _SET
	#undef FLAG
}

EnemyEditorDialog::EnemyEditorDialog(guydata const& ref, char const* str, int32_t index):
	local_enemyref(ref), enemyname(str), index(index),
	list_enemies(GUI::ListData::enemyclass()),
	list_weapons(GUI::ListData::enemyweapons()),
	list_anim(GUI::ListData::enemyanim()),
	list_itemsets(GUI::ListData::itemsets())
{
	get_palette(oldpal);
}

EnemyEditorDialog::EnemyEditorDialog(int32_t index):
	EnemyEditorDialog(guysbuf[index], guy_string[index], index)
{}

static size_t enmtabs[4] = {0, 0, 0, 0};
static int32_t enemy_scroll = 0;

//{ Macros

#define ACTION_FIELD_WID 2_em
#define ATTR_WID 6_em
#define ATTR_LAB_WID 12_em

#define WH_FIELD(txt, mem) \
Label(textAlign=2,padding = 0_px,text=txt), \
TextField( \
	padding = 0_px, forceFitW = true, \
	type = GUI::TextField::type::INT_DECIMAL, \
	high = 20, val = local_enemyref.mem, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_enemyref.mem = val; \
	} \
)

#define ENM_ATTRIBUTE(ind) \
l_attributes[ind] = Label(minwidth = ATTR_LAB_WID, textAlign = 2), \
ib_attributes[ind] = Button(forceFitH = true, text = "?", \
	disabled = true, \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Attribute Info",h_attributes[ind]).show(); \
	}), \
TextField( \
	fitParent = true, minwidth = 8_em, \
	type = GUI::TextField::type::INT_DECIMAL, \
	val = local_enemyref.guymisc[ind], \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		local_enemyref.guymisc[ind] = val; \
	})

//}

std::shared_ptr<GUI::Widget> EnemyEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	char titlebuf[256];
	sprintf(titlebuf, "Enemy Editor (%d): %s", index, enemyname.c_str());
	if(is_large)
	{
		window = Window(
			use_vsync = true,
			title = titlebuf,
			info = "Edit enemies.",
			onEnter = message::OK,
			onClose = message::CANCEL,
			Column(
				Row(
					Rows<3>(padding = 0_px,
						Label(text = "Name:"),
						TextField(
							fitParent = true,
							maxLength = 63,
							text = enemyname,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
							{
								enemyname = str;
								char buf[256];
								sprintf(buf, "Enemy Editor (%d): %s", index, enemyname.c_str());
								window->setTitle(buf);
							}
						),
						DummyWidget(),
						Label(text = "Type:"),
						DropDownList(data = list_enemies,
							fitParent = true, padding = 0_px,
							selectedValue = local_enemyref.family,
							onSelectionChanged = message::ENEMYCLASS
						),
						Button(width = 1.5_em, forceFitH = true, padding = 0_px, text = "?", hAlign = 1.0, onPressFunc = [&]()
						{
							/*InfoDialog(ZI.getEnemyClassName(local_enemyref.family),
								ZI.getEnemyClassHelp(local_enemyref.family)
							).show();*/
						})
					)
				),
				TabPanel(
					ptr = &enmtabs[0],
					TabRef(name = "Basic", TabPanel(
						ptr = &enmtabs[1],
						TabRef(name = "Data", Column(
							Row(
								Column(
									Rows<7>(
										DummyWidget(),
										DummyWidget(),
										Label(textAlign=1,text="Old"),
										DummyWidget(),
										Label(textAlign=1,text="Spec."),
										DummyWidget(),
										Label(textAlign=1,text="New"),
										Button(text = "?", height = 24_lpx,
										onPressFunc = [&]()
										{
											InfoDialog("Tile info",
												"The tiles used by the enemy. Which tile is used and when depends on the circumstances.\n"
												"The 'Old' tile is only used if 'Use New Enemy Tiles' is disabled, mainly used for more NES-styled animation.\n"
												"The 'New' tile is used if 'Use New Enemy Tiles' is enabled, used for more modern animation.\n"
												"The 'Spec.' tile is used in special circumstances, such as broken shield tiles for walking enemies, Gleeok neck tiles, etc.").show();
										}),
										DummyWidget(),
										oldtile = SelTileSwatch(
											tile = local_enemyref.tile,
											cset = ((local_enemyref.cset & 0x0F) == 14 ? 13 : (local_enemyref.cset & 0x0F)),
											fakecs14 = true,
											showvals = false,
											padding = 0_px,
											onSelectFunc = [&](int32_t t, int32_t c)
											{
												local_enemyref.tile = t;
												updateCSet(c);
												c = (c == 13 ? 14 : c);
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= c&0x0F;
											}
										),
										DummyWidget(),
										specialtile = SelTileSwatch(
											tile = local_enemyref.s_tile,
											cset = ((local_enemyref.cset & 0x0F) == 14 ? 13 : (local_enemyref.cset & 0x0F)),
											fakecs14 = true,
											showvals = false,
											padding = 0_px,
											onSelectFunc = [&](int32_t t, int32_t c)
											{
												local_enemyref.s_tile = t;
												updateCSet(c);
												c = (c == 13 ? 14 : c);
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= c&0x0F;
											}
										),
										DummyWidget(),
										newtile = SelTileSwatch(
											tile = local_enemyref.e_tile,
											cset = ((local_enemyref.cset & 0x0F) == 14 ? 13 : (local_enemyref.cset & 0x0F)),
											fakecs14 = true,
											showvals = false,
											padding = 0_px,
											onSelectFunc = [&](int32_t t, int32_t c)
											{
												local_enemyref.e_tile = t;
												updateCSet(c);
												c = (c == 13 ? 14 : c);
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= c&0x0F;
											}
										),
										Button(text = "?", rowSpan = 2, height = 24_lpx, topPadding = 10_lpx,
										onPressFunc = [&]()
										{
											InfoDialog("Width and Height",
												"The width and height of the protected tile block for Old, Special, and New animation, respectively.\n"
												"The protected tile block will give a warning whenever you attempt to copy over or delete the block.\n"
												"Max width is 20, max height is 20.").show();
										}),
										WH_FIELD("W:", width),
										WH_FIELD("W:", s_width),
										WH_FIELD("W:", e_width),
										WH_FIELD("H:", height),
										WH_FIELD("H:", s_height),
										WH_FIELD("H:", e_height)
									)
								),
								Column(
									Rows<4>(
										Label(text="Health:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 32767,
											val = local_enemyref.hp,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.hp = val;
											}
										),
										Label(text="Random Rate:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 16,
											val = local_enemyref.rate,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.rate = val;
											}
										),
										Label(text="Cnt. Damage:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 32767,
											val = local_enemyref.dp,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.dp = val;
											}
										),
										Label(text="Halt Rate:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 16,
											val = local_enemyref.hrate,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.hrate = val;
											}
										),
										Label(text="Wpn. Damage:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 32767,
											val = local_enemyref.wdp,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.wdp = val;
											}
										),
										Label(text="Homing Factor:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -255,
											high = 255,
											val = local_enemyref.homing,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.homing = val;
											}
										),
										Label(text="Hunger:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											low = -4,
											high = 4,
											val = local_enemyref.grumble,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.grumble = val;
											}
										),
										Label(text="Step Speed:", hAlign = 1.0),
										TextField(
											fitParent = true,
											type = GUI::TextField::type::INT_DECIMAL,
											high = 1000,
											val = local_enemyref.step,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_enemyref.step = val;
											}
										)
									)
								)
							),
							Row
							(
								Columns<4>
								(
									Label(text="Weapon:", hAlign = 1.0),
									Label(text="Item Dropset:", hAlign = 1.0),
									Label(text="Old Animation:", hAlign = 1.0),
									Label(text="New Animation:", hAlign = 1.0),
									DropDownList(
										fitParent = true,
										data = list_weapons,
										selectedValue = local_enemyref.weapon,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.weapon = val;
										}
									),
									DropDownList(
										fitParent = true,
										data = list_itemsets,
										selectedValue = local_enemyref.item_set,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.item_set = val;
										}
									),
									DropDownList(
										fitParent = true,
										disabled = get_bit(quest_rules, qr_NEWENEMYTILES),
										data = list_anim,
										selectedValue = local_enemyref.anim,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.anim = val;
										}
									),
									DropDownList(
										fitParent = true,
										disabled = !get_bit(quest_rules, qr_NEWENEMYTILES),
										data = list_anim,
										selectedValue = local_enemyref.e_anim,
										onSelectFunc = [&](int32_t val)
										{
											local_enemyref.e_anim = val;
										}
									),
									palbox = Checkbox(
										text = "Use Boss Pal CSet", leftPadding = 8_px, hAlign = 0.0,
										checked = ((local_enemyref.cset & 0x0F) == 14),
										colSpan = 2,
										boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
										onToggleFunc = [&](bool state)
										{
											if (state)
											{
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= 14&0x0F;
												updateCSet(13);
												paltext->setDisabled(false);
											}
											else
											{
												local_enemyref.cset &= 0xF0;
												local_enemyref.cset |= 8&0x0F;
												updateCSet(8);
												paltext->setDisabled(true);
											}
										}
									),
									Label(text="Boss Palette CSet:", leftPadding = 8_px, hAlign = 1.0),
									Label(text="Old Frame Rate:", leftPadding = 8_px, hAlign = 1.0),
									Label(text="New Frame Rate:", leftPadding = 8_px, hAlign = 1.0),
									paltext = TextField(
										fitParent = true,
										disabled = (!((local_enemyref.cset & 0x0F) == 14)),
										type = GUI::TextField::type::INT_DECIMAL,
										low = -1,
										high = 29,
										val = local_enemyref.bosspal,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_enemyref.bosspal = val;
											if(val>-1)
											{
												load_cset(RAMpal,csBOSS-1,pSprite(val));
												set_palette(RAMpal);
											}
											else 
											{
												load_cset(RAMpal,csBOSS-1,csBOSS);
												set_palette(RAMpal);
											}
										}
									),
									TextField(
										fitParent = true,
										type = GUI::TextField::type::INT_DECIMAL,
										disabled = get_bit(quest_rules, qr_NEWENEMYTILES),
										high = 256,
										val = local_enemyref.frate,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_enemyref.frate = val;
										}
									),
									TextField(
										fitParent = true,
										type = GUI::TextField::type::INT_DECIMAL,
										disabled = (!get_bit(quest_rules, qr_NEWENEMYTILES)),
										high = 256,
										val = local_enemyref.e_frate,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_enemyref.e_frate = val;
										}
									)
								)
							)
						)),
						TabRef(name = "Attribs", ScrollingPane(
							scrollPos = enemy_scroll,
							fitParent = true,
							onScrollChangedFunc = [&](int32_t val)
							{
								enemy_scroll = val;
							},
							Row(
								Rows<3>(
									ENM_ATTRIBUTE(0),
									ENM_ATTRIBUTE(1),
									ENM_ATTRIBUTE(2),
									ENM_ATTRIBUTE(3),
									ENM_ATTRIBUTE(4),
									ENM_ATTRIBUTE(5),
									ENM_ATTRIBUTE(6),
									ENM_ATTRIBUTE(7),
									ENM_ATTRIBUTE(8),
									ENM_ATTRIBUTE(9),
									ENM_ATTRIBUTE(10),
									ENM_ATTRIBUTE(11),
									ENM_ATTRIBUTE(12),
									ENM_ATTRIBUTE(13),
									ENM_ATTRIBUTE(14),
									ENM_ATTRIBUTE(15)
								),
								Rows<3>(
									ENM_ATTRIBUTE(16),
									ENM_ATTRIBUTE(17),
									ENM_ATTRIBUTE(18),
									ENM_ATTRIBUTE(19),
									ENM_ATTRIBUTE(20),
									ENM_ATTRIBUTE(21),
									ENM_ATTRIBUTE(22),
									ENM_ATTRIBUTE(23),
									ENM_ATTRIBUTE(24),
									ENM_ATTRIBUTE(25),
									ENM_ATTRIBUTE(26),
									ENM_ATTRIBUTE(27),
									ENM_ATTRIBUTE(28),
									ENM_ATTRIBUTE(29),
									ENM_ATTRIBUTE(30),
									ENM_ATTRIBUTE(31)
								)
							)
						))
					))
				),		
				Row(
					vAlign = 1.0,
					spacing = 2_em,
					Button(
						focused = true,
						text = "OK",
						minwidth = 90_lpx,
						onClick = message::OK),
					Button(
						text = "Cancel",
						minwidth = 90_lpx,
						onClick = message::CANCEL)
				)
			)
		);
	}
	
	if (local_enemyref.cset == 14 && local_enemyref.bosspal>-1)
	{
		load_cset(RAMpal,csBOSS-1,pSprite(local_enemyref.bosspal));
		updateCSet(13);
	}
	else 
	{
		load_cset(RAMpal,csBOSS-1,csBOSS);
	}
	set_palette(RAMpal);
	loadEnemyClass();
	return window;
}

bool EnemyEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::ENEMYCLASS:
		{
			local_enemyref.family = int32_t(msg.argument);
			loadEnemyClass();
			return false;
		}

		case message::OK:
			saved = false;
			guysbuf[index] = local_enemyref;
			strcpy(guy_string[index], enemyname.c_str());
			[[fallthrough]];

		case message::CANCEL:
		default:
			memcpy(RAMpal, oldpal, sizeof(oldpal));
			set_palette(RAMpal);
			return true;
	}
}

void EnemyEditorDialog::updateCSet(int32_t cset)
{
	oldtile->setCSet(cset);
	specialtile->setCSet(cset);
	newtile->setCSet(cset);
	if (cset != 13) 
	{
		palbox->setChecked(false);
		paltext->setDisabled(true);
	}
}

void EnemyEditorDialog::loadEnemyClass()
{
	EnemyNameInfoGUI inf;
	loadinfo(&inf, local_enemyref);
	
	#define __SET(obj, mem) \
	l_##obj->setText(inf.mem.size() ? inf.mem : defInfo.mem); \
	h_##obj = (inf.h_##mem.size() ? inf.h_##mem : ""); \
	if(ib_##obj) \
		ib_##obj->setDisabled(h_##obj.empty());
	__SET(attributes[0], attributes[0]);
	__SET(attributes[1], attributes[1]);
	__SET(attributes[2], attributes[2]);
	__SET(attributes[3], attributes[3]);
	__SET(attributes[4], attributes[4]);
	__SET(attributes[5], attributes[5]);
	__SET(attributes[6], attributes[6]);
	__SET(attributes[7], attributes[7]);
	__SET(attributes[8], attributes[8]);
	__SET(attributes[9], attributes[9]);
	__SET(attributes[10], attributes[10]);
	__SET(attributes[11], attributes[11]);
	__SET(attributes[12], attributes[12]);
	__SET(attributes[13], attributes[13]);
	__SET(attributes[14], attributes[14]);
	__SET(attributes[15], attributes[15]);
	__SET(attributes[16], attributes[16]);
	__SET(attributes[17], attributes[17]);
	__SET(attributes[18], attributes[18]);
	__SET(attributes[19], attributes[19]);
	__SET(attributes[20], attributes[20]);
	__SET(attributes[21], attributes[21]);
	__SET(attributes[22], attributes[22]);
	__SET(attributes[23], attributes[23]);
	__SET(attributes[24], attributes[24]);
	__SET(attributes[25], attributes[25]);
	__SET(attributes[26], attributes[26]);
	__SET(attributes[27], attributes[27]);
	__SET(attributes[28], attributes[28]);
	__SET(attributes[29], attributes[29]);
	__SET(attributes[30], attributes[30]);
	__SET(attributes[31], attributes[31]);
	#undef __SET
}