#include "itemwizard.h"
#include "info.h"
#include "alertfunc.h"
#include "base/zsys.h"
#include "tiles.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "items.h"
#include "zc/weapons.h"
#include "sfx.h"
#include "base/qrs.h"
#include "zinfo.h"
#include "base/combo.h"
#include "base/misctypes.h"

extern bool saved;
extern comboclass *combo_class_buf;
extern int32_t CSet;
extern int32_t numericalFlags;
extern script_data *comboscripts[NUMSCRIPTSCOMBODATA];
extern itemdata *itemsbuf;

char *ordinal(int32_t num);
using std::string;
using std::to_string;

bool hasItemWizard(int32_t type)
{
	switch(type)
	{
		case itype_shield:
		case itype_note:
			return true;
	}
	return false;
}

void call_item_wizard(ItemEditorDialog& dlg)
{
	ItemWizardDialog(dlg).show();
}

ItemWizardDialog::ItemWizardDialog(ItemEditorDialog& parent) : local_ref(parent.local_itemref),
	dest_ref(parent.local_itemref), src_ref(parent.local_itemref),
	parent(parent), flags(0),
	list_sfx(GUI::ZCListData::sfxnames(true))
{
	memset(rs_sz, 0, sizeof(rs_sz));
}

void ItemWizardDialog::setRadio(size_t rs, size_t ind)
{
	for(size_t q = 0; q < rs_sz[rs]; ++q)
	{
		auto& radio = rset[rs][q];
		radio->setChecked(ind==q);
	}
}
size_t ItemWizardDialog::getRadio(size_t rs)
{
	for(size_t q = 0; q < rs_sz[rs]; ++q)
	{
		if(rset[rs][q]->getChecked())
			return q;
	}
	if(rs_sz[rs] > 0)
		rset[rs][0]->setChecked(true);
	return 0;
}

#define RESET(member) (local_ref.member = src_ref.member)
#define ZERO(member) (local_ref.member = 0)
#define RESET_ZERO(member,flag) (local_ref.member = (flag ? 0 : src_ref.member))
void ItemWizardDialog::update(bool first)
{
	switch(local_ref.family)
	{
		case itype_shield:
		{
			bool active = (local_ref.flags & item_flag9);
			bool move = active && (local_ref.flags & item_flag10);
			disable(0, !active);
			disable(1, !move);
			break;
		}
		case itype_note:
			break;
	}
	disable(-1, true); //always disabled
	pendDraw();
}
void ItemWizardDialog::endUpdate()
{
	update();
	switch(local_ref.family)
	{
		case itype_shield:
		{
			bool active = (local_ref.flags & item_flag9);
			bool move = active && (local_ref.flags & item_flag10);
			bool src_active = (src_ref.flags & item_flag9);
			bool src_move = src_active && (src_ref.flags & item_flag10);
			
			if(!move)
			{
				if(src_move)
				{
					local_ref.misc7 = 0;
					local_ref.misc8 = 0;
				}
				else
				{
					local_ref.misc7 = src_ref.misc7;
					local_ref.misc8 = src_ref.misc8;
				}
			}
			if(!active)
			{
				if(src_active)
				{
					local_ref.misc6 = 0;
					SETFLAG(local_ref.flags, item_flag5|item_flag6|
						item_flag7|item_flag8|item_flag10|item_flag11, false);
				}
				else
				{
					local_ref.misc6 = src_ref.misc6;
					CPYFLAG(local_ref.flags, item_flag5|item_flag6|
						item_flag7|item_flag8|item_flag10|item_flag11, src_ref.flags);
				}
			}
			local_ref.misc1 |= local_ref.misc2; //blockflags are req for refl flags
			break;
		}
		case itype_note:
			break;
	}
}

