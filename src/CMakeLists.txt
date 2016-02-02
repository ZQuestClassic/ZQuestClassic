# Keep the module path local for easier grabbing.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND ZC_ZC_FILES_SRC
	"aglogo.cpp"
	"colors.cpp"
	"debug.cpp"
	"decorations.cpp"
	"defdata.cpp"
	"editbox.cpp"
	"EditboxModel.cpp"
	"EditboxView.cpp"
	"ending.cpp"
	"ffscript.cpp"
	"gamedata.cpp"
	"gui.cpp"
	"guys.cpp"
	"init.cpp"
	"items.cpp"
	"jwin.cpp"
	"jwinfsel.cpp"
	"link.cpp"
	"maps.cpp"
	"matrix.cpp"
	"md5.cpp"
	"messageManager.cpp"
	"messageRenderer.cpp"
	"messageStream.cpp"
	"midi.cpp"
	"pal.cpp"
	"particles.cpp"
	"qst.cpp"
	"script_drawing.cpp"
	"sprite.cpp"
	"subscr.cpp"
	"tab_ctl.cpp"
	"tiles.cpp"
	"title.cpp"
	"weapons.cpp"
	"win32.cpp" # Definitely windows exclusive.
	"zc_custom.cpp"
	"zc_init.cpp"
	"zc_items.cpp"
	"zc_sprite.cpp"
	"zc_subscr.cpp"
	"zc_sys.cpp"
	"zelda.cpp"
	"zscriptversion.cpp"
	"zsys.cpp"
)

source_group("Source Files" FILES ${ZC_ZC_FILES_SRC})

list(APPEND ZC_ZC_FILES_HPP
	"aglogo.h"
	"colors.h"
	"debug.h"
	"decorations.h"
	"defdata.h"
	"editbox.h"
	"ending.h"
	"font.h"
	"fontsdat.h"
	"gamedata.h"
	"gfxpal.h"
	"gui.h"
	"guys.h"
	"init.h"
	"items.h"
	"jwin.h"
	"jwinfsel.h"
	"link.h"
	"load_gif.h"
	"maps.h"
	"matrix.h"
	"md5.h"
	"mem_debug.h"
	"messageManager.h"
	"messageRenderer.h"
	"messageStream.h"
	"midi.h"
	"pal.h"
	"particles.h"
	"qst.h"
	"rendertarget.h"
	"save_gif.h"
	"script_drawing.h"
	"sfx.h"
	"sprite.h"
	"subscr.h"
	"tab_ctl.h"
	"tiles.h"
	"title.h"
	"vectorset.h"
	"weapons.h"
	"zc_alleg.h"
	"zc_array.h"
	"zc_custom.h"
	"zc_init.h"
	"zc_malloc.h"
	"zc_math.h"
	"zc_subscr.h"
	"zc_sys.h"
	"zconsole.h"
	"zdefs.h"
	"zelda.h"
	"zeldadat.h"
	"zq_class.h"
	"zq_cset.h"
	"zq_custom.h"
	"zq_doors.h"
	"zq_files.h"
	"zq_init.h"
	"zq_misc.h"
	"zq_subscr.h"
	"zq_tiles.h"
	"zquest.h"
	"zquestdat.h"
	"zsys.h"
)

source_group("Header Files" FILES ${ZC_ZC_FILES_HPP})

list(APPEND ZC_SEQUENCE_FILES_SRC
	"sequence/gameOver.cpp"
	"sequence/ganonIntro.cpp"
	"sequence/getBigTriforce.cpp"
	"sequence/getTriforce.cpp"
	"sequence/potion.cpp"
	"sequence/sequence.cpp"
	"sequence/whistle.cpp"
)

source_group("Source Files\\\\Sequence" FILES ${ZC_SEQUENCE_FILES_SRC})

list(APPEND ZC_SEQUENCE_FILES_HPP
	"sequence/gameOver.h"
	"sequence/ganonIntro.h"
	"sequence/getBigTriforce.h"
	"sequence/getTriforce.h"
	"sequence/potion.h"
	"sequence/sequence.h"
	"sequence/whistle.h"
)

source_group("Header Files\\\\Sequence" FILES ${ZC_SEQUENCE_FILES_HPP})

list(APPEND ZC_ITEM_FILES_SRC
	"item/clock.cpp"
	"item/dinsFire.cpp"
	"item/faroresWind.cpp"
	"item/hookshot.cpp"
	"item/itemEffect.cpp"
	"item/nayrusLove.cpp"
)

source_group("Source Files\\\\Item" FILES ${ZC_ITEM_FILES_SRC})

list(APPEND ZC_ITEM_FILES_HPP
	"item/clock.h"
	"item/dinsFire.h"
	"item/faroresWind.h"
	"item/hookshot.h"
	"item/itemAction.h"
	"item/itemEffect.h"
	"item/nayrusLove.h"
)

source_group("Header Files\\\\Item" FILES ${ZC_ITEM_FILES_HPP})

list(APPEND ZC_GUI_FILES_SRC
	"gui/alert.cpp"
	"gui/contents.cpp"
	"gui/controller.cpp"
	"gui/dialog.cpp"
	"gui/manager.cpp"
)

