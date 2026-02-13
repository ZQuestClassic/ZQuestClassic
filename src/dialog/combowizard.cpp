#include "combowizard.h"
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
#include "weap_data_editor.h"

extern int32_t CSet;
extern int32_t numericalFlags;
extern script_data *comboscripts[NUMSCRIPTSCOMBODATA];

char *ordinal(int32_t num);
using std::string;
using std::to_string;

bool is_misc_lweapon(newcombo const& cmb);

bool hasComboWizard(int32_t type)
{
	switch(type)
	{
		// Not adding
		// case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
		// case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
		// case cSLASH: case cSLASHTOUCHY:
		//!Todo combo wizards -Em
		// case cTRIGGERGENERIC: case cCSWITCH:
		// case cSWITCHHOOK: case cCSWITCHBLOCK:
		case cSAVE: case cSAVE2:
		case cCUTSCENETRIG: case cCUTSCENEEFFECT:
		case cSIGNPOST:
		case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		case cLOCKBLOCK2: case cBOSSLOCKBLOCK2:
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
		case cSLASHNEXT:
		case cTALLGRASS: case cTALLGRASSTOUCHY: case cTALLGRASSNEXT:
		case cBUSH: case cBUSHTOUCHY: case cFLOWERS: case cSLASHNEXTTOUCHY:
		case cSLASHITEM: case cSLASHNEXTITEMTOUCHY:
		case cSLASHNEXTITEM: case cBUSHNEXT: case cSLASHITEMTOUCHY:
		case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
		case cCVUP: case cCVDOWN: case cCVLEFT: case cCVRIGHT:
		case cARMOS: case cGRAVE: case cBSGRAVE:
		case cSTEP: case cSTEPSAME: case cSTEPALL: case cSTEPCOPY:
		case cTRIGNOFLAG: case cSTRIGNOFLAG:
		case cTRIGFLAG: case cSTRIGFLAG:
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		case cSLOPE: case cSHOOTER: case cWATER: case cSHALLOWWATER:
		case cSTEPSFX: case cTORCH: case cMIRRORNEW: case cCRUMBLE:
		case cMIRROR: case cMIRRORSLASH: case cMIRRORBACKSLASH: case cMAGICPRISM: case cMAGICPRISM4:
		case cICY:
			return true;
	}
	return false;
}

void call_combo_wizard(ComboEditorDialog& dlg)
{
	ComboWizardDialog(dlg).show();
}

ComboWizardDialog::ComboWizardDialog(ComboEditorDialog& parent) : local_ref(parent.local_comboref),
	dest_ref(parent.local_comboref), src_ref(parent.local_comboref),
	parent(parent), flags(0),
	list_lwscript(GUI::ZCListData::lweapon_script()),
	list_ewscript(GUI::ZCListData::eweapon_script()),
	list_sprites(GUI::ZCListData::miscsprites()),
	list_dropsets(GUI::ZCListData::dropsets(true)),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_counters(GUI::ZCListData::counters(true,true)),
	list_dirs(GUI::ZCListData::dirs(8,false)),
	list_torch_shapes(GUI::ZCListData::light_shapes()),
	alt_refs()
{
	memset(rs_sz, 0, sizeof(rs_sz));
}

static const GUI::ListData list_clippings
{
	{ "Bush Leaves", 0 },
	{ "Flowers", 2 },
	{ "Grass", 3 },
};

