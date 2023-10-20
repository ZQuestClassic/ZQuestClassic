#ifndef ZC_DIALOG_VECTORPICK_H
#define ZC_DIALOG_VECTORPICK_H

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/window.h>
#include <functional>
#include "base/containers.h"


template<typename Sz>
class VectorPickDialog: public GUI::Dialog<VectorPickDialog<Sz>>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD
	};

	VectorPickDialog(bounded_vec<Sz,int32_t>& vec, bool zsint);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bounded_vec<Sz,int32_t> local_vec;
	bounded_vec<Sz,int32_t>& dest_vec;
	bool zsint;
	size_t pg;
	
	std::shared_ptr<GUI::Window> window;
};
extern template class VectorPickDialog<byte>;
extern template class VectorPickDialog<word>;
extern template class VectorPickDialog<dword>;

template<typename Sz>
class MapPickDialog: public GUI::Dialog<MapPickDialog<Sz>>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD
	};

	MapPickDialog(bounded_map<Sz,int32_t>& mp, bool zsint);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bounded_map<Sz,int32_t> local_map;
	bounded_map<Sz,int32_t>& dest_map;
	bool zsint;
	size_t pg;
	
	std::shared_ptr<GUI::Window> window;
};
extern template class MapPickDialog<byte>;
extern template class MapPickDialog<word>;
extern template class MapPickDialog<dword>;

class NumPickDialog: public GUI::Dialog<NumPickDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD
	};

	NumPickDialog(std::string const& lbl,std::optional<int32_t>& retv,
		int32_t snum, bool zsint, int32_t max = 0, int32_t min = 0);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::string const& labeltext;
	int32_t local_val;
	int32_t min,max;
	bool zsint;
	optional<int32_t>& retv;
	
	std::shared_ptr<GUI::Window> window;
};

template<typename Sz>
inline void call_edit_vector(bounded_vec<Sz,int32_t>& vec, bool zsint)
{
	VectorPickDialog<Sz>(vec, zsint).show();
}
template<typename Sz>
inline void call_edit_map(bounded_map<Sz,int32_t>& mp, bool zsint)
{
	MapPickDialog<Sz>(mp, zsint).show();
}
optional<int32_t> call_get_num(std::string const& lbl, int32_t dv, int32_t max = 0, int32_t min = 0);
optional<zfix> call_get_zfix(std::string const& lbl, zfix dv, zfix max = 0_zf, zfix min = 0_zf);
#endif