source_group("Source Files\\\\Gui" FILES ${ZC_GUI_FILES_SRC})

list(APPEND ZC_GUI_FILES_HPP
	"gui/alert.h"
	"gui/bitmap.h"
	"gui/button.h"
	"gui/checkbox.h"
	"gui/contents.h"
	"gui/controller.h"
	"gui/dialog.h"
	"gui/factory.h"
	"gui/key.h"
	"gui/list.h"
	"gui/manager.h"
	"gui/menu.h"
	"gui/menuItem.h"
	"gui/mouse.h"
	"gui/serialContainer.h"
	"gui/spinner.h"
	"gui/tabPanel.h"
	"gui/text.h"
	"gui/textBox.h"
	"gui/textField.h"
	"gui/widget.h"
	"gui/window.h"
)

source_group("Header Files\\\\Gui" FILES ${ZC_GUI_FILES_HPP})

list(APPEND ZC_GUI_ALLEGRO_FILES_SRC
	"gui/allegro/bitmap.cpp"
	"gui/allegro/button.cpp"
	"gui/allegro/checkbox.cpp"
	"gui/allegro/column.cpp"
	"gui/allegro/comboBox.cpp"
	"gui/allegro/common.cpp"
	"gui/allegro/controller.cpp"
	"gui/allegro/dummy.cpp"
	"gui/allegro/editableText.cpp"
	"gui/allegro/factory.cpp"
	"gui/allegro/frame.cpp"
	"gui/allegro/list.cpp"
	"gui/allegro/renderer.cpp"
	"gui/allegro/row.cpp"
	"gui/allegro/scrollbar.cpp"
	"gui/allegro/scrollingPane.cpp"
	"gui/allegro/serialContainer.cpp"
	"gui/allegro/standardWidget.cpp"
	"gui/allegro/tab.cpp"
	"gui/allegro/tabBar.cpp"
	"gui/allegro/tabPanel.cpp"
	"gui/allegro/text.cpp"
	"gui/allegro/textField.cpp"
	"gui/allegro/window.cpp"
)

source_group("Source Files\\\\Gui\\\\Allegro" FILES ${ZC_GUI_ALLEGRO_FILES_SRC})

list(APPEND ZC_GUI_ALLEGRO_FILES_HPP
	"gui/allegro/bitmap.h"
	"gui/allegro/button.h"
	"gui/allegro/checkbox.h"
	"gui/allegro/column.h"
	"gui/allegro/comboBox.h"
	"gui/allegro/common.h"
	"gui/allegro/controller.h"
	"gui/allegro/dummy.h"
	"gui/allegro/editableText.h"
	"gui/allegro/factory.h"
	"gui/allegro/frame.h"
	"gui/allegro/list.h"
	"gui/allegro/manager.h"
	"gui/allegro/renderer.h"
	"gui/allegro/row.h"
	"gui/allegro/scrollbar.h"
	"gui/allegro/scrollingPane.h"
	"gui/allegro/serialContainer.h"
	"gui/allegro/standardWidget.h"
	"gui/allegro/tab.h"
	"gui/allegro/tabBar.h"
	"gui/allegro/tabPanel.h"
	"gui/allegro/text.h"
	"gui/allegro/textField.h"
	"gui/allegro/widget.h"
	"gui/allegro/window.h"
)

source_group("Header Files\\\\Gui\\\\Allegro" FILES ${ZC_GUI_ALLEGRO_FILES_HPP})

list(APPEND ZC_ANGELSCRIPT_FILES_SRC
	"angelscript/aszc.cpp"
	"angelscript/scriptContext.cpp"
	"angelscript/scriptData.cpp"
	"angelscript/util.cpp"
)

source_group("Source Files\\\\AngelScript" FILES ${ZC_ANGELSCRIPT_FILES_SRC})

list(APPEND ZC_ANGELSCRIPT_FILES_HPP
	"angelscript/aszc.h"
	"angelscript/scriptContext.h"
	"angelscript/scriptData.h"
	"angelscript/util.h"
)

source_group("Header Files\\\\AngelScript" FILES ${ZC_ANGELSCRIPT_FILES_HPP})

list(APPEND ZC_ANGELSCRIPT_SCRIPT_FILES_SRC
	"angelscript/scriptarray/scriptarray.cpp"
	"angelscript/scriptbuilder/scriptbuilder.cpp"
	"angelscript/scriptmath/scriptmath.cpp"
	"angelscript/scriptstdstring/scriptstdstring.cpp"
	"angelscript/scriptstdstring/scriptstdstring_utils.cpp"
)

source_group("Source Files\\\\AngelScript\\\\Helpers" FILES ${ZC_ANGELSCRIPT_SCRIPT_FILES_SRC})

list(APPEND ZC_ANGELSCRIPT_SCRIPT_FILES_HPP
	"angelscript/scriptarray/scriptarray.h"
	"angelscript/scriptbuilder/scriptbuilder.h"
	"angelscript/scriptmath/scriptmath.h"
	"angelscript/scriptstdstring/scriptstdstring.h"
)

