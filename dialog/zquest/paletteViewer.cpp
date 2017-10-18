#include "paletteViewer.h"
#include "../../gui/factory.h"
#include <boost/bind.hpp>

#include "../../gui/bitmap.h"

namespace
{

class PaletteViewerBitmap: public GUI::BitmapBackend
{
public:
    void initialize()
    {
        // There should be a better way to handle colors...
        renderer->initialize(257, 257);
        for(int cset=0; cset<16; cset++)
        {
            for(int color=0; color<16; color++)
            {
                int x=color*16;
                int y=cset*16;
                renderer->fillRect(x, y, 17, 17, 241); // 241 is black
                renderer->fillRect(x+1, y+1, 15, 15, cset*16+color);
            }
        }
    }
    
    void redraw()
    {
        // Never changes - nothing to do
    }
};

}

GUI::Widget* PaletteViewer::createDialog(const GUI::WidgetFactory& f)
{
    return f.window("Current palette",
      f.column(GUI::Contents(
        f.bitmap(new PaletteViewerBitmap(), 1),
        f.button("O&K", boost::bind(&PaletteViewer::shutDown, this))
      ))
    );
}
