cmake_minimum_required(VERSION 3.5)
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)
project (ZeldaClassic)

#############################################################
# Global settings for the different platforms
#############################################################

if(UNIX AND NOT APPLE)
        set(LINUX TRUE)
endif()

if(WIN32)	
	if(MSVC)
		set(ALLEGROLIB "$<$<CONFIG:DEBUG>:${CMAKE_SOURCE_DIR}/libs/win32dbg/alld_crt.lib>" 
					   "$<$<CONFIG:RELEASE>:${CMAKE_SOURCE_DIR}/libs/win32/alleg_crt.lib>")
		
		add_definitions(-DALLEGRO_MSVC)
		
		set(IMAGELIBS "$<$<CONFIG:DEBUG>:${CMAKE_SOURCE_DIR}/libs/win32dbg/libpng.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/zlibd.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/loadpng.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/libjpgal.lib>"
		              "$<$<CONFIG:RELEASE>:${CMAKE_SOURCE_DIR}/libs/win32/libpng.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/zlib.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/loadpng.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/libjpgal.lib>")
		
		set(SOUNDLIBS "$<$<CONFIG:DEBUG>:${CMAKE_SOURCE_DIR}/libs/win32dbg/aldmd.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/almp3.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/alogg.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/dumbd.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32dbg/gme.lib>"
		              "$<$<CONFIG:RELEASE>:${CMAKE_SOURCE_DIR}/libs/win32/aldmb.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/almp3.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/alogg.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/dumb.lib$<SEMICOLON>${CMAKE_SOURCE_DIR}/libs/win32/gme.lib>")

		if(MSVC_VERSION GREATER 1500) 
			add_definitions(-DALLEGRO_HAVE_STDINT_H)
		endif()
		
		add_compile_options("$<$<CONFIG:DEBUG>:/MTd>")
		add_compile_options("$<$<CONFIG:RELEASE>:/MT>")
		
		# REQUIRED or else MSVC throws compiler error(!)
		add_compile_options("$<$<CONFIG:RELEASE>:/fp:fast>")
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCD /SUBSYSTEM:WINDOWS /DEBUG")
	endif()
