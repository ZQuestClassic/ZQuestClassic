#include "combowizard.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include "../tiles.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include "weapons.h"

extern bool saved;
extern zcmodule moduledata;
extern newcombo *combobuf;
extern comboclass *combo_class_buf;
extern int32_t CSet;
extern int32_t numericalFlags;
extern script_data *comboscripts[NUMSCRIPTSCOMBODATA];
char *ordinal(int32_t num);
using std::string;
using std::to_string;

bool hasComboWizard(int32_t type)
{
	switch(type)
	{
		// case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
		// case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
		// case cTRIGGERGENERIC: case cCSWITCH: case cSIGNPOST:
		// case cSLASH: case cSLASHITEM: case cBUSH: case cFLOWERS: case cTALLGRASS:
		// case cTALLGRASSNEXT:case cSLASHNEXT: case cSLASHNEXTITEM: case cBUSHNEXT:
		// case cSLASHTOUCHY: case cSLASHITEMTOUCHY: case cBUSHTOUCHY: case cFLOWERSTOUCHY:
		// case cTALLGRASSTOUCHY: case cSLASHNEXTTOUCHY: case cSLASHNEXTITEMTOUCHY:
		// case cBUSHNEXTTOUCHY: case cSTEP: case cSTEPSAME: case cSTEPALL:
		// case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		// case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		// case cARMOS: case cBSGRAVE: case cGRAVE:
		// case cSTEPSFX: case cSWITCHHOOK: case cCSWITCHBLOCK:
		// case cSAVE: case cSAVE2:
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		case cSLOPE: case cSHOOTER:
			return true;
	}
	return false;
}

#define RELOAD() parent.loadComboType(&local_ref)

void call_combo_wizard(ComboEditorDialog& dlg)
{
	ComboWizardDialog(dlg).show();
}

ComboWizardDialog::ComboWizardDialog(ComboEditorDialog& parent) : parent(parent),
	local_ref(parent.local_comboref), flags(0),
	list_sprites(GUI::ZCListData::miscsprites()),
	list_lwscript(GUI::ZCListData::lweapon_script()),
	list_ewscript(GUI::ZCListData::eweapon_script())
{
	memset(rs_sz, 0, sizeof(rs_sz));
}

static const GUI::ListData list_dirs
{
	{ "Up", 0 },
	{ "Down", 1 },
	{ "Left", 2 },
	{ "Right", 3 },
	{ "Left-Up", 4 },
	{ "Right-Up", 5 },
	{ "Left-Down", 6 },
	{ "Right-Down", 7 }
};

void ComboWizardDialog::setRadio(size_t rs, size_t ind)
{
	for(size_t q = 0; q < rs_sz[rs]; ++q)
	{
		auto& radio = rset[rs][q];
		radio->setChecked(ind==q);
	}
}
size_t ComboWizardDialog::getRadio(size_t rs)
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

