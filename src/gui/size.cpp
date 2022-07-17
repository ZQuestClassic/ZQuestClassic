#include "size.h"
#include "common.h"
#include "../zc_alleg.h"
#include "zquest.h"

namespace GUI
{

int32_t Size::emSize()
{
	static const int32_t em = text_height(is_large ? lfont_l : nfont);
	return em;
}

Size Size::sized(int32_t size) noexcept
{
	return Size(is_large ? size : (size*2/3));
}

}
