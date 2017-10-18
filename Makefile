-include makefile.inc

AUDIO_LIBS = -lgme -lalogg -lalmp3 -laldmb -ldumb
IMAGE_LIBS = -ljpgal -lldpng -lpng -lz
# -lSDLmain -lSDL
#LINKOPTS = -pg -g
#LINKOPTS = -pg
#OPTS = -pg -g
#OPTS = -pg
#OPTS = -O3
#OPTS = -DPTW32_STATIC_LIB -O3
#COMPRESS = 1

ifdef USE_ENCRYPTION
 OPTS = -DPTW32_STATIC_LIB -O3 -DUSE_ENCRYPTION
 ENCRYPTION_CPP = encryptionEnabled.cpp
else
 OPTS = -DPTW32_STATIC_LIB -O3
 ENCRYPTION_CPP = encryptionDisabled.cpp
endif

CFLAGBASE2 = -Wno-long-long -W -Wall -Wshadow -Wpointer-arith
#CFLAGBASE2 = -Wno-long-long -Wno-write-strings

ifndef COMPILE_FOR_MACOSX
 CFLAGBASE = $(CFLAGBASE2)
else
 CFLAGBASE = $(CFLAGBASE2)
endif

ifdef COMPILE_FOR_WIN
  SINGLE_QUOTE_HELPER = ^"
  REPO_LOCATION = _zcrepo
  ALLEG_LIB = -lalleg -lws2_32
  SFLAG = -s
  WINFLAG = -mwindows -static-libgcc -static-libstdc++
  PLATEXT = -w
  EXEEXT = .exe
  ZC_ICON = zc_icon.o
  ZC_ICON_DEPS = zc_icon.rc zc_icon.ico
  ZC_ICON_CMD = windres --use-temp-file -J rc -O coff -i zc_icon.rc -o zc_icon.o
  ZQ_ICON = zq_icon.o
  ZQ_ICON_DEPS = zq_icon.rc zq_icon.ico
  ZQ_ICON_CMD = windres --use-temp-file -J rc -O coff -i zq_icon.rc -o zq_icon.o
  RV_ICON = rv_icon.o
  RV_ICON_DEPS = rv_icon.rc rv_icon.ico
  RV_ICON_CMD = windres --use-temp-file -J rc -O coff -i rv_icon.rc -o rv_icon.o
  ZC_PLATFORM = Windows
  CC = g++
  CFLAG = $(CFLAGBASE) `pkg-config --cflags gtk+-3.0` -pedantic -I./include/dumb/ -I./include/alogg/ -I./include/almp3 -I./include/lpng1212/ -I./include/loadpng/ -I./include/lpng1212/ -I./include/jpgalleg-2.5/ -I./include/zlib123/ -I./gme -I.
  LIBDIR = -L./libs/mingw `pkg-config --libs gtk+-3.0`
  
  ZCSOUND_SO = libs/mingw/libzcsound.dll
  ZCSOUND_LIB = -Wl,-rpath,. -lzcsound
  ZCSOUND_LINKOPTS = $(LINKOPTS) -shared -Wl,-soname,zcsound.dll
  ZCSOUND_ALLEG_LIB = $(ALLEG_LIB)
else
ifdef COMPILE_FOR_LINUX
  PLATEXT = -l
  ALLEG_LIB = `allegro-config --libs --static`
  SFLAG = -s
  ZC_ICON = zc_icon.o
  ZC_ICON_DEPS = zc_icon.c
  ZC_ICON_CMD = $(CC) $(OPTS) $(CFLAG) -c zc_icon.c -o zc_icon.o $(SFLAG)
  ZQ_ICON = zq_icon.o
  ZQ_ICON_DEPS = zq_icon.c
  ZQ_ICON_CMD = $(CC) $(OPTS) $(CFLAG) -c zq_icon.c -o zq_icon.o $(SFLAG)
  RV_ICON = rv_icon.o
  RV_ICON_DEPS = rv_icon.c
  RV_ICON_CMD = $(CC) $(OPTS) $(CFLAG) -c rv_icon.c -o rv_icon.o $(SFLAG)
  ZC_PLATFORM = Linux
  CC = g++ 
  CFLAG = `pkg-config --cflags gtk+-3.0` -I./include -I../include -I./include/alogg -I./include/almp3 -I.
  LIBDIR = -L./libs/linux `pkg-config --libs gtk+-3.0`
  SINGLE_INSTANCE_O = single_instance.o
  
  ZCSOUND_SO = libs/linux/libzcsound.so
  ZCSOUND_LIB = -Wl,-rpath,. -lzcsound
  ZCSOUND_LINKOPTS = $(LINKOPTS) -shared -Wl,-soname,zcsound.so
else
ifdef COMPILE_FOR_DOS
  ALLEG_LIB = -lalleg
  STDCXX_LIB = -lstdcxx
  EXEEXT = .exe
  ZC_PLATFORM = DOS
  CC = gpp
  CFLAG = $(CFLAGBASE) -Werror
else
ifdef COMPILE_FOR_MACOSX
  ECHO_HELPER = \
  SINGLE_QUOTE_HELPER = \"
  PLATEXT = -m
  ALLEG_LIB = -framework Cocoa -framework Allegro -lalleg-main -arch i386 
  ZC_PLATFORM = Mac OS X
  CFLAG = $(CFLAGBASE) `pkg-config --cflags gtk+-3.0` -pedantic -arch i386 -I./include/dumb/ -I./include/alogg/ -I./include/almp3/ -I./include/libjpgal/ -I.
  CC = g++
  LIBDIR= -L./libs/osx `pkg-config --libs gtk+-3.0`
  DATA = output/common/
  #SINGLE_INSTANCE_O = single_instance.o
  
  ZCSOUND_SO = libs/osx/libzcsound.dylib
  ZCSOUND_LIB = -lzcsound
  ZCSOUND_LINKOPTS = $(LINKOPTS) -shared
  ZCSOUND_ALLEG_LIB = $(ALLEG_LIB)
else
ifdef COMPILE_FOR_MACOSX_UNIVERSAL
  ECHO_HELPER = \
  SINGLE_QUOTE_HELPER = \"
  PLATEXT = -mu
  ALLEG_LIB = -framework Cocoa -framework Allegro -lalleg-main
  ZC_PLATFORM = Mac OS X Universal
  CFLAG = `pkg-config --cflags gtk+-3.0` -pedantic -Wno-long-long -Wall -arch i386 -arch ppc -DMACOSX_
  CC = g++
  LIBDIR= -L./libs/osx `pkg-config --libs gtk+-3.0`
  DATA = output/common/
  LINKOPTS = -arch i386 -arch ppc
  #SINGLE_INSTANCE_O = single_instance.o
  
  ZCSOUND_SO = libs/osx/libzcsound.dylib
  ZCSOUND_LIB = -lzcsound
  ZCSOUND_LINKOPTS = $(LINKOPTS) -shared
  ZCSOUND_ALLEG_LIB = $(ALLEG_LIB)
else
ifdef COMPILE_FOR_MACOSX_SNOW_LEOPARD
  PLATEXT = -msl
  ALLEG_LIB = -framework Cocoa -framework Allegro -lalleg-main
  ZC_PLATFORM = Mac OS X Universal
  CFLAG = `pkg-config --cflags gtk+-3.0` -pedantic -Wno-long-long -Wall -arch i386 -arch ppc -DMACOSX_
  CC = g++
  LIBDIR= -L./libs/osx `pkg-config --libs gtk+-3.0`
  DATA = output/common/
  LINKOPTS = -arch i386 -arch ppc
  #SINGLE_INSTANCE_O = single_instance.o
  
  ZCSOUND_SO = libs/osx/libzcsound.dylib
  ZCSOUND_LIB = -lzcsound
  ZCSOUND_LINKOPTS = $(LINKOPTS) -shared
  ZCSOUND_ALLEG_LIB = $(ALLEG_LIB)
else
ifdef COMPILE_FOR_GP2X
  PLATEXT = -g
  EXEEXT = .gpe
  ALLEG_LIB = -lalleg -lpthread -static
  ZC_PLATFORM = GP2X
  CFLAG = $(CFLAGBASE) -Werror -I/devkitGP2X/include
  CC = arm-linux-g++
  AUDIO_LIBS = -L/devkitGP2X/lib -lalspc -lalogg -lalmp3 -laldmb -ldumb
  IMAGE_LIBS = -L/devkitGP2X/lib -ljpgal -lldpng -lpng -lz
  SINGLE_INSTANCE_O = single_instance.o
