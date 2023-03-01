#include "dialog_runner.h"
#include "common.h"
#include "base/gui.h"
#include "../jwin_a5.h"

using std::shared_ptr;
extern int32_t zq_screen_w, zq_screen_h;

namespace GUI
{

/* Called from jwin procs when an event occurs.
 * d->d1 is the index of the child responsible.
 * c is the event.
 */
int32_t dialog_proc(int32_t msg, DIALOG *d, int32_t c)
{
	auto* dr = static_cast<DialogRunner*>(d->dp);
	if(dr->render_froze)
	{
		dr->render_froze = false;
		dr->forceDraw();
		unfreeze_render();
		update_hw_screen(true);
	}
	if(msg == MSG_GUI_EVENT)
	{
		MessageDispatcher md(dr->widgets[d->d1], dr->sendMessage);
		int32_t ret = dr->widgets[d->d1]->onEvent(c, md);
		if(dr->done)
			return D_EXIT;
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
		dr->forceDraw();
		return D_O_K;
	}
	else
		return D_O_K;
}

DialogRunner::DialogRunner(): focused(-1), redrawPending(false), done(false), realized(false),
	running(false), x(0), y(0), render_froze(false), rerun_dlg(false)
{
	w = zq_screen_w;
	h = zq_screen_h;
}

void DialogRunner::set_dlg_sz(int nx, int ny, int nw, int nh)
{
	x=nx;
	y=ny;
	w=nw;
	h=nh;
}
void DialogRunner::clear()
{
	focused = -1;
	redrawPending = false;
	done = false;
	realized = false;
	running = false;
	render_froze = false;
	rerun_dlg = false;
	x = y = 0;
	w = zq_screen_w;
	h = zq_screen_h;
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

	dialogConstructed.emit();
}

void DialogRunner::runInner(std::shared_ptr<Widget> root)
{
	realize(root);
	realized = true;
	rerun_dlg = false;
	
	popup_zqdialog_start_a5(x,y,w,h);
	
	new_gui_popup_dialog(alDialog.data(), focused, done, running);
	
	if(rerun_dlg)
	{
		freeze_render(); //don't allow the closing of the dialog to render
		render_froze = true;
	}
	
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