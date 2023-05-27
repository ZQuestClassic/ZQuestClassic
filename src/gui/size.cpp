#include "gui/size.h"
#include "gui/common.h"
#include "base/zc_alleg.h"
#include "base/fonts.h"

extern int dlgfontheight;
namespace GUI
{

int32_t Size::emSize()
{
	return dlgfontheight;
}

}