void ItemWizardDialog::updateTitle()
{
	switch(local_ref.family)
	{
		default:
			ityname = ZI.getItemClassName(local_ref.family);
			break;
	}
	window->setTitle("Item Wizard (" + ityname + ")");
}
void item_default(itemdata& ref)
{
	ref.misc1 = 0;
	ref.misc2 = 0;
	ref.misc3 = 0;
	ref.misc4 = 0;
	ref.misc5 = 0;
	ref.misc6 = 0;
	ref.misc7 = 0;
	ref.misc8 = 0;
	ref.misc9 = 0;
	ref.misc10 = 0;
	ref.flags &= ~(item_flag1|item_flag2|item_flag3|item_flag4|
		item_flag5|item_flag6|item_flag7|item_flag8|
		item_flag9|item_flag10|item_flag11|item_flag12|
		item_flag13|item_flag14|item_flag15);
	switch(ref.family)
	{
		case itype_shield:
			ref.flags |= item_flag1; //protects front
			ref.misc1 = sh_rock|sh_arrow|sh_brang;
			break;
		case itype_note:
			ref.misc1 = 0;
			break;
	}
}
template <typename T1, typename T2>
std::shared_ptr<GUI::Widget> ItemWizardDialog::CBOX_IMPL(string const& name, T1* mem, T2 bit, optional<string> inf)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	std::shared_ptr<GUI::Widget> cb = Checkbox(
		text = name, hAlign = 0.0,
		checked = ((*mem)&bit),
		onToggleFunc = [&,mem,bit](bool state)
		{
			SETFLAG((*mem),bit,state);
			update();
		}
	);
	if(!inf)
		return cb;
	std::shared_ptr<GUI::Widget> ibtn = inf->empty() ? DINFOBTN() : INFOBTN(*inf);
	return Row(padding = 0_px, hAlign = 0.0,
		ibtn,
		cb
	);
}
#define CBOX(name,member,bit) \
CBOX_IMPL(name, &local_ref.member, bit)

#define CBOX_DI(name,member,bit) \
CBOX_IMPL(name, &local_ref.member, bit, "")

#define CBOX_I(name,member,bit,inf) \
CBOX_IMPL(name, &local_ref.member, bit, inf)