endif
endif
endif
endif
endif
endif
endif

ZELDA_PREFIX = zelda
ZQUEST_PREFIX = zquest
ROMVIEW_PREFIX = romview

ZELDA_EXE = $(ZELDA_PREFIX)$(PLATEXT)$(EXEEXT)
ZQUEST_EXE = $(ZQUEST_PREFIX)$(PLATEXT)$(EXEEXT)
ROMVIEW_EXE = $(ROMVIEW_PREFIX)$(PLATEXT)$(EXEEXT)

GTK_GUI_OBJECTS = gui/gtk/bitmap.o gui/gtk/button.o gui/gtk/buttonRow.o gui/gtk/checkbox.o gui/gtk/controller.o gui/gtk/factory.o gui/gtk/frame.o gui/gtk/manager.o gui/gtk/serialContainer.o gui/gtk/spinner.o gui/gtk/tabPanel.o gui/gtk/text.o gui/gtk/textBox.o gui/gtk/textField.o gui/gtk/util.o gui/gtk/widget.o gui/gtk/window.o

ALLEGRO_GUI_OBJECTS = gui/allegro/bitmap.o gui/allegro/button.o gui/allegro/checkbox.o gui/allegro/column.o gui/allegro/common.o gui/allegro/controller.o gui/allegro/dummy.o gui/allegro/editableText.o gui/allegro/factory.o gui/allegro/renderer.o gui/allegro/row.o gui/allegro/scrollbar.o gui/allegro/scrollingPane.o gui/allegro/serialContainer.o gui/allegro/standardWidget.o gui/allegro/tab.o gui/allegro/tabBar.o gui/allegro/tabPanel.o gui/allegro/text.o gui/allegro/textField.o gui/allegro/window.o

ZELDA_OBJECTS = aglogo.o colors.o debug.o decorations.o defdata.o editbox.o EditboxModel.o EditboxView.o encryption.o ending.o enemyAttack.o ffc.o ffscript.o fontClass.o gamedata.o gui.o guys.o init.o items.o jwin.o jwinfsel.o link.o linkHandler.o linkZScriptInterface.o load_gif.o maps.o matrix.o md5.o message.o messageManager.o messageRenderer.o messageStream.o midi.o pal.o particles.o qst.o refInfo.o room.o save_gif.o screenFreezeState.o screenWipe.o script_drawing.o sequence.o $(SINGLEINSTANCE_O) sfxAllegro.o sfxClass.o sfxManager.o sound.o sprite.o subscr.o tab_ctl.o tiles.o title.o weapons.o zc_custom.o zc_init.o zc_items.o zc_sprite.o zc_subscr.o zc_sys.o zelda.o zscriptversion.o zsys.o \
item/clock.o item/dinsFire.o item/hookshot.o item/faroresWind.o item/itemEffect.o item/nayrusLove.o \
sequence/gameOver.o sequence/ganonIntro.o sequence/getBigTriforce.o sequence/getTriforce.o sequence/potion.o sequence/whistle.o \
angelscript/aszc.o angelscript/scriptData.o angelscript/util.o angelscript/scriptarray/scriptarray.o angelscript/scriptbuilder/scriptbuilder.o angelscript/scriptmath/scriptmath.o angelscript/scriptstdstring/scriptstdstring.o \
tempStuff.o \
$(ZC_ICON)

ZQUEST_OBJECTS = zquest.o colors.o defdata.o dummyZQ.o editbox.o EditboxModel.o EditboxView.o encryption.o ffc.o gamedata.o gui.o init.o items.o jwin.o jwinfsel.o load_gif.o md5.o messageList.o midi.o particles.o qst.o questReport.o refInfo.o save_gif.o sprite.o subscr.o tab_ctl.o tiles.o zc_custom.o zq_class.o zq_cset.o zq_custom.o zq_doors.o zq_files.o zq_items.o zq_init.o zq_misc.o zq_sprite.o zq_strings.o zq_subscr.o zq_tiles.o zqscale.o zsys.o ffasm.o parser/AST.o parser/BuildVisitors.o parser/ByteCode.o parser/DataStructs.o parser/GlobalSymbols.o parser/lex.yy.o parser/ParseError.o parser/ScriptParser.o parser/SymbolVisitors.o parser/TypeChecker.o parser/UtilVisitors.o parser/y.tab.o \
guiBitmapRenderer.o \
gui/alert.o gui/contents.o gui/controller.o gui/dialog.o gui/manager.o \
$(ALLEGRO_GUI_OBJECTS) \
dialog/bitmap/tilePreview.o dialog/bitmap/tileSelector.o \
dialog/zquest/cheatEditor.o dialog/zquest/infoShopEditor.o dialog/zquest/paletteViewer.o dialog/zquest/questRules.o dialog/zquest/shopEditor.o dialog/zquest/simpleListSelector.o dialog/zquest/tileEditor.o dialog/zquest/tileSelector.o dialog/zquest/tileSelectorBackend.o dialog/zquest/zscriptEditor.o dialog/zquest/zscriptMain.o \
angelscript/scriptData.o \
$(ZQ_ICON)

ROMVIEW_OBJECTS = editbox.o EditboxModel.o EditboxView.o gui.o jwin.o jwinfsel.o load_gif.o romview.o save_gif.o tab_ctl.o zqscale.o zsys.o \
$(RV_ICON)

ZCSOUND_OBJECTS = zcmusic.o zcmusicd.o

.PHONY: default veryclean clean all msg dos win windows linux gp2x test done

default: all
msg:
	@echo Compiling Zelda Classic for $(ZC_PLATFORM)...
done:
	@echo Done!
clean:
	rm -f $(ZELDA_OBJECTS) $(ZQUEST_OBJECTS) $(ROMVIEW_OBJECTS) $(ZCSOUND_OBJECTS)
veryclean: clean
	rm -f $(ZELDA_EXE) $(ZQUEST_EXE) $(ROMVIEW_EXE) $(ZCSOUND_SO)

test:
ifndef COMPILE_FOR_WIN
ifndef COMPILE_FOR_DOS
ifndef COMPILE_FOR_LINUX
ifndef COMPILE_FOR_MACOSX
ifndef COMPILE_FOR_GP2X
ifndef COMPILE_FOR_MACOSX_UNIVERSAL
ifndef COMPILE_FOR_MACOSX_SNOW_LEOPARD
	#change this if you want to change the default platform
	@make win
endif
endif
endif
endif
endif
endif
endif


dos:
	@echo COMPILE_FOR_DOS=1 > makefile.inc
	@make
windows: win
win:
	@echo COMPILE_FOR_WIN=1 > makefile.inc
	@make
linux:
	@echo COMPILE_FOR_LINUX=1 > makefile.inc
	@make
macosx:
	@echo COMPILE_FOR_MACOSX=1 > makefile.inc
	@make
gp2x:
	@echo COMPILE_FOR_GP2X=1 > makefile.inc
	@make
macosx-u:
	@echo COMPILE_FOR_MACOSX_UNIVERSAL=1 > makefile.inc
	@make
macosx-sl:
	@echo COMPILE_FOR_MACOSX_SNOW_LEOPARD=1 > makefile.inc
	@make

all: test msg $(ZCSOUND_SO) $(ZELDA_EXE) $(ZQUEST_EXE) $(ROMVIEW_EXE) done

$(ZCSOUND_SO): $(ZCSOUND_OBJECTS)
	$(CC) $(ZCSOUND_LINKOPTS) -o $(ZCSOUND_SO) $(ZCSOUND_OBJECTS) $(LIBDIR) $(AUDIO_LIBS) $(ZCSOUND_ALLEG_LIB) $(SFLAG) $(WINFLAG)
zcmusic.o: zcmusic.cpp mutex.h zc_alleg.h zcmusic.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -fpic -c zcmusic.cpp -o zcmusic.o $(SFLAG) $(WINFLAG)
zcmusicd.o: zcmusicd.cpp zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -fpic -c zcmusicd.cpp -o zcmusicd.o $(SFLAG) $(WINFLAG)

