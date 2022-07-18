#include "size.h"
#include "common.h"
#include "base/zc_alleg.h"

extern FONT* lfont_l;
extern FONT* nfont;

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