std::shared_ptr<GUI::Widget> ItemWizardDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	std::shared_ptr<GUI::Grid> windowRow;
	window = Window(
		//use_vsync = true,
		onClose = message::CANCEL,
		Column(
			windowRow = Row(padding = 0_px),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Default",
					minwidth = 90_px,
					onClick = message::DEFAULT),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	bool wip = false;
	switch(local_ref.family)
	{
		case itype_shield:
		{
			static size_t tabpos = 0, tabpos2 = 0;
			
			windowRow->add(TabPanel(ptr = &tabpos,
				TabRef(name = "BlockFlags",
					TabPanel(ptr = &tabpos2,
						TabRef(name = "Block",
							Columns<8>(
								CBOX_DI("Rocks", misc1, sh_rock),
								CBOX_DI("Arrows", misc1, sh_arrow),
								CBOX_DI("Boomerangs", misc1, sh_brang),
								CBOX_DI("Fireballs", misc1, sh_fireball),
								CBOX_DI("Fireballs (Boss)", misc1, sh_fireball2),
								CBOX_DI("Swords", misc1, sh_sword),
								CBOX_DI("Magic", misc1, sh_magic),
								CBOX_DI("Flames", misc1, sh_flame),
								CBOX_I("Flames 2", misc1, sh_flame2, "Might ALWAYS be blocked regardless, depending on 'Broken Enemy Fire/Arrow Reflecting'."+QRHINT({qr_BROKEN_FLAME_ARROW_REFLECTING})),
								CBOX_I("Custom W. (ALL)", misc1, sh_script, "Checking this has the same effect as checking all 10 below."),
								CBOX_DI("Custom W. 1", misc1, sh_script1),
								CBOX_DI("Custom W. 2", misc1, sh_script2),
								CBOX_DI("Custom W. 3", misc1, sh_script3),
								CBOX_DI("Custom W. 4", misc1, sh_script4),
								CBOX_DI("Custom W. 5", misc1, sh_script5),
								CBOX_DI("Custom W. 6", misc1, sh_script6),
								CBOX_DI("Custom W. 7", misc1, sh_script7),
								CBOX_DI("Custom W. 8", misc1, sh_script8),
								CBOX_DI("Custom W. 9", misc1, sh_script9),
								CBOX_DI("Custom W. 10", misc1, sh_script10),
								CBOX_DI("Light Beams", misc1, sh_lightbeam)
							)
						),
						TabRef(name = "Reflect",
							Columns<8>(
								CBOX_DI("Rocks", misc2, sh_rock),
								CBOX_I("Arrows", misc2, sh_arrow, "Might not be reflectable, depending on 'Broken Enemy Fire/Arrow Reflecting'."+QRHINT({qr_BROKEN_FLAME_ARROW_REFLECTING})),
								push_widg(-1,CBOX_DI("Boomerangs", misc2, sh_brang)),
								CBOX_DI("Fireballs", misc2, sh_fireball),
								CBOX_DI("Fireballs (Boss)", misc2, sh_fireball2),
								CBOX_DI("Swords", misc2, sh_sword),
								CBOX_DI("Magic", misc2, sh_magic),
								CBOX_I("Flames", misc2, sh_flame, "Might not be reflectable, depending on 'Broken Enemy Fire/Arrow Reflecting'."+QRHINT({qr_BROKEN_FLAME_ARROW_REFLECTING})),
								CBOX_I("Flames 2", misc2, sh_flame2, "Might not be reflectable, depending on 'Broken Enemy Fire/Arrow Reflecting'."+QRHINT({qr_BROKEN_FLAME_ARROW_REFLECTING})),
								CBOX_I("Custom W. (ALL)", misc2, sh_script, "Checking this has the same effect as checking all 10 below."),
								CBOX_DI("Custom W. 1", misc2, sh_script1),
								CBOX_DI("Custom W. 2", misc2, sh_script2),
								CBOX_DI("Custom W. 3", misc2, sh_script3),
								CBOX_DI("Custom W. 4", misc2, sh_script4),
								CBOX_DI("Custom W. 5", misc2, sh_script5),
								CBOX_DI("Custom W. 6", misc2, sh_script6),
								CBOX_DI("Custom W. 7", misc2, sh_script7),
								CBOX_DI("Custom W. 8", misc2, sh_script8),
								CBOX_DI("Custom W. 9", misc2, sh_script9),
								CBOX_DI("Custom W. 10", misc2, sh_script10),
								CBOX_DI("Light Beams", misc2, sh_lightbeam)
							)
						)
					)
				),
				TabRef(name = "Other",
					Row(
						Column(
							Row(
								Frame(title = "Protects",
									info = "These sides of the Hero are protected while the shield is in effect",
									Column(padding = 0_px,
										CBOX("Front", flags, item_flag1),
										CBOX("Back", flags, item_flag2),
										CBOX("Left", flags, item_flag3),
										CBOX("Right", flags, item_flag4)
									)
								),
								push_widg(0,Column(
									Frame(title = "Inactive Prot",
										info = "These sides of the Hero are protected while the shield is NOT in effect",
										Column(padding = 0_px,
											CBOX("Front", flags, item_flag5),
											CBOX("Back", flags, item_flag6),
											CBOX("Left", flags, item_flag7),
											CBOX("Right", flags, item_flag8)
										)
									)
								)),
								push_widg(0,Column(
									Frame(title = "Buttons:",
										info = "If checked, these buttons will activate the shield, even if it isn't assigned to a button.",
										_INTBTN_PANEL_HELPER(local_ref.misc5)
									)
								))
							),
							Rows<3>(
								Label(text = "Block SFX"),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_ref.usesound,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.usesound = val;
									}),
								INFOBTN("Plays when the shield successfully blocks or reflects a weapon"),
								//
								push_widg(0,Label(text = "Active SFX")),
								push_widg(0,DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_ref.usesound2,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.usesound2 = val;
									})),
								push_widg(0,INFOBTN("Plays when the shield button is pressed"))
							)
						),
						Column(
							CBOX_I("Active Use", flags, item_flag9,
								"If enabled, the shield only protects the Hero (and provides"
								" its' LTM) while it is equipped to a button that is being held."),
							push_widg(0,CBOX_I("Change Speed", flags, item_flag10, "Change the Hero's walking speed while the shield is active")),
							push_widg(0,CBOX_I("Lock Direction", flags, item_flag11, "When the shield is activated, lock the Hero's direction until it is released.")),
							push_widg(0,Rows<3>(
								Label(text = "Inactive PTM:", hAlign = 1.0),
								TextField(maxLength = 11,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, val = local_ref.misc6,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.misc6 = val;
									}),
								INFOBTN("Hero Tile Modifier to use while shield is inactive"),
								//
								push_widg(1,Label(text = "Speed Percentage:", hAlign = 1.0)),
								push_widg(1,TextField(maxLength = 11,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, val = local_ref.misc7,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.misc7 = val;
									})),
								push_widg(1,INFOBTN("A percentage multiplier for the Hero's movement speed. A negative value will give that amount *more* speed; i.e. '-100' is the same as '200'.")),
								//
								push_widg(1,Label(text = "Speed Bonus:", hAlign = 1.0)),
								push_widg(1,TextField(maxLength = 11,
									type = GUI::TextField::type::INT_DECIMAL,
									fitParent = true, val = local_ref.misc8,
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										local_ref.misc8 = val;
									})),
								push_widg(1,INFOBTN("A step value (in 100ths of a pixel per frame) to be added to the Hero's speed."))
							))
						)
					)
				)
			));
			break;
		}
		case itype_note:
		{
			auto& messagestr = local_ref.misc1;
			
			lists[0] = GUI::ZCListData::strings(true);
			windowRow->add(Column(
				Rows<3>(
					Label(text = "String:", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = messagestr,
						onSelectFunc = [&](int32_t val)
						{
							messagestr = val;
						}),
					INFOBTN("The string to play. Negative values are special, reading the string number from somewhere else.")
				),
				Rows<3>(
					Label(text = "Opening Sound"),
					DropDownList(data = list_sfx,
						fitParent = true, selectedValue = local_ref.usesound,
						onSelectFunc = [&](int32_t val)
						{
							local_ref.usesound = val;
						}),
					INFOBTN("Plays when the note is opened")
				)
			));
			break;
		}
		default: //Should be unreachable
			wip = true;
			windowRow->add(Button(text = "Exit",minwidth = 90_px,onClick = message::CANCEL));
			break;
	}
	update(true);
	updateTitle();
	if(wip)
		InfoDialog("WIP","The '" + ityname + "' wizard is WIP,"
			" and may not represent all options.").show();
	return window;
}