$(ZELDA_EXE): $(ZELDA_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ZELDA_EXE) $(ZELDA_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(ZCSOUND_LIB) $(ALLEG_LIB) $(STDCXX_LIB) $(ZC_ICON) $(SFLAG) $(WINFLAG)
ifdef COMPRESS
	upx --best $(ZELDA_EXE)
endif
ifdef 0
	rm zc/zelda-m
	cp zelda-m zc/zelda-m
	zc/zelda-m
endif
ifdef COMPILE_FOR_MACOSX
	rm -rf "Zelda Classic.app"
	fixbundle $(ZELDA_EXE) -e
	cp Info1.plist $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>Zelda Classic</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>zc_icon.icns</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '  <key>CFBundleIdentifier</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '  <string>com.armageddon.Zelda Classic</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	cat $(ZELDA_EXE).app/Contents/tempinfo Info2.plist > $(ZELDA_EXE).app/Contents/Info.plist
	rm $(ZELDA_EXE).app/Contents/tempinfo
	cp "zc_icon.icns" $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)zelda.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)sfx.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)fonts.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)qst.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)zelda.nsf $(ZELDA_EXE).app/Contents/Resources/
	cp "ag_nofull.cfg" $(ZELDA_EXE).app/Contents/Resources/ag.cfg
	cp $(DATA)1st.qst $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)2nd.qst $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)3rd.qst $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)4th.qst $(ZELDA_EXE).app/Contents/Resources/
	
	cp libs/osx/libzcsound.dylib $(ZELDA_EXE).app/Contents/Frameworks/
	install_name_tool -change libs/osx/libzcsound.dylib @executable_path/../Frameworks/libzcsound.dylib $(ZELDA_EXE).app/Contents/MacOS/$(ZELDA_EXE)
	
	mv $(ZELDA_EXE).app/Contents/MacOS/$(ZELDA_EXE) "$(ZELDA_EXE).app/Contents/MacOS/Zelda Classic"
	mv $(ZELDA_EXE).app "Zelda Classic.app"
endif
ifdef COMPILE_FOR_MACOSX_UNIVERSAL
	rm -rf "Zelda Classic.app"
	fixbundle $(ZELDA_EXE) -e
	cp Info1.plist $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>Zelda Classic</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>zc_icon.icns</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '  <key>CFBundleIdentifier</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '  <string>com.armageddon.Zelda Classic</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	cat $(ZELDA_EXE).app/Contents/tempinfo Info2.plist > $(ZELDA_EXE).app/Contents/Info.plist
	rm $(ZELDA_EXE).app/Contents/tempinfo
	cp "zc_icon.icns" $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)zelda.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)sfx.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)fonts.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)qst.dat $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)zelda.nsf $(ZELDA_EXE).app/Contents/Resources/
	cp "ag_nofull.cfg" $(ZELDA_EXE).app/Contents/Resources/ag.cfg
	cp $(DATA)1st.qst $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)2nd.qst $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)3rd.qst $(ZELDA_EXE).app/Contents/Resources/
	cp $(DATA)4th.qst $(ZELDA_EXE).app/Contents/Resources/

	cp libs/osx/libzcsound.dylib $(ZELDA_EXE).app/Contents/Frameworks/
	install_name_tool -change libs/osx/libzcsound.dylib @executable_path/../Frameworks/libzcsound.dylib $(ZELDA_EXE).app/Contents/MacOS/$(ZELDA_EXE)

	mv $(ZELDA_EXE).app/Contents/MacOS/$(ZELDA_EXE) "$(ZELDA_EXE).app/Contents/MacOS/Zelda Classic"
	mv $(ZELDA_EXE).app "Zelda Classic.app"
endif

$(ZQUEST_EXE): $(ZQUEST_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ZQUEST_EXE) $(ZQUEST_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(ZCSOUND_LIB) $(ALLEG_LIB) $(STDCXX_LIB) $(ZQ_ICON) $(SFLAG) $(WINFLAG)
ifdef COMPRESS
	upx --best $(ZQUEST_EXE)
endif
ifdef COMPILE_FOR_MACOSX
	rm -rf "ZQuest Editor.app"
	fixbundle $(ZQUEST_EXE) -e
	chmod 755 $(ZQUEST_EXE)
	cp Info1.plist $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<string>ZQuest Editor</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<string>zq_icon.icns</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '  <key>CFBundleIdentifier</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '  <string>com.armageddon.ZQuest Editor</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	cat $(ZQUEST_EXE).app/Contents/tempinfo Info2.plist > $(ZQUEST_EXE).app/Contents/Info.plist
	rm $(ZQUEST_EXE).app/Contents/tempinfo
	cp "zq_icon.icns" $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zquest.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)sfx.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)qst.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zelda.nsf $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)fonts.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zquest.txt $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zscript.txt $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zstrings.txt $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)std.zh $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)std_constants.zh $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)std_functions.zh $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)string.zh $(ZQUEST_EXE).app/Contents/Resources/
	
	cp libs/osx/libzcsound.dylib $(ZQUEST_EXE).app/Contents/Frameworks/
	install_name_tool -change libs/osx/libzcsound.dylib @executable_path/../Frameworks/libzcsound.dylib $(ZQUEST_EXE).app/Contents/MacOS/$(ZQUEST_EXE)
	
	mv $(ZQUEST_EXE).app/Contents/MacOS/$(ZQUEST_EXE) "$(ZQUEST_EXE).app/Contents/MacOS/ZQuest Editor"
	mv $(ZQUEST_EXE).app "ZQuest Editor.app"
endif
ifdef COMPILE_FOR_MACOSX_UNIVERSAL
	rm -rf "ZQuest Editor.app"
	fixbundle $(ZQUEST_EXE) -e
	chmod 755 $(ZQUEST_EXE)
	cp Info1.plist $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<string>ZQuest Editor</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<string>zq_icon.icns</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '  <key>CFBundleIdentifier</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '  <string>com.armageddon.ZQuest Editor</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	cat $(ZQUEST_EXE).app/Contents/tempinfo Info2.plist > $(ZQUEST_EXE).app/Contents/Info.plist
	rm $(ZQUEST_EXE).app/Contents/tempinfo
	cp "zq_icon.icns" $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zquest.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)sfx.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)qst.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zelda.nsf $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)fonts.dat $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zquest.txt $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zscript.txt $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)zstrings.txt $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)std.zh $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)std_constants.zh $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)std_functions.zh $(ZQUEST_EXE).app/Contents/Resources/
	cp $(DATA)string.zh $(ZQUEST_EXE).app/Contents/Resources/
	
	cp libs/osx/libzcsound.dylib $(ZQUEST_EXE).app/Contents/Frameworks/
	install_name_tool -change libs/osx/libzcsound.dylib @executable_path/../Frameworks/libzcsound.dylib $(ZQUEST_EXE).app/Contents/MacOS/$(ZQUEST_EXE)
	
	mv $(ZQUEST_EXE).app/Contents/MacOS/$(ZQUEST_EXE) "$(ZQUEST_EXE).app/Contents/MacOS/ZQuest Editor"
	mv $(ZQUEST_EXE).app "ZQuest Editor.app"
endif

