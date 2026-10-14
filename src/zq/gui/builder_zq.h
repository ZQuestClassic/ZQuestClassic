#ifndef ZQ_GUI_BUILDER_ZQ_H_
#define ZQ_GUI_BUILDER_ZQ_H_

// The GUI builder (gui/builder.h), plus the editor's own widgets.

#include "gui/builder.h"
#include "zq/gui/seltile_swatch.h"
#include "zq/gui/selcombo_swatch.h"
#include "zq/gui/selherotile_swatch.h"
#include "zq/gui/tileanim_frame.h"
#include "zq/gui/msgstr_preview.h"
#include "zq/gui/palette_frame.h"
#include "zq/gui/cornerselect.h"
#include "zq/gui/misc_cset_sel.h"
#include "zq/gui/misc_color_sel.h"
#include "zq/gui/misc_color_row.h"
#include "zq/gui/dmap_minimap.h"
#include "zq/gui/dmap_mapgrid.h"
#include "zq/gui/quest_list_view.h"
#include "zq/gui/dmap_frame.h"
#include "zq/gui/regiongrid.h"
#include "zq/gui/engraving.h"

namespace GUI::Internal
{

inline std::shared_ptr<SelTileSwatch> makeSelTileSwatch()
{
	return std::make_shared<SelTileSwatch>();
}

inline std::shared_ptr<SelComboSwatch> makeSelComboSwatch()
{
	return std::make_shared<SelComboSwatch>();
}

inline std::shared_ptr<CornerSwatch> makeCornerSwatch()
{
	return std::make_shared<CornerSwatch>();
}

inline std::shared_ptr<SelHeroTileSwatch> makeSelHeroTileSwatch()
{
	return std::make_shared<SelHeroTileSwatch>();
}

inline std::shared_ptr<TileFrame> makeTileFrame()
{
	return std::make_shared<TileFrame>();
}

inline std::shared_ptr<PaletteFrame> makePaletteFrame()
{
	return std::make_shared<PaletteFrame>();
}

inline std::shared_ptr<MsgPreview> makeMsgPreview()
{
	return std::make_shared<MsgPreview>();
}

inline std::shared_ptr<MiscCSetSel> makeMiscCSetSel()
{
	return std::make_shared<MiscCSetSel>();
}

inline std::shared_ptr<MiscColorSel> makeMiscColorSel()
{
	return std::make_shared<MiscColorSel>();
}

inline std::shared_ptr<MiscColorRow> makeMiscColorRow()
{
	return std::make_shared<MiscColorRow>();
}

inline std::shared_ptr<DMapMinimap> makeDMapMinimap()
{
	return std::make_shared<DMapMinimap>();
}

inline std::shared_ptr<DMapMapGrid> makeDMapMapGrid()
{
	return std::make_shared<DMapMapGrid>();
}

inline std::shared_ptr<QuestListView> makeQuestListView()
{
	return std::make_shared<QuestListView>();
}

inline std::shared_ptr<DMapFrame> makeDMapFrame()
{
	return std::make_shared<DMapFrame>();
}

inline std::shared_ptr<Engraving> makeEngraving()
{
	return std::make_shared<Engraving>();
}

inline std::shared_ptr<RegionGrid> makeRegionGrid()
{
	return std::make_shared<RegionGrid>();
}

} // namespace GUI::Internal

