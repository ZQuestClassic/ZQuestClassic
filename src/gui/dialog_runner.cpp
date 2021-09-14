#include "dialog_runner.h"
#include "common.h"
#include "../gui.h"
#include "../jwin.h"

using std::shared_ptr;

namespace GUI
{

/* Called from jwin procs when an event occurs.
 * d->d1 is the index of the child responsible.
 * c is the event.
 */
int dialog_proc(int msg, DIALOG *d, int c)
{
	auto* dr=static_cast<DialogRunner*>(d->dp);
	if(msg==MSG_GUI_EVENT)
	{
		int ret=dr->widgets[d->d1]->onEvent(c, dr->sendMessage);
		if(dr->done)
			return D_EXIT;
		else
		{
			dr->redrawPending=true;
			return ret;
		}
	}
	else if(msg==MSG_IDLE && dr->redrawPending)
	{
		// In the old system, many things sent messages by closing the dialog,
		// and the return value from do_zqdialog() became the message.
		// Some widgets don't have code to indicate that they need redrawn
		// since the dialog would be closed and reopened in that case.
		dr->redrawPending=false;
		return D_REDRAW;
	}
	else
		return D_O_K;
}

DialogRunner::DialogRunner(): focused(-1), redrawPending(false), done(false)
{}

DialogRef DialogRunner::push(shared_ptr<Widget> owner, DIALOG dlg)
{
	auto pos=alDialog.size();
	if(owner->getFocused())
		focused=pos;
	widgets.emplace_back(std::move(owner));
	alDialog.push_back(dlg);
	return DialogRef(this, pos);
}

void DialogRunner::realize(shared_ptr<Widget> root)
{
	alDialog.push_back({
		dialog_proc, // proc
		0, 0, 0, 0, // x, y, width, height
		0, 0, // fg, bg
		0, // key
		0, // flags
		0, 0, // d1, d2
		this, nullptr, &newGuiMarker // dp, dp2, dp3
	});

	root->calculateSize();
	if(is_large)
		root->arrange(0, 0, 800, 600);
	else
		root->arrange(0, 0, 320, 240);
	root->realize(*this);

	alDialog.push_back({
		nullptr, // proc
		0, 0, 0, 0, // x, y, width, height
		0, 0, // fg, bg
		0, // key
		0, // flags
		0, 0, // d1, d2
		nullptr, nullptr, nullptr // dp1, dp2, dp3
	});
}

void DialogRunner::runInner(std::shared_ptr<Widget> root)
{
	realize(root);
	new_gui_popup_dialog(alDialog.data(), focused, done);
}

}