$(ROMVIEW_EXE): $(ROMVIEW_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ROMVIEW_EXE) $(ROMVIEW_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(RV_ICON) $(SFLAG) $(WINFLAG)
ifdef COMPRESS
	upx --best $(ZQUEST_EXE)
endif
ifdef COMPILE_FOR_MACOSX
	rm -rf "ROM Viewer.app"
	fixbundle $(ROMVIEW_EXE) -e
	chmod 755 $(ROMVIEW_EXE)
	cp Info1.plist $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<string>ROM Viewer</string>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<string>rv_icon.icns</string>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '  <key>CFBundleIdentifier</key>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '  <string>com.armageddon.ROMView</string>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	cat $(ROMVIEW_EXE).app/Contents/tempinfo Info2.plist > $(ROMVIEW_EXE).app/Contents/Info.plist
	rm $(ROMVIEW_EXE).app/Contents/tempinfo
	cp "rv_icon.icns" $(ROMVIEW_EXE).app/Contents/Resources/
	cp $(DATA)fonts.dat $(ROMVIEW_EXE).app/
	mv $(ROMVIEW_EXE).app/Contents/MacOS/$(ROMVIEW_EXE) "$(ROMVIEW_EXE).app/Contents/MacOS/ROMView"
	mv $(ROMVIEW_EXE).app "ROM Viewer.app"
endif
ifdef COMPILE_FOR_MACOSX_UNIVERSAL
	rm -rf "ROM Viewer.app"
	fixbundle $(ROMVIEW_EXE) -e
	chmod 755 $(ROMVIEW_EXE)
	cp Info1.plist $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<string>ROM Viewer</string>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '	<string>rv_icon.icns</string>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '  <key>CFBundleIdentifier</key>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	echo '  <string>com.armageddon.ROMView</string>' >> $(ROMVIEW_EXE).app/Contents/tempinfo
	cat $(ROMVIEW_EXE).app/Contents/tempinfo Info2.plist > $(ROMVIEW_EXE).app/Contents/Info.plist
	rm $(ROMVIEW_EXE).app/Contents/tempinfo
	cp "rv_icon.icns" $(ROMVIEW_EXE).app/Contents/Resources/
	cp $(DATA)fonts.dat $(ROMVIEW_EXE).app/
	mv $(ROMVIEW_EXE).app/Contents/MacOS/$(ROMVIEW_EXE) "$(ROMVIEW_EXE).app/Contents/MacOS/ROMView"
	mv $(ROMVIEW_EXE).app "ROM Viewer.app"
endif


aglogo.o: aglogo.cpp gamedata.h zc_alleg.h zdefs.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c aglogo.cpp -o aglogo.o $(SFLAG) $(WINFLAG)
colors.o: colors.cpp colors.h gamedata.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c colors.cpp -o colors.o $(SFLAG) $(WINFLAG)
debug.o: debug.cpp zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c debug.cpp -o debug.o $(SFLAG) $(WINFLAG)
decorations.o: decorations.cpp decorations.h gamedata.h jwin.h maps.h sfx.h sprite.h tab_ctl.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c decorations.cpp -o decorations.o $(SFLAG) $(WINFLAG)
defdata.o: defdata.cpp defdata.h gamedata.h guys.h items.h sfx.h sprite.h weapons.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c defdata.cpp -o defdata.o $(SFLAG) $(WINFLAG)
dummyZQ.o: dummyZQ.cpp
	$(CC) $(OPTS) $(CFLAG) -c dummyZQ.cpp -o dummyZQ.o $(SFLAG) $(WINFLAG)
editbox.o: editbox.cpp EditboxNew.h jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c editbox.cpp -o editbox.o $(SFLAG) $(WINFLAG)
EditboxModel.o: EditboxModel.cpp editbox.h EditboxNew.h gamedata.h gui.h jwin.h tab_ctl.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxModel.cpp -o EditboxModel.o $(SFLAG) $(WINFLAG)
EditboxView.o: EditboxView.cpp EditboxNew.h jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxView.cpp -o EditboxView.o $(SFLAG) $(WINFLAG)
encryption.o: $(ENCRYPTION_CPP) encryption.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c $(ENCRYPTION_CPP) -o encryption.o $(SFLAG) $(WINFLAG)
ending.o: ending.cpp aglogo.h colors.h ending.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h init.h
	$(CC) $(OPTS) $(CFLAG) -c ending.cpp -o ending.o $(SFLAG) $(WINFLAG)
enemyAttack.o: enemyAttack.cpp enemyAttack.h guys.h link.h zdefs.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c enemyAttack.cpp -o enemyAttack.o $(SFLAG) $(WINFLAG)
ffasm.o: ffasm.cpp ffasm.h ffscript.h gamedata.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c ffasm.cpp -o ffasm.o $(SFLAG) $(WINFLAG)
ffc.o: ffc.cpp ffc.h refInfo.h types.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c ffc.cpp -o ffc.o $(SFLAG) $(WINFLAG)
ffscript.o: ffscript.cpp aglogo.h colors.h ffc.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_init.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h rendertarget.h
	$(CC) $(OPTS) $(CFLAG) -c ffscript.cpp -o ffscript.o $(SFLAG) $(WINFLAG)
font.o: /allegro/tools/datedit.h font.cpp font.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c font.cpp -o font.o $(SFLAG) $(WINFLAG)
fontClass.o: fontClass.cpp fontClass.h
	$(CC) $(OPTS) $(CFLAG) -c fontClass.cpp -o fontClass.o $(SFLAG) $(WINFLAG)
gamedata.o: gamedata.cpp gamedata.h items.h jwin.h sfx.h sprite.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c gamedata.cpp -o gamedata.o $(SFLAG) $(WINFLAG)
gui.o: gui.cpp colors.h debug.h gamedata.h gui.h items.h jwin.h jwinfsel.h midi.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c gui.cpp -o gui.o $(SFLAG) $(WINFLAG)
guys.o: guys.cpp aglogo.h colors.h defdata.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h room.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h angelscript/scriptData.h
	$(CC) $(OPTS) $(CFLAG) -c guys.cpp -o guys.o $(SFLAG) $(WINFLAG)
init.o: init.cpp gamedata.h gui.h init.h jwin.h sfx.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(CFLAG) -c init.cpp -o init.o $(SFLAG) $(WINFLAG)
items.o: items.cpp gamedata.h items.h jwin.h maps.h sfx.h sound.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c items.cpp -o items.o $(SFLAG) $(WINFLAG)
jmenu.o: jmenu.cpp gamedata.h jwin.h sfx.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c jmenu.cpp -o jmenu.o $(SFLAG) $(WINFLAG)
jwin.o: jwin.cpp editbox.h gamedata.h jwin.h tab_ctl.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c jwin.cpp -o jwin.o $(SFLAG) $(WINFLAG)
jwinfsel.o: jwinfsel.cpp jwin.h jwinfsel.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c jwinfsel.cpp -o jwinfsel.o $(SFLAG) $(WINFLAG)
link.o: link.cpp aglogo.h colors.h decorations.h ffc.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h screenWipe.h sfx.h sound.h sprite.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h item/itemEffect.h
	$(CC) $(OPTS) $(CFLAG) -c link.cpp -o link.o $(SFLAG) $(WINFLAG)
linkHandler.o: linkHandler.cpp linkHandler.h decorations.h guys.h link.h sequence.h sfxManager.h zc_sys.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c linkHandler.cpp -o linkHandler.o $(SFLAG) $(WINFLAG)
linkZScriptInterface.o: linkZScriptInterface.cpp linkZScriptInterface.h link.h zdefs.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c linkZScriptInterface.cpp -o linkZScriptInterface.o $(SFLAG) $(WINFLAG)
load_gif.o: load_gif.cpp load_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c load_gif.cpp -o load_gif.o $(SFLAG) $(WINFLAG)
maps.o: maps.cpp aglogo.h colors.h ffc.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h particles.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h rendertarget.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c maps.cpp -o maps.o $(SFLAG) $(WINFLAG)
matrix.o: matrix.cpp gamedata.h matrix.h zc_alleg.h zc_sys.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c matrix.cpp -o matrix.o $(SFLAG) $(WINFLAG)
md5.o: md5.cpp md5.h
	$(CC) $(OPTS) $(CFLAG) -c md5.cpp -o md5.o $(SFLAG) $(WINFLAG)
message.o: message.cpp message.h fontClass.h link.h messageManager.h messageStream.h pal.h sound.h zc_sys.h zdefs.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c message.cpp -o message.o $(SFLAG) $(WINFLAG)
messageManager.o: messageManager.cpp messageManager.h link.h message.h weapons.h zc_sys.h zdefs.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c messageManager.cpp -o messageManager.o $(SFLAG) $(WINFLAG)
messageRenderer.o: messageRenderer.cpp messageRenderer.h subscr.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c messageRenderer.cpp -o messageRenderer.o $(SFLAG) $(WINFLAG)
messageStream.o: messageStream.cpp messageStream.h
	$(CC) $(OPTS) $(CFLAG) -c messageStream.cpp -o messageStream.o $(SFLAG) $(WINFLAG)
midi.o: midi.cpp gamedata.h jwin.h midi.h tab_ctl.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c midi.cpp -o midi.o $(SFLAG) $(WINFLAG)
pal.o: pal.cpp aglogo.h colors.h gamedata.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c pal.cpp -o pal.o $(SFLAG) $(WINFLAG)
particles.o: particles.cpp gamedata.h particles.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c particles.cpp -o particles.o $(SFLAG) $(WINFLAG)
qst.o: qst.cpp colors.h defdata.h encryption.h ffc.h font.h gamedata.h guys.h items.h jwin.h jwinfsel.h md5.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c qst.cpp -o qst.o $(SFLAG) $(WINFLAG)
questReport.o: questReport.cpp questReport.h editbox.h EditboxNew.h gui.h jwin.h mem_debug.h tiles.h zc_alleg.h zdefs.h zsys.h zq_class.h zq_misc.h zquest.h
	$(CC) $(OPTS) $(CFLAG) -c questReport.cpp -o questReport.o $(SFLAG) $(WINFLAG)
refInfo.o: refInfo.cpp refInfo.h types.h
	$(CC) $(OPTS) $(CFLAG) -c refInfo.cpp -o refInfo.o $(SFLAG) $(WINFLAG)
romview.o: romview.cpp fontsdat.h gamedata.h jwin.h jwinfsel.h load_gif.h save_gif.h tab_ctl.h zc_alleg.h zdefs.h zqscale.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c romview.cpp -o romview.o $(SFLAG) $(WINFLAG)
room.o: room.cpp room.h gamedata.h guys.h items.h link.h maps.h messageManager.h sound.h zc_sys.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c room.cpp -o room.o $(SFLAG) $(WINFLAG)
rv_icon.o: $(RV_ICON_DEPS)
	$(RV_ICON_CMD)
save_gif.o: save_gif.cpp save_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c save_gif.cpp -o save_gif.o $(SFLAG) $(WINFLAG)
screenFreezeState.o: screenFreezeState.cpp screenFreezeState.h maps.h messageManager.h tiles.h zdefs.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c screenFreezeState.cpp -o screenFreezeState.o $(SFLAG) $(WINFLAG)
screenWipe.o: screenWipe.cpp screenWipe.h zc_alleg.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c screenWipe.cpp -o screenWipe.o $(SFLAG) $(WINFLAG)
script_drawing.o: script_drawing.cpp ffscript.h maps.h rendertarget.h script_drawing.h tiles.h zc_alleg.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c script_drawing.cpp -o script_drawing.o $(SFLAG) $(WINFLAG)
sequence.o: sequence.cpp sequence.h link.h sfx.h sfxManager.h sequence/sequence.h sequence/gameOver.h sequence/ganonIntro.h sequence/getBigTriforce.h sequence/getTriforce.h sequence/potion.h sequence/whistle.h
	$(CC) $(OPTS) $(CFLAG) -c sequence.cpp -o sequence.o $(SFLAG) $(WINFLAG)
sfxAllegro.o: sfxAllegro.cpp sfxAllegro.h sound.h
	$(CC) $(OPTS) $(CFLAG) -c sfxAllegro.cpp -o sfxAllegro.o $(SFLAG) $(WINFLAG)
sfxClass.o: sfxClass.cpp sfxClass.h sfxAllegro.h
	$(CC) $(OPTS) $(CFLAG) -c sfxClass.cpp -o sfxClass.o $(SFLAG) $(WINFLAG)
sfxManager.o: sfxManager.cpp sfxManager.h sfxAllegro.h sfxClass.h
	$(CC) $(OPTS) $(CFLAG) -c sfxManager.cpp -o sfxManager.o $(SFLAG) $(WINFLAG)
$(SINGLEINSTANCE_O): $(SINGLEINSTANCE_CPP) single_instance.h
	$(CC) $(OPTS) $(CFLAG) -c $(SINGLEINSTANCE_CPP) -o $(SINGLEINSTANCE_O) $(SFLAG) $(WINFLAG)
sound.o: sound.cpp sound.h zc_alleg.h zc_sys.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c sound.cpp -o sound.o $(SFLAG) $(WINFLAG)
sprite.o: sprite.cpp sprite.h entityPtr.h gamedata.h tiles.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c sprite.cpp -o sprite.o $(SFLAG) $(WINFLAG)
subscr.o: subscr.cpp aglogo.h colors.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c subscr.cpp -o subscr.o $(SFLAG) $(WINFLAG)
tab_ctl.o: tab_ctl.cpp tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c tab_ctl.cpp -o tab_ctl.o $(SFLAG) $(WINFLAG)
tiles.o: tiles.cpp gamedata.h jwin.h tab_ctl.h tiles.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c tiles.cpp -o tiles.o $(SFLAG) $(WINFLAG)
title.o: title.cpp aglogo.h colors.h encryption.h gamedata.h gui.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h title.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c title.cpp -o title.o $(SFLAG) $(WINFLAG)
weapons.o: weapons.cpp aglogo.h colors.h gamedata.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c weapons.cpp -o weapons.o $(SFLAG) $(WINFLAG)
zc_custom.o: zc_custom.cpp gamedata.h jwin.h sfx.h tab_ctl.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_custom.cpp -o zc_custom.o $(SFLAG) $(WINFLAG)
zc_icon.o: $(ZC_ICON_DEPS)
	$(ZC_ICON_CMD)
zc_init.o: zc_init.cpp aglogo.h colors.h gamedata.h gui.h init.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_init.h zcmusic.h zdefs.h zelda.h zeldadat.h zq_init.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_init.cpp -o zc_init.o $(SFLAG) $(WINFLAG)
zc_items.o: zc_items.cpp gamedata.h guys.h jwin.h maps.h sfx.h sprite.h tab_ctl.h weapons.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_items.cpp -o zc_items.o $(SFLAG) $(WINFLAG)
zc_sprite.o: zc_sprite.cpp gamedata.h jwin.h maps.h sfx.h sound.h sprite.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_sprite.cpp -o zc_sprite.o $(SFLAG) $(WINFLAG)
zc_subscr.o: zc_subscr.cpp aglogo.h colors.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_subscr.cpp -o zc_subscr.o $(SFLAG) $(WINFLAG)
zc_sys.o: zc_sys.cpp aglogo.h colors.h debug.h gamedata.h gui.h guys.h init.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h particles.h qst.h screenWipe.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_init.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_sys.cpp -o zc_sys.o $(SFLAG) $(WINFLAG)
zelda.o: zelda.cpp aglogo.h colors.h ending.h ffc.h ffscript.h fontsdat.h gamedata.h guys.h init.h items.h jwin.h jwinfsel.h link.h load_gif.h maps.h matrix.h pal.h particles.h qst.h save_gif.h sfx.h sound.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h rendertarget.h
	$(CC) $(OPTS) $(CFLAG) -c zelda.cpp -o zelda.o $(SFLAG) $(WINFLAG)
zq_class.o: zq_class.cpp colors.h encryption.h ffc.h gamedata.h gui.h items.h jwin.h jwinfsel.h maps.h md5.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zq_class.h zq_misc.h zq_subscr.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_class.cpp -o zq_class.o $(SFLAG) $(WINFLAG)
zq_cset.o: zq_cset.cpp colors.h gamedata.h gfxpal.h gui.h jwin.h jwinfsel.h midi.h pal.h sfx.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_cset.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_cset.cpp -o zq_cset.o $(SFLAG) $(WINFLAG)
zq_custom.o: zq_custom.cpp defdata.h ffasm.h ffscript.h gamedata.h gui.h guys.h init.h items.h jwin.h jwinfsel.h midi.h sfx.h sprite.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zq_custom.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_custom.cpp -o zq_custom.o $(SFLAG) $(WINFLAG)
zq_doors.o: zq_doors.cpp gamedata.h gui.h jwin.h jwinfsel.h midi.h sfx.h sprite.h tab_ctl.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zq_class.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_doors.cpp -o zq_doors.o $(SFLAG) $(WINFLAG)
zq_files.o: zq_files.cpp gamedata.h gui.h items.h jwin.h jwinfsel.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_custom.h zq_files.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_files.cpp -o zq_files.o $(SFLAG) $(WINFLAG)
zq_icon.o: $(ZQ_ICON_DEPS)
	$(ZQ_ICON_CMD)
zq_init.o: zq_init.cpp gamedata.h gui.h init.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_init.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_init.cpp -o zq_init.o $(SFLAG) $(WINFLAG)
zq_items.o: zq_items.cpp gamedata.h zc_alleg.h zdefs.h zq_class.h
	$(CC) $(OPTS) $(CFLAG) -c zq_items.cpp -o zq_items.o $(SFLAG) $(WINFLAG)
zq_misc.o: zq_misc.cpp colors.h gamedata.h items.h jwin.h jwinfsel.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_misc.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_misc.cpp -o zq_misc.o $(SFLAG) $(WINFLAG)
zq_sprite.o: zq_sprite.cpp gamedata.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c zq_sprite.cpp -o zq_sprite.o $(SFLAG) $(WINFLAG)
zq_strings.o: zq_strings.cpp zq_strings.h zquest.h
	$(CC) $(OPTS) $(CFLAG) -c zq_strings.cpp -o zq_strings.o $(SFLAG) $(WINFLAG)
zq_subscr.o: zq_subscr.cpp gamedata.h gui.h init.h items.h jwin.h jwinfsel.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_misc.h zq_subscr.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_subscr.cpp -o zq_subscr.o $(SFLAG) $(WINFLAG)
zq_tiles.o: zq_tiles.cpp colors.h gamedata.h gui.h items.h jwin.h jwinfsel.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zq_class.h zq_misc.h zq_tiles.h zqscale.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zq_tiles.cpp -o zq_tiles.o $(SFLAG) $(WINFLAG)
zqscale.o: zqscale.cpp
	$(CC) $(OPTS) $(CFLAG) -c zqscale.cpp -o zqscale.o $(SFLAG) $(WINFLAG)
zquest.o: zquest.cpp colors.h editbox.h EditboxNew.h ffasm.h ffc.h ffscript.h fontsdat.h gamedata.h gui.h items.h jwin.h jwinfsel.h load_gif.h midi.h parser/Compiler.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_cset.h zq_custom.h zq_doors.h zq_files.h zq_init.h zq_misc.h zq_subscr.h zq_tiles.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zquest.cpp -o zquest.o $(SFLAG) $(WINFLAG)
zscriptversion.o: zscriptversion.cpp zelda.h link.h
	$(CC) $(OPTS) $(CFLAG) -c zscriptversion.cpp -o zscriptversion.o $(SFLAG) $(WINFLAG)
zsys.o: zsys.cpp gamedata.h jwin.h tab_ctl.h zc_alleg.h zc_sys.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zsys.cpp -o zsys.o $(SFLAG) $(WINFLAG)

angelscript/aszc.o: angelscript/aszc.cpp angelscript/aszc.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/aszc.cpp -o angelscript/aszc.o $(SFLAG) $(WINFLAG)
angelscript/scriptData.o: angelscript/scriptData.cpp angelscript/scriptData.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/scriptData.cpp -o angelscript/scriptData.o $(SFLAG) $(WINFLAG)
angelscript/util.o: angelscript/util.cpp angelscript/util.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/util.cpp -o angelscript/util.o $(SFLAG) $(WINFLAG)
angelscript/scriptarray/scriptarray.o: angelscript/scriptarray/scriptarray.cpp angelscript/scriptarray/scriptarray.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/scriptarray/scriptarray.cpp -o angelscript/scriptarray/scriptarray.o $(SFLAG) $(WINFLAG)
angelscript/scriptbuilder/scriptbuilder.o: angelscript/scriptbuilder/scriptbuilder.cpp angelscript/scriptbuilder/scriptbuilder.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/scriptbuilder/scriptbuilder.cpp -o angelscript/scriptbuilder/scriptbuilder.o $(SFLAG) $(WINFLAG)
angelscript/scriptmath/scriptmath.o: angelscript/scriptmath/scriptmath.cpp angelscript/scriptmath/scriptmath.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/scriptmath/scriptmath.cpp -o angelscript/scriptmath/scriptmath.o $(SFLAG) $(WINFLAG)
angelscript/scriptstdstring/scriptstdstring.o: angelscript/scriptstdstring/scriptstdstring.cpp angelscript/scriptstdstring/scriptstdstring.h
	$(CC) $(OPTS) $(CFLAG) -c angelscript/scriptstdstring/scriptstdstring.cpp -o angelscript/scriptstdstring/scriptstdstring.o $(SFLAG) $(WINFLAG)

item/clock.o: item/clock.cpp item/clock.h link.h
	$(CC) $(OPTS) $(CFLAG) -c item/clock.cpp -o item/clock.o $(SFLAG) $(WINFLAG)
item/dinsFire.o: item/dinsFire.cpp item/dinsFire.h entityPtr.h link.h zc_sys.h zdefs.h zelda.h item/itemAction.h
	$(CC) $(OPTS) $(CFLAG) -c item/dinsFire.cpp -o item/dinsFire.o $(SFLAG) $(WINFLAG)
item/faroresWind.o: item/faroresWind.cpp item/faroresWind.h link.h particles.h zc_sys.h zelda.h item/itemAction.h
	$(CC) $(OPTS) $(CFLAG) -c item/faroresWind.cpp -o item/faroresWind.o $(SFLAG) $(WINFLAG)
item/hookshot.o: item/hookshot.cpp item/hookshot.h link.h zelda.h item/itemAction.h
	$(CC) $(OPTS) $(CFLAG) -c item/hookshot.cpp -o item/hookshot.o $(SFLAG) $(WINFLAG)
item/itemEffect.o: item/itemEffect.cpp item/itemEffect.h debug.h items.h sfxManager.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c item/itemEffect.cpp -o item/itemEffect.o $(SFLAG) $(WINFLAG)
item/nayrusLove.o: item/nayrusLove.cpp item/nayrusLove.h decorations.h entityPtr.h link.h sfxClass.h sound.h zc_sys.h zdefs.h zelda.h item/itemAction.h item/itemEffect.h
	$(CC) $(OPTS) $(CFLAG) -c item/nayrusLove.cpp -o item/nayrusLove.o $(SFLAG) $(WINFLAG)

parser/AST.o: parser/AST.cpp parser/AST.h parser/Compiler.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/AST.cpp -o parser/AST.o $(SFLAG) $(WINFLAG)
parser/BuildVisitors.o: parser/BuildVisitors.cpp zsyssimple.h parser/AST.h parser/BuildVisitors.h parser/ByteCode.h parser/Compiler.h parser/DataStructs.h parser/ParseError.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/BuildVisitors.cpp -o parser/BuildVisitors.o $(SFLAG) $(WINFLAG)
parser/ByteCode.o: parser/ByteCode.cpp zsyssimple.h parser/AST.h parser/ByteCode.h parser/Compiler.h parser/DataStructs.h parser/ParseError.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/ByteCode.cpp -o parser/ByteCode.o $(SFLAG) $(WINFLAG)
parser/DataStructs.o: parser/DataStructs.cpp zsyssimple.h parser/AST.h parser/Compiler.h parser/DataStructs.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/DataStructs.cpp -o parser/DataStructs.o $(SFLAG) $(WINFLAG)
parser/GlobalSymbols.o: parser/GlobalSymbols.cpp zsyssimple.h parser/AST.h parser/ByteCode.h parser/Compiler.h parser/DataStructs.h parser/GlobalSymbols.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/GlobalSymbols.cpp -o parser/GlobalSymbols.o $(SFLAG) $(WINFLAG)
parser/lex.yy.o: parser/lex.yy.cpp zsyssimple.h parser/AST.h parser/Compiler.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/lex.yy.cpp -o parser/lex.yy.o $(SFLAG) $(WINFLAG)
parser/ParseError.o: parser/ParseError.cpp zsyssimple.h parser/AST.h parser/Compiler.h parser/ParseError.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/ParseError.cpp -o parser/ParseError.o $(SFLAG) $(WINFLAG)
parser/ScriptParser.o: parser/ScriptParser.cpp zsyssimple.h parser/AST.h parser/BuildVisitors.h parser/ByteCode.h parser/Compiler.h parser/DataStructs.h parser/GlobalSymbols.h parser/ParseError.h parser/SymbolVisitors.h parser/TypeChecker.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/ScriptParser.cpp -o parser/ScriptParser.o $(SFLAG) $(WINFLAG)
parser/SymbolVisitors.o: parser/SymbolVisitors.cpp parser/AST.h parser/Compiler.h parser/DataStructs.h parser/ParseError.h parser/SymbolVisitors.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/SymbolVisitors.cpp -o parser/SymbolVisitors.o $(SFLAG) $(WINFLAG)
parser/TypeChecker.o: parser/TypeChecker.cpp zsyssimple.h parser/AST.h parser/Compiler.h parser/DataStructs.h parser/GlobalSymbols.h parser/ParseError.h parser/TypeChecker.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/TypeChecker.cpp -o parser/TypeChecker.o $(SFLAG) $(WINFLAG)
parser/UtilVisitors.o: parser/UtilVisitors.cpp zsyssimple.h parser/AST.h parser/Compiler.h parser/ParseError.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/UtilVisitors.cpp -o parser/UtilVisitors.o $(SFLAG) $(WINFLAG)
parser/y.tab.o: parser/y.tab.cpp zsyssimple.h parser/AST.h parser/Compiler.h parser/UtilVisitors.h parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c parser/y.tab.cpp -o parser/y.tab.o $(SFLAG) $(WINFLAG)

guiBitmapRenderer.o: guiBitmapRenderer.cpp guiBitmapRenderer.h
	$(CC) $(OPTS) $(CFLAG) -c guiBitmapRenderer.cpp -o guiBitmapRenderer.o $(SFLAG) $(WINFLAG)

gui/alert.o: gui/alert.cpp gui/alert.h gui/dialog.h gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c gui/alert.cpp -o gui/alert.o $(SFLAG) $(WINFLAG)
gui/contents.o: gui/contents.cpp gui/contents.h
	$(CC) $(OPTS) $(CFLAG) -c gui/contents.cpp -o gui/contents.o $(SFLAG) $(WINFLAG)
gui/controller.o: gui/controller.cpp gui/controller.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c gui/controller.cpp -o gui/controller.o $(SFLAG) $(WINFLAG)
gui/dialog.o: gui/dialog.cpp gui/dialog.h gui/controller.h gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c gui/dialog.cpp -o gui/dialog.o $(SFLAG) $(WINFLAG)
gui/manager.o: gui/manager.cpp gui/manager.h gui/alert.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c gui/manager.cpp -o gui/manager.o $(SFLAG) $(WINFLAG)

gui/allegro/bitmap.o: gui/allegro/bitmap.cpp gui/allegro/bitmap.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/bitmap.cpp -o gui/allegro/bitmap.o $(SFLAG) $(WINFLAG)
gui/allegro/button.o: gui/allegro/button.cpp gui/allegro/button.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/button.cpp -o gui/allegro/button.o $(SFLAG) $(WINFLAG)
gui/allegro/checkbox.o: gui/allegro/checkbox.cpp gui/allegro/checkbox.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/checkbox.cpp -o gui/allegro/checkbox.o $(SFLAG) $(WINFLAG)
gui/allegro/column.o: gui/allegro/column.cpp gui/allegro/column.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/column.cpp -o gui/allegro/column.o $(SFLAG) $(WINFLAG)
gui/allegro/comboBox.o: gui/allegro/comboBox.cpp gui/allegro/comboBox.h gui/allegro/list.h gui/list.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/comboBox.cpp -o gui/allegro/comboBox.o $(SFLAG) $(WINFLAG)
gui/allegro/controller.o: gui/allegro/controller.cpp gui/allegro/controller.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/controller.cpp -o gui/allegro/controller.o $(SFLAG) $(WINFLAG)
gui/allegro/dummy.o: gui/allegro/dummy.cpp gui/allegro/dummy.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/dummy.cpp -o gui/allegro/dummy.o $(SFLAG) $(WINFLAG)
gui/allegro/editableText.o: gui/allegro/editableText.cpp gui/allegro/editableText.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/editableText.cpp -o gui/allegro/editableText.o $(SFLAG) $(WINFLAG)
gui/allegro/factory.o: gui/allegro/factory.cpp gui/allegro/factory.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/factory.cpp -o gui/allegro/factory.o $(SFLAG) $(WINFLAG)
gui/allegro/frame.o: gui/allegro/frame.cpp gui/allegro/frame.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/frame.cpp -o gui/allegro/frame.o $(SFLAG) $(WINFLAG)
gui/allegro/list.o: gui/allegro/list.cpp gui/allegro/list.h gui/allegro/scrollingPane.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/list.cpp -o gui/allegro/list.o $(SFLAG) $(WINFLAG)
gui/allegro/renderer.o: gui/allegro/renderer.cpp gui/allegro/renderer.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/renderer.cpp -o gui/allegro/renderer.o $(SFLAG) $(WINFLAG)
gui/allegro/row.o: gui/allegro/row.cpp gui/allegro/row.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/row.cpp -o gui/allegro/row.o $(SFLAG) $(WINFLAG)
gui/allegro/scrollbar.o: gui/allegro/scrollbar.cpp gui/allegro/scrollbar.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/scrollbar.cpp -o gui/allegro/scrollbar.o $(SFLAG) $(WINFLAG)
gui/allegro/scrollingPane.o: gui/allegro/scrollingPane.cpp gui/allegro/scrollingPane.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/scrollingPane.cpp -o gui/allegro/scrollingPane.o $(SFLAG) $(WINFLAG)
gui/allegro/serialContainer.o: gui/allegro/serialContainer.cpp gui/allegro/serialContainer.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/serialContainer.cpp -o gui/allegro/serialContainer.o $(SFLAG) $(WINFLAG)
gui/allegro/standardWidget.o: gui/allegro/standardWidget.cpp gui/allegro/standardWidget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/standardWidget.cpp -o gui/allegro/standardWidget.o $(SFLAG) $(WINFLAG)
gui/allegro/tab.o: gui/allegro/tab.cpp gui/allegro/tab.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/tab.cpp -o gui/allegro/tab.o $(SFLAG) $(WINFLAG)
gui/allegro/tabBar.o: gui/allegro/tabBar.cpp gui/allegro/tabBar.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/tabBar.cpp -o gui/allegro/tabBar.o $(SFLAG) $(WINFLAG)
gui/allegro/tabPanel.o: gui/allegro/tabPanel.cpp gui/allegro/tabPanel.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/tabPanel.cpp -o gui/allegro/tabPanel.o $(SFLAG) $(WINFLAG)
gui/allegro/text.o: gui/allegro/text.cpp gui/allegro/text.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/text.cpp -o gui/allegro/text.o $(SFLAG) $(WINFLAG)
gui/allegro/textField.o: gui/allegro/textField.cpp gui/allegro/textField.h gui/allegro/editableText.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/textField.cpp -o gui/allegro/textField.o $(SFLAG) $(WINFLAG)
gui/allegro/common.o: gui/allegro/common.cpp gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/common.cpp -o gui/allegro/common.o $(SFLAG) $(WINFLAG)
gui/allegro/window.o: gui/allegro/window.cpp gui/allegro/window.h gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c gui/allegro/window.cpp -o gui/allegro/window.o $(SFLAG) $(WINFLAG)

gui/gtk/bitmap.o: gui/gtk/bitmap.cpp gui/gtk/bitmap.h guiBitmapRenderer.h gui/mouse.h gui/widget.h gui/bitmap.h gui/gtk/util.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/bitmap.cpp -o gui/gtk/bitmap.o $(SFLAG) $(WINFLAG)
gui/gtk/button.o: gui/gtk/button.cpp gui/gtk/button.h gui/gtk/util.h gui/widget.h gui/button.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/button.cpp -o gui/gtk/button.o $(SFLAG) $(WINFLAG)
gui/gtk/buttonRow.o: gui/gtk/buttonRow.cpp gui/gtk/buttonRow.h gui/gtk/util.h gui/widget.h gui/serialContainer.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/buttonRow.cpp -o gui/gtk/buttonRow.o $(SFLAG) $(WINFLAG)
gui/gtk/checkbox.o: gui/gtk/checkbox.cpp gui/gtk/checkbox.h gui/gtk/util.h gui/widget.h gui/checkbox.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/checkbox.cpp -o gui/gtk/checkbox.o $(SFLAG) $(WINFLAG)
gui/gtk/controller.o: gui/gtk/controller.cpp gui/gtk/controller.h gui/controller.h gui/widget.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/controller.cpp -o gui/gtk/controller.o $(SFLAG) $(WINFLAG)
gui/gtk/factory.o: gui/gtk/factory.cpp gui/gtk/factory.h gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/factory.cpp -o gui/gtk/factory.o $(SFLAG) $(WINFLAG)
gui/gtk/frame.o: gui/gtk/frame.cpp gui/gtk/frame.h gui/gtk/util.h gui/widget.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/frame.cpp -o gui/gtk/frame.o $(SFLAG) $(WINFLAG)
gui/gtk/manager.o: gui/gtk/manager.cpp gui/gtk/manager.h gui/manager.h gui/gtk/factory.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/manager.cpp -o gui/gtk/manager.o $(SFLAG) $(WINFLAG)
gui/gtk/serialContainer.o: gui/gtk/serialContainer.cpp gui/gtk/factory.h gui/gtk/util.h gui/widget.h gui/serialContainer.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/serialContainer.cpp -o gui/gtk/serialContainer.o $(SFLAG) $(WINFLAG)
gui/gtk/spinner.o: gui/gtk/spinner.cpp gui/gtk/spinner.h gui/widget.h gui/spinner.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/spinner.cpp -o gui/gtk/spinner.o $(SFLAG) $(WINFLAG)
gui/gtk/tabPanel.o: gui/gtk/tabPanel.cpp gui/gtk/tabPanel.h gui/gtk/util.h gui/widget.h gui/tabPanel.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/tabPanel.cpp -o gui/gtk/tabPanel.o $(SFLAG) $(WINFLAG)
gui/gtk/text.o: gui/gtk/text.cpp gui/gtk/text.h gui/gtk/util.h gui/widget.h gui/text.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/text.cpp -o gui/gtk/text.o $(SFLAG) $(WINFLAG)
gui/gtk/textBox.o: gui/gtk/textBox.cpp gui/gtk/textBox.h gui/gtk/util.h gui/widget.h gui/textBox.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/textBox.cpp -o gui/gtk/textBox.o $(SFLAG) $(WINFLAG)
gui/gtk/textField.o: gui/gtk/textField.cpp gui/gtk/window.h gui/gtk/util.h gui/widget.h gui/textField.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/textField.cpp -o gui/gtk/textField.o $(SFLAG) $(WINFLAG)
gui/gtk/util.o: gui/gtk/util.cpp gui/gtk/util.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/util.cpp -o gui/gtk/util.o $(SFLAG) $(WINFLAG)
gui/gtk/widget.o: gui/gtk/widget.cpp gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/widget.cpp -o gui/gtk/widget.o $(SFLAG) $(WINFLAG)
gui/gtk/window.o: gui/gtk/window.cpp gui/gtk/window.h gui/key.h gui/gtk/util.h gui/widget.h gui/window.h gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c gui/gtk/window.cpp -o gui/gtk/window.o $(SFLAG) $(WINFLAG)

dialog/bitmap/tilePreview.o: dialog/bitmap/tilePreview.cpp dialog/bitmap/tilePreview.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/bitmap/tilePreview.cpp -o dialog/bitmap/tilePreview.o $(SFLAG) $(WINFLAG)
dialog/bitmap/tileSelector.o: dialog/bitmap/tileSelector.cpp dialog/bitmap/tileSelector.h dialog/zquest/tileSelector.h gui/bitmap.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/bitmap/tileSelector.cpp -o dialog/bitmap/tileSelector.o $(SFLAG) $(WINFLAG)

dialog/zquest/cheatEditor.o: dialog/zquest/cheatEditor.cpp dialog/zquest/cheatEditor.h gui/dialog.h zdefs.h gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/cheatEditor.cpp -o dialog/zquest/cheatEditor.o $(SFLAG) $(WINFLAG)
dialog/zquest/infoShopEditor.o: dialog/zquest/infoShopEditor.cpp dialog/zquest/infoShopEditor.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/infoShopEditor.cpp -o dialog/zquest/infoShopEditor.o $(SFLAG) $(WINFLAG)
dialog/zquest/paletteViewer.o: dialog/zquest/paletteViewer.cpp dialog/zquest/paletteViewer.h gui/dialog.h gui/bitmap.h gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/paletteViewer.cpp -o dialog/zquest/paletteViewer.o $(SFLAG) $(WINFLAG)
dialog/zquest/questRules.o: dialog/zquest/questRules.cpp dialog/zquest/questRules.h zdefs.h  zsys.h gui/dialog.h gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/questRules.cpp -o dialog/zquest/questRules.o $(SFLAG) $(WINFLAG)
dialog/zquest/shopEditor.o: dialog/zquest/shopEditor.cpp dialog/zquest/shopEditor.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/shopEditor.cpp -o dialog/zquest/shopEditor.o $(SFLAG) $(WINFLAG)
dialog/zquest/simpleListSelector.o: dialog/zquest/simpleListSelector.cpp dialog/zquest/simpleListSelector.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/simpleListSelector.cpp -o dialog/zquest/simpleListSelector.o $(SFLAG) $(WINFLAG)
dialog/zquest/tileEditor.o: dialog/zquest/tileEditor.cpp dialog/zquest/tileEditor.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/tileEditor.cpp -o dialog/zquest/tileEditor.o $(SFLAG) $(WINFLAG)
dialog/zquest/tileSelector.o: dialog/zquest/tileSelector.cpp dialog/zquest/tileSelector.h dialog/bitmap/tileSelector.h gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/tileSelector.cpp -o dialog/zquest/tileSelector.o $(SFLAG) $(WINFLAG)
dialog/zquest/tileSelectorBackend.o: dialog/zquest/tileSelectorBackend.cpp dialog/zquest/tileSelectorBackend.cpp dialog/zquest/tileSelector.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/tileSelectorBackend.cpp -o dialog/zquest/tileSelectorBackend.o $(SFLAG) $(WINFLAG)
dialog/zquest/zscriptEditor.o: dialog/zquest/zscriptEditor.cpp dialog/zquest/zscriptEditor.h gui/dialog.h gui/factory.h gui/manager.h gui/text.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/zscriptEditor.cpp -o dialog/zquest/zscriptEditor.o $(SFLAG) $(WINFLAG)
dialog/zquest/zscriptMain.o: dialog/zquest/zscriptMain.cpp dialog/zquest/zscriptMain.h dialog/zquest/zscriptEditor.h gui/dialog.h gui/factory.h gui/manager.h gui/text.h
	$(CC) $(OPTS) $(CFLAG) -c dialog/zquest/zscriptMain.cpp -o dialog/zquest/zscriptMain.o $(SFLAG) $(WINFLAG)

sequence/gameOver.o: sequence/gameOver.cpp sequence/gameOver.h link.h sfx.h sound.h zelda.h sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c sequence/gameOver.cpp -o sequence/gameOver.o $(SFLAG) $(WINFLAG)
sequence/ganonIntro.o: sequence/ganonIntro.cpp sequence/ganonIntro.h guys.h link.h sfx.h sound.h zc_sys.h zelda.h sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c sequence/ganonIntro.cpp -o sequence/ganonIntro.o $(SFLAG) $(WINFLAG)
sequence/getBigTriforce.o: sequence/getBigTriforce.cpp sequence/getBigTriforce.h link.h sound.h zelda.h sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c sequence/getBigTriforce.cpp -o sequence/getBigTriforce.o $(SFLAG) $(WINFLAG)
sequence/getTriforce.o: sequence/getTriforce.cpp sequence/getTriforce.h link.h sound.h zelda.h zc_alleg.h sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c sequence/getTriforce.cpp -o sequence/getTriforce.o $(SFLAG) $(WINFLAG)
sequence/potion.o: sequence/potion.cpp sequence/potion.h link.h sfxClass.h zelda.h sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c sequence/potion.cpp -o sequence/potion.o $(SFLAG) $(WINFLAG)
sequence/whistle.o: sequence/whistle.cpp sequence/whistle.h link.h sfxClass.h weapons.h zc_sys.h zelda.h sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c sequence/whistle.cpp -o sequence/whistle.o $(SFLAG) $(WINFLAG)

