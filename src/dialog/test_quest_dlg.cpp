#include "test_quest_dlg.h"
#include "alertfunc.h"
#include "process_managment.h"
#include "zq_class.h"
#include <gui/builder.h>

int32_t onSave();
int32_t onSaveAs();
extern char *filepath;
extern bool saved, first_save;
extern dmap *DMaps;

static int32_t test_start_dmap = 0, test_start_screen = 0;
static process_killer test_killer;

void call_testqst_dialog()
{
	if(!first_save) //Require quest file is saved (i.e. not 'Untitled' fresh from File->New)
	{
		onSaveAs();
		if(!first_save)
			return;
	}
	if(!saved) //Warn on unsaved changes
	{
		AlertFuncDialog("Save",
			"Unsaved changes will not be tested!",
			3, 0, //3 buttons, where buttons[0] is focused
			"Save", onSave,
			"Save As", onSaveAs,
			"Test", NULL
		).show();
	}
	test_start_dmap = -1;
	test_start_screen = Map.getCurrScr();
	int32_t pal = Map.getcolor();
	for(auto q = 0; q < MAXDMAPS; ++q)
	{
		if(DMaps[q].map == Map.getCurrMap())
		{
			if(pal == DMaps[q].color)
			{
				test_start_dmap = q;
				break;
			}
			if(test_start_dmap < 0)
				test_start_dmap = q;
		}
	}
	if(test_start_dmap < 0) test_start_dmap = 0;
	TestQstDialog().show();
}

TestQstDialog::TestQstDialog() : dmap_list(GUI::ListData::dmaps(true))
{}

std::shared_ptr<GUI::Widget> TestQstDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Test Quest",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			Rows<2>(
				Label(text = "Start DMap:"),
				DropDownList(data = dmap_list,
					fitParent = true,
					selectedValue = test_start_dmap,
					onSelectFunc = [&](int32_t val)
					{
						test_start_dmap = val;
					}
				),
				Label(text = "Start Screen:"),
				TextField(
					fitParent = true, minwidth = 4_em,
					type = GUI::TextField::type::SWAP_BYTE,
					low = 0x00, high = 0x7F, val = test_start_screen,
					swap_type = 1,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
					{
						test_start_screen = val;
					})
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "Test",
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

bool TestQstDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			if(!fileexists("zelda.exe"))
			{
				InfoDialog("Error", "'zelda.exe' not found!").show();
				return true;
			}
			test_killer.kill();
			char buf[2048] = {0};
			sprintf(buf, "zelda.exe -test \"%s\" %d %d", filepath, test_start_dmap, test_start_screen);
			test_killer = launch_process(buf);
		}
		return true;
		
		case message::CANCEL:
		default:
			return true;
	}
}
