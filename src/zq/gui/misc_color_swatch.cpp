#include "misc_color_swatch.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include "zq/zquest.h"
#include <cassert>
#include <utility>
#include <allegro/internal/aintern.h>

int32_t newg_misccolors_proc(int32_t msg, DIALOG* d, int32_t)
{
    int32_t mul = 12;

    bool dis = d->flags & D_DISABLED;
    GUI::MiscColorSwatch* ptr = (GUI::MiscColorSwatch*)d->dp3;
    switch (msg)
    {
    case MSG_CLICK:
        if (ptr->GetHexClicked() != -1)
        {
            int32_t color_col = vbound(((gui_mouse_x() - ptr->getX() - 8) / mul), 0, 15);
            int32_t color_row = vbound(((gui_mouse_y() - ptr->getY() - 10) / mul), 0, 13);
            ptr->SetColor((color_row*16)+color_col);
        }

        break;

    case MSG_DRAW:
        for (int32_t i = 0; i < 10; i++)
        {
            textprintf_centre_ex(screen, font, d->x + 8 + 4 + (i * mul), d->y, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%d", i);
        }

        for (int32_t i = 0; i < 6; i++)
        {
            textprintf_centre_ex(screen, font, d->x + 8 + 4 + ((10 + i) * mul), d->y, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%c", i + 'A');
        }

        for (int32_t i = 0; i < 10; i++)
        {
            textprintf_right_ex(screen, font, d->x + 6, d->y + (i * mul) + 10, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%d", i);
        }

        for (int32_t i = 0; i < 2; i++)
        {
            textprintf_right_ex(screen, font, d->x + 6, d->y + ((i + 10) * mul) + 10, jwin_pal[jcBOXFG], jwin_pal[jcBOX], "%c", i + 'A');
        }

        jwin_draw_frame(screen, d->x + 6, d->y + 8, int32_t(132 * 1.5) - 2, int32_t(100 * 1.5) - 2, FR_DEEP);

        for (int32_t i = 0; i < 192; i++)
        {
            int32_t px2 = d->x + int32_t(((i & 15) << 3) * 1.5) + 8;
            int32_t py2 = d->y + int32_t(((i >> 4) << 3) * 1.5) + 8 + 2;
            rectfill(screen, px2, py2, px2 + (mul - 1), py2 + (mul - 1), i);
        }

        break;
    }

    return D_O_K;
}

namespace GUI
{
    MiscColorSwatch::MiscColorSwatch() :
        hexclicked(-1)
    {}

    void MiscColorSwatch::click()
    {
        if (alDialog && allowDraw())
        {
            if (alDialog.message(MSG_CLICK, 0) & D_REDRAW)
            {
                pendDraw();
            }
        }
    }

    void MiscColorSwatch::setHexClicked(int16_t val)
    {
        hexclicked = val;
    }

    void MiscColorSwatch::setColor(byte val)
    {
        ASSERT(hexclicked > 0 && hexclicked < 17); //valid index check
        color_buf[hexclicked] = val;
    }

    byte MiscColorSwatch::getHexClicked()
    {
        return hexclicked;
    }

    byte MiscColorSwatch::getColor()
    {
        ASSERT(hexclicked > 0 && hexclicked < 17); //valid index check
        return color_buf[hexclicked];
    }

    void MiscColorSwatch::applyVisibility(bool visible)
    {
        Widget::applyVisibility(visible);
        if (alDialog) alDialog.applyVisibility(visible);
    }

    void MiscColorSwatch::applyDisabled(bool dis)
    {
        Widget::applyDisabled(dis);
        if (alDialog) alDialog.applyDisabled(dis);
    }

    void MiscColorSwatch::applyFont(FONT* newFont)
    {
        if (alDialog)
        {
            alDialog->dp2 = newFont;
            pendDraw();
        }
        Widget::applyFont(newFont);
    }

    void MiscColorSwatch::realize(DialogRunner& runner)
    {
        Widget::realize(runner);
        alDialog = runner.push(shared_from_this(), DIALOG{
            newGUIProc<newg_misccolors_proc>,
            x, y, getHeight(), getHeight(),
            0, 0,
            0,
            getFlags(),
            0, 0, // d1, d2,
            nullptr, widgFont, this // dp, dp2, dp3
            });
    }

    int32_t MiscColorSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
    {
        assert(event == geCLICK);
        if (onClickFunc)
            onClickFunc();
        return -1;
    }
}