#include "test_quest_dlg.h"
#include "alertfunc.h"
#include "process_managment.h"
#include "zq_class.h"
#include "gui/builder.h"
#include "zc_list_data.h"

int32_t onSave();
int32_t onSaveAs();
extern char *filepath;
extern bool saved, first_save;
extern dmap *DMaps;

static int32_t test_start_dmap = 0, test_start_screen = 0, test_ret_sqr = 0;
static process_killer test_killer;

static const GUI::ListData retsqrlist
{
	{ "A", 0 },
	{ "B", 1 },
	{ "C", 2 },
	{ "D", 3 }
};

static bool do_save()
{
	onSave();
	return true;
}
static bool do_save_as()
{
	onSaveAs();
	return true;
}
static bool skip = false;
static bool do_skip()
{
	skip = true;
	return true;
}

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
		skip = false;
		AlertFuncDialog("Save",
			"Unsaved changes will not be tested!",
			3, 0, //3 buttons, where buttons[0] is focused
			"Save", do_save,
			"Save As", do_save_as,
			"Test", do_skip
		).show();
		
		if(!(skip || saved))
			return;
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

TestQstDialog::TestQstDialog() : dmap_list(GUI::ZCListData::dmaps(true))
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
					}),
				Label(text = "Return Square:"),
				DropDownList(
					maxwidth = 10_em,
					data = retsqrlist,
					selectedValue = test_ret_sqr,
					onSelectFunc = [&](int32_t val)
					{
						test_ret_sqr = val;
					}
				)
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
			if(!fileexists(ZELDA_FILE))
			{
				InfoDialog("Error", ZELDA_FILE " not found!").show();
				return true;
			}
			test_killer.kill();

			char arg2[5];
			sprintf(arg2, "%d", test_start_dmap);
			char arg3[5];
			sprintf(arg3, "%d", test_start_screen);
			char arg4[5];
			sprintf(arg4, "%d", test_ret_sqr);
			const char* argv[] = {
#ifndef _WIN32
				ZELDA_FILE,
#endif
				"-test",
				filepath,
				arg2,
				arg3,
				arg4,
				NULL
			};

			test_killer = launch_process(ZELDA_FILE, argv);
		}
		return true;
		
		case message::CANCEL:
		default:
			return true;
	}
}