elseif(LINUX)
	link_directories(/usr/lib/i386-linux-gnu)
	link_directories(./libs/linux)
	set(ALLEGROLIB alleg)
	add_definitions(-DALLEGRO_UNIX)
	execute_process(COMMAND "${CMAKE_SOURCE_DIR}/allegro/configure --enable-static=yes --enable-shared=no --enable-ossdigi=no --enable-ossmidi=no --enable-esddigi=no --enable-artsdigi=no --enable-sgialdigi=no --enable-jackdigi=no --enable-xwin-dga2=no --enable-vga=no --enable-svgalib=no" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/allegro")
	
	set(IMAGELIBS ldpng jpgal png z)
	set(SOUNDLIBS aldmb almp3 alogg dumb gme asound)
	set(X11_LIBRARIES X11 Xext Xcursor Xxf86vm pthread Xpm)
	
	add_compile_options("-m32")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
endif()

#############################################################
# Header and source file lists
#############################################################

include_directories(./allegro/include)
include_directories(./include/lpng1212)
include_directories(./include/zlib123)
include_directories(./include/loadpng)
include_directories(./include/jpgalleg-2.5)
include_directories(./include/dumb)
include_directories(./include/alogg)
include_directories(./include/almp3)
include_directories(./include/gme)

set(ZCSOUNDSOURCES
	src/zcmusic.cpp 
	src/zcmusicd.cpp
)

set(ROMVIEWSOURCES
	src/editbox.cpp 
	src/EditboxModel.cpp 
	src/EditboxView.cpp 
	src/gui.cpp src/jwin.cpp 
	src/jwinfsel.cpp 
	src/load_gif.cpp 
	src/romview.cpp 
	src/save_gif.cpp 
	src/tab_ctl.cpp 
	src/zqscale.cpp 
	src/zsys.cpp
)

set(ZELDASOURCES
	src/aglogo.cpp
	src/colors.cpp
	src/debug.cpp
	src/decorations.cpp
	src/defdata.cpp
	src/editbox.cpp 
	src/EditboxModel.cpp 
	src/EditboxView.cpp 
	src/ending.cpp 
	src/ffscript.cpp 
	src/gamedata.cpp 
	src/gui.cpp 
	src/guys.cpp 
	src/init.cpp 
	src/items.cpp 
	src/jwin.cpp 
	src/jwinfsel.cpp 
	src/link.cpp 
	src/load_gif.cpp 
	src/maps.cpp 
	src/matrix.cpp 
	src/md5.cpp 
	src/midi.cpp 
	src/pal.cpp 
	src/particles.cpp 
	src/qst.cpp 
	src/save_gif.cpp 
	src/script_drawing.cpp 
	src/sprite.cpp 
	src/subscr.cpp 
	src/tab_ctl.cpp 
	src/tiles.cpp 
	src/title.cpp 
	src/weapons.cpp 
	src/win32.cpp 
	src/zc_custom.cpp 
	src/zc_init.cpp 
	src/zc_items.cpp 
	src/zc_sprite.cpp 
	src/zc_subscr.cpp 
	src/zc_sys.cpp 
	src/zelda.cpp 
	src/zscriptversion.cpp 
	src/zsys.cpp
)

set(ZQUESTSOURCES
	src/zquest.cpp 
	src/colors.cpp 
	src/defdata.cpp 
	src/editbox.cpp 
	src/EditboxModel.cpp 
	src/EditboxView.cpp 
	src/gamedata.cpp 
	src/gui.cpp 
	src/init.cpp 
	src/items.cpp 
	src/jwin.cpp 
	src/jwinfsel.cpp 
	src/load_gif.cpp 
	src/md5.cpp 
	src/midi.cpp 
	src/particles.cpp 
	src/qst.cpp 
	src/questReport.cpp 
	src/save_gif.cpp 
	src/sprite.cpp 
	src/subscr.cpp 
	src/tab_ctl.cpp 
	src/tiles.cpp 
	src/win32.cpp 
	src/zc_custom.cpp 
	src/zq_class.cpp 
	src/zq_cset.cpp 
	src/zq_custom.cpp 
	src/zq_doors.cpp 
	src/zq_files.cpp 
	src/zq_items.cpp 
	src/zq_init.cpp 
	src/zq_misc.cpp 
	src/zq_rules.cpp 
	src/zq_sprite.cpp 
	src/zq_strings.cpp 
	src/zq_subscr.cpp 
	src/zq_tiles.cpp 
	src/zqscale.cpp 
	src/zsys.cpp 
	src/ffasm.cpp 	
)

#############################################################
# Build the ZScript parser
#############################################################

find_package(BISON QUIET)
find_package(FLEX QUIET)

if(${BISON_FOUND} AND ${FLEX_FOUND})
	if(MSVC)
		set(FLEXFLAGS "--wincompat")
	endif()
	FLEX_TARGET(ZScriptLexer ${CMAKE_SOURCE_DIR}/src/parser/ffscript.lpp ${CMAKE_SOURCE_DIR}/src/parser/lex.yy.cpp COMPILE_FLAGS ${FLEXFLAGS})
	BISON_TARGET(ZScriptParser ${CMAKE_SOURCE_DIR}/src/parser/ffscript.ypp ${CMAKE_SOURCE_DIR}/src/parser/y.tab.cpp)
	add_flex_bison_dependency(ZScriptLexer ZScriptParser)	
	set(ZSCRIPTSOURCES
		src/parser/AST.cpp 
		src/parser/BuildVisitors.cpp 
		src/parser/ByteCode.cpp 
		src/parser/DataStructs.cpp 
		src/parser/GlobalSymbols.cpp 
		src/parser/ParseError.cpp 
		src/parser/ScriptParser.cpp 
		src/parser/SymbolVisitors.cpp 
		src/parser/TypeChecker.cpp 
		src/parser/UtilVisitors.cpp 
		${BISON_ZScriptParser_OUTPUTS} 
		${FLEX_ZScriptLexer_OUTPUTS}
	)
else()
	message(WARNING "Flex and Bison not found. ZQuest will be compiled without the ZScript parser.")
	set(ZSCRIPTSOURCES src/parser/NoScriptParser.cpp)
endif()

if(WIN32)
	if(MSVC AND (MSVC_VERSION GREATER 1600))
		set(ZCSOUNDLIBSEXTRA legacy_stdio_definitions)
	endif()
endif()
add_library(zcsound SHARED ${ZCSOUNDSOURCES})
target_link_libraries(zcsound ${ALLEGROLIB} ${SOUNDLIBS} ${ZCSOUNDLIBSEXTRA})



if(WIN32)
	set(ROMVIEWEXTRASOURCES rv_icon.rc)
elseif(LINUX)
	set(ROMVIEWEXTRASOURCES rv_icon.c)
	set(ROMVIEWLIBSEXTRA ${X11_LIBRARIES})
endif()
add_executable(romview ${ROMVIEWSOURCES} ${ROMVIEWEXTRASOURCES})
target_link_libraries(romview ${ALLEGROLIB} ${IMAGELIBS} ${SOUNDLIBS} ${ROMVIEWLIBSEXTRA})


if(WIN32)
	set(ZELDAEXTRASOURCES zc_icon.rc)
elseif(LINUX)
	set(ZELDAEXTRASOURCES zc_icon.c src/single_instance_unix.cpp)
	set(ZELDALIBSEXTRA ${X11_LIBRARIES})
endif()
add_executable(zelda ${ZELDASOURCES} ${ZELDAEXTRASOURCES})
target_link_libraries(zelda zcsound ${ALLEGROLIB} ${IMAGELIBS} ${ZELDALIBSEXTRA})


if(WIN32)
	set(ZQUESTEXTRASOURCES zq_icon.rc)
elseif(LINUX)
	set(ZQUESTEXTRASOURCES zq_icon.c src/single_instance_unix.cpp)
	set(ZQUESTLIBSEXTRA ${X11_LIBRARIES})
endif()
add_executable(zquest ${ZQUESTSOURCES} ${ZSCRIPTSOURCES} ${ZQUESTEXTRASOURCES})
target_link_libraries(zquest zcsound ${ALLEGROLIB} ${IMAGELIBS} ${ZQUESTLIBSEXTRA})

if(MSVC)
	target_compile_definitions(zcsound PRIVATE ZCM_DLL)
	target_compile_definitions(zelda PRIVATE ZCM_DLL_IMPORT)
	target_compile_definitions(zquest PRIVATE ZCM_DLL_IMPORT)
endif()