void ComboWizardDialog::update(bool first)
{
	switch(local_ref.type)
	{
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		{
			auto rad0 = getRadio(0);
			SETFLAG(local_ref.usrflags,cflag1,rad0);
			tfs[0]->setDisabled(!rad0);
			break;
		}
		case cSLOPE:
		{
			tfs[0]->setVal(local_ref.attrishorts[0]);
			tfs[1]->setVal(local_ref.attrishorts[1]);
			tfs[2]->setVal(local_ref.attrishorts[2]);
			tfs[3]->setVal(local_ref.attrishorts[3]);
			tfs[4]->setVal(local_ref.attributes[0]);
			break;
		}
		case cSHOOTER:
		{
			auto& weap_type = local_ref.attribytes[1];
			bool lw = weap_type < wEnemyWeapons;
			if(weap_type >= wScript1 && weap_type <= wScript10)
				lw = (local_ref.usrflags&cflag5)!=0;
			if(first || (!(flags&1) != !lw)) //init or lw status changed
			{
				switcher[0]->switchTo(lw?0:1);
				ddls[lw?3:4]->setSelectedValue(local_ref.attribytes[5]);
				SETFLAG(flags, 1, lw);
			}
			//
			size_t adir = getRadio(0);
			ddls[5]->setDisabled(adir!=0);
			tfs[2]->setDisabled(adir!=1);
			break;
		}
	}
}
void ComboWizardDialog::endUpdate()
{
	update();
	switch(local_ref.type)
	{
		case cSHOOTER:
		{
			//Angle stuff
			size_t adir = getRadio(0);
			SETFLAG(local_ref.usrflags,cflag1,adir!=0);
			int32_t& a0 = local_ref.attributes[0];
			switch(adir)
			{
				case 0:
					a0 = ddls[3]->getSelectedValue()*10000;
					break;
				case 1: //Angle
					a0 = tfs[2]->getVal();
					a0 = SMART_WRAP(a0, 360*10000);
					break;
				case 2:
					a0 = -1*10000;
					break;
				case 3:
					a0 = -2*10000;
					break;
				case 4:
					a0 = -3*10000;
					break;
			}
			//Rate stuff
			int16_t& rate = local_ref.attrishorts[0];
			int16_t& high_rate = local_ref.attrishorts[1];
			SETFLAG(local_ref.usrflags,cflag2,high_rate != rate);
			if(high_rate != rate)
			{
				if(high_rate < rate)
					zc_swap(high_rate,rate);
			}
			else if(parent.local_comboref.usrflags&cflag2)
			{
				high_rate = 0;
			}
			else
			{
				high_rate = parent.local_comboref.attrishorts[1];
			}
			//Proximity stuff
			int32_t& a1 = local_ref.attributes[1];
			if(!(local_ref.usrflags&cflag4))
			{
				if(!(parent.local_comboref.usrflags&cflag4))
					a1 = parent.local_comboref.attributes[1];
				else a1 = 0;
			}
			//Spread stuff
			byte& b3 = local_ref.attribytes[3];
			int32_t& a3 = local_ref.attributes[3];
			if(!(local_ref.usrflags&cflag7))
			{
				if(!(parent.local_comboref.usrflags&cflag7))
				{
					a3 = parent.local_comboref.attributes[3];
					b3 = parent.local_comboref.attribytes[3];
				}
				else
				{
					a3 = 0;
					b3 = 0;
				}
			}
			break;
		}
	}
}

#define IH_BTN(hei, inf) \
Button(height = hei, text = "?", \
	onPressFunc = [=]() \
	{ \
		InfoDialog("Info",inf).show(); \
	})
#define DDH sized(16_px, 21_px)

