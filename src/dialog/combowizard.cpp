#include "combowizard.h"
#include "info.h"
#include "alert.h"
#include "base/zsys.h"
#include "../tiles.h"
#include "gui/builder.h"
#include "zc_list_data.h"

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
		// case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		// case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		// case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		// case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		// case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		// case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		// case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		// case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		// case cARMOS: case cBSGRAVE: case cGRAVE:
		// case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		// case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		// case cSTEPSFX: case cSWITCHHOOK: case cCSWITCHBLOCK:
		// case cSAVE: case cSAVE2:
		case cSLOPE:
			return true;
	}
	return false;
}

void call_combo_wizard(ComboEditorDialog& dlg)
{
	ComboWizardDialog(dlg).show();
}

void ComboWizardDialog::update()
{
	switch(local_ref.type)
	{
		case cSLOPE:
		{
			tfs[0]->setVal(local_ref.attrishorts[0]);
			tfs[1]->setVal(local_ref.attrishorts[1]);
			tfs[2]->setVal(local_ref.attrishorts[2]);
			tfs[3]->setVal(local_ref.attrishorts[3]);
			tfs[4]->setVal(local_ref.attributes[0]);
			break;
		}
	}
}

std::shared_ptr<GUI::Widget> ComboWizardDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	std::shared_ptr<GUI::Grid> windowRow;
	window = Window(
		//use_vsync = true,
		title = "Combo Wizard",
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
	
	switch(local_ref.type)
	{
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
					Label(text = parent.l_attrishort[0], textAlign = 2),
					tfs[0] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = x1,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							x1 = val;
						}),
					INFOBTN(parent.h_attrishort[0]),
					//
					Label(text = parent.l_attrishort[1], textAlign = 2),
					tfs[1] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = y1,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							y1 = val;
						}),
					INFOBTN(parent.h_attrishort[1]),
					//
					Label(text = parent.l_attrishort[2], textAlign = 2),
					tfs[2] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = x2,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							x2 = val;
						}),
					INFOBTN(parent.h_attrishort[2]),
					//
					Label(text = parent.l_attrishort[3], textAlign = 2),
					tfs[3] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_SSHORT,
						low = -32768, high = 32767, val = y2,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							y2 = val;
						}),
					INFOBTN(parent.h_attrishort[3]),
					//
					Label(text = parent.l_attribute[0], textAlign = 2),
					tfs[4] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_ZSINT,
						val = slip,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							slip = val;
						}),
					INFOBTN(parent.h_attribute[0])
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
		default: //Should be unreachable
			windowRow->add(Button(text = "Exit",minwidth = 90_lpx,onClick = message::CANCEL));
			break;
	}
	
	return window;
}

bool ComboWizardDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			parent.local_comboref = local_ref;
			return true;

		case message::CANCEL:
		default:
			return true;
	}
	return false;
}

