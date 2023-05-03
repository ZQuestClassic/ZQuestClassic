#include "frame.h"
#include "dialog_runner.h"
#include "../jwin.h"

// int32_t d_dummy_proc(int32_t, DIALOG*, int32_t)
// {
	// return D_O_K;
// }

namespace GUI
{

Frame::Frame(): frameStyle(style::ETCHED)
{}

void Frame::setTitle(const std::string& newTitle)
{
	if(newTitle.empty())
		title = "";
	else title = " "+newTitle+" ";
	if(titleDlg)
	{
		titleDlg->dp = title.data();
		titleDlg.message(MSG_START,0);
		if(infoDlg)
		{
			infoDlg->x = titleDlg->x+titleDlg->w+2;
			infoDlg->y = titleDlg->y;
		}
	}
}
void Frame::setInfo(const std::string& newInfo)
{
	info = newInfo;
}

void Frame::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog)
		alDialog.applyVisibility(visible);
	if(titleDlg)
		titleDlg.applyVisibility(visible);
	if(infoDlg)
		infoDlg.applyVisibility(visible);
	if(content)
		content->setExposed(visible);
}

void Frame::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog)
		alDialog.applyDisabled(dis);
	if(titleDlg)
		titleDlg.applyDisabled(dis);
	if(infoDlg)
		infoDlg.applyDisabled(dis);
	if(content)
		content->applyDisabled(dis);
}

void Frame::calculateSize()
{
	if(content)
	{
		content->calculateSize();
		int32_t extraHeight;
		if(!title.empty())
			extraHeight = 10;
		else
			extraHeight = 8;
		setPreferredWidth(Size::pixels(content->getTotalWidth()+8));
		setPreferredHeight(Size::pixels(content->getTotalHeight()+extraHeight));
	}
	else
	{
		setPreferredWidth(10_em);
		setPreferredHeight(5_em);
	}
	Widget::calculateSize();
}

void Frame::arrange(int32_t contX, int32_t contY, int32_t contW, int32_t contH)
{
	Widget::arrange(contX, contY, contW, contH);
	if(content)
	{
		if(!title.empty())
			content->arrange(x+4, y+8, getWidth()-8, getHeight()-10);
		else
			content->arrange(x+4, y+4, getWidth()-8, getHeight()-8);
	}
}

void Frame::realize(DialogRunner& runner)
{
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<jwin_frame_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		static_cast<int32_t>(frameStyle), 0, // d1, d2
		nullptr, nullptr, nullptr // dp, dp2, dp3
	});

	if(!title.empty())
	{
		titleDlg = runner.push(shared_from_this(), DIALOG {
			newGUIProc<jwin_text_proc>,
			x+5, y-3, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			0, 0,
			title.data(), widgFont, nullptr // dp, dp2, dp3
		});
		titleDlg.message(MSG_START,0);
	}
	else // No title
	{
		titleDlg = runner.push(shared_from_this(), DIALOG {
			newGUIProc<jwin_text_proc>,
			x+5, y-3, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			0, 0,
			(void*)"", widgFont, nullptr // dp, dp2, dp3
		});
		titleDlg.message(MSG_START,0);
	}
	if(!info.empty())
	{
		infoDlg = runner.push(shared_from_this(), DIALOG {
			newGUIProc<jwin_infobtn_proc>,
			titleDlg->x+titleDlg->w+2, titleDlg->y, text_length(widgFont,"?")*3, titleDlg->h,
			0, 0,
			0,
			getFlags(),
			0, 0,
			&info, widgFont, nullptr //dp, dp2, dp3
		});
	}
	if(content)
		content->realize(runner);
}

}
