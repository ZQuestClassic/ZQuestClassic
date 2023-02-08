#include "size.h"
#include "common.h"
#include "base/zc_alleg.h"
#include "base/fonts.h"

namespace GUI
{

int32_t Size::emSize()
{
	static const int32_t em = text_height(get_custom_font(CFONT_DLG));
	return em;
}

}