source_group("Header Files\\\\AngelScript\\\\Helpers" FILES ${ZC_ANGELSCRIPT_SCRIPT_FILES_HPP})

list(APPEND ZC_ANGELSCRIPT_MAIN_FILES_SRC
	"angelscript/angelscript/as_atomic.cpp"
	"angelscript/angelscript/as_builder.cpp"
	"angelscript/angelscript/as_bytecode.cpp"
	"angelscript/angelscript/as_callfunc.cpp"
	"angelscript/angelscript/as_callfunc_arm.cpp"
	"angelscript/angelscript/as_callfunc_mips.cpp"
	"angelscript/angelscript/as_callfunc_ppc.cpp"
	"angelscript/angelscript/as_callfunc_ppc_64.cpp"
	"angelscript/angelscript/as_callfunc_sh4.cpp"
	"angelscript/angelscript/as_callfunc_x64_gcc.cpp"
	"angelscript/angelscript/as_callfunc_x64_mingw.cpp"
	"angelscript/angelscript/as_callfunc_x64_msvc.cpp"
	"angelscript/angelscript/as_callfunc_x86.cpp"
	"angelscript/angelscript/as_callfunc_xenon.cpp"
	"angelscript/angelscript/as_compiler.cpp"
	"angelscript/angelscript/as_configgroup.cpp"
	"angelscript/angelscript/as_context.cpp"
	"angelscript/angelscript/as_datatype.cpp"
	"angelscript/angelscript/as_gc.cpp"
	"angelscript/angelscript/as_generic.cpp"
	"angelscript/angelscript/as_globalproperty.cpp"
	"angelscript/angelscript/as_memory.cpp"
	"angelscript/angelscript/as_module.cpp"
	"angelscript/angelscript/as_objecttype.cpp"
	"angelscript/angelscript/as_outputbuffer.cpp"
	"angelscript/angelscript/as_parser.cpp"
	"angelscript/angelscript/as_restore.cpp"
	"angelscript/angelscript/as_scriptcode.cpp"
	"angelscript/angelscript/as_scriptengine.cpp"
	"angelscript/angelscript/as_scriptfunction.cpp"
	"angelscript/angelscript/as_scriptnode.cpp"
	"angelscript/angelscript/as_scriptobject.cpp"
	"angelscript/angelscript/as_string.cpp"
	"angelscript/angelscript/as_string_util.cpp"
	"angelscript/angelscript/as_thread.cpp"
	"angelscript/angelscript/as_tokenizer.cpp"
	"angelscript/angelscript/as_typeinfo.cpp"
	"angelscript/angelscript/as_variablescope.cpp"
)

source_group("Source Files\\\\AngelScript\\\\Main" FILES ${ZC_ANGELSCRIPT_MAIN_FILES_SRC})

list(APPEND ZCDATA_ALL_FILES_SRC
	${ZC_ZC_FILES_SRC}
	${ZC_SEQUENCE_FILES_SRC}
	${ZC_ITEM_FILES_SRC}
	${ZC_GUI_FILES_SRC}
	${ZC_GUI_ALLEGRO_FILES_SRC}
	${ZC_ANGELSCRIPT_FILES_SRC}
	${ZC_ANGELSCRIPT_SCRIPT_FILES_SRC}
	${ZC_ANGELSCRIPT_MAIN_FILES_SRC}
)

list(APPEND ZCDATA_ALL_FILES_HPP
	${ZC_ZC_FILES_HPP}
	${ZC_SEQUENCE_FILES_HPP}
	${ZC_ITEM_FILES_HPP}
	${ZC_GUI_FILES_HPP}
	${ZC_GUI_ALLEGRO_FILES_HPP}
	${ZC_ANGELSCRIPT_FILES_HPP}
	${ZC_ANGELSCRIPT_SCRIPT_FILES_HPP}
)

add_executable("ZeldaClassic" ${ZCDATA_ALL_FILES_SRC} ${ZCDATA_ALL_FILES_HPP})

set_property(TARGET "ZeldaClassic" PROPERTY FOLDER "Executables")

if (WIN32)
	
endif()

list(APPEND ZC_ALL_INCLUDE_DIRS
	"${Boost_INCLUDE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${ZC_SRC_DIR}"
	"${ZC_INCLUDE_DIR}/almp3"
	"${ZC_INCLUDE_DIR}/alogg"
	"${ZC_INCLUDE_DIR}/dumb"
	"${ZC_INCLUDE_DIR}/gme"
	"${ZC_INCLUDE_DIR}/jpgalleg-2.5"
	"${ZC_INCLUDE_DIR}/loadpng"
	"${ZC_INCLUDE_DIR}/lpng1212"
	"${ZC_INCLUDE_DIR}/pthread"
	"${ZC_INCLUDE_DIR}/zlib123"
	"${ZC_INCLUDE_DIR}"
	"${ZC_ALLEGRO_DIR}"
)

target_include_directories("ZeldaClassic" PUBLIC ${ZC_ALL_INCLUDE_DIRS})

