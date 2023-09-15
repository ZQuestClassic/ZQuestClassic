#include "gui/dialog_runner.h"
#include "gui/common.h"
#include "base/gui.h"
#include "jwin.h"

using std::shared_ptr;
bool mid_quitting_dlg = false;
namespace GUI
{

/* Called from jwin procs when an event occurs.
 * d->d1 is the index of the child responsible.
 * c is the event.
 */
int32_t dialog_proc(int32_t msg, DIALOG *d, int32_t c)
{
	auto* dr = static_cast<DialogRunner*>(d->dp);
	if(!mid_quitting_dlg)
	{
		if(close_button_quit)
		{
			dialog_open_quit = true;
			handle_close_btn_quit();
			dialog_open_quit = false;
		}
		if(exiting_program)
		{
			dr->done = true;
			return D_CLOSE;
		}
	}
	if(msg == MSG_GUI_EVENT)
	{
		MessageDispatcher md(dr->widgets[d->d1], dr->sendMessage);
		int32_t ret = dr->widgets[d->d1]->onEvent(c, md);
		if(dr->done)
			return D_CLOSE;
		else
		{
			dr->redrawPending = true;
			return ret;
		}
	}
	else if((msg == MSG_IDLE || msg == MSG_VSYNC) && dr->redrawPending)
	{
		// In the old system, many things sent messages by closing the dialog,
		// and the return value from do_zqdialog() became the message.
		// Some widgets don't have code to indicate that they need redrawn
		// since the dialog would be closed and reopened in that case.
		acquire_screen();
		broadcast_dialog_message(MSG_DRAW, 0);
		release_screen();
		dr->redrawPending = false;
		return D_REDRAWME;
	}
	else
		return D_O_K;
}

DialogRunner::DialogRunner(): focused(-1), redrawPending(false), done(false), realized(false),
	running(false)
{}

void DialogRunner::clear()
{
	focused = -1;
	redrawPending = false;
	done = false;
	realized = false;
	running = false;
	widgets.clear();
	alDialog.clear();
}

DialogRef DialogRunner::push(shared_ptr<Widget> owner, DIALOG dlg)
{
	auto pos = alDialog.size();
	if(owner->getFocused())
		focused = pos;
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
	root->arrange(0, 0, LARGE_W, LARGE_H);
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
	realized = true;
	popup_zqdialog_start_a5();
	new_gui_popup_dialog(alDialog.data(), focused, done, running);
	popup_zqdialog_end_a5();
}

}

void close_new_gui_dlg(DIALOG* d)
{
	while(d->dp3 != &newGuiMarker)
	{
		--d;
	}
	auto* dr = static_cast<GUI::DialogRunner*>(d->dp);
	dr->close();
}