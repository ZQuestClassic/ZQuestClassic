#include "gui/frame.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"

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

void Frame::setStyle(style newStyle)
{
	frameStyle = newStyle;
	if (alDialog)
	{
		alDialog->d1 = int(newStyle);
	}
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
		int extraHeight = 8;
		if(!title.empty())
			extraHeight += text_height(widgFont);
		
		int title_wid = title.empty() ? 0 : (text_length(widgFont,title.c_str())+10);
		if(!info.empty())
			title_wid += text_length(widgFont,"?")*3+2;
		int content_wid = content->getTotalWidth()+8;
		setPreferredWidth(Size::pixels(zc_max(title_wid,content_wid)));
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
		int cnt_y = title.empty() ? y+4 : (y-3+text_height(widgFont)+2);
		int extraHeight = 8;
		if(!title.empty())
			extraHeight += text_height(widgFont);
		
		content->arrange(x+4, cnt_y, getWidth()-8, getHeight()-extraHeight);
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

int32_t Frame::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCLICK);
	
	if(infoMessage >= 0)
		sendMessage(infoMessage, MessageArg::none);
	return -1;
}

}