static itemdata* _instance = nullptr;
static bool defaulted = false;
static bool def_some()
{
	item_default(*_instance);
	defaulted = true;
	return true;
}
bool do_item_default(itemdata& ref)
{
	defaulted = false;
	_instance = &ref;
	AlertFuncDialog("Are you sure?",
		"Reset the item attributes/flags to default?",
		""
	).add_buttons(1,
		{ "Yes", "Cancel" },
		{ def_some, nullptr }
	).show();
	return defaulted;
}
bool ItemWizardDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::UPDATE:
			return false;
		case message::OK:
			endUpdate();
			dest_ref = local_ref;
			return true;

		case message::DEFAULT:
		{
			if(do_item_default(local_ref))
				rerun_dlg = true;
			return rerun_dlg;
		}
		case message::CANCEL:
			return true;
		
		case message::RSET0: case message::RSET1: case message::RSET2: case message::RSET3: case message::RSET4:
		case message::RSET5: case message::RSET6: case message::RSET7: case message::RSET8: case message::RSET9:
			setRadio(int32_t(msg.message)-int32_t(message::RSET0), int32_t(msg.argument));
			update();
			return false;
	}
	return false;
}

std::shared_ptr<GUI::Widget> ItemWizardDialog::push_widg(int id, std::shared_ptr<GUI::Widget> widg)
{
	widgs[id].push_back(widg);
	return widg;
}

void ItemWizardDialog::disable(int id, bool dis)
{
	auto it = widgs.find(id);
	if(it == widgs.end())
		return;
	for(auto& ptr : it->second)
		ptr->setDisabled(dis);
}

