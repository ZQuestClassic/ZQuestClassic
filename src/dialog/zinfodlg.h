#ifndef ZC_DIALOG_ZINFODLG_H_
#define ZC_DIALOG_ZINFODLG_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "zinfo.h"

void call_zinf_dlg();

class ZInfoDialog: public GUI::Dialog<ZInfoDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, SAVE, LOAD };

	ZInfoDialog(zinfo const& cpyfrom);
	ZInfoDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	enum
	{
		FLD_IC_NAME,
		FLD_IC_HELP,
		FLD_CT_NAME,
		FLD_CT_HELP,
		FLD_MF_NAME,
		FLD_MF_HELP,
		FLD_CTR_NAME,
		FLD_WPN_NAME,
		FLD_ETYPE_NAME,
		FLD_LI_NAME,
		FLD_LI_ABBR,
		FLD_LI_HELP,
		NUM_FIELDS
	};
	enum
	{
		LBL_IC_HELP,
		LBL_CT_HELP,
		LBL_MF_HELP,
		LBL_LI_HELP,
		NUM_LBLS
	};
	std::shared_ptr<GUI::TextField> fields[NUM_FIELDS];
	std::shared_ptr<GUI::Checkbox> defcheck[NUM_FIELDS];
	std::shared_ptr<GUI::Label> helplbl[NUM_LBLS];
	zinfo lzinfo;
	GUI::ListData list_itemclass, list_combotype, list_mapflag,
		list_counters, list_weapon, list_efamilies, list_level_items;
};

#endif