namespace GUI::Builder
{

ZCGUI_BUILDER_START(SelTileSwatch)
	ZCGUI_ACCEPT_PROP(tile, setTile, int32_t)
	ZCGUI_ACCEPT_PROP(cset, setCSet, int32_t)
	ZCGUI_ACCEPT_PROP(flip, setFlip, int32_t)
	ZCGUI_ACCEPT_PROP(showFlip, setShowFlip, bool)
	ZCGUI_ACCEPT_PROP(showvals, setShowVals, bool)
	ZCGUI_ACCEPT_PROP(mini, setIsMini, bool)
	ZCGUI_ACCEPT_PROP(showT0, setShowT0, bool)
	ZCGUI_ACCEPT_PROP(minicorner, setMiniCrn, int32_t)
	ZCGUI_ACCEPT_PROP(tilewid, setTileWid, int32_t)
	ZCGUI_ACCEPT_PROP(tilehei, setTileHei, int32_t)
	ZCGUI_ACCEPT_PROP(deftile, setDefTile, int32_t)
	ZCGUI_ACCEPT_PROP(defcs, setDefCS, int32_t)
	ZCGUI_ACCEPT_PROP(minionly, setMiniOnly, bool)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(onSelectFunc, setOnSelectFunc, GUI::function<void(int32_t,int32_t,int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(SelTileSwatch, SelTileSwatch, makeSelTileSwatch)

ZCGUI_BUILDER_START(SelComboSwatch)
	ZCGUI_ACCEPT_PROP(combo, setCombo, int32_t)
	ZCGUI_ACCEPT_PROP(cset, setCSet, int32_t)
	ZCGUI_ACCEPT_PROP(showvals, setShowVals, bool)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(onSelectFunc, setOnSelectFunc, GUI::function<void(int32_t,int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(SelComboSwatch, SelComboSwatch, makeSelComboSwatch)

ZCGUI_BUILDER_START(CornerSwatch)
	ZCGUI_ACCEPT_PROP(val, setVal, int32_t)
	ZCGUI_ACCEPT_PROP(color, setColor, int32_t)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(onSelectFunc, setOnSelectFunc, GUI::function<void(int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(CornerSwatch, CornerSwatch, makeCornerSwatch)

ZCGUI_BUILDER_START(SelHeroTileSwatch)
	ZCGUI_ACCEPT_PROP(heroSprite, setHeroSprite, int32_t)
	ZCGUI_ACCEPT_PROP(dir, setDir, int32_t)
	ZCGUI_ACCEPT_PROP(frames, setFrames, int32_t)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(SelHeroTileSwatch, SelHeroTileSwatch, makeSelHeroTileSwatch)

ZCGUI_BUILDER_START(TileFrame)
	ZCGUI_ACCEPT_PROP(tile, setTile, int32_t)
	ZCGUI_ACCEPT_PROP(cset, setCSet, int32_t)
	ZCGUI_ACCEPT_PROP(cset2, setCSet2, int32_t)
	ZCGUI_ACCEPT_PROP(speed, setSpeed, int32_t)
	ZCGUI_ACCEPT_PROP(frames, setFrames, int32_t)
	ZCGUI_ACCEPT_PROP(delay, setDelay, int32_t)
	ZCGUI_ACCEPT_PROP(skipx, setSkipX, int32_t)
	ZCGUI_ACCEPT_PROP(skipy, setSkipY, int32_t)
	ZCGUI_ACCEPT_PROP(tilewid, setTileW, int32_t)
	ZCGUI_ACCEPT_PROP(tilehei, setTileH, int32_t)
	ZCGUI_ACCEPT_PROP(do_sized, setDoSized, bool)
	ZCGUI_ACCEPT_PROP(flip, setFlip, int32_t)
	ZCGUI_ACCEPT_PROP(flashcs, setFlashCS, int32_t)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(TileFrame, TileFrame, makeTileFrame)

ZCGUI_BUILDER_START(PaletteFrame)
	ZCGUI_ACCEPT_PROP(bitmap, setBitmap, BITMAP*)
	ZCGUI_ACCEPT_PROP(cdata, setColorData, byte*)
	ZCGUI_ACCEPT_PROP(palette, setPal, PALETTE*)
	ZCGUI_ACCEPT_PROP(count, setCount, uint8_t)
	ZCGUI_ACCEPT_PROP(onUpdate, setOnUpdate, GUI::function<void()>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(PaletteFrame, PaletteFrame, makePaletteFrame)

ZCGUI_BUILDER_START(MsgPreview)
	ZCGUI_ACCEPT_PROP(data, setData, MsgStr const*)
	ZCGUI_ACCEPT_PROP(indx, setIndex, int32_t)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(MsgPreview, MsgPreview, makeMsgPreview)

ZCGUI_BUILDER_START(MiscCSetSel)
	ZCGUI_ACCEPT_PROP(c1, setC1, int32_t)
	ZCGUI_ACCEPT_PROP(c2, setC2, int32_t)
	ZCGUI_ACCEPT_PROP(onUpdate, setOnUpdate, GUI::function<void(int32_t,int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(MiscCSetSel, MiscCSetSel, makeMiscCSetSel)

ZCGUI_BUILDER_START(MiscColorSel)
	ZCGUI_ACCEPT_PROP(c1, setC1, int32_t)
	ZCGUI_ACCEPT_PROP(c2, setC2, int32_t)
	ZCGUI_ACCEPT_PROP(onUpdate, setOnUpdate, GUI::function<void(int32_t,int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(MiscColorSel, MiscColorSel, makeMiscColorSel)

ZCGUI_BUILDER_START(MiscColorRow)
	ZCGUI_ACCEPT_PROP(system, setSys, bool)
	ZCGUI_ACCEPT_PROP(val, setVal, int32_t)
	ZCGUI_ACCEPT_PROP(cset, setCS, int32_t)
	ZCGUI_ACCEPT_PROP(onUpdate, setOnUpdate, GUI::function<void(int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(MiscColorRow, MiscColorRow, makeMiscColorRow)

ZCGUI_BUILDER_START(DMapMinimap)
	ZCGUI_ACCEPT_PROP(curMap, setCurMap, int32_t)
	ZCGUI_ACCEPT_PROP(smallDMap, setSmallDMap, bool)
	ZCGUI_ACCEPT_PROP(offset, setOffset, int32_t)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DMapMinimap, DMapMinimap, makeDMapMinimap)

ZCGUI_BUILDER_START(QuestListView)
	ZCGUI_ACCEPT_PROP(onSelectionChanged, onSelectionChanged, Dialog::message)
	ZCGUI_ACCEPT_PROP(onDClick, onDClick, Dialog::message)
	ZCGUI_ACCEPT_PROP(onSelectFunc, setOnSelectFunc, GUI::function<void(int32_t)>)
	ZCGUI_ACCEPT_PROP(onDClickFunc, setOnDClickFunc, GUI::function<void(int32_t)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(QuestListView, QuestListView, makeQuestListView)

ZCGUI_BUILDER_START(DMapMapGrid)
	ZCGUI_ACCEPT_PROP(mapGridPtr, setMapGridPtr, byte*)
	ZCGUI_ACCEPT_PROP(continueScreen, setContinueScreen, int32_t)
	ZCGUI_ACCEPT_PROP(compassScreen, setCompassScreen, bool)
	ZCGUI_ACCEPT_PROP(smallDMap, setSmallDMap, bool)
	ZCGUI_ACCEPT_PROP(onUpdate, setOnUpdate, GUI::function<void(byte*, byte, byte)>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DMapMapGrid, DMapMapGrid, makeDMapMapGrid)

ZCGUI_BUILDER_START(DMapFrame)
	ZCGUI_ACCEPT_PROP(Dmap, setDMap, int32_t)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(DMapFrame, DMapFrame, makeDMapFrame)

ZCGUI_BUILDER_START(RegionGrid)
	ZCGUI_ACCEPT_PROP(localRegionsData, setLocalRegionsData, regions_data*)
	ZCGUI_ACCEPT_PROP(onUpdate, setOnUpdate, GUI::function<void()>)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(RegionGrid, RegionGrid, makeRegionGrid)

ZCGUI_BUILDER_START(Engraving)
	ZCGUI_ACCEPT_PROP(data, setSlot, int32_t)
ZCGUI_BUILDER_END()
ZCGUI_BUILDER_FUNCTION(Engraving, Engraving, makeEngraving)

} // namespace GUI::Builder

#endif