void ComboWizardDialog::updateTitle()
{
	switch(local_ref.type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
			ctyname = "Warp";
			break;
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
			ctyname = "Damage";
			break;
		default:
			ctyname = ZI.getComboTypeName(local_ref.type);
			break;
	}
	window->setTitle("Combo Wizard (" + ctyname + ")");
}
std::shared_ptr<GUI::Widget> ComboWizardDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	std::shared_ptr<GUI::Grid> windowRow;
	window = Window(
		//use_vsync = true,
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			windowRow = Row(padding = 0_px),
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
	
	thelp = getComboTypeHelpText(local_ref.type);
	
	bool wip = false;
	switch(local_ref.type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		{
			byte& warp_sfx = local_ref.attribytes[0];
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem const& itm){return isWarpType(itm.value);});
			windowRow->add(
				Rows<3>(
					Label(text = "Type:", hAlign = 1.0),
					ddls[0] = DropDownList(data = lists[0],
						fitParent = true, selectedValue = local_ref.type,
						onSelectFunc = [&](int32_t val)
						{
							local_ref.type = val;
						}),
					INFOBTN(thelp),
					//
					Label(text = "Warp Sound:", hAlign = 1.0),
					ddls[1] = DropDownList(data = parent.list_sfx,
						fitParent = true, selectedValue = warp_sfx,
						onSelectFunc = [&](int32_t val)
						{
							warp_sfx = val;
						}),
					INFOBTN(parent.h_attribyte[0])
				)
			);
			break;
		}
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		{
			int32_t& damage = local_ref.attributes[0];
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem const& itm){return isDamageType(itm.value);});
			rs_sz[0] = 2;
			windowRow->add(
				Column(padding = 0_px,
					Rows<3>(
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = !(local_ref.usrflags&cflag1),
							text = "Type-based Damage",
							indx = 0,
							onToggle = message::RSET0
						),
						//Label(text = "Type:", hAlign = 1.0),
						ddls[0] = DropDownList(data = lists[0],
							fitParent = true, selectedValue = local_ref.type,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.type = val;
								updateTitle();
							}),
						INFOBTN(thelp),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag1,
							text = "Custom Damage",
							indx = 1,
							onToggle = message::RSET0
						),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = damage, disabled = !(local_ref.usrflags&cflag1),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								damage = val;
							}),
						INFOBTN("The amount of damage, in HP, to deal. Negative amounts heal."
							"\nFor healing, the lowest healing amount combo you"
							" are standing on takes effect."
							"\nFor damage, the greatest amount takes priority unless"
							" 'Quest->Options->Combos->Lesser Damage Combos Take Priority' is checked.")
						//
					),
					Rows<2>(
						INFOBTN("Does not knock the player back when damaging them if checked."
							" Otherwise, knocks the player in the direction opposite"
							" the one they face."),
						cboxes[0] = Checkbox(
							text = "No Knockback",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag2, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag2,state);
							}
						)
					)
				)
			);
			break;
		}
		case cSLOPE:
		{
			local_ref.walk -= local_ref.walk & 0x0F; //nonsolid combo
			int16_t& x1 = local_ref.attrishorts[0];
			int16_t& y1 = local_ref.attrishorts[1];
			int16_t& x2 = local_ref.attrishorts[2];
			int16_t& y2 = local_ref.attrishorts[3];
			int32_t& slip = local_ref.attributes[0];
			windowRow->add(Row(
				Rows<3>(
					Label(text = "X Offset 1:", hAlign = 1.0),
					tfs[0] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = x1,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							x1 = val;
						}),
					Button(text = "?", rowSpan = 4, fitParent = true,
						onPressFunc = [=]()
						{
							InfoDialog("Info","The starting and ending X,Y"
								" points of the slope line, as offsets from"
								" the top-left of the combo.").show();
						}),
					//
					Label(text = "Y Offset 1:", hAlign = 1.0),
					tfs[1] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = y1,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							y1 = val;
						}),
					//button above rowspans here
					//
					Label(text = "X Offset 2:", hAlign = 1.0),
					tfs[2] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = x2,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							x2 = val;
						}),
					//button above rowspans here
					//
					Label(text = "Y Offset 2:", hAlign = 1.0),
					tfs[3] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = y2,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							y2 = val;
						}),
					//button above rowspans here
					//
					Label(text = "Slipperiness:", hAlign = 1.0),
					tfs[4] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_ZSINT,
						val = slip,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							slip = val;
						}),
					INFOBTN("Pixels per frame to slide down the slope in sideview.")
				),
				Rows<4>(
					Label(colSpan = 4, text = "Preset Slopes"),
					//
					Button(text = "1x1", fitParent = true,
						onPressFunc = [&]()
						{
							if(x1 < x2)
							{
								x1 = 0;
								x2 = 15;
							}
							else
							{
								x1 = 15;
								x2 = 0;
							}
							if(y1 < y2)
							{
								y1 = 0;
								y2 = 15;
							}
							else
							{
								y1 = 15;
								y2 = 0;
							}
							update();
						}),
					Button(text = "DownSlope", fitParent = true,
						onPressFunc = [&]()
						{
							if((y1 < y2) != (x1 < x2))
							{
								zc_swap(y1,y2);
								update();
							}
						}),
					Button(text = "UpSlope", fitParent = true,
						onPressFunc = [&]()
						{
							if((y1 < y2) == (x1 < x2))
							{
								zc_swap(y1,y2);
								update();
							}
						}),
					DummyWidget(),
					//
					Label(colSpan = 4, text = "Aligns"),
					Button(text = "TopLeft", fitParent = true,
						onPressFunc = [&]()
						{
							int32_t wid = abs(x1-x2), hei = abs(y1-y2);
							bool up = (y1 < y2) != (x1 < x2);
							x1 = 0;
							x2 = wid;
							y1 = up ? hei : 0;
							y2 = up ? 0 : hei;
							update();
						}),
					Button(text = "TopRight", fitParent = true,
						onPressFunc = [&]()
						{
							int32_t wid = abs(x1-x2), hei = abs(y1-y2);
							bool up = (y1 < y2) != (x1 < x2);
							x1 = 15-wid;
							x2 = 15;
							y1 = up ? hei : 0;
							y2 = up ? 0 : hei;
							update();
						}),
					Button(text = "BottomLeft", fitParent = true,
						onPressFunc = [&]()
						{
							int32_t wid = abs(x1-x2), hei = abs(y1-y2);
							bool up = (y1 < y2) != (x1 < x2);
							x1 = 0;
							x2 = wid;
							y1 = up ? 15 : 15-hei;
							y2 = up ? 15-hei : 15;
							update();
						}),
					Button(text = "BottomRight", fitParent = true,
						onPressFunc = [&]()
						{
							int32_t wid = abs(x1-x2), hei = abs(y1-y2);
							bool up = (y1 < y2) != (x1 < x2);
							x1 = 15-wid;
							x2 = 15;
							y1 = up ? 15 : 15-hei;
							y2 = up ? 15-hei : 15;
							update();
						})
				)
			));
			break;
		}
		case cSHOOTER:
		{
			byte& shot_sfx = local_ref.attribytes[0];
			byte& weap_type = local_ref.attribytes[1];
			byte& weap_sprite = local_ref.attribytes[2];
			int16_t& damage = local_ref.attrishorts[2];
			int32_t& step = local_ref.attributes[2];
			
			byte& unblockable = local_ref.attribytes[4];
			byte& script = local_ref.attribytes[5];
			
			int32_t& angle_dir = local_ref.attributes[0];
			
			int16_t& rate = local_ref.attrishorts[0];
			int16_t& high_rate = local_ref.attrishorts[1];
			if(!(local_ref.usrflags&cflag2))
				high_rate = rate;
			
			int32_t& prox = local_ref.attributes[1];
			if(prox < 0) prox = 0;
			
			byte& shot_count = local_ref.attribytes[3];
			int32_t& spread = local_ref.attributes[3];
			spread = SMART_WRAP(spread, 360*10000);
			if(shot_count < 2) shot_count = 2;
			
			size_t seladir = 0;
			int32_t angle = 0;
			int32_t dir = 0;
			if(local_ref.usrflags&cflag1)
			{
				if(angle_dir < 0)
				{
					seladir = 4;
					switch(angle_dir/10000)
					{
						case -1:
							seladir = 2;
							break;
						case -2:
							seladir = 3;
							break;
						case -3:
							break;
					}
				}
				else
				{
					seladir = 1;
					angle = vbound(angle_dir,0,3599999);
					dir = AngleToDir(WrapAngle(DegreesToRadians(angle/10000.0)));
				}
			}
			else
			{
				dir = NORMAL_DIR(angle_dir / 10000);
			}
			
			rs_sz[0] = 5;
			
			static size_t tabpos = 0;
			windowRow->add(TabPanel(ptr = &tabpos,
				TabRef(name = "1", Rows<2>(
					Rows<3>(
						Label(text = "Shot SFX:", hAlign = 1.0),
						ddls[0] = DropDownList(data = parent.list_sfx,
							fitParent = true, selectedValue = shot_sfx,
							onSelectFunc = [&](int32_t val)
							{
								shot_sfx = val;
							}),
						INFOBTN("SFX to play when shooting a weapon"),
						//
						Label(text = "Sprite:", hAlign = 1.0),
						ddls[2] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = weap_sprite,
							onSelectFunc = [&](int32_t val)
							{
								weap_sprite = val;
							}),
						INFOBTN("The sprite of the spawned weapon"),
						//
						Label(text = "Weapon Type:", hAlign = 1.0),
						ddls[1] = DropDownList(data = parent.list_weaptype,
							fitParent = true, selectedValue = weap_type,
							onSelectFunc = [&](int32_t val)
							{ 
								weap_type = val;
								update();
							}),
						INFOBTN("The LWeapon or EWeapon ID to be shot"),
						//
						Label(text = "Script:", hAlign = 1.0),
						switcher[0] = Switcher(
							ddls[3] = DropDownList(data = list_lwscript,
								fitParent = true, selectedValue = script,
								onSelectFunc = [&](int32_t val)
								{
									script = val;
								}),
							ddls[4] = DropDownList(data = list_ewscript,
								fitParent = true, selectedValue = script,
								onSelectFunc = [&](int32_t val)
								{
									script = val;
								})
						),
						INFOBTN("LWeapon or EWeapon script ID to attach to the fired weapons."
								"\nNote that there is no way to supply InitD to such scripts.")
					),
					Rows<2>(
						Label(text = "Unblockable"),
						INFOBTNL("The following checkboxes can make the weapon bypass"
							"types of blocking."),
						cboxes[0] = Checkbox(
							text = "Bypass 'Block' Defense",
							hAlign = 0.0, colSpan = 2,
							checked = unblockable & WPNUNB_BLOCK, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(unblockable,WPNUNB_BLOCK,state);
							}
						),
						cboxes[1] = Checkbox(
							text = "Bypass 'Ignore' Defense",
							hAlign = 0.0, colSpan = 2,
							checked = unblockable & WPNUNB_IGNR, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(unblockable,WPNUNB_IGNR,state);
							}
						),
						cboxes[2] = Checkbox(
							text = "Bypass enemy/player shield blocking",
							hAlign = 0.0, colSpan = 2,
							checked = unblockable & WPNUNB_SHLD, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(unblockable,WPNUNB_SHLD,state);
							}
						),
						cboxes[3] = Checkbox(
							text = "Bypass player shield reflecting",
							hAlign = 0.0, colSpan = 2,
							checked = unblockable & WPNUNB_REFL, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(unblockable,WPNUNB_REFL,state);
							}
						)
					),
					Row(colSpan = 2, padding = 0_px,
						Rows<2>(padding = 0_px,
							cboxes[6] = Checkbox(
								text = "Insta-shot", hAlign = 0.0,
								checked = local_ref.usrflags&cflag3,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag3,state);
								}
							),
							INFOBTN("Shoots when the timer starts, rather than ends"),
							cboxes[7] = Checkbox(
								text = "Custom Weapons are LWeapons", hAlign = 0.0,
								checked = local_ref.usrflags&cflag5,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag5,state);
									update();
								}
							),
							INFOBTN("If a 'Custom Weapon' ID is used, it will be treated as an LWeapon with this checked, and an EWeapon otherwise.")
						),
						Rows<2>(padding = 0_px,
							cboxes[8] = Checkbox(
								text = "Auto-rotate sprite", hAlign = 0.0,
								checked = local_ref.usrflags&cflag6,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag6,state);
								}
							),
							INFOBTN("Attempt to rotate the sprite to match the weapon's angle"),
							cboxes[9] = Checkbox(
								text = "Boss Fireball", hAlign = 0.0,
								checked = local_ref.usrflags&cflag8,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag8,state);
								}
							),
							INFOBTN("If a fireball weapon type is used, it will be considered a 'boss' fireball.")
						)
					)
				)),
				TabRef(name = "2", Row(
					Rows<3>(
						Label(text = "Damage", hAlign = 1.0),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_SSHORT,
							low = -32768, high = 32767, val = damage,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								damage = val;
							}),
						INFOBTN("The damage of the spawned weapon"),
						//
						Label(text = "Step Speed:", hAlign = 1.0),
						tfs[1] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = step,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								step = val;
							}),
						INFOBTN("The speed of the weapon, in 100ths px/frame"),
						//
						Label(text = "Min Rate:", hAlign = 1.0),
						tfs[3] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_SSHORT,
							low = 0, high = 32767, val = rate,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								rate = val;
							}),
						Button(text = "?", rowSpan = 2, fitParent = true,
							onPressFunc = [=]()
							{
								InfoDialog("Info","The fire rates of the shooter."
									" If 2 different rates are given, the rate will vary randomly"
									" between the 2 rates."
									"\nIf both rates are '0', the shooter will not fire"
									" (unless triggered via the Triggers tab's 'ComboType Effects')").show();
							}),
						//
						Label(text = "Max Rate:", hAlign = 1.0),
						tfs[4] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_SSHORT,
							low = 0, high = 32767, val = high_rate,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								high_rate = val;
							}),
						//button above rowspans here
						//
						cboxes[4] = Checkbox(
							text = "Stop Proximity:", hAlign = 1.0,
							checked = local_ref.usrflags&cflag4,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag4,state);
								tfs[5]->setDisabled(!state);
							}
						),
						tfs[5] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							low = 0, val = prox, disabled = !(local_ref.usrflags&cflag4),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								prox = val;
							}),
						INFOBTN("If enabled, the shooter will stop shooting when the player"
							" is within this distance (in pixels) of the combo."),
						//
						cboxes[5] = Checkbox(
							text = "Multishot", colSpan = 3,
							checked = local_ref.usrflags&cflag7,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag7,state);
								tfs[6]->setDisabled(!state);
								tfs[7]->setDisabled(!state);
							}
						),
						//
						Label(text = "Shot Count:", hAlign = 1.0),
						tfs[6] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 2, high = 255, val = shot_count,
							disabled = !(local_ref.usrflags&cflag7),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								shot_count = val;
							}),
						INFOBTN("How many shots to shoot, if Multishot is enabled"),
						//
						Label(text = "Shot Spread:", hAlign = 1.0),
						tfs[7] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = spread,
							disabled = !(local_ref.usrflags&cflag7),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								spread = val;
							}),
						INFOBTN("Angle in degrees between shots, if Multishot is enabled")
					),
					Rows<3>(
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = seladir==0,
							text = "Direction",
							indx = 0,
							onToggle = message::RSET0
						),
						IH_BTN(DDH,"The direction to shoot in"),
						ddls[5] = DropDownList(data = list_dirs,
							fitParent = true, selectedValue = dir,
							disabled = seladir != 0,
							onSelectFunc = [&](int32_t val)
							{
								angle_dir = val*10000;
							}),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = seladir==1,
							text = "Angle",
							indx = 1,
							onToggle = message::RSET0
						),
						IH_BTN(DDH,"The angle to shoot in"),
						tfs[2] = TextField(
							forceFitH = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							disabled = seladir != 1,
							val = angle,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								angle_dir = val;
							}),
						//
						rset[0][2] = Radio(
							hAlign = 0.0,
							checked = seladir==2,
							text = "Aimed 4-Dir",
							indx = 2,
							onToggle = message::RSET0
						),
						IH_BTN(DDH,"Aim 4-directionally at the player"),
						DummyWidget(),
						//
						rset[0][3] = Radio(
							hAlign = 0.0,
							checked = seladir==3,
							text = "Aimed 8-Dir",
							indx = 3,
							onToggle = message::RSET0
						),
						IH_BTN(DDH,"Aim 8-directionally at the player"),
						DummyWidget(),
						//
						rset[0][4] = Radio(
							hAlign = 0.0,
							checked = seladir==4,
							text = "Aimed 360",
							indx = 4,
							onToggle = message::RSET0
						),
						IH_BTN(DDH,"Aim angularly at the player"),
						DummyWidget()
						//
					)
				))
			));
			break;
		}
		default: //Should be unreachable
			wip = true;
			windowRow->add(Button(text = "Exit",minwidth = 90_lpx,onClick = message::CANCEL));
			break;
	}
	update(true);
	updateTitle();
	if(wip)
		InfoDialog("WIP","The '" + ctyname + "' wizard is WIP,"
			" and may not be fully functional!").show();
	return window;
}

bool ComboWizardDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::UPDATE:
			return false;
		case message::OK:
			endUpdate();
			parent.local_comboref = local_ref;
			return true;

		case message::CANCEL:
		default:
			return true;
		
		case message::RSET0: case message::RSET1: case message::RSET2: case message::RSET3: case message::RSET4:
		case message::RSET5: case message::RSET6: case message::RSET7: case message::RSET8: case message::RSET9:
			setRadio(int32_t(msg.message)-int32_t(message::RSET0), int32_t(msg.argument));
			update();
			return false;
	}
	return false;
}