static const GUI::ListData list_chest_content
{
	{ "Screen Catchall ('Special Item')", -1 },
	{ "Screen->D[0]", -10 },
	{ "Screen->D[1]", -11 },
	{ "Screen->D[2]", -12 },
	{ "Screen->D[3]", -13 },
	{ "Screen->D[4]", -14 },
	{ "Screen->D[5]", -15 },
	{ "Screen->D[6]", -16 },
	{ "Screen->D[7]", -17 },
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
		case cCVUP: case cCVDOWN: case cCVLEFT: case cCVRIGHT:
		{
			auto rad0 = getRadio(0);
			SETFLAG(local_ref.usrflags,cflag2,rad0);
			tfs[0]->setDisabled(!rad0);
			tfs[1]->setDisabled(!rad0);
			tfs[2]->setDisabled(!rad0);
			break;
		}
		case cTALLGRASS: case cTALLGRASSTOUCHY: case cTALLGRASSNEXT:
			[[fallthrough]];
		case cBUSH: case cBUSHTOUCHY: case cFLOWERS: case cSLASHNEXTTOUCHY:
		case cSLASHITEM: case cSLASHNEXTITEMTOUCHY:
		case cSLASHNEXTITEM: case cBUSHNEXT: case cSLASHITEMTOUCHY:
		case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
		{
			auto rad0 = getRadio(0);
			ddls[0]->setDisabled(rad0 != 1);
			ddls[1]->setDisabled(rad0 != 2);
			auto rad1 = getRadio(1);
			ddls[2]->setDisabled(rad1 != 1);
			ddls[3]->setDisabled(rad1 != 2);
			auto rad2 = getRadio(2);
			ddls[4]->setDisabled(rad2 != 1);
			break;
		}
		case cSLASHNEXT:
		{
			auto rad0 = getRadio(0);
			ddls[0]->setDisabled(rad0 != 1);
			ddls[1]->setDisabled(rad0 != 2);
			auto rad2 = getRadio(2);
			ddls[4]->setDisabled(rad2 != 1);
			break;
		}
		case cSTEP: case cSTEPSAME: case cSTEPALL: case cSTEPCOPY:
		{
			auto r0 = getRadio(0);
			local_ref.type = cSTEP+r0;
			cboxes[0]->setDisabled(r0==3);
			ddls[0]->setDisabled(r0==3);
			ddls[1]->setDisabled(r0==3);
			break;
		}
		case cWATER: case cSHALLOWWATER:
		{
			bool hpmod = local_ref.usrflags&cflag2;
			tfs[2]->setDisabled(!hpmod);
			tfs[3]->setDisabled(!hpmod);
			ddls[2]->setDisabled(!hpmod);
			tfs[4]->setDisabled(!hpmod);
			ddls[3]->setDisabled(!hpmod);
			cboxes[2]->setDisabled(!hpmod);
			cboxes[3]->setDisabled(!hpmod);
			cboxes[4]->setDisabled(!hpmod);
			break;
		}
		case cSLOPE:
		{
			tfs[0]->setVal(local_ref.c_attributes[16]);
			tfs[1]->setVal(local_ref.c_attributes[17]);
			tfs[2]->setVal(local_ref.c_attributes[18]);
			tfs[3]->setVal(local_ref.c_attributes[19]);
			tfs[4]->setVal(local_ref.c_attributes[0].getZLong());
			break;
		}
		case cSHOOTER:
		{
			auto& weap_type = local_ref.c_attributes[9];
			bool lw = weap_type < wEnemyWeapons;
			if(weap_type >= wScript1 && weap_type <= wScript10)
				lw = (local_ref.usrflags&cflag5)!=0;
			if(first || (!(flags&1) != !lw)) //init or lw status changed
			{
				switcher[0]->switchTo(lw?0:1);
				ddls[lw?3:4]->setSelectedValue(local_ref.c_attributes[13]);
				SETFLAG(flags, 1, lw);
			}
			//
			size_t adir = getRadio(0);
			ddls[5]->setDisabled(adir!=0);
			tfs[2]->setDisabled(adir!=1);
			bool misc_w_data = (local_ref.usrflags & cflag10) != 0;
			ddls[3]->setDisabled(misc_w_data);
			ddls[4]->setDisabled(misc_w_data);
			lbls[0]->setDisabled(misc_w_data);
			btns[0]->setDisabled(misc_w_data);
			grids[0]->setDisabled(misc_w_data);
			break;
		}
		case cSTEPSFX:
		{
			grids[0]->setDisabled(!(local_ref.usrflags&cflag1));
			grids[1]->setDisabled(!(local_ref.usrflags&cflag1));
			break;
		}
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
		{
			size_t lvl = 0;
			if(local_ref.type == cCHEST)
				lvl = 1;
			else if(local_ref.type == cBOSSCHEST)
				lvl = 2;
			else if(local_ref.type == cLOCKEDCHEST)
				lvl = 3;
			
			tpan[0]->setDisabled(1, lvl < 1); //Opening
			tpan[0]->setDisabled(2, lvl < 1); //Content
			tpan[0]->setDisabled(3, lvl < 1); //Prompts
			tpan[0]->setDisabled(4, lvl < 2); //Locking
			grids[0]->setDisabled(lvl < 3); //Locking
			
			cboxes[0]->setDisabled(lvl < 1); //'Use Special Item State'
			//
			
			auto rad0 = getRadio(0);
			
			tfs[0]->setDisabled(rad0==0);
			
			auto rad1 = getRadio(1);
			
			ddls[2]->setDisabled(rad1!=0);
			ddls[3]->setDisabled(rad1!=1);
			
			//Prompts
			bool prompt = lvl > 0 && cboxes[1]->getChecked();
			bool lockprompt = lvl > 1 && prompt && cboxes[2]->getChecked();
			
			frames[0]->setDisabled(!prompt);
			frames[1]->setDisabled(!lockprompt);
			cmbswatches[1]->setCSet(cmbswatches[0]->getCSet());
			cboxes[2]->setDisabled(lvl < 2 || !prompt);
			
			//Locking
			bool itemkey = lvl == 3 && (local_ref.usrflags&cflag1);
			bool counterkey = lvl == 3 && !(itemkey && (local_ref.usrflags&cflag2));
			
			frames[2]->setDisabled(!itemkey);
			frames[3]->setDisabled(!counterkey);
			
			if(counterkey)
			{
				auto rad2 = getRadio(2);
				ddls[5]->setDisabled(rad2 != 1);
				
				rset[3][0]->setDisabled(rad2 != 1);
				btns[0]->setDisabled(rad2 != 1);
				rset[3][1]->setDisabled(rad2 != 1);
				btns[1]->setDisabled(rad2 != 1);
				rset[3][2]->setDisabled(rad2 != 1);
				btns[2]->setDisabled(rad2 != 1);
			}
			break;
		}
		case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		case cLOCKBLOCK2: case cBOSSLOCKBLOCK2:
		{
			size_t lvl = 0;
			if(local_ref.type == cBOSSLOCKBLOCK)
				lvl = 2;
			else if(local_ref.type == cLOCKBLOCK)
				lvl = 3;
			
			tpan[0]->setDisabled(1, lvl < 1); //Opening
			tpan[0]->setDisabled(2, lvl < 1); //Prompts
			tpan[0]->setDisabled(3, lvl < 2); //Locking
			grids[0]->setDisabled(lvl < 3); //Locking
			//
			
			auto rad0 = getRadio(0);
			
			tfs[0]->setDisabled(rad0==0);
			
			//Prompts
			bool prompt = lvl > 0 && cboxes[1]->getChecked();
			bool lockprompt = lvl > 1 && prompt && cboxes[2]->getChecked();
			
			frames[0]->setDisabled(!prompt);
			frames[1]->setDisabled(!lockprompt);
			cmbswatches[1]->setCSet(cmbswatches[0]->getCSet());
			cboxes[2]->setDisabled(lvl < 2 || !prompt);
			
			//Locking
			bool itemkey = lvl == 3 && (local_ref.usrflags&cflag1);
			bool counterkey = lvl == 3 && !(itemkey && (local_ref.usrflags&cflag2));
			
			frames[2]->setDisabled(!itemkey);
			frames[3]->setDisabled(!counterkey);
			
			if(counterkey)
			{
				auto rad2 = getRadio(2);
				ddls[5]->setDisabled(rad2 != 1);
				
				rset[3][0]->setDisabled(rad2 != 1);
				btns[0]->setDisabled(rad2 != 1);
				rset[3][1]->setDisabled(rad2 != 1);
				btns[1]->setDisabled(rad2 != 1);
				rset[3][2]->setDisabled(rad2 != 1);
				btns[2]->setDisabled(rad2 != 1);
			}
			break;
		}
		case cCUTSCENETRIG:
		{
			bool ending = (local_ref.usrflags & cflag1);
			cboxes[1]->setDisabled(ending);
			grids[0]->setDisabled(ending);
			ddls[0]->setDisabled(ending);
			break;
		}
		case cICY:
		{
			bool player = (local_ref.usrflags & cflag2);
			grids[0]->setDisabled(!player);
			break;
		}
		case cCUTSCENEEFFECT:
			break;
		case cSAVE: case cSAVE2:
			break;
	}
}
void ComboWizardDialog::endUpdate()
{
	update();
	switch(local_ref.type)
	{
		case cSTEPCOPY:
		{
			if(src_ref.type == cSTEPCOPY)
			{
				local_ref.c_attributes[8] = src_ref.c_attributes[8];
				local_ref.c_attributes[9] = src_ref.c_attributes[9];
				local_ref.usrflags = src_ref.usrflags;
			}
			else
			{
				local_ref.c_attributes[8] = 0;
				local_ref.c_attributes[9] = 0;
				local_ref.usrflags &= ~cflag1;
			}
			break;
		}
		case cWATER: case cSHALLOWWATER:
		{
			if(!(local_ref.usrflags&cflag2))
			{
				if(src_ref.usrflags&cflag2)
				{
					local_ref.c_attributes[9] = 0;
					local_ref.c_attributes[10] = 0;
					local_ref.c_attributes[11] = 0;
					local_ref.c_attributes[1] = 0;
					local_ref.c_attributes[2] = 0;
				}
				else
				{
					local_ref.c_attributes[9] = src_ref.c_attributes[9];
					local_ref.c_attributes[10] = src_ref.c_attributes[10];
					local_ref.c_attributes[11] = src_ref.c_attributes[11];
					local_ref.c_attributes[1] = src_ref.c_attributes[1];
					local_ref.c_attributes[2] = src_ref.c_attributes[2];
					local_ref.usrflags &= ~(cflag5|cflag6|cflag7);
					local_ref.usrflags |= src_ref.usrflags & (cflag5|cflag6|cflag7);
				}
			}
			break;
		}
		case cARMOS: case cGRAVE: case cBSGRAVE:
		{
			zfix& e1 = local_ref.c_attributes[8];
			zfix& e2 = local_ref.c_attributes[9];
			bool fl1 = src_ref.usrflags&cflag1;
			bool fl2 = src_ref.usrflags&cflag2;
			
			if(e1 == e2)
				e2 = 0;
			
			if(e1 && e2)
			{
				//both good
				local_ref.usrflags |= cflag1|cflag2;
			}
			else if(e1 || e2)
			{
				if(!e1) //make e1 the valid one
					zc_swap(e1,e2);
				local_ref.usrflags |= cflag1;
				local_ref.usrflags &= ~cflag2;
				local_ref.c_attributes[9] = (fl2 ? 0_zf : src_ref.c_attributes[9]);
			}
			else
			{
				local_ref.usrflags &= ~(cflag1|cflag2);
				local_ref.c_attributes[8] = (fl1 ? 0_zf : src_ref.c_attributes[8]);
				local_ref.c_attributes[9] = (fl2 ? 0_zf : src_ref.c_attributes[9]);
			}
			break;
		}
		case cSHOOTER:
		{
			//Angle stuff
			size_t adir = getRadio(0);
			SETFLAG(local_ref.usrflags,cflag1,adir!=0);
			zfix& angle_dir = local_ref.c_attributes[0];
			switch(adir)
			{
				case 0:
					angle_dir = ddls[5]->getSelectedValue();
					break;
				case 1: //Angle
					angle_dir = zslongToFix(tfs[2]->getVal());
					angle_dir = SMART_WRAP(angle_dir, 360_zf);
					break;
				case 2:
					angle_dir = -1;
					break;
				case 3:
					angle_dir = -2;
					break;
				case 4:
					angle_dir = -3*10000;
					break;
			}
			//Rate stuff
			zfix& rate = local_ref.c_attributes[16];
			zfix& high_rate = local_ref.c_attributes[17];
			SETFLAG(local_ref.usrflags,cflag2,high_rate != rate);
			if(high_rate != rate)
			{
				if(high_rate < rate)
					zc_swap(high_rate,rate);
			}
			else if(src_ref.usrflags&cflag2)
			{
				high_rate = 0;
			}
			else
			{
				high_rate = src_ref.c_attributes[17];
			}
			//Proximity stuff
			zfix& prox = local_ref.c_attributes[1];
			if(!(local_ref.usrflags&cflag4))
			{
				if(!(src_ref.usrflags&cflag4))
					prox = 0;
				else prox = src_ref.c_attributes[1];
			}
			//Spread stuff
			zfix& shot_count = local_ref.c_attributes[11];
			zfix& spread = local_ref.c_attributes[3];
			if(!(local_ref.usrflags&cflag7))
			{
				if(src_ref.usrflags&cflag7)
				{
					spread = 0;
					shot_count = 0;
				}
				else
				{
					spread = src_ref.c_attributes[3];
					shot_count = src_ref.c_attributes[11];
				}
			}
			// misc weap data
			zfix& unblockable = local_ref.c_attributes[12];
			zfix& script = local_ref.c_attributes[13];
			if(local_ref.usrflags & cflag10)
			{
				unblockable = 0;
				script = 0;
			}
			break;
		}
		case cTALLGRASS: case cTALLGRASSTOUCHY: case cTALLGRASSNEXT:
			[[fallthrough]];
		case cBUSH: case cBUSHTOUCHY: case cFLOWERS: case cSLASHNEXTTOUCHY:
		case cSLASHITEM: case cSLASHNEXTITEMTOUCHY:
		case cSLASHNEXTITEM: case cBUSHNEXT: case cSLASHITEMTOUCHY:
		case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
		{
			size_t decoty = getRadio(0);
			zfix& decospr = local_ref.c_attributes[8];
			zfix& dropitem = local_ref.c_attributes[9];
			zfix& cutsfx = local_ref.c_attributes[10];
			
			SETFLAG(local_ref.usrflags,cflag1,decoty != 0);
			SETFLAG(local_ref.usrflags,cflag10,decoty == 1);
			switch(decoty)
			{
				case 0:
					decospr = 0;
					break;
				case 1:
					decospr = ddls[0]->getSelectedValue();
					break;
				case 2:
					decospr = ddls[1]->getSelectedValue();
					break;
			}
			
			size_t dropty = getRadio(1);
			switch(dropty)
			{
				case 0:
					dropitem = 0;
					break;
				case 1:
					dropitem = ddls[2]->getSelectedValue();
					break;
				case 2:
					dropitem = ddls[3]->getSelectedValue();
					break;
			}
			SETFLAG(local_ref.usrflags,cflag2,dropty>0);
			SETFLAG(local_ref.usrflags,cflag11,dropty==2);
			
			size_t sfxty = getRadio(2);
			SETFLAG(local_ref.usrflags,cflag3,sfxty==1);
			if(sfxty==0)
				cutsfx = 0;
			else cutsfx = ddls[4]->getSelectedValue();
			break;
		}
		case cSLASHNEXT:
		{
			size_t decoty = getRadio(0);
			zfix& decospr = local_ref.c_attributes[8];
			zfix& cutsfx = local_ref.c_attributes[10];
			
			SETFLAG(local_ref.usrflags,cflag1,decoty != 0);
			SETFLAG(local_ref.usrflags,cflag10,decoty == 1);
			switch(decoty)
			{
				case 0:
					decospr = 0;
					break;
				case 1:
					decospr = ddls[0]->getSelectedValue();
					break;
				case 2:
					decospr = ddls[1]->getSelectedValue();
					break;
			}
			
			size_t sfxty = getRadio(2);
			SETFLAG(local_ref.usrflags,cflag3,sfxty==1);
			if(sfxty==0)
				cutsfx = 0;
			else cutsfx = ddls[4]->getSelectedValue();
			break;
		}
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
		{
			size_t lvl = 0;
			if(local_ref.type == cCHEST)
				lvl = 1;
			else if(local_ref.type == cBOSSCHEST)
				lvl = 2;
			else if(local_ref.type == cLOCKEDCHEST)
				lvl = 3;
			
			zfix& exstate = local_ref.c_attributes[13];
			auto rad0 = getRadio(0);
			
			if(rad0==0)
				exstate = 0;
			SETFLAG(local_ref.usrflags,cflag16,rad0);
			
			zfix& contains_item = local_ref.c_attributes[18];
			auto rad1 = getRadio(1);
			
			contains_item = ddls[rad1==1 ? 3 : 2]->getSelectedValue();
			
			//Prompts
			bool prompt = lvl > 0 && cboxes[1]->getChecked();
			bool lockprompt = lvl > 1 && prompt && cboxes[2]->getChecked();
			
			zfix& prompt_combo = local_ref.c_attributes[1];
			zfix& prompt_combo2 = local_ref.c_attributes[2];
			zfix& prompt_cset = local_ref.c_attributes[12];
			zfix& prompt_xoff = local_ref.c_attributes[16];
			zfix& prompt_yoff = local_ref.c_attributes[17];
			prompt_combo = 0;
			prompt_cset = 0;
			prompt_xoff = 12;
			prompt_yoff = -8;
			prompt_combo2 = 0;
			if(prompt)
			{
				prompt_combo = cmbswatches[0]->getCombo();
				prompt_cset = cmbswatches[0]->getCSet();
				prompt_xoff = tfs[1]->getVal();
				prompt_yoff = tfs[2]->getVal();
				if(lockprompt)
					prompt_combo2 = cmbswatches[1]->getCombo();
			}
			
			//Locking
			bool itemkey = lvl == 3 && (local_ref.usrflags&cflag1);
			bool counterkey = lvl == 3 && !(itemkey && (local_ref.usrflags&cflag2));
			
			auto rad2 = getRadio(2);
			zfix& reqitem = local_ref.c_attributes[8];
			zfix& usecounter = local_ref.c_attributes[9];
			zfix& amount = local_ref.c_attributes[0];
			
			if(!itemkey)
			{
				local_ref.usrflags &= ~(cflag2|cflag5);
				reqitem = 0;
			}
			else reqitem = ddls[4]->getSelectedValue();
			
			usecounter = 0;
			amount = 0;
			SETFLAG(local_ref.usrflags,cflag4,counterkey && rad2==1);
			if(counterkey)
			{
				if(rad2 == 1)
					usecounter = ddls[5]->getSelectedValue();
				amount = tfs[3]->getVal();
				
				auto rad3 = getRadio(3);
				if(rad2 == 0)
					rad3 = 0;
				SETFLAG(local_ref.usrflags, cflag8, rad3 == 1);
				SETFLAG(local_ref.usrflags, cflag6, rad3 == 2);
			}
			break;
		}
		case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		case cLOCKBLOCK2: case cBOSSLOCKBLOCK2:
		{
			size_t lvl = 0;
			if(local_ref.type == cBOSSLOCKBLOCK)
				lvl = 2;
			else if(local_ref.type == cLOCKBLOCK)
				lvl = 3;
			
			zfix& exstate = local_ref.c_attributes[13];
			auto rad0 = getRadio(0);
			
			if(rad0==0)
				exstate = 0;
			SETFLAG(local_ref.usrflags,cflag16,rad0);
			
			//Prompts
			bool prompt = lvl > 0 && cboxes[1]->getChecked();
			bool lockprompt = lvl > 1 && prompt && cboxes[2]->getChecked();
			
			zfix& prompt_combo = local_ref.c_attributes[1];
			zfix& prompt_combo2 = local_ref.c_attributes[2];
			zfix& prompt_cset = local_ref.c_attributes[12];
			zfix& prompt_xoff = local_ref.c_attributes[16];
			zfix& prompt_yoff = local_ref.c_attributes[17];
			prompt_combo = 0;
			prompt_cset = 0;
			prompt_xoff = 12;
			prompt_yoff = -8;
			prompt_combo2 = 0;
			if(prompt)
			{
				prompt_combo = cmbswatches[0]->getCombo();
				prompt_cset = cmbswatches[0]->getCSet();
				prompt_xoff = tfs[1]->getVal();
				prompt_yoff = tfs[2]->getVal();
				if(lockprompt)
					prompt_combo2 = cmbswatches[1]->getCombo();
			}
			
			//Locking
			bool itemkey = lvl == 3 && (local_ref.usrflags&cflag1);
			bool counterkey = lvl == 3 && !(itemkey && (local_ref.usrflags&cflag2));
			
			auto rad2 = getRadio(2);
			zfix& reqitem = local_ref.c_attributes[8];
			zfix& usecounter = local_ref.c_attributes[9];
			zfix& amount = local_ref.c_attributes[0];
			
			if(!itemkey)
			{
				local_ref.usrflags &= ~(cflag2|cflag5);
				reqitem = 0;
			}
			else reqitem = ddls[4]->getSelectedValue();
			
			usecounter = 0;
			amount = 0;
			SETFLAG(local_ref.usrflags,cflag4,counterkey && rad2==1);
			if(counterkey)
			{
				if(rad2 == 1)
					usecounter = ddls[5]->getSelectedValue();
				amount = tfs[3]->getVal();
				
				auto rad3 = getRadio(3);
				if(rad2 == 0)
					rad3 = 0;
				SETFLAG(local_ref.usrflags, cflag8, rad3 == 1);
				SETFLAG(local_ref.usrflags, cflag6, rad3 == 2);
			}
			break;
		}
		case cSIGNPOST:
		{
			bool prompt = cboxes[1]->getChecked();
			zfix& prompt_combo = local_ref.c_attributes[9];
			zfix& prompt_cset = local_ref.c_attributes[12];
			zfix& prompt_xoff = local_ref.c_attributes[16];
			zfix& prompt_yoff = local_ref.c_attributes[17];
			prompt_combo = 0;
			prompt_cset = 0;
			prompt_xoff = 12;
			prompt_yoff = -8;
			if(prompt)
			{
				prompt_combo = cmbswatches[0]->getCombo();
				prompt_cset = cmbswatches[0]->getCSet();
				prompt_xoff = tfs[1]->getVal();
				prompt_yoff = tfs[2]->getVal();
			}
			break;
		}
		case cCUTSCENETRIG:
		{
			bool ending = (local_ref.usrflags & cflag1);
			if(ending)
			{
				local_ref.c_attributes[0] = 0;
				local_ref.c_attributes[8] = 0;
				SETFLAG(local_ref.usrflags,cflag2,false);
			}
			break;
		}
		case cCRUMBLE:
		{
			if(dest_ref.c_attributes[8] == CMBTY_CRUMBLE_RESET
				&& local_ref.c_attributes[8] != CMBTY_CRUMBLE_RESET)
				local_ref.c_attributes[17] = 0;
			break;
		}
		case cICY:
		{
			bool player = (local_ref.usrflags & cflag2);
			bool wasplayer = (src_ref.usrflags & cflag2);
			if(wasplayer && !player)
			{
				local_ref.c_attributes[8] = 0;
				local_ref.c_attributes[9] = 0;
				local_ref.c_attributes[0] = 0;
				local_ref.c_attributes[1] = 0;
				local_ref.c_attributes[2] = 0;
			}
			break;
		}
		case cCUTSCENEEFFECT:
			break;
		case cSAVE: case cSAVE2:
		{
			if (!(local_ref.usrflags & cflag1))
				local_ref.c_attributes[8] = 0;
			if (!(local_ref.usrflags & cflag2))
				local_ref.c_attributes[9] = 0;
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
#define DDH 21_px

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
		case cTRIGNOFLAG: case cSTRIGNOFLAG:
		case cTRIGFLAG: case cSTRIGFLAG:
			ctyname = "Step->Secrets";
			break;
		case cSTEP: case cSTEPSAME: case cSTEPALL: case cSTEPCOPY:
			ctyname = "Step->Next";
			break;
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
			ctyname = "Damage";
			break;
		case cCVUP: case cCVDOWN: case cCVLEFT: case cCVRIGHT:
			ctyname = "Conveyor";
			break;
		case cGRAVE: case cBSGRAVE:
			ctyname = "Grave";
			break;
		default:
			ctyname = ZI.getComboTypeName(local_ref.type);
			break;
	}
	window->setTitle("Combo Wizard (" + ctyname + ")");
}
void combo_default(newcombo& ref, bool typeonly)
{
	if(typeonly)
	{
		memset(ref.c_attributes, 0, sizeof(ref.c_attributes));
		ref.usrflags = 0;
	}
	else
	{
		auto ty = ref.type;
		ref.clear();
		ref.type = ty;
	}
	switch(ref.type)
	{
		case cSLOPE:
			ref.c_attributes[16] = 15;
			ref.c_attributes[19] = 15;
			break;
		case cWATER:
			ref.c_attributes[0] = 4;
			break;
		case cSHOOTER:
			ref.c_attributes[9] = ewArrow;
			ref.c_attributes[10] = 19;
			ref.c_attributes[16] = 60;
			ref.c_attributes[18] = 2;
			ref.c_attributes[0] = -1;
			ref.c_attributes[1] = 4;
			ref.c_attributes[2] = 200;
			ref.usrflags = cflag1 | cflag4;
			break;
		case cSTEPSFX:
			ref.c_attributes[9] = ewBomb;
			ref.c_attributes[11] = 76;
			ref.c_attributes[0] = 4;
			ref.usrflags = cflag1;
			break;
		case cTORCH:
			ref.c_attributes[8] = 32;
			break;
		case cMIRRORNEW:
			for(byte q = 0; q < 8; ++q)
				ref.c_attributes[q + 8] = q;
		[[fallthrough]];
		case cMIRROR: case cMIRRORSLASH: case cMIRRORBACKSLASH: case cMAGICPRISM: case cMAGICPRISM4:
			ref.c_attributes[0] = 0;
			break;
		case cCRUMBLE:
			ref.c_attributes[16] = 45;
			break;
		case cICY:
			ref.usrflags |= cflag1|cflag2;
			ref.c_attributes[8] = 75;
			ref.c_attributes[9] = 15;
			ref.c_attributes[0] = 0.0400_zf;
			ref.c_attributes[1] = 0.0200_zf;
			ref.c_attributes[2] = 1.5000_zf;
			break;
		//CHESTS
		case cLOCKEDCHEST:
			ref.c_attributes[0] = 1;
			[[fallthrough]];
		case cBOSSCHEST:
			[[fallthrough]];
		case cCHEST:
			ref.c_attributes[10] = 0x01;
			ref.c_attributes[18] = -1;
			ref.usrflags = cflag7|cflag9|cflag11|cflag12;
			if(!typeonly)
				ref.flag = mfARMOS_ITEM;
			[[fallthrough]];
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
			break;
		//LOCKBLOCKS
		case cLOCKBLOCK:
			ref.c_attributes[0] = 1;
			[[fallthrough]];
		case cBOSSLOCKBLOCK:
			ref.c_attributes[11] = WAV_DOOR;
			[[fallthrough]];
		case cLOCKBLOCK2: case cBOSSLOCKBLOCK2:
			break;
		case cCUTSCENEEFFECT:
		{
			switch(ref.c_attributes[8].getTrunc())
			{
				default:
					ref.c_attributes[8] = CUTEFF_PLAYER_WALK;
				[[fallthrough]];
				case CUTEFF_PLAYER_WALK:
					ref.c_attributes[0] = ref.c_attributes[1] = 0;
					ref.c_attributes[2] = 0;
					SETFLAG(ref.usrflags, cflag1, true);
					SETFLAG(ref.usrflags, cflag2, false);
					break;
			}
			break;
		}
		case cSAVE: case cSAVE2:
		{
			ref.c_attributes[8] = 100;
			ref.c_attributes[9] = 100;
			ref.usrflags |= (cflag1|cflag2);
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
	switch(local_ref.type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
		{
			zfix& warp_sfx = local_ref.c_attributes[8];
			warp_sfx.doTrunc();
			
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem& itm){return isWarpType(itm.value);});
			windowRow->add(
				Rows<3>(
					Label(text = "Type:", hAlign = 1.0),
					ddls[0] = DropDownList(data = lists[0],
						fitParent = true, selectedValue = local_ref.type,
						onSelectFunc = [&](int32_t val)
						{
							local_ref.type = val;
						}),
					INFOBTN_REF(getComboTypeHelpText(local_ref.type)),
					//
					Label(text = "Warp Sound:", hAlign = 1.0),
					ddls[1] = DropDownList(data = parent.list_sfx,
						fitParent = true, selectedValue = warp_sfx,
						onSelectFunc = [&](int32_t val)
						{
							warp_sfx = val;
						}),
					INFOBTN("SFX to play during the warp")
				)
			);
			break;
		}
		case cCUTSCENETRIG:
		{
			zfix& errsfx = local_ref.c_attributes[8];
			int32_t& btns = local_ref.c_attributes[0].val;
			errsfx.doTrunc();
			
			grids[0] = Columns<7>();
			static const char* btn_names[] = {"Up","Down","Left","Right","A","B","Start","L","R","Map","Ex1","Ex2","Ex3","Ex4","StickUp","StickDown","StickLeft","StickRight"};
			for(int q = 0; q < 18; ++q)
			{
				grids[0]->add(Checkbox(
					text = btn_names[q], hAlign = 0.0,
					checked = (btns&(1<<q)),
					onToggleFunc = [&,q](bool state)
					{
						SETFLAG(btns,(1<<q),state);
					}
				));
			}
			windowRow->add(
				Column(
					Rows<3>(
						cboxes[0] = Checkbox(
								text = "End Cutscene", hAlign = 0.0,
								checked = (local_ref.usrflags&cflag1),
								colSpan = 2,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag1,state);
								}
							),
						INFOBTN("If checked, triggering this combo with ComboType Effects will end any active cutscene."),
						//
						cboxes[1] = Checkbox(
								text = "Disable F6", hAlign = 0.0,
								checked = (local_ref.usrflags&cflag2),
								colSpan = 2,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag2,state);
								}
							),
						INFOBTN("The cutscene activated by this combo will not allow F6"),
						//
						Label(text = "Error SFX:"),
						ddls[0] = DropDownList(data = parent.list_sfx,
							selectedValue = errsfx,
							onSelectFunc = [&](int32_t val)
							{
								errsfx = val;
							}),
						INFOBTN("The SFX to play when pressing a disabled button"),
						//
						Label(text = "Allowed Buttons", colSpan = 2),
						INFOBTN("The following buttons will be ALLOWED during the cutscene.")
					),
					grids[0]
				)
			);
			break;
		}
		case cSLOPE:
		{
			local_ref.walk -= local_ref.walk & 0x0F; //nonsolid combo
			zfix& x1 = local_ref.c_attributes[0];
			zfix& y1 = local_ref.c_attributes[1];
			zfix& x2 = local_ref.c_attributes[2];
			zfix& y2 = local_ref.c_attributes[3];
			int32_t& slip = local_ref.c_attributes[0].val;
			
			x1.doTrunc(); y1.doTrunc();
			x2.doTrunc(); y2.doTrunc();
			
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
		case cTRIGNOFLAG: case cSTRIGNOFLAG:
		case cTRIGFLAG: case cSTRIGFLAG:
		{
			zfix& trig_sfx = local_ref.c_attributes[8];
			trig_sfx.doTrunc();
			
			bool perm = (local_ref.type==cTRIGFLAG||local_ref.type==cSTRIGFLAG);
			bool sens = (local_ref.type==cSTRIGNOFLAG||local_ref.type==cSTRIGFLAG);
			windowRow->add(
				Rows<3>(
					INFOBTN_EX("If checked, the combo will trigger secrets permanently.",hAlign = 1.0, forceFitH = true),
					cboxes[0] = Checkbox(
							text = "Permanent", hAlign = 0.0,
							checked = perm, colSpan = 2,
							onToggleFunc = [&](bool state)
							{
								switch(local_ref.type)
								{
									case cTRIGNOFLAG:
										if(state) local_ref.type = cTRIGFLAG;
										break;
									case cSTRIGNOFLAG:
										if(state) local_ref.type = cSTRIGFLAG;
										break;
									case cTRIGFLAG:
										if(!state) local_ref.type = cTRIGNOFLAG;
										break;
									case cSTRIGFLAG:
										if(!state) local_ref.type = cSTRIGNOFLAG;
										break;
								}
							}
						),
					//
					INFOBTN_EX("If checked, the combo will trigger even if barely touched.",hAlign = 1.0, forceFitH = true),
					cboxes[1] = Checkbox(
							text = "Sensitive", hAlign = 0.0,
							checked = sens, colSpan = 2,
							onToggleFunc = [&](bool state)
							{
								switch(local_ref.type)
								{
									case cTRIGNOFLAG:
										if(state) local_ref.type = cSTRIGNOFLAG;
										break;
									case cTRIGFLAG:
										if(state) local_ref.type = cSTRIGFLAG;
										break;
									case cSTRIGFLAG:
										if(!state) local_ref.type = cTRIGFLAG;
										break;
									case cSTRIGNOFLAG:
										if(!state) local_ref.type = cTRIGNOFLAG;
										break;
								}
							}
						),
					//
					Label(text = "Trigger Sound:", hAlign = 1.0),
					ddls[0] = DropDownList(data = parent.list_sfx,
						fitParent = true, selectedValue = trig_sfx,
						onSelectFunc = [&](int32_t val)
						{
							trig_sfx = val;
						}),
					INFOBTN("SFX to play when triggered")
				)
			);
			break;
		}
		case cSTEP: case cSTEPSAME: case cSTEPALL: case cSTEPCOPY:
		{
			zfix& step_sfx = local_ref.c_attributes[8];
			zfix& req_item = local_ref.c_attributes[9];
			step_sfx.doTrunc();
			req_item.doTrunc();
			
			if(local_ref.type == cSTEPCOPY)
			{
				step_sfx = 0;
				req_item = 0;
				local_ref.usrflags &= ~cflag1;
			}
			rs_sz[0] = 4;
			lists[0] = GUI::ZCListData::items(true).filter(
				[&](GUI::ListItem& itm)
				{
					if(itm.value == 0) //Remove item 0
						return false;
					if(itm.value == -1) //Change the none value to 0
						itm.value = 0;
					return true;
				});
			windowRow->add(Rows<4>(
				Label(text = "Step Sound:", hAlign = 1.0),
				ddls[0] = DropDownList(data = parent.list_sfx,
					fitParent = true, selectedValue = step_sfx,
					colSpan = 2,
					onSelectFunc = [&](int32_t val)
					{
						step_sfx = val;
					}),
				INFOBTN("SFX to play during the warp"),
				//
				Label(text = "Req. Item:", hAlign = 1.0),
				ddls[1] = DropDownList(data = lists[0],
					fitParent = true, selectedValue = req_item,
					colSpan = 2,
					onSelectFunc = [&](int32_t val)
					{
						req_item = val;
					}),
				INFOBTN("Item ID that must be owned in order to trigger."),
				//
				DummyWidget(),
				DummyWidget(width = 15_em),
				rset[0][0] = Radio(
					hAlign = 0.0,
					checked = local_ref.type == cSTEP,
					text = "Standard",
					indx = 0,
					onToggle = message::RSET0
				),
				INFOBTN(getComboTypeHelpText(cSTEP)),
				//
				DummyWidget(colSpan = 2),
				rset[0][1] = Radio(
					hAlign = 0.0,
					checked = local_ref.type == cSTEPSAME,
					text = "Same",
					indx = 1,
					onToggle = message::RSET0
				),
				INFOBTN(getComboTypeHelpText(cSTEPSAME)),
				//
				DummyWidget(colSpan = 2),
				rset[0][2] = Radio(
					hAlign = 0.0,
					checked = local_ref.type == cSTEPALL,
					text = "All",
					indx = 2,
					onToggle = message::RSET0
				),
				INFOBTN(getComboTypeHelpText(cSTEPALL)),
				//
				DummyWidget(colSpan = 2),
				rset[0][3] = Radio(
					hAlign = 0.0,
					checked = local_ref.type == cSTEPCOPY,
					text = "Copycat",
					indx = 3,
					onToggle = message::RSET0
				),
				INFOBTN(getComboTypeHelpText(cSTEPCOPY)),
				//
				DummyWidget(colSpan = 2),
				cboxes[0] = Checkbox(
					text = "Heavy", hAlign = 0.0,
					checked = local_ref.usrflags&cflag1,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(local_ref.usrflags,cflag1,state);
					}
				),
				INFOBTN("Requires Heavy Boots to trigger")
				//
			));
			break;
		}
		case cARMOS: case cGRAVE: case cBSGRAVE:
		{
			lists[0] = GUI::ZCListData::enemies(true).filter(
				[&](GUI::ListItem& itm){return unsigned(itm.value)<MAXGUYS;});
			zfix& e1 = local_ref.c_attributes[8];
			zfix& e2 = local_ref.c_attributes[9];
			e1.doTrunc();
			e2.doTrunc();
			
			bool armos = local_ref.type==cARMOS;
			
			if(!(local_ref.usrflags&cflag1))
			{
				e1 = 0;
				e2 = 0;
			}
			else if(!(local_ref.usrflags&cflag2))
				e2 = 0;
			
			std::shared_ptr<GUI::Grid> endrow = Rows<2>(padding = 0_px,
					colSpan = 3, hAlign = 1.0);
			if(armos)
			{
				endrow->add(cboxes[0] = Checkbox(
						text = "Handle Large", hAlign = 0.0,
							checked = local_ref.usrflags&cflag3,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag3,state);
							}
						));
				endrow->add(INFOBTN("If the specified enemy is larger"
						" than 1x1 tile, attempt to use armos"
						" combos that take up its' size"));
			}
			else
			{
				endrow->add(cboxes[0] = Checkbox(
						text = "Next", hAlign = 0.0,
							checked = local_ref.type==cBSGRAVE,
							onToggleFunc = [&](bool state)
							{
								local_ref.type = state ? cBSGRAVE : cGRAVE;
							}
						));
				endrow->add(INFOBTN("If the combo should become the next combo"
					" after spawning its' enemy."));
			}
			string cty = armos ? "Armos" : "Grave";
			windowRow->add(Rows<3>(
				Label(text = "Enemy 1:", hAlign = 1.0),
				ddls[0] = DropDownList(data = lists[0],
					fitParent = true, selectedValue = e1,
					onSelectFunc = [&](int32_t val)
					{
						e1 = val;
					}),
				Button(text = "?", rowSpan = 2, fitParent = true,
					onPressFunc = [=]()
					{
						InfoDialog("Info","The enemies to spawn."
							" If both are set, one will be randomly chosen."
							" If only one is set, it will be used."
							"\nIf neither is set, uses the enemy flagged as 'Spawned by "+cty+"'").show();
					}),
				//
				Label(text = "Enemy 2:", hAlign = 1.0),
				ddls[1] = DropDownList(data = lists[0],
					fitParent = true, selectedValue = e2,
					onSelectFunc = [&](int32_t val)
					{
						e2 = val;
					}),
				//rowspans from button above
				//
				endrow
			));
			break;
		}
		case cWATER: case cSHALLOWWATER:
		{
			static size_t tabpos = 0;
			//Deep only
			lists[0] = GUI::ListData({
				{ "Solid is Solid", 0 },
				{ "Solid is Land", cflag3 },
				{ "Solid is Shallow", cflag4 }
			});
			auto sel_val = (local_ref.usrflags&cflag4) ? cflag4 : (local_ref.usrflags&cflag3);
			zfix& flipp_level = local_ref.c_attributes[8];
			zfix& drown_sfx = local_ref.c_attributes[12];
			zfix& drown_damage = local_ref.c_attributes[0];
			
			zfix& ripple_sprite = local_ref.c_attributes[14];
			
			//Shallow only
			int shallow_indx = get_qr(qr_OLD_SHALLOW_SFX) ? 8 : 13;
			zfix& splash_sfx = local_ref.c_attributes[shallow_indx];
			
			//Both
			lists[1] = GUI::ZCListData::itemclass(true,true);
			zfix& hp_delay = local_ref.c_attributes[9];
			zfix& req_ic = local_ref.c_attributes[10];
			zfix& req_it_lvl = local_ref.c_attributes[11];
			zfix& hp_mod = local_ref.c_attributes[1];
			zfix& mod_sfx = local_ref.c_attributes[2];
			
			flipp_level.doTrunc();
			drown_sfx.doTrunc();
			drown_damage.doTrunc();
			ripple_sprite.doTrunc();
			splash_sfx.doTrunc();
			hp_delay.doTrunc();
			req_ic.doTrunc();
			req_it_lvl.doTrunc();
			hp_mod.doTrunc();
			mod_sfx.doTrunc();
			
			std::shared_ptr<GUI::Grid> mainrow;
			if(local_ref.type == cWATER) //deep
			{
				mainrow = Row(padding = 0_px,
					Rows<2>(
						cboxes[0] = Checkbox(
							text = "Is Lava", hAlign = 1.0,
							checked = local_ref.usrflags&cflag1,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag1,state);
							}
						),
						INFOBTN("If a liquid is Lava, it uses a different drowning sprite, and only flippers with the"
							" 'Can Swim In Lava' flag set will apply."),
						//
						ddls[0] = DropDownList(data = lists[0],
							fitParent = true, selectedValue = sel_val,
							maxwidth = 30_em,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.usrflags &= ~(cflag3|cflag4);
								local_ref.usrflags |= val;
							}),
						INFOBTN("Whether solid areas of the combo are treated as"
							" solid, walkable land, or walkable shallow water."),
						//
						Label(text = "Solidity:", hAlign = 1.0),
						Button(text = "?", fitParent = true,
							rowSpan = 2,
							onPressFunc = [&]()
							{
								InfoDialog("Solidity","The pink-highlighted corners of the combo"
									" will be treated as either solid walls, walkable land,"
									" or walkable shallow water based on the dropdown above.").show();
							}),
						//
						cswatchs[0] = CornerSwatch(
							val = solidity_to_flag(local_ref.walk&0xF),
							color = vc(12), hAlign = 1.0,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.walk &= ~0xF;
								local_ref.walk |= solidity_to_flag(val);
							})
						//button rowspans here
					),
					Rows<3>(
						Label(text = "Minimum Flipper Level:", hAlign = 1.0),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 1, high = 255, val = flipp_level,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								flipp_level = val;
							}),
						INFOBTN("The minimum level flippers required to swim in the water."
							" Flippers of lower level will have no effect."),
						//
						Label(text = "Drown Damage:", hAlign = 1.0),
						tfs[1] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
							val = drown_damage,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								drown_damage = val;
							}),
						INFOBTN("The amount of damage dealt when drowning, in HP points."
							" If negative, drowning will heal the Hero."),
						//
						Label(text = "Drown SFX:", hAlign = 1.0),
						ddls[1] = DropDownList(data = parent.list_sfx,
							fitParent = true, selectedValue = drown_sfx,
							onSelectFunc = [&](int32_t val)
							{
								drown_sfx = val;
							}),
						INFOBTN("The SFX played when drowning"),
						//
						Label(text = "Ripple Sprite:", hAlign = 1.0),
						ddls[2] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = ripple_sprite,
							onSelectFunc = [&](int32_t val)
							{
								ripple_sprite = val;
							}),
						INFOBTN("The sprite used to display ripples in shallow liquid.")
					)
				);
			}
			else //shallow
			{
				mainrow = Rows<3>(
					Label(text = "Splash SFX:", hAlign = 1.0),
					ddls[1] = DropDownList(data = parent.list_sfx,
						fitParent = true, selectedValue = splash_sfx,
						onSelectFunc = [&](int32_t val)
						{
							splash_sfx = val;
						}),
					INFOBTN("The SFX played when walking in"),
					//
					Label(text = "Ripple Sprite:", hAlign = 1.0),
					ddls[2] = DropDownList(data = list_sprites,
						fitParent = true, selectedValue = ripple_sprite,
						onSelectFunc = [&](int32_t val)
						{
							ripple_sprite = val;
						}),
					INFOBTN("The sprite used to display ripples in shallow liquid.")
				);
			}
			
			windowRow->add(TabPanel(ptr = &tabpos,
				TabRef(name = "Main",
					mainrow
				),
				TabRef(name = "Passive HP Mod",
					Rows<3>(
						cboxes[1] = Checkbox(
								text = "Passive HP Mod", colSpan = 3,
								checked = local_ref.usrflags&cflag2,
								onToggleFunc = [&](bool state)
								{
									SETFLAG(local_ref.usrflags,cflag2,state);
									update();
								}
							),
						//
						Label(text = "HP Modification:", hAlign = 1.0),
						tfs[2] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
							val = hp_mod,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								hp_mod = val;
							}),
						INFOBTN("How much HP should be modified by (negative for damage)"),
						//
						Label(text = "HP Delay:", hAlign = 1.0),
						tfs[3] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 0, high = 255, val = hp_delay,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								hp_delay = val;
							}),
						INFOBTN("The number of frames between HP modifications"),
						//
						Label(text = "Req Itemclass:", hAlign = 1.0),
						ddls[2] = DropDownList(data = lists[1],
							fitParent = true, selectedValue = req_ic,
							onSelectFunc = [&](int32_t val)
							{
								req_ic = val;
							}),
						INFOBTN("The itemclass, if any, that will stop the HP modification if owned."),
						//
						Label(text = "Req Item Level:", hAlign = 1.0),
						tfs[4] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 1, high = 255, val = req_it_lvl,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								req_it_lvl = val;
							}),
						INFOBTN("The minimum level you must own of the 'Req Itemclass' for it to"
							" stop hp modification."),
						//
						Label(text = "Mod SFX:", hAlign = 1.0),
						ddls[3] = DropDownList(data = parent.list_sfx,
							fitParent = true, selectedValue = mod_sfx,
							onSelectFunc = [&](int32_t val)
							{
								mod_sfx = val;
							}),
						INFOBTN("The SFX played every 'HP Delay' frames the Hero is in the liquid."),
						//
						Rows<2>(padding = 0_px, colSpan = 3, hAlign = 1.0,
							cboxes[2] = Checkbox(
									text = "Mod SFX Only if HP changed", hAlign = 0.0,
									checked = local_ref.usrflags&cflag6,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag6,state);
									}
								),
							INFOBTN("Only play the HP Mod SFX when HP actually changes"),
							//
							cboxes[3] = Checkbox(
									text = "Rings affect HP Mod", hAlign = 0.0,
									checked = local_ref.usrflags&cflag5,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag5,state);
									}
								),
							INFOBTN("Ring items defense reduces damage from HP Mod"),
							//
							cboxes[4] = Checkbox(
									text = "Damage triggers hit anim", hAlign = 0.0,
									checked = local_ref.usrflags&cflag7,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag7,state);
									}
								),
							INFOBTN("HP Mod Damage triggers the hit animation and invincibility frames")
						)
					)
				)
			));
			break;
		}
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
		{
			int32_t& damage = local_ref.c_attributes[0].val;
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem& itm){return isDamageType(itm.value);});
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
						INFOBTN_REF(getComboTypeHelpText(local_ref.type)),
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
						INFOBTN("Does not knock the Hero back when damaging them if checked."
							" Otherwise, knocks the Hero in the direction opposite"
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
		case cSHOOTER:
		{
			zfix& shot_sfx = local_ref.c_attributes[8];
			zfix& weap_type = local_ref.c_attributes[9];
			zfix& weap_sprite = local_ref.c_attributes[10];
			zfix& damage = local_ref.c_attributes[18];
			zfix& step = local_ref.c_attributes[2];
			
			zfix& unblockable = local_ref.c_attributes[12];
			zfix& script = local_ref.c_attributes[13];
			zfix& parentid = local_ref.c_attributes[14];
			
			zfix& angle_dir = local_ref.c_attributes[0];
			
			zfix& rate = local_ref.c_attributes[16];
			zfix& high_rate = local_ref.c_attributes[17];
			if(!(local_ref.usrflags&cflag2))
				high_rate = rate;
			
			zfix& prox = local_ref.c_attributes[1];
			if(prox < 0) prox = 0;
			
			zfix& shot_count = local_ref.c_attributes[11];
			zfix& spread = local_ref.c_attributes[3];
			
			shot_sfx.doTrunc();
			weap_type.doTrunc();
			weap_sprite.doTrunc();
			damage.doTrunc();
			step.doTrunc();
			unblockable.doTrunc();
			script.doTrunc();
			parentid.doTrunc();
			rate.doTrunc();
			high_rate.doTrunc();
			shot_count.doTrunc();
			
			spread = SMART_WRAP(spread, 360_zf);
			if(shot_count < 2) shot_count = 2;
			
			size_t seladir = 0;
			zfix angle = 0;
			int32_t dir = 0;
			if(local_ref.usrflags&cflag1)
			{
				if(angle_dir < 0)
				{
					seladir = 4;
					switch(angle_dir.getTrunc())
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
					angle = vbound(angle_dir,0_zf,359.9999_zf);
					dir = AngleToDir(WrapAngle(DegreesToRadians(double(angle))));
				}
			}
			else
			{
				dir = NORMAL_DIR(angle_dir.getTrunc());
			}
			
			rs_sz[0] = 5;
			
			lists[0] = GUI::ZCListData::items(true).filter(
				[&](GUI::ListItem& itm)
				{
					if(itm.value == 0) //Remove item 0
						return false;
					if(itm.value == -1) //Change the none value to 0
						itm.value = 0;
					return true;
				});
			
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
						Label(text = "Parent Item:", hAlign = 1.0),
						ddls[6] = DropDownList(data = lists[0],
							fitParent = true, selectedValue = parentid,
							onSelectFunc = [&](int32_t val)
							{ 
								parentid = val;
								update();
							}),
						INFOBTN("The item ID to use as the 'parent item' of the weapon. Only used for LWeapons."
							"\nThis affects various attributes of certain lweapons, such as a bomb's fuse."),
						//
						lbls[0] = Label(text = "Script:", hAlign = 1.0),
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
						btns[0] = INFOBTN("LWeapon or EWeapon script ID to attach to the fired weapons."
								"\nNote that there is no way to supply InitD to such scripts.")
					),
					grids[0] = Rows<2>(
						Label(text = "Unblockable"),
						INFOBTN_EX("The following checkboxes can make the weapon bypass"
							" types of blocking.", hAlign = 0.0, forceFitH = true),
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
							text = "Bypass enemy/Hero shield blocking",
							hAlign = 0.0, colSpan = 2,
							checked = unblockable & WPNUNB_SHLD, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(unblockable,WPNUNB_SHLD,state);
							}
						),
						cboxes[3] = Checkbox(
							text = "Bypass Hero shield reflecting",
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
					Column(padding = 0_px,
						Frame(padding = 0_px, fitParent = true,
							Rows<3>(hAlign = 1.0,
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
									val = step.getZLong(),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										step = zslongToFix(val);
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
									low = 0, val = prox.getZLong(), disabled = !(local_ref.usrflags&cflag4),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										prox = zslongToFix(val);
									}),
								INFOBTN("If enabled, the shooter will stop shooting when the Hero"
									" is within this distance (in pixels) of the combo."),
								//
								cboxes[10] = Checkbox(
									hAlign = 1.0, boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
									text = "Invert Prox", colSpan = 2,
									checked = local_ref.usrflags&cflag9,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag9,state);
									}
								),
								INFOBTN("If checked, the shooter will only shoot when the Hero is closer, rather than farther.")
							)
						),
						Frame(padding = 0_px, fitParent = true,
							Rows<3>(hAlign = 1.0,
								cboxes[5] = Checkbox(
									hAlign = 1.0, boxPlacement = GUI::Checkbox::boxPlacement::RIGHT,
									text = "Multishot", colSpan = 2,
									checked = local_ref.usrflags&cflag7,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag7,state);
										tfs[6]->setDisabled(!state);
										tfs[7]->setDisabled(!state);
									}
								),
								INFOBTN("Shoot more than one projectile"),
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
									val = spread.getZLong(),
									disabled = !(local_ref.usrflags&cflag7),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										spread = zslongToFix(val);
									}),
								INFOBTN("Angle in degrees between shots, if Multishot is enabled")
							)
						)
					),
					Column(padding = 0_px,
						Frame(padding = 0_px,
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
										angle_dir = val;
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
									val = angle.getZLong(),
									onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
									{
										angle_dir = zslongToFix(val);
									}),
								//
								rset[0][2] = Radio(
									hAlign = 0.0,
									checked = seladir==2,
									text = "Aimed 4-Dir",
									indx = 2,
									onToggle = message::RSET0
								),
								IH_BTN(DDH,"Aim 4-directionally at the Hero"),
								DummyWidget(),
								//
								rset[0][3] = Radio(
									hAlign = 0.0,
									checked = seladir==3,
									text = "Aimed 8-Dir",
									indx = 3,
									onToggle = message::RSET0
								),
								IH_BTN(DDH,"Aim 8-directionally at the Hero"),
								DummyWidget(),
								//
								rset[0][4] = Radio(
									hAlign = 0.0,
									checked = seladir==4,
									text = "Aimed 360",
									indx = 4,
									onToggle = message::RSET0
								),
								IH_BTN(DDH,"Aim angularly at the Hero"),
								DummyWidget()
								//
							)
						),
						Frame(padding = 0_px,
							Rows<2>(
								INFOBTN("The 'Misc Weapon Data' will be applied to the weapon(s) after they are shot."),
								Checkbox(
									text = "Apply Misc Weapon Data",
									checked = local_ref.usrflags&cflag10,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag10,state);
										update();
									}
								),
								Button(text = "Misc Weapon Data",
									colSpan = 2,
									onPressFunc = [&]()
									{
										call_weap_data_editor(local_ref.misc_weap_data, is_misc_lweapon(local_ref), true);
									})
							)
						)
					)
				))
			));
			break;
		}
		case cSTEPSFX:
		{
			zfix& shot_sfx = local_ref.c_attributes[8];
			zfix& weap_type = local_ref.c_attributes[9];
			zfix& weap_dir = local_ref.c_attributes[10];
			zfix& weap_sprite = local_ref.c_attributes[11];
			zfix& parentid = local_ref.c_attributes[12];
			zfix& damage = local_ref.c_attributes[0];
			shot_sfx.doTrunc();
			weap_type.doTrunc();
			weap_dir.doTrunc();
			weap_sprite.doTrunc();
			parentid.doTrunc();
			damage.doTrunc();
			
			if(damage < 1) damage = 4;
			
			lists[0] = GUI::ZCListData::items(true).filter(
				[&](GUI::ListItem& itm)
				{
					if(itm.value == 0) //Remove item 0
						return false;
					if(itm.value == -1) //Change the none value to 0
						itm.value = 0;
					return true;
				});
			
			windowRow->add(
				Columns<2>(
					Rows<3>(hAlign = 1.0, vAlign = 1.0,
						Label(text = "SFX:", hAlign = 1.0),
						ddls[0] = DropDownList(data = parent.list_sfx,
							fitParent = true, selectedValue = shot_sfx,
							onSelectFunc = [&](int32_t val)
							{
								shot_sfx = val;
							}),
						INFOBTN("SFX to play when activated.")
					),
					grids[0] = Rows<3>(hAlign = 1.0, vAlign = 0.0,
						Label(text = "Weapon Sprite:", hAlign = 1.0),
						ddls[1] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = weap_sprite,
							onSelectFunc = [&](int32_t val)
							{
								weap_sprite = val;
							}),
						INFOBTN("The sprite of the spawned weapon"),
						//
						Label(text = "Weapon Type:", hAlign = 1.0),
						ddls[2] = DropDownList(data = parent.list_weaptype,
							fitParent = true, selectedValue = weap_type,
							onSelectFunc = [&](int32_t val)
							{ 
								weap_type = val;
								update();
							}),
						INFOBTN("The LWeapon or EWeapon ID to be shot"),
						//
						Label(text = "Weapon Dir:", hAlign = 1.0),
						ddls[3] = DropDownList(data = list_dirs,
							fitParent = true, selectedValue = weap_dir,
							onSelectFunc = [&](int32_t val)
							{ 
								weap_dir = val;
								update();
							}),
						INFOBTN("The direction for the weapon to shoot"),
						//
						Label(text = "Parent Item:", hAlign = 1.0),
						ddls[4] = DropDownList(data = lists[0],
							fitParent = true, selectedValue = parentid,
							onSelectFunc = [&](int32_t val)
							{ 
								parentid = val;
								update();
							}),
						INFOBTN("The item ID to use as the 'parent item' of the weapon. Only used for LWeapons."
							"\nThis affects various attributes of certain lweapons, such as a bomb's fuse."),
						//
						Label(text = "Damage:", hAlign = 1.0),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
							low = 1, high = 214748, val = damage,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								damage = val;
							}),
						INFOBTN("The damage of the spawned weapon.")
					),
					Rows<2>(hAlign = 0.0, vAlign = 1.0,
						INFOBTN("Spawns a weapon when triggered, and by default advances to the next combo in the combo list."),
						cboxes[0] = Checkbox(
							text = "Landmine (Step->Weapon)",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag1, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag1,state);
								update();
							}
						)
					),
					grids[1] = Rows<2>(hAlign = 0.0, vAlign = 0.0,
						INFOBTN("Script weapon IDs for 'Weapon Type' are EWeapons by default; if checked, they will be LWeapons instead."),
						cboxes[1] = Checkbox(
							text = "Script Weapon IDs spawn LWeapons",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag2, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag2,state);
								update();
							}
						),
						INFOBTN("If checked, the combo will not advance to the next combo when triggered."
							" This may cause the landmine to trigger multiple times in a row."),
						cboxes[2] = Checkbox(
							text = "Don't Advance",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag3, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag3,state);
								update();
							}
						),
						INFOBTN("If the weapon type is a Script weapon and 'Script Weapon IDs spawn LWeapons' is checked, or the weapon type is"
							" a sparkle type, it will immediately damage the Hero (knocking them back none)."),
						cboxes[3] = Checkbox(
							text = "Direct Damage Script LW / Sparkles",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag4, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag4,state);
								update();
							}
						),
						//
						INFOBTN("The 'Misc Weapon Data' will be applied to the weapon after it spawns."),
						Checkbox(
							text = "Apply Misc Weapon Data",
							checked = local_ref.usrflags&cflag10,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag10,state);
								update();
							}
						),
						Button(text = "Misc Weapon Data",
							colSpan = 2,
							onPressFunc = [&]()
							{
								call_weap_data_editor(local_ref.misc_weap_data, is_misc_lweapon(local_ref), true);
							})
					)
				)
			);
			break;
		}
		case cTORCH:
		{
			zfix& radius = local_ref.c_attributes[8];
			zfix& shape = local_ref.c_attributes[9];
			zfix& dir = local_ref.c_attributes[10];
			radius.doTrunc();
			shape.doTrunc();
			dir.doTrunc();
			
			windowRow->add(
				Rows<3>(
					Label(text = "Size", hAlign = 1.0),
					TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_BYTE,
						low = 0, high = 255, val = radius,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							radius = val;
						}),
					INFOBTN("The size in pixels of the light area. Acts as a radius for most shapes."),
					Label(text = "Shape", hAlign = 1.0),
					DropDownList(data = list_torch_shapes,
						fitParent = true, selectedValue = shape,
						onSelectFunc = [&](int32_t val)
						{
							shape = val;
						}),
					INFOBTN("The shape to cast light in."),
					Label(text = "Direction", hAlign = 1.0),
					DropDownList(data = list_dirs,
						fitParent = true, selectedValue = dir,
						onSelectFunc = [&](int32_t val)
						{
							dir = val;
						}),
					INFOBTN("The direction to cast light in, if the shape is directional (like a cone)")
				)
			);
			break;
		}
		case cMIRRORNEW:
		{
			zfix& refls_up = local_ref.c_attributes[8+up];
			zfix& refls_down = local_ref.c_attributes[8+down];
			zfix& refls_left = local_ref.c_attributes[8+left];
			zfix& refls_right = local_ref.c_attributes[8+right];
			zfix& refls_l_up = local_ref.c_attributes[8+l_up];
			zfix& refls_r_up = local_ref.c_attributes[8+r_up];
			zfix& refls_l_down = local_ref.c_attributes[8+l_down];
			zfix& refls_r_down = local_ref.c_attributes[8+r_down];
			refls_up.doTrunc(); refls_down.doTrunc(); refls_left.doTrunc(); refls_right.doTrunc();
			refls_l_up.doTrunc(); refls_r_up.doTrunc(); refls_l_down.doTrunc(); refls_r_down.doTrunc();
			
			lists[0] = list_dirs;
			lists[0].add("Block",8);
			windowRow->add(
				Rows<3>(
					Label(text = "Up Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_up,
						onSelectFunc = [&](int32_t val)
						{
							refls_up = val;
						}),
					INFOBTN("Weapons/light beams facing up (coming from below) will move in this direction."),
					Label(text = "Down Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_down,
						onSelectFunc = [&](int32_t val)
						{
							refls_down = val;
						}),
					INFOBTN("Weapons/light beams facing down (coming from above) will move in this direction."),
					Label(text = "Left Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_left,
						onSelectFunc = [&](int32_t val)
						{
							refls_left = val;
						}),
					INFOBTN("Weapons/light beams facing left (coming from the right) will move in this direction."),
					Label(text = "Right Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_right,
						onSelectFunc = [&](int32_t val)
						{
							refls_right = val;
						}),
					INFOBTN("Weapons/light beams facing right (coming from the left) will move in this direction."),
					Label(text = "Up-Left Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_l_up,
						onSelectFunc = [&](int32_t val)
						{
							refls_l_up = val;
						}),
					INFOBTN("Weapons facing up-left (coming from down-right) will move in this direction."),
					Label(text = "Up-Right Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_r_up,
						onSelectFunc = [&](int32_t val)
						{
							refls_r_up = val;
						}),
					INFOBTN("Weapons facing up-right (coming from down-left) will move in this direction."),
					Label(text = "Down-Left Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_l_down,
						onSelectFunc = [&](int32_t val)
						{
							refls_l_down = val;
						}),
					INFOBTN("Weapons facing down-left (coming from up-right) will move in this direction."),
					Label(text = "Down-Right Reflect", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = refls_r_down,
						onSelectFunc = [&](int32_t val)
						{
							refls_r_down = val;
						}),
					INFOBTN("Weapons facing down-right (coming from up-left) will move in this direction.")
				)
			);
		}
		[[fallthrough]];
		case cMIRROR: case cMIRRORSLASH: case cMIRRORBACKSLASH: case cMAGICPRISM: case cMAGICPRISM4:
		{
			int32_t& refl_flags = local_ref.c_attributes[0].val;
			windowRow->add(
				Row(
					Label(text = "Reflect Flags:", hAlign = 1.0),
					Button(
						width = 1.5_em, padding = 0_px, forceFitH = true,
						text = "P", hAlign = 1.0, onPressFunc = [&]()
						{
							auto flags = refl_flags;
							static const vector<CheckListInfo> refltypes =
							{
								{ "Rock" },
								{ "Arrow" },
								{ CheckListInfo::DISABLED, "Boomerang" },
								{ "Fireball" },
								{ "Sword / Beam" },
								{ "Magic" },
								{ "Flame" },
								{ "Script (all)" },
								{ "Fireball 2" },
								{ "Light Beam" },
								{ "Script 1" },
								{ "Script 2" },
								{ "Script 3" },
								{ "Script 4" },
								{ "Script 5" },
								{ "Script 6" },
								{ "Script 7" },
								{ "Script 8" },
								{ "Script 9" },
								{ "Script 10" },
								{ "Flame 2" },
								{ "LW Wind" },
								{ "EW Wind" },
							};
							if(!call_checklist_dialog("Select weapon types to reflect",refltypes,flags,8))
								return;
							refl_flags = flags;
						}
					),
					INFOBTN_EX("These weapons will be reflected. If none are checked, all weapons will be reflected.", forceFitH = true, padding = 0_px)
				)
			);
			break;
		}
		case cICY:
		{
			zfix& start_spd = local_ref.c_attributes[8];
			zfix& leeway_frames = local_ref.c_attributes[9];
			int32_t& accel = local_ref.c_attributes[0].val;
			int32_t& decel = local_ref.c_attributes[1].val;
			int32_t& max_spd = local_ref.c_attributes[2].val;
			start_spd.doTrunc();
			leeway_frames.doTrunc();
			
			windowRow->add(
				Column(padding = 0_px,
					Rows<2>(
						INFOBTN("Pushable blocks pushed onto this combo will slide past it, if nothing blocks their way."),
						Checkbox(
							text = "Slides Blocks",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag1, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag1,state);
							}),
						INFOBTN("The Hero will slip and slide on the ice. Requires Newer Hero Movement." + QRHINT({qr_NEW_HERO_MOVEMENT2})),
						Checkbox(
							text = "Slides Hero",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag2, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag2,state);
								update();
							})
					),
					grids[0] = Rows<3>(
						Label(text = "Start Speed Percentage", hAlign = 1.0),
						INFOBTN("The percentage of the Hero's movement speed to carry onto the ice when first stepping onto it."),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 0, high = 255, val = start_spd,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								start_spd = val;
							}),
						Label(text = "Entry Leeway Frames", hAlign = 1.0),
						INFOBTN("For this many frames after entering the ice, the Hero will have additional traction. The traction gradually decreases over time until the frames are up."),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 0, high = 255, val = leeway_frames,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								leeway_frames = val;
							}),
						Label(text = "Acceleration", hAlign = 1.0),
						INFOBTN("Speed gained when holding a direction, in pixels per frame."),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							low = 0, high = SWAP_MAX, val = accel,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								accel = val;
							}),
						Label(text = "Deceleration", hAlign = 1.0),
						INFOBTN("Speed lost when not holding a direction, in pixels per frame."),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							low = 0, high = SWAP_MAX, val = decel,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								decel = val;
							}),
						Label(text = "Max Speed", hAlign = 1.0),
						INFOBTN("The highest speed that can be reached, in pixels per frame."),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							low = 0, high = SWAP_MAX, val = max_spd,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								max_spd = val;
							})
					)
				)
			);
			break;
		}
		case cCVUP: case cCVDOWN: case cCVLEFT: case cCVRIGHT:
		{
			int32_t& xspd = local_ref.c_attributes[0].val;
			int32_t& yspd = local_ref.c_attributes[1].val;
			zfix& rate = local_ref.c_attributes[8];
			rate.doTrunc();
			
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem& itm){return isConveyorType(itm.value);});
			rs_sz[0] = 2;
			windowRow->add(
				Column(padding = 0_px,
					Rows<4>(
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = !(local_ref.usrflags&cflag2),
							text = "Type-based Movement",
							indx = 0,
							onToggle = message::RSET0
						),
						Label(text = "Type:", hAlign = 1.0),
						ddls[0] = DropDownList(data = lists[0],
							fitParent = true, selectedValue = local_ref.type,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.type = val;
								updateTitle();
							}),
						INFOBTN_REF(getComboTypeHelpText(local_ref.type)),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag2,
							text = "Custom Movement",
							indx = 1,
							onToggle = message::RSET0
						),
						Label(text = "XSpeed:", hAlign = 1.0),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = xspd, disabled = !(local_ref.usrflags&cflag2),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								xspd = val;
							}),
						INFOBTN("Pixels moved in the X direction per rate frames"),
						
						DummyWidget(),
						Label(text = "YSpeed:", hAlign = 1.0),
						tfs[1] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = yspd, disabled = !(local_ref.usrflags&cflag2),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								yspd = val;
							}),
						INFOBTN("Pixels moved in the Y direction per rate frames"),
						
						DummyWidget(),
						Label(text = "Rate:", hAlign = 1.0),
						tfs[2] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							val = rate, disabled = !(local_ref.usrflags&cflag2),
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								rate = val;
							}),
						INFOBTN("Every this many frames the conveyor moves by the set speeds."
							"\nIf set to 0, acts as if set to 1.")
						//
					),
					Rows_Columns<2, 3>(
						INFOBTN("While the conveyor is moving the Hero, they are 'stunned'."),
						cboxes[0] = Checkbox(
							text = "Stunned while moving",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag1, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag1,state);
							}
						),
						INFOBTN("Forces the Hero to face in the conveyor's direction"),
						cboxes[1] = Checkbox(
							text = "Force Dir",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag3, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag3,state);
							}
						),
						INFOBTN("Uses the half-combo-grid to help avoid getting stuck on corners"),
						cboxes[2] = Checkbox(
							text = "Smart Corners",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag4, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag4,state);
							}
						),
						INFOBTN("If the Hero has boots with the 'heavy' flag, the conveyor will not push them."),
						cboxes[3] = Checkbox(
							text = "Heavy Boots Disable",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag5, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag5,state);
							}
						),
						INFOBTN("The Hero will walk the speed/dir of the conveyor, unable to walk against it. Requires Newer Hero Movement." + QRHINT({qr_NEW_HERO_MOVEMENT2})),
						cboxes[4] = Checkbox(
							text = "Force Walk",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag6, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag6,state);
							}
						),
						INFOBTN("Moves sprites that are in the z-axis."),
						cboxes[5] = Checkbox(
							text = "Affects sprites in the air",
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag7, fitParent = true,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag7,state);
							}
						)
					)
				)
			);
			break;
		}
		case cTALLGRASS: case cTALLGRASSTOUCHY: case cTALLGRASSNEXT:
		{
			zfix& decospr = local_ref.c_attributes[8];
			zfix& dropitem = local_ref.c_attributes[9];
			zfix& cutsfx = local_ref.c_attributes[10];
			zfix& walksfx = local_ref.c_attributes[11];
			zfix& grass_spr = local_ref.c_attributes[14];
			decospr.doTrunc();
			dropitem.doTrunc();
			cutsfx.doTrunc();
			walksfx.doTrunc();
			grass_spr.doTrunc();
			
			auto radmode = 0;
			if(local_ref.usrflags&cflag1)
			{
				radmode = 2;
				if(local_ref.usrflags&cflag10)
					radmode = 1;
			}
			auto radmode2 = 0;
			if(local_ref.usrflags&cflag2)
			{
				radmode2 = 1;
				if(local_ref.usrflags&cflag11)
					radmode2 = 2;
			}
			
			auto defcut = cutsfx;
			if(!(local_ref.usrflags&cflag3))
				defcut = QMisc.miscsfx[sfxBUSHGRASS];
			
			windowRow->add(
				Column(padding = 0_px,
					Rows<3>(
						Label(text = "Sprite", colSpan = 3),
						//
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = radmode == 0,
							text = "Default Sprite",
							indx = 0,
							onToggle = message::RSET0
						),
						DummyWidget(),
						INFOBTN("Display default clippings"),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = radmode == 1,
							text = "Custom Clipping",
							indx = 1,
							onToggle = message::RSET0
						),
						ddls[0] = DropDownList(data = list_clippings,
							fitParent = true, selectedValue = radmode==1 ? decospr : 0_zf,
							disabled = radmode != 1,
							onSelectFunc = [&](int32_t val)
							{
								decospr = val;
							}),
						INFOBTN("Which clipping sprite to display when broken"),
						//
						rset[0][2] = Radio(
							hAlign = 0.0,
							checked = radmode == 2,
							text = "Custom Sprite",
							indx = 2,
							onToggle = message::RSET0
						),
						ddls[1] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = radmode==2 ? decospr : 0_zf,
							disabled = radmode != 2,
							onSelectFunc = [&](int32_t val)
							{
								decospr = val;
							}),
						INFOBTN("Which custom sprite to display when broken"),
						//
						//
						Label(text = "Drops", colSpan = 3),
						//
						rset[1][0] = Radio(
							hAlign = 0.0,
							checked = radmode2 == 0,
							text = "Default Dropset",
							set = 1, indx = 0,
							onToggle = message::RSET1
						),
						DummyWidget(),
						INFOBTN("Drop an item from the default dropset"),
						//
						rset[1][1] = Radio(
							hAlign = 0.0,
							checked = radmode2 == 1,
							text = "Custom Dropset",
							set = 1, indx = 1,
							onToggle = message::RSET1
						),
						ddls[2] = DropDownList(data = list_dropsets,
							fitParent = true, selectedValue = radmode==1 ? dropitem : 0_zf,
							disabled = radmode != 1,
							onSelectFunc = [&](int32_t val)
							{
								dropitem = val;
							}),
						INFOBTN("Which dropset to drop an item from when broken"),
						//
						rset[1][2] = Radio(
							hAlign = 0.0,
							checked = radmode2 == 2,
							text = "Custom Item",
							set = 1, indx = 2,
							onToggle = message::RSET1
						),
						ddls[3] = DropDownList(data = parent.list_items,
							fitParent = true, selectedValue = radmode==2 ? dropitem : 0_zf,
							disabled = radmode != 2,
							onSelectFunc = [&](int32_t val)
							{
								dropitem = val;
							}),
						INFOBTN("Which item to drop when broken"),
						//
						//
						Label(text = "SFX", colSpan = 3),
						//
						rset[2][0] = Radio(
							hAlign = 0.0,
							checked = !(local_ref.usrflags&cflag3),
							text = "Default Cut SFX",
							set = 2, indx = 0,
							onToggle = message::RSET2
						),
						DummyWidget(),
						INFOBTN("Play the default cut SFX"),
						//
						rset[2][1] = Radio(
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag3,
							text = "Custom Cut SFX",
							set = 2, indx = 1,
							onToggle = message::RSET2
						),
						ddls[4] = DropDownList(data = list_sfx,
							fitParent = true, selectedValue = defcut,
							disabled = !(local_ref.usrflags&cflag3),
							onSelectFunc = [&](int32_t val)
							{
								cutsfx = val;
							}),
						INFOBTN("What SFX to play when cut"),
						//
						Label(text = "Walk SFX"),
						ddls[5] = DropDownList(data = list_sfx,
							fitParent = true, selectedValue = walksfx,
							onSelectFunc = [&](int32_t val)
							{
								walksfx = val;
							}),
						INFOBTN("The SFX to play when the Hero walks through this combo. If 0, no sound is played."),
						//
						Label(text = "Grass Sprite:", hAlign = 1.0),
						ddls[6] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = grass_spr,
							onSelectFunc = [&](int32_t val)
							{
								grass_spr = val;
							}),
						INFOBTN("The sprite used to display when walking through tall grass.")
					)
				)
			);
			rs_sz[0] = 3;
			rs_sz[1] = 3;
			rs_sz[2] = 2;
			break;
		}
		case cBUSH: case cBUSHTOUCHY: case cFLOWERS: case cSLASHNEXTTOUCHY:
		case cSLASHITEM: case cSLASHNEXTITEMTOUCHY:
		case cSLASHNEXTITEM: case cBUSHNEXT: case cSLASHITEMTOUCHY:
		case cFLOWERSTOUCHY: case cBUSHNEXTTOUCHY:
		{
			zfix& decospr = local_ref.c_attributes[8];
			zfix& dropitem = local_ref.c_attributes[9];
			zfix& cutsfx = local_ref.c_attributes[10];
			decospr.doTrunc();
			dropitem.doTrunc();
			cutsfx.doTrunc();
			
			auto radmode = 0;
			if(local_ref.usrflags&cflag1)
			{
				radmode = 2;
				if(local_ref.usrflags&cflag10)
					radmode = 1;
			}
			auto radmode2 = 0;
			if(local_ref.usrflags&cflag2)
			{
				radmode2 = 1;
				if(local_ref.usrflags&cflag11)
					radmode2 = 2;
			}
			
			auto defcut = cutsfx;
			if(!(local_ref.usrflags&cflag3))
				defcut = QMisc.miscsfx[sfxBUSHGRASS];
			
			windowRow->add(
				Column(padding = 0_px,
					Rows<3>(
						Label(text = "Sprite", colSpan = 3),
						//
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = radmode == 0,
							text = "Default Sprite",
							indx = 0,
							onToggle = message::RSET0
						),
						DummyWidget(),
						INFOBTN("Display default clippings"),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = radmode == 1,
							text = "Custom Clipping",
							indx = 1,
							onToggle = message::RSET0
						),
						ddls[0] = DropDownList(data = list_clippings,
							fitParent = true, selectedValue = radmode==1 ? decospr : 0_zf,
							disabled = radmode != 1,
							onSelectFunc = [&](int32_t val)
							{
								decospr = val;
							}),
						INFOBTN("Which clipping sprite to display when broken"),
						//
						rset[0][2] = Radio(
							hAlign = 0.0,
							checked = radmode == 2,
							text = "Custom Sprite",
							indx = 2,
							onToggle = message::RSET0
						),
						ddls[1] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = radmode==2 ? decospr : 0_zf,
							disabled = radmode != 2,
							onSelectFunc = [&](int32_t val)
							{
								decospr = val;
							}),
						INFOBTN("Which custom sprite to display when broken"),
						//
						//
						Label(text = "Drops", colSpan = 3),
						//
						rset[1][0] = Radio(
							hAlign = 0.0,
							checked = radmode2 == 0,
							text = "Default Dropset",
							set = 1, indx = 0,
							onToggle = message::RSET1
						),
						DummyWidget(),
						INFOBTN("Drop an item from the default dropset"),
						//
						rset[1][1] = Radio(
							hAlign = 0.0,
							checked = radmode2 == 1,
							text = "Custom Dropset",
							set = 1, indx = 1,
							onToggle = message::RSET1
						),
						ddls[2] = DropDownList(data = list_dropsets,
							fitParent = true, selectedValue = radmode2==1 ? dropitem : 0_zf,
							disabled = radmode2 != 1,
							onSelectFunc = [&](int32_t val)
							{
								dropitem = val;
							}),
						INFOBTN("Which dropset to drop an item from when broken"),
						//
						rset[1][2] = Radio(
							hAlign = 0.0,
							checked = radmode2 == 2,
							text = "Custom Item",
							set = 1, indx = 2,
							onToggle = message::RSET1
						),
						ddls[3] = DropDownList(data = parent.list_items,
							fitParent = true, selectedValue = radmode2==2 ? dropitem : 0_zf,
							disabled = radmode2 != 2,
							onSelectFunc = [&](int32_t val)
							{
								dropitem = val;
							}),
						INFOBTN("Which item to drop when broken"),
						//
						//
						Label(text = "SFX", colSpan = 3),
						//
						rset[2][0] = Radio(
							hAlign = 0.0,
							checked = !(local_ref.usrflags&cflag3),
							text = "Default Cut SFX",
							set = 2, indx = 0,
							onToggle = message::RSET2
						),
						DummyWidget(),
						INFOBTN("Play the default cut SFX"),
						//
						rset[2][1] = Radio(
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag3,
							text = "Custom Cut SFX",
							set = 2, indx = 1,
							onToggle = message::RSET2
						),
						ddls[4] = DropDownList(data = list_sfx,
							fitParent = true, selectedValue = defcut,
							disabled = !(local_ref.usrflags&cflag3),
							onSelectFunc = [&](int32_t val)
							{
								cutsfx = val;
							}),
						INFOBTN("What SFX to play when cut")
					)
				)
			);
			rs_sz[0] = 3;
			rs_sz[1] = 3;
			rs_sz[2] = 2;
			break;
		}
		case cSLASHNEXT:
		{
			zfix& decospr = local_ref.c_attributes[8];
			zfix& cutsfx = local_ref.c_attributes[10];
			decospr.doTrunc();
			cutsfx.doTrunc();
			
			auto radmode = 0;
			if(local_ref.usrflags&cflag1)
			{
				radmode = 2;
				if(local_ref.usrflags&cflag10)
					radmode = 1;
			}
			
			auto defcut = cutsfx;
			if(!(local_ref.usrflags&cflag3))
				defcut = QMisc.miscsfx[sfxBUSHGRASS];
			
			windowRow->add(
				Column(padding = 0_px,
					Rows<3>(
						Label(text = "Sprite", colSpan = 3),
						//
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = radmode == 0,
							text = "Default Sprite",
							indx = 0,
							onToggle = message::RSET0
						),
						DummyWidget(),
						INFOBTN("Display default clippings"),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = radmode == 1,
							text = "Custom Clipping",
							indx = 1,
							onToggle = message::RSET0
						),
						ddls[0] = DropDownList(data = list_clippings,
							fitParent = true, selectedValue = radmode==1 ? decospr : 0_zf,
							disabled = radmode != 1,
							onSelectFunc = [&](int32_t val)
							{
								decospr = val;
							}),
						INFOBTN("Which clipping sprite to display when broken"),
						//
						rset[0][2] = Radio(
							hAlign = 0.0,
							checked = radmode == 2,
							text = "Custom Sprite",
							indx = 2,
							onToggle = message::RSET0
						),
						ddls[1] = DropDownList(data = list_sprites,
							fitParent = true, selectedValue = radmode==2 ? decospr : 0_zf,
							disabled = radmode != 2,
							onSelectFunc = [&](int32_t val)
							{
								decospr = val;
							}),
						INFOBTN("Which custom sprite to display when broken"),
						//
						//
						Label(text = "SFX", colSpan = 3),
						//
						rset[2][0] = Radio(
							hAlign = 0.0,
							checked = !(local_ref.usrflags&cflag3),
							text = "Default Cut SFX",
							set = 2, indx = 0,
							onToggle = message::RSET2
						),
						DummyWidget(),
						INFOBTN("Play the default cut SFX"),
						//
						rset[2][1] = Radio(
							hAlign = 0.0,
							checked = local_ref.usrflags&cflag3,
							text = "Custom Cut SFX",
							set = 2, indx = 1,
							onToggle = message::RSET2
						),
						ddls[4] = DropDownList(data = list_sfx,
							fitParent = true, selectedValue = defcut,
							disabled = !(local_ref.usrflags&cflag3),
							onSelectFunc = [&](int32_t val)
							{
								cutsfx = val;
							}),
						INFOBTN("What SFX to play when cut")
					)
				)
			);
			rs_sz[0] = 3;
			rs_sz[2] = 2;
			break;
		}
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
		{
			size_t lvl = 0;
			if(local_ref.type == cCHEST)
				lvl = 1;
			else if(local_ref.type == cBOSSCHEST)
				lvl = 2;
			else if(local_ref.type == cLOCKEDCHEST)
				lvl = 3;
			
			zfix& exstate = local_ref.c_attributes[13];
			zfix& openbtn = local_ref.c_attributes[10];
			zfix& opensfx = local_ref.c_attributes[11];
			zfix& contains_item = local_ref.c_attributes[18];
			exstate.doTrunc();
			openbtn.doTrunc();
			opensfx.doTrunc();
			contains_item.doTrunc();
			
			auto radmode0 = 0;
			if(local_ref.usrflags&cflag16)
				radmode0 = 1;
			
			auto radmode1 = 0;
			auto spitem_def = contains_item;
			auto normitem_def = 0;
			if(valid_item_id(contains_item))
			{
				radmode1 = 1;
				spitem_def = -1;
				normitem_def = contains_item;
			}
			
			auto radmode2 = 0;
			if(local_ref.usrflags & cflag4)
				radmode2 = 1;
			auto radmode3 = 0;
			if(local_ref.usrflags & cflag8)
				radmode3 = 1;
			else if(local_ref.usrflags & cflag6)
				radmode3 = 2;
			
			zfix& prompt_combo = local_ref.c_attributes[1];
			zfix& prompt_combo2 = local_ref.c_attributes[2];
			zfix& prompt_cset = local_ref.c_attributes[12];
			zfix& prompt_xoff = local_ref.c_attributes[16];
			zfix& prompt_yoff = local_ref.c_attributes[17];
			prompt_combo.doTrunc();
			prompt_combo2.doTrunc();
			prompt_cset.doTrunc();
			prompt_xoff.doTrunc();
			prompt_yoff.doTrunc();
			
			int def_prompt_combo = 0;
			int def_prompt_combo2 = 0;
			int def_prompt_cset = 0;
			int def_prompt_xoff = 12;
			int def_prompt_yoff = -8;
			if(local_ref.usrflags&cflag13)
			{
				def_prompt_combo = prompt_combo;
				def_prompt_cset = prompt_cset;
				def_prompt_xoff = prompt_xoff;
				def_prompt_yoff = prompt_yoff;
				if(lvl > 1) //boss or locked
					def_prompt_combo2 = prompt_combo2;
			}
			
			zfix& reqitem = local_ref.c_attributes[8];
			zfix& usecounter = local_ref.c_attributes[9];
			zfix& amount = local_ref.c_attributes[0];
			zfix& messagestr = local_ref.c_attributes[3];
			reqitem.doTrunc();
			usecounter.doTrunc();
			amount.doTrunc();
			messagestr.doTrunc();
			
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem& itm)
				{
					switch(itm.value)
					{
						case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
							itm.tag = 0;
							return true;
						case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
							itm.tag = 1;
							return true;
					}
					return false;
				}).tagsort();
			lists[1] = parent.list_items.copy().filter(
				[](GUI::ListItem& itm)
				{
					return (get_item_data(itm.value).flags & item_gamedata) != 0;
				});
			windowRow->add(Column(
				Row(
					ddls[0] = DropDownList(data = lists[0],
						fitParent = true, selectedValue = local_ref.type,
						onSelectFunc = [&](int32_t val)
						{
							local_ref.type = val;
							updateTitle();
							update();
						}),
					INFOBTN_REF(getComboTypeHelpText(local_ref.type))
				),
				tpan[0] = TabPanel(
					TabRef(name = "State", Rows<3>(
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = radmode0 == 0,
							text = "Chest State",
							indx = 0,
							onToggle = message::RSET0
						),
						DummyWidget(),
						INFOBTN("Chest will use the standard 'Chest'/'Locked Chest'/'Boss Chest' screen state."),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = radmode0 == 1,
							text = "Ex State",
							indx = 1,
							onToggle = message::RSET0
						),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							val = exstate, disabled = radmode0 == 0,
							high = 31,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								exstate = val;
							}),
						INFOBTN("Which ExtraState (0 to 31) to use instead of the standard screen state"),
						//
						cboxes[0] = Checkbox(
							text = "Use Special Item State", hAlign = 0.0,
							checked = local_ref.usrflags&cflag7,
							colSpan = 2,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_ref.usrflags,cflag7,state);
							}
						),
						INFOBTN("Link the contained item to 'Special Item' screen state."
							"\nThis means that picking up the item will count as collecting the screen's Special Item."
							"\nThis does NOT make the chest contain the item set in Screen Data - see the Content tab for that.")
					)),
					TabRef(name = "Opening", Column(
						Row(
							Rows<2>(padding = 0_px,
								Checkbox(
									text = "Can't use from top", hAlign = 0.0,
									checked = local_ref.usrflags&cflag9,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag9,state);
									}
								),
								INFOBTN("Cannot be activated standing to the top side if checked"),
								//
								Checkbox(
									text = "Can't use from bottom", hAlign = 0.0,
									checked = local_ref.usrflags&cflag10,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag10,state);
									}
								),
								INFOBTN("Cannot be activated standing to the bottom side if checked"),
								//
								Checkbox(
									text = "Can't use from left", hAlign = 0.0,
									checked = local_ref.usrflags&cflag11,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag11,state);
									}
								),
								INFOBTN("Cannot be activated standing to the left side if checked"),
								//
								Checkbox(
									text = "Can't use from right", hAlign = 0.0,
									checked = local_ref.usrflags&cflag12,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag12,state);
									}
								),
								INFOBTN("Cannot be activated standing to the right side if checked")
							),
							INTBTN_PANEL(openbtn,"Buttons:","Which buttons should interact with the chest?"
									"\nIf no buttons are selected, walking into the chest will interact with it.")
						),
						Row(
							Label(text = "Open SFX:"),
							ddls[1] = DropDownList(data = list_sfx,
								fitParent = true, selectedValue = opensfx,
								onSelectFunc = [&](int32_t val)
								{
									opensfx = val;
								}),
							INFOBTN("The sound to play when opening the chest")
						)
					)),
					TabRef(name = "Content", Rows<3>(
						Label(text = "Contains:", colSpan = 3),
						//
						rset[1][0] = Radio(
							hAlign = 0.0,
							checked = radmode1 == 0,
							text = "Variable Content:",
							indx = 0,
							onToggle = message::RSET1,
							set = 1
						),
						ddls[2] = DropDownList(data = list_chest_content,
							fitParent = true, selectedValue = spitem_def,
							onSelectFunc = [&](int32_t val)
							{
								contains_item = val;
							}),
						INFOBTN("Which value to use as an item for the chest to contain"),
						//
						rset[1][1] = Radio(
							hAlign = 0.0,
							checked = radmode1 == 1,
							text = "Specific Item:",
							indx = 1,
							onToggle = message::RSET1,
							set = 1
						),
						ddls[3] = DropDownList(data = parent.list_items,
							fitParent = true, selectedValue = normitem_def,
							onSelectFunc = [&](int32_t val)
							{
								contains_item = val;
							}),
						INFOBTN("Which item to put in the chest")
					)),
					TabRef(name = "Prompts", Row(
						Columns<2>(padding = 0_px,
							Row(padding = 0_px,
								cboxes[1] = Checkbox(
									text = "Display Prompt", hAlign = 0.0,
									checked = local_ref.usrflags&cflag13,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag13,state);
										update();
									}
								),
								INFOBTN("Displays a prompt combo when able to interact")
							),
							frames[0] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<3>(
									Label(text = "Prompt Combo:"),
									cmbswatches[0] = SelComboSwatch(
										combo = def_prompt_combo,
										cset = def_prompt_cset,
										onSelectFunc = [&](int32_t cmb, int32_t c)
										{
											prompt_combo = cmb;
											prompt_cset = c;
											cmbswatches[1]->setCSet(prompt_cset);
										}
									),
									INFOBTN("The combo/cset to use for the prompt"),
									//
									Label(text = "Prompt XOffset:"),
									tfs[1] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_SSHORT,
										low = -32768, high = 32767, val = def_prompt_xoff,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											prompt_xoff = val;
										}),
									INFOBTN("The x-offset in pixels of the prompt"),
									//
									Label(text = "Prompt YOffset:"),
									tfs[2] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_SSHORT,
										low = -32768, high = 32767, val = def_prompt_yoff,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											prompt_yoff = val;
										}),
									INFOBTN("The y-offset in pixels of the prompt")
								)
							),
							Row(padding = 0_px,
								cboxes[2] = Checkbox(
									text = "Separate Locked Prompt", hAlign = 0.0,
									checked = lvl > 1 && (local_ref.usrflags&cflag13) && def_prompt_combo2,
									disabled = lvl < 2 || !(local_ref.usrflags&cflag13),
									onToggleFunc = [&](bool state)
									{
										update();
									}
								),
								INFOBTN("Displays a separate prompt combo when unable to unlock")
							),
							frames[1] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Row(
									Label(text = "Locked Prompt Combo:"),
									cmbswatches[1] = SelComboSwatch(
										combo = def_prompt_combo2,
										cset = def_prompt_cset,
										onSelectFunc = [&](int32_t cmb, int32_t c)
										{
											prompt_combo2 = cmb;
											prompt_cset = c;
											cmbswatches[0]->setCSet(prompt_cset);
										}
									),
									INFOBTN("The combo to use for the locked prompt")
								)
							)
						)
					)),
					TabRef(name = "Locking", Column(
						grids[0] = Rows<2>(padding = 0_px,
							Row(padding = 0_px,
								cboxes[3] = Checkbox(
									text = "Use Item", hAlign = 0.0,
									checked = local_ref.usrflags&cflag1,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag1,state);
										update();
									}
								),
								INFOBTN("Allow an item in your inventory to unlock the chest")
							),
							Label(text = "Counter"),
							frames[2] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<3>(
									cboxes[4] = Checkbox(
										text = "Require Item (disables counters)", hAlign = 0.0,
										checked = local_ref.usrflags&cflag2,
										colSpan = 2,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.usrflags,cflag2,state);
											update();
										}
									),
									INFOBTN("Only the required item can open this chest (instead of ALSO allowing a key/counter)"),
									//
									cboxes[5] = Checkbox(
										text = "Consume Item", hAlign = 0.0,
										checked = local_ref.usrflags&cflag5,
										colSpan = 2,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.usrflags,cflag5,state);
											update();
										}
									),
									INFOBTN("Consume the required item instead of simply requiring its presence"),
									//
									Label(text = "Required Item:"),
									ddls[4] = DropDownList(data = lists[1],
										fitParent = true, selectedValue = (local_ref.usrflags&cflag1) ? reqitem : 0_zf,
										onSelectFunc = [&](int32_t val)
										{
											reqitem = val;
										}),
									INFOBTN("The item that can open the chest.")
								)
							),
							frames[3] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<4>(
									rset[2][0] = Radio(
										hAlign = 0.0,
										checked = radmode2 == 0,
										text = "Keys",
										indx = 0,
										onToggle = message::RSET2,
										colSpan = 3, set = 2
									),
									INFOBTN("Use level or general keys"),
									//
									rset[2][1] = Radio(
										hAlign = 0.0,
										checked = radmode2 == 1,
										text = "Specified Counter:",
										indx = 1,
										onToggle = message::RSET2,
										colSpan = 2, set = 2
									),
									ddls[5] = DropDownList(data = list_counters,
										fitParent = true, selectedValue = (radmode2==1) ? int(usecounter) : crMONEY,
										onSelectFunc = [&](int32_t val)
										{
											usecounter = val;
										}),
									INFOBTN("Use a specified counter instead of keys"),
									//
									DummyWidget(),
									lbls[0] = Label(text = "Amount:"),
									tfs[3] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
										low = 1, high = 65535, val = amount,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											amount = val;
										}),
									INFOBTN("The amount of keys/specified counter to cost"),
									//
									
									//Radio for 'normal'/'no drain'/'thief'
									rset[3][0] = Radio(
										hAlign = 0.0,
										checked = radmode3 == 0,
										text = "Normal Cost",
										indx = 0,
										onToggle = message::RSET3,
										colSpan = 3, set = 3
									),
									btns[0] = INFOBTN("Spend the specified cost to open, if you have enough."),
									//
									rset[3][1] = Radio(
										hAlign = 0.0,
										checked = radmode3 == 1,
										text = "No Drain",
										indx = 1,
										onToggle = message::RSET3,
										colSpan = 3, set = 3
									),
									btns[1] = INFOBTN("Open if the specified cost is met, without spending."),
									//
									rset[3][2] = Radio(
										hAlign = 0.0,
										checked = radmode3 == 2,
										text = "Thief",
										indx = 2,
										onToggle = message::RSET3,
										colSpan = 3, set = 3
									),
									btns[2] = INFOBTN("Spend the specified cost to open, if you have enough."
										"\nIf you don't have enough, steal what you do have!")
								)
							)
						),
						Row(
							Label(text = "Locked String:", hAlign = 1.0),
							ddls[1] = DropDownList(data = parent.list_strings,
								fitParent = true, selectedValue = messagestr,
								onSelectFunc = [&](int32_t val)
								{
									messagestr = val;
								}),
							INFOBTN("The string to play when failing to open. Negative values are special, reading the string number from somewhere else.")
						)
					))
				)
			));
			rs_sz[0] = 2;
			rs_sz[1] = 2;
			rs_sz[2] = 2;
			rs_sz[3] = 3;
			break;
		}
		case cLOCKBLOCK: case cBOSSLOCKBLOCK:
		case cLOCKBLOCK2: case cBOSSLOCKBLOCK2:
		{
			size_t lvl = 0;
			if(local_ref.type == cBOSSLOCKBLOCK)
				lvl = 2;
			else if(local_ref.type == cLOCKBLOCK)
				lvl = 3;
			
			zfix& exstate = local_ref.c_attributes[13];
			zfix& openbtn = local_ref.c_attributes[10];
			zfix& opensfx = local_ref.c_attributes[11];
			exstate.doTrunc();
			openbtn.doTrunc();
			opensfx.doTrunc();
			
			auto radmode0 = 0;
			if(local_ref.usrflags&cflag16)
				radmode0 = 1;
			
			auto radmode2 = 0;
			if(local_ref.usrflags & cflag4)
				radmode2 = 1;
			auto radmode3 = 0;
			if(local_ref.usrflags & cflag8)
				radmode3 = 1;
			else if(local_ref.usrflags & cflag6)
				radmode3 = 2;
			
			zfix& prompt_combo = local_ref.c_attributes[1];
			zfix& prompt_combo2 = local_ref.c_attributes[2];
			zfix& prompt_cset = local_ref.c_attributes[12];
			zfix& prompt_xoff = local_ref.c_attributes[16];
			zfix& prompt_yoff = local_ref.c_attributes[17];
			prompt_combo.doTrunc();
			prompt_combo2.doTrunc();
			prompt_cset.doTrunc();
			prompt_xoff.doTrunc();
			prompt_yoff.doTrunc();
			
			int def_prompt_combo = 0;
			int def_prompt_combo2 = 0;
			int def_prompt_cset = 0;
			int def_prompt_xoff = 12;
			int def_prompt_yoff = -8;
			if(local_ref.usrflags&cflag13)
			{
				def_prompt_combo = prompt_combo;
				def_prompt_cset = prompt_cset;
				def_prompt_xoff = prompt_xoff;
				def_prompt_yoff = prompt_yoff;
				if(lvl > 1) //boss or locked
					def_prompt_combo2 = prompt_combo2;
			}
			
			zfix& reqitem = local_ref.c_attributes[8];
			zfix& usecounter = local_ref.c_attributes[9];
			zfix& amount = local_ref.c_attributes[0];
			zfix& messagestr = local_ref.c_attributes[3];
			reqitem.doTrunc();
			usecounter.doTrunc();
			amount.doTrunc();
			messagestr.doTrunc();
			
			lists[0] = GUI::ZCListData::combotype(true).filter(
				[](GUI::ListItem& itm)
				{
					switch(itm.value)
					{
						case cLOCKBLOCK: case cBOSSLOCKBLOCK:
							itm.tag = 0;
							return true;
						case cLOCKBLOCK2: case cBOSSLOCKBLOCK2:
							itm.tag = 1;
							return true;
					}
					return false;
				}).tagsort();
			lists[1] = parent.list_items.copy().filter(
				[](GUI::ListItem& itm)
				{
					return (get_item_data(itm.value).flags & item_gamedata) != 0;
				});
			windowRow->add(Column(
				Row(
					ddls[0] = DropDownList(data = lists[0],
						fitParent = true, selectedValue = local_ref.type,
						onSelectFunc = [&](int32_t val)
						{
							local_ref.type = val;
							updateTitle();
							update();
						}),
					INFOBTN_REF(getComboTypeHelpText(local_ref.type))
				),
				tpan[0] = TabPanel(
					TabRef(name = "State", Rows<3>(
						rset[0][0] = Radio(
							hAlign = 0.0,
							checked = radmode0 == 0,
							text = "Block State",
							indx = 0,
							onToggle = message::RSET0
						),
						DummyWidget(),
						INFOBTN("Block will use the standard 'Lockblock'/'Boss Block' screen state."),
						//
						rset[0][1] = Radio(
							hAlign = 0.0,
							checked = radmode0 == 1,
							text = "Ex State",
							indx = 1,
							onToggle = message::RSET0
						),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							val = exstate, disabled = radmode0 == 0,
							high = 31,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								exstate = val;
							}),
						INFOBTN("Which ExtraState (0 to 31) to use instead of the standard screen state")
					)),
					TabRef(name = "Opening", Column(
						Row(
							Rows<2>(padding = 0_px,
								Checkbox(
									text = "Can't use from top", hAlign = 0.0,
									checked = local_ref.usrflags&cflag9,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag9,state);
									}
								),
								INFOBTN("Cannot be activated standing to the top side if checked"),
								//
								Checkbox(
									text = "Can't use from bottom", hAlign = 0.0,
									checked = local_ref.usrflags&cflag10,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag10,state);
									}
								),
								INFOBTN("Cannot be activated standing to the bottom side if checked"),
								//
								Checkbox(
									text = "Can't use from left", hAlign = 0.0,
									checked = local_ref.usrflags&cflag11,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag11,state);
									}
								),
								INFOBTN("Cannot be activated standing to the left side if checked"),
								//
								Checkbox(
									text = "Can't use from right", hAlign = 0.0,
									checked = local_ref.usrflags&cflag12,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag12,state);
									}
								),
								INFOBTN("Cannot be activated standing to the right side if checked")
							),
							Column(padding = 0_px,
								Row(padding = 0_px,
									Label(text = "Buttons:"),
									INFOBTN("Which buttons should interact with the lockblock?"
										"\nIf no buttons are selected, walking into the chest will interact with it.")
								),
								Columns<4>(
									Checkbox(
										text = "A", hAlign = 0.0,
										checked = openbtn&0x1,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x1,state);
										}
									),
									Checkbox(
										text = "B", hAlign = 0.0,
										checked = openbtn&0x2,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x2,state);
										}
									),
									Checkbox(
										text = "L", hAlign = 0.0,
										checked = openbtn&0x4,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x4,state);
										}
									),
									Checkbox(
										text = "R", hAlign = 0.0,
										checked = openbtn&0x8,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x8,state);
										}
									),
									Checkbox(
										text = "Ex1", hAlign = 0.0,
										checked = openbtn&0x10,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x10,state);
										}
									),
									Checkbox(
										text = "Ex2", hAlign = 0.0,
										checked = openbtn&0x20,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x20,state);
										}
									),
									Checkbox(
										text = "Ex3", hAlign = 0.0,
										checked = openbtn&0x40,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x40,state);
										}
									),
									Checkbox(
										text = "Ex4", hAlign = 0.0,
										checked = openbtn&0x80,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x80,state);
										}
									)
								)
							)
						),
						Row(
							Label(text = "Unlock SFX:"),
							ddls[1] = DropDownList(data = list_sfx,
								fitParent = true, selectedValue = opensfx,
								onSelectFunc = [&](int32_t val)
								{
									opensfx = val;
								}),
							INFOBTN("The sound to play when opening the block")
						)
					)),
					TabRef(name = "Prompts", Row(
						Columns<2>(padding = 0_px,
							Row(padding = 0_px,
								cboxes[1] = Checkbox(
									text = "Display Prompt", hAlign = 0.0,
									checked = local_ref.usrflags&cflag13,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag13,state);
										update();
									}
								),
								INFOBTN("Displays a prompt combo when able to interact")
							),
							frames[0] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<3>(
									Label(text = "Prompt Combo:"),
									cmbswatches[0] = SelComboSwatch(
										combo = def_prompt_combo,
										cset = def_prompt_cset,
										onSelectFunc = [&](int32_t cmb, int32_t c)
										{
											prompt_combo = cmb;
											prompt_cset = c;
											cmbswatches[1]->setCSet(prompt_cset);
										}
									),
									INFOBTN("The combo/cset to use for the prompt"),
									//
									Label(text = "Prompt XOffset:"),
									tfs[1] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_SSHORT,
										low = -32768, high = 32767, val = def_prompt_xoff,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											prompt_xoff = val;
										}),
									INFOBTN("The x-offset in pixels of the prompt"),
									//
									Label(text = "Prompt YOffset:"),
									tfs[2] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_SSHORT,
										low = -32768, high = 32767, val = def_prompt_yoff,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											prompt_yoff = val;
										}),
									INFOBTN("The y-offset in pixels of the prompt")
								)
							),
							Row(padding = 0_px,
								cboxes[2] = Checkbox(
									text = "Separate Locked Prompt", hAlign = 0.0,
									checked = lvl > 1 && (local_ref.usrflags&cflag13) && def_prompt_combo2,
									disabled = lvl < 2 || !(local_ref.usrflags&cflag13),
									onToggleFunc = [&](bool state)
									{
										update();
									}
								),
								INFOBTN("Displays a separate prompt combo when unable to unlock")
							),
							frames[1] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Row(
									Label(text = "Locked Prompt Combo:"),
									cmbswatches[1] = SelComboSwatch(
										combo = def_prompt_combo2,
										cset = def_prompt_cset,
										onSelectFunc = [&](int32_t cmb, int32_t c)
										{
											prompt_combo2 = cmb;
											prompt_cset = c;
											cmbswatches[0]->setCSet(prompt_cset);
										}
									),
									INFOBTN("The combo to use for the locked prompt")
								)
							)
						)
					)),
					TabRef(name = "Locking", Column(
						grids[0] = Rows<2>(padding = 0_px,
							Row(padding = 0_px,
								cboxes[3] = Checkbox(
									text = "Use Item", hAlign = 0.0,
									checked = local_ref.usrflags&cflag1,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag1,state);
										update();
									}
								),
								INFOBTN("Allow an item in your inventory to unlock the block")
							),
							Label(text = "Counter"),
							frames[2] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<3>(
									cboxes[4] = Checkbox(
										text = "Require Item (disables counters)", hAlign = 0.0,
										checked = local_ref.usrflags&cflag2,
										colSpan = 2,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.usrflags,cflag2,state);
											update();
										}
									),
									INFOBTN("Only the required item can open this block (instead of ALSO allowing a key/counter)"),
									//
									cboxes[5] = Checkbox(
										text = "Consume Item", hAlign = 0.0,
										checked = local_ref.usrflags&cflag5,
										colSpan = 2,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.usrflags,cflag5,state);
											update();
										}
									),
									INFOBTN("Consume the required item instead of simply requiring its presence"),
									//
									Label(text = "Required Item:"),
									ddls[4] = DropDownList(data = lists[1],
										fitParent = true, selectedValue = (local_ref.usrflags&cflag1) ? reqitem : 0_zf,
										onSelectFunc = [&](int32_t val)
										{
											reqitem = val;
										}),
									INFOBTN("The item that can open the block.")
								)
							),
							frames[3] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<4>(
									rset[2][0] = Radio(
										hAlign = 0.0,
										checked = radmode2 == 0,
										text = "Keys",
										indx = 0,
										onToggle = message::RSET2,
										colSpan = 3, set = 2
									),
									INFOBTN("Use level or general keys"),
									//
									rset[2][1] = Radio(
										hAlign = 0.0,
										checked = radmode2 == 1,
										text = "Specified Counter:",
										indx = 1,
										onToggle = message::RSET2,
										colSpan = 2, set = 2
									),
									ddls[5] = DropDownList(data = list_counters,
										fitParent = true, selectedValue = (radmode2==1) ? int(usecounter) : crMONEY,
										onSelectFunc = [&](int32_t val)
										{
											usecounter = val;
										}),
									INFOBTN("Use a specified counter instead of keys"),
									//
									DummyWidget(),
									lbls[0] = Label(text = "Amount:"),
									tfs[3] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_ZSINT_NO_DEC,
										low = 1, high = 65535, val = amount,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											amount = val;
										}),
									INFOBTN("The amount of keys/specified counter to cost"),
									//
									
									//Radio for 'normal'/'no drain'/'thief'
									rset[3][0] = Radio(
										hAlign = 0.0,
										checked = radmode3 == 0,
										text = "Normal Cost",
										indx = 0,
										onToggle = message::RSET3,
										colSpan = 3, set = 3
									),
									btns[0] = INFOBTN("Spend the specified cost to open, if you have enough."),
									//
									rset[3][1] = Radio(
										hAlign = 0.0,
										checked = radmode3 == 1,
										text = "No Drain",
										indx = 1,
										onToggle = message::RSET3,
										colSpan = 3, set = 3
									),
									btns[1] = INFOBTN("Open if the specified cost is met, without spending."),
									//
									rset[3][2] = Radio(
										hAlign = 0.0,
										checked = radmode3 == 2,
										text = "Thief",
										indx = 2,
										onToggle = message::RSET3,
										colSpan = 3, set = 3
									),
									btns[2] = INFOBTN("Spend the specified cost to open, if you have enough."
										"\nIf you don't have enough, steal what you do have!")
								)
							)
						),
						Row(
							Label(text = "Locked String:", hAlign = 1.0),
							ddls[1] = DropDownList(data = parent.list_strings,
								fitParent = true, selectedValue = messagestr,
								onSelectFunc = [&](int32_t val)
								{
									messagestr = val;
								}),
							INFOBTN("The string to play when failing to open. Negative values are special, reading the string number from somewhere else.")
						)
					))
				)
			));
			rs_sz[0] = 2;
			rs_sz[2] = 2;
			rs_sz[3] = 3;
			break;
		}
		case cSIGNPOST:
		{
			int32_t& messagestr = local_ref.c_attributes[0].val;
			
			zfix& openbtn = local_ref.c_attributes[10];
			zfix& prompt_combo = local_ref.c_attributes[9];
			zfix& prompt_cset = local_ref.c_attributes[12];
			zfix& prompt_xoff = local_ref.c_attributes[16];
			zfix& prompt_yoff = local_ref.c_attributes[17];
			openbtn.doTrunc();
			prompt_combo.doTrunc();
			prompt_cset.doTrunc();
			prompt_xoff.doTrunc();
			prompt_yoff.doTrunc();
			
			int def_prompt_combo = 0;
			int def_prompt_cset = 0;
			int def_prompt_xoff = 12;
			int def_prompt_yoff = -8;
			if(local_ref.usrflags&cflag13)
			{
				def_prompt_combo = prompt_combo;
				def_prompt_cset = prompt_cset;
				def_prompt_xoff = prompt_xoff;
				def_prompt_yoff = prompt_yoff;
			}
			
			windowRow->add(
				tpan[0] = TabPanel(
					TabRef(name = "String", Rows<3>(
						Label(text = "String:", hAlign = 1.0),
						ddls[1] = DropDownList(data = parent.list_strings,
							fitParent = true, selectedValue = messagestr/10000,
							onSelectFunc = [&](int32_t val)
							{
								messagestr = val*10000;
							}),
						INFOBTN("The string to play. Negative values are special, reading the string number from somewhere else.")
					)),
					TabRef(name = "Reading", Column(
						Row(
							Rows<2>(padding = 0_px,
								Checkbox(
									text = "Can't use from top", hAlign = 0.0,
									checked = local_ref.usrflags&cflag9,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag9,state);
									}
								),
								INFOBTN("Cannot be activated standing to the top side if checked"),
								//
								Checkbox(
									text = "Can't use from bottom", hAlign = 0.0,
									checked = local_ref.usrflags&cflag10,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag10,state);
									}
								),
								INFOBTN("Cannot be activated standing to the bottom side if checked"),
								//
								Checkbox(
									text = "Can't use from left", hAlign = 0.0,
									checked = local_ref.usrflags&cflag11,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag11,state);
									}
								),
								INFOBTN("Cannot be activated standing to the left side if checked"),
								//
								Checkbox(
									text = "Can't use from right", hAlign = 0.0,
									checked = local_ref.usrflags&cflag12,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag12,state);
									}
								),
								INFOBTN("Cannot be activated standing to the right side if checked")
							),
							Column(padding = 0_px,
								Row(padding = 0_px,
									Label(text = "Buttons:"),
									INFOBTN("Which buttons should interact with the signpost?"
										"\nIf no buttons are selected, walking into the chest will interact with it.")
								),
								Columns<4>(
									Checkbox(
										text = "A", hAlign = 0.0,
										checked = openbtn&0x1,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x1,state);
										}
									),
									Checkbox(
										text = "B", hAlign = 0.0,
										checked = openbtn&0x2,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x2,state);
										}
									),
									Checkbox(
										text = "L", hAlign = 0.0,
										checked = openbtn&0x4,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x4,state);
										}
									),
									Checkbox(
										text = "R", hAlign = 0.0,
										checked = openbtn&0x8,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x8,state);
										}
									),
									Checkbox(
										text = "Ex1", hAlign = 0.0,
										checked = openbtn&0x10,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x10,state);
										}
									),
									Checkbox(
										text = "Ex2", hAlign = 0.0,
										checked = openbtn&0x20,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x20,state);
										}
									),
									Checkbox(
										text = "Ex3", hAlign = 0.0,
										checked = openbtn&0x40,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x40,state);
										}
									),
									Checkbox(
										text = "Ex4", hAlign = 0.0,
										checked = openbtn&0x80,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(openbtn,0x80,state);
										}
									)
								)
							)
						)
					)),
					TabRef(name = "Prompts", Row(
						Columns<2>(padding = 0_px,
							Row(padding = 0_px,
								cboxes[1] = Checkbox(
									text = "Display Prompt", hAlign = 0.0,
									checked = local_ref.usrflags&cflag13,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.usrflags,cflag13,state);
										update();
									}
								),
								INFOBTN("Displays a prompt combo when able to interact")
							),
							frames[0] = Frame(padding = 0_px,vAlign = 0.5,fitParent = true,
								Rows<3>(
									Label(text = "Prompt Combo:"),
									cmbswatches[0] = SelComboSwatch(
										combo = def_prompt_combo,
										cset = def_prompt_cset,
										onSelectFunc = [&](int32_t cmb, int32_t c)
										{
											prompt_combo = cmb;
											prompt_cset = c;
											cmbswatches[0]->setCSet(prompt_cset);
										}
									),
									INFOBTN("The combo/cset to use for the prompt"),
									//
									Label(text = "Prompt XOffset:"),
									tfs[1] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_SSHORT,
										low = -32768, high = 32767, val = def_prompt_xoff,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											prompt_xoff = val;
										}),
									INFOBTN("The x-offset in pixels of the prompt"),
									//
									Label(text = "Prompt YOffset:"),
									tfs[2] = TextField(
										fitParent = true, minwidth = 8_em,
										type = GUI::TextField::type::SWAP_SSHORT,
										low = -32768, high = 32767, val = def_prompt_yoff,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											prompt_yoff = val;
										}),
									INFOBTN("The y-offset in pixels of the prompt")
								)
							)
						)
					))
				)
			);
			break;
		}
		case cCRUMBLE:
		{
			zfix& crumble_type = local_ref.c_attributes[8];
			zfix& crumble_sens = local_ref.c_attributes[9];
			zfix& crumble_time = local_ref.c_attributes[16];
			zfix& reset_change = local_ref.c_attributes[17];
			crumble_type.doTrunc();
			crumble_sens.doTrunc();
			crumble_time.doTrunc();
			reset_change.doTrunc();
			lists[0] = GUI::ListData({
				{ "Reset", CMBTY_CRUMBLE_RESET, "Timer resets when stepped off of, can also change combo" },
				{ "Cumulative", CMBTY_CRUMBLE_CUMULATIVE, "Timer does not reset, just pauses when stepped off of" },
				{ "Inevitable", CMBTY_CRUMBLE_INEVITABLE, "Timer keeps going even if stepped off of" }
			});
			windowRow->add(Column(
				Rows<3>(
					Label(text = "Crumble Type:", hAlign = 1.0),
					ddls[0] = DropDownList(data = lists[0],
						fitParent = true, selectedValue = crumble_type,
						onSelectFunc = [&](int32_t val)
						{
							crumble_type = val;
							frames[0]->setDisabled(val != CMBTY_CRUMBLE_RESET);
						}),
					INFOBTN_REF(lists[0].findInfo(crumble_type)),
					DummyWidget(),
					Checkbox(text = "Continuous", _EX_RBOX,
						checked = (local_ref.usrflags&cflag1),
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_ref.usrflags,cflag1,state);
						}),
					INFOBTN("If the next combo is a Crumbling combo of the 'Inevitable'"
						" type, it will continue crumbling automatically.")
				),
				Frame(
					Rows<3>(
						Label(text = "Crumble Sensitivity:"),
						tfs[0] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_BYTE,
							low = 0, high = 255, val = crumble_sens,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								crumble_sens = val;
							}),
						INFOBTN("This many pixels of leeway are given. 0 = fully sensitive."
							"\nIf leeway exceeds combo/ffc size, no crumbling will occur."),
						//
						Label(text = "Crumble Time:"),
						tfs[1] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_SSHORT,
							low = 0, high = 32767, val = crumble_time,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								crumble_time = val;
							}),
						INFOBTN("The time, in frames, before the combo crumbles into the next combo in the combo list.")
					)
				),
				frames[0] = Frame(
					Rows<3>(
						Label(text = "Reset Change:"),
						tfs[2] = TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_SSHORT,
							low = -32768, high = 32767, val = reset_change,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								reset_change = val;
							}),
						INFOBTN("If non-zero, the combo will change by this amount (ex. 1 = Next, -1 = Prev)"
							" when the Hero steps off of the combo before it finishes crumbling. (Might be used to reset a crumble animation)")
					)
				)
			));
			frames[0]->setDisabled(crumble_type != CMBTY_CRUMBLE_RESET);	
			break;
		}
		case cCUTSCENEEFFECT:
		{
			zfix& effect_type = local_ref.c_attributes[8];
			effect_type.doTrunc();
			lists[0] = GUI::ListData({
				{ "Player Walk", CUTEFF_PLAYER_WALK, "The player auto-walks to the destination (angularly) (when triggered via '->ComboType Effects')."
					"\nTriggers 'ComboType Causes->' when the player finishes the auto-walk (either because they reached their destination,"
					" or because they reached the edge of the screen on their way to the destination, triggering just before scrolling occurs)"
					"\nNOTE: The player will walk ignoring solidity and most combo types in a straight line to the destination."
					"\nRequires 'Newer Hero Movement'." + QRHINT({qr_NEW_HERO_MOVEMENT2}) }
			});
			std::shared_ptr<GUI::Grid> g;
			switch(effect_type.getTrunc())
			{
				case CUTEFF_PLAYER_WALK:
				{
					auto rel_flags = cflag1|cflag2;
					lists[1] = GUI::ListData({
						{ "Absolute", 0 },
						{ "Relative to Combo", cflag1 },
						{ "Relative to Player", cflag2 }
					});
					int32_t& dest_x = local_ref.c_attributes[0].val;
					int32_t& dest_y = local_ref.c_attributes[1].val;
					int32_t& move_speed = local_ref.c_attributes[2].val;
					g = Rows<3>(
						Label(text = "Dest X:", hAlign = 1.0),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = dest_x,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								dest_x = val;
							}),
						INFOBTN("The X position the player will walk to."),
						Label(text = "Dest Y:", hAlign = 1.0),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = dest_y,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								dest_y = val;
							}),
						INFOBTN("The Y position the player will walk to."),
						Label(text = "Dest Position:", hAlign = 1.0),
						DropDownList(data = lists[1],
							fitParent = true, selectedValue = local_ref.usrflags & rel_flags,
							onSelectFunc = [&](int32_t val)
							{
								local_ref.usrflags = (local_ref.usrflags & ~rel_flags) | val;
							}),
						INFOBTN("If the 'Dest X'/'Dest Y' are Absolute within the region, Relative to the Combo's position,"
							" or Relative to the Player's current position."),
						Label(text = "Move Speed", hAlign = 1.0),
						TextField(
							fitParent = true, minwidth = 8_em,
							type = GUI::TextField::type::SWAP_ZSINT,
							val = move_speed,
							onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
							{
								move_speed = val;
							}),
						INFOBTN("The speed the player will move at, in pixels per frame. If '0', the player will use their normal step speed.")
					);
					break;
				}
				default: // bad type, default and reload
				{
					effect_type = CUTEFF_PLAYER_WALK;
					combo_default(local_ref, true);
					refresh_dlg();
					break;
				}
			}
			std::shared_ptr<GUI::Grid> col;
			windowRow->add(col = Column(
				Row(
					ddls[0] = DropDownList(data = lists[0],
						fitParent = true, selectedValue = effect_type,
						onSelectFunc = [&](int32_t val)
						{
							alt_refs[effect_type] = local_ref;
							
							effect_type = val;
							
							auto ref_it = alt_refs.find(val);
							if(ref_it != alt_refs.end())
								local_ref = ref_it->second;
							else
								combo_default(local_ref, true);
						}),
					INFOBTN_REF(lists[0].findInfo(effect_type))
				)
			));
			if(g) col->add(g);
			break;
		}
		case cSAVE: case cSAVE2:
		{
			zfix& heal_hp = local_ref.c_attributes[8];
			zfix& heal_mp = local_ref.c_attributes[9];
			zfix& save_menu = local_ref.c_attributes[10];
			heal_hp.doTrunc();
			heal_mp.doTrunc();
			save_menu.doTrunc();
			
			if(!(local_ref.usrflags&cflag1))
				heal_hp = 0;
			if(!(local_ref.usrflags&cflag2))
				heal_mp = 0;
			if (save_menu > NUM_SAVE_MENUS)
				save_menu = 0;
			
			lists[0] = GUI::ZCListData::savemenus(true, true, true);
			
			windowRow->add(Column(
				Rows<3>(
					Checkbox(text = "Heal HP to %", _EX_RBOX,
						checked = local_ref.usrflags&cflag1,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_ref.usrflags, cflag1, state);
							tfs[0]->setDisabled(!state);
						}),
					tfs[0] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_BYTE,
						low = 0, high = 100, val = heal_hp,
						disabled = !(local_ref.usrflags&cflag1),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							heal_hp = val;
						}),
					INFOBTN("Heals the player to the specified percentage of their HP, if their HP was less than that amount."),
					Checkbox(text = "Heal MP to %", _EX_RBOX,
						checked = local_ref.usrflags&cflag2,
						onToggleFunc = [&](bool state)
						{
							SETFLAG(local_ref.usrflags, cflag2, state);
							tfs[1]->setDisabled(!state);
						}),
					tfs[1] = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_BYTE,
						low = 0, high = 100, val = heal_mp,
						disabled = !(local_ref.usrflags&cflag2),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
						{
							heal_mp = val;
						}),
					INFOBTN("Heals the player to the specified percentage of their MP, if their MP was less than that amount."),
					Checkbox(text = "Default 'Save+Quit' Menu", _EX_RBOX, colSpan = 2,
						checked = local_ref.type == cSAVE2,
						onToggleFunc = [&](bool state)
						{
							local_ref.type = state ? cSAVE2 : cSAVE;
						}),
					INFOBTN("Determines which save menu to display, if 'Save Menu:' is set to [Default].")
				),
				Rows<3>(
					Label(text = "Save Menu:", hAlign = 1.0),
					DropDownList(data = lists[0],
						fitParent = true, selectedValue = save_menu,
						onSelectFunc = [&](int32_t val)
						{
							save_menu = val;
						}),
					INFOBTN("Which save menu to display when activated.")
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
		InfoDialog("WIP","The '" + ctyname + "' wizard is WIP,"
			" and may not be fully functional!").show();
	return window;
}

static newcombo* _instance = nullptr;
static bool defaulted = false;
static bool def_all()
{
	combo_default(*_instance, false);
	defaulted = true;
	return true;
}
static bool def_some()
{
	combo_default(*_instance, true);
	defaulted = true;
	return true;
}
bool do_combo_default(newcombo& ref)
{
	defaulted = false;
	_instance = &ref;
	AlertFuncDialog("Are you sure?",
		"Reset the combo to default?",
		""
	).add_buttons(2,
		{ "Whole Combo", "Only attributes/flags", "Cancel" },
		{ def_all, def_some, nullptr }
	).show();
	return defaulted;
}
bool ComboWizardDialog::handleMessage(const GUI::DialogMessage<message>& msg)
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
			if(do_combo_default(local_ref))
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

