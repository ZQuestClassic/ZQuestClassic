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
  CFLAG = $(CFLAGBASE) -pedantic -I./include/dumb/ -I./include/alogg/ -I./include/almp3 -I./include/lpng1212/ -I./include/loadpng/ -I./include/lpng1212/ -I./include/jpgalleg-2.5/ -I./include/zlib123/ -I./include/gme/ -I./src/
  LIBDIR = -L./libs/mingw
  
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
  ZC_ICON_CMD = $(CC) $(OPTS) $(CFLAG) -c src/zc_icon.c -o zc_icon.o $(SFLAG)
  ZQ_ICON = zq_icon.o
  ZQ_ICON_DEPS = zq_icon.c
  ZQ_ICON_CMD = $(CC) $(OPTS) $(CFLAG) -c src/zq_icon.c -o zq_icon.o $(SFLAG)
  RV_ICON = rv_icon.o
  RV_ICON_DEPS = rv_icon.c
  RV_ICON_CMD = $(CC) $(OPTS) $(CFLAG) -c src/rv_icon.c -o rv_icon.o $(SFLAG)
  ZC_PLATFORM = Linux
  CC = g++ 
  CFLAG = -I./include -I../include -I./include/alogg -I./include/almp3 -I./src/
  LIBDIR = -L./libs/linux
  SINGLE_INSTANCE_O = obj/single_instance.o
  
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
  CFLAG = $(CFLAGBASE) -pedantic -arch i386 -I./include/dumb/ -I./include/alogg/ -I./include/almp3/ -I./include/libjpgal/ -I./src/
  CC = g++
  LIBDIR= -L./libs/osx
  DATA = output/common/
  # Never got this working on Mac...
  #SINGLE_INSTANCE_O = obj/single_instance.o
  
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
  CFLAG = -pedantic -Wno-long-long -Wall -arch i386 -arch ppc -DMACOSX_
  CC = g++
  LIBDIR= -L./libs/osx
  DATA = output/common/
  LINKOPTS = -arch i386 -arch ppc
  #SINGLE_INSTANCE_O = obj/single_instance.o
  
  ZCSOUND_SO = libs/osx/libzcsound.dylib
  ZCSOUND_LIB = -lzcsound
  ZCSOUND_LINKOPTS = $(LINKOPTS) -shared
  ZCSOUND_ALLEG_LIB = $(ALLEG_LIB)
else
ifdef COMPILE_FOR_MACOSX_SNOW_LEOPARD
  PLATEXT = -msl
  ALLEG_LIB = -framework Cocoa -framework Allegro -lalleg-main
  ZC_PLATFORM = Mac OS X Universal
  CFLAG = -pedantic -Wno-long-long -Wall -arch i386 -arch ppc -DMACOSX_
  CC = g++
  LIBDIR= -L./libs/osx
  DATA = output/common/
  LINKOPTS = -arch i386 -arch ppc
  #SINGLE_INSTANCE_O = obj/single_instance.o
  
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
  SINGLE_INSTANCE_O = obj/single_instance.o
endif
endif
endif
endif
endif
endif
endif

ZELDA_PREFIX = bin/zelda
ZQUEST_PREFIX = bin/zquest
ROMVIEW_PREFIX = bin/romview

ZELDA_EXE = $(ZELDA_PREFIX)$(PLATEXT)$(EXEEXT)
ZQUEST_EXE = $(ZQUEST_PREFIX)$(PLATEXT)$(EXEEXT)
ROMVIEW_EXE = $(ROMVIEW_PREFIX)$(PLATEXT)$(EXEEXT)

GTK_GUI_OBJECTS = obj/gui/gtk/bitmap.o obj/gui/gtk/button.o obj/gui/gtk/buttonRow.o obj/gui/gtk/checkbox.o obj/gui/gtk/controller.o obj/gui/gtk/factory.o obj/gui/gtk/frame.o obj/gui/gtk/manager.o obj/gui/gtk/serialContainer.o obj/gui/gtk/spinner.o obj/gui/gtk/tabPanel.o obj/gui/gtk/text.o obj/gui/gtk/textBox.o obj/gui/gtk/textField.o obj/gui/gtk/util.o obj/gui/gtk/widget.o obj/gui/gtk/window.o

ALLEGRO_GUI_OBJECTS = obj/gui/allegro/bitmap.o obj/gui/allegro/button.o obj/gui/allegro/checkbox.o obj/gui/allegro/column.o obj/gui/allegro/comboBox.o obj/gui/allegro/common.o obj/gui/allegro/controller.o obj/gui/allegro/dummy.o obj/gui/allegro/editableText.o obj/gui/allegro/factory.o obj/gui/allegro/frame.o obj/gui/allegro/list.o obj/gui/allegro/renderer.o obj/gui/allegro/row.o obj/gui/allegro/scrollbar.o obj/gui/allegro/scrollingPane.o obj/gui/allegro/serialContainer.o obj/gui/allegro/standardWidget.o obj/gui/allegro/tab.o obj/gui/allegro/tabBar.o obj/gui/allegro/tabPanel.o obj/gui/allegro/text.o obj/gui/allegro/textField.o obj/gui/allegro/window.o

ZELDA_OBJECTS = obj/aglogo.o obj/colors.o obj/debug.o obj/decorations.o obj/defdata.o obj/editbox.o obj/EditboxModel.o obj/EditboxView.o obj/encryption.o obj/ending.o obj/enemyAttack.o obj/ffc.o obj/ffscript.o obj/fontClass.o obj/gamedata.o obj/gui.o obj/guys.o obj/init.o obj/inventory.o obj/items.o obj/jwin.o obj/jwinfsel.o obj/link.o obj/load_gif.o obj/maps.o obj/matrix.o obj/md5.o obj/message.o obj/messageManager.o obj/messageRenderer.o obj/messageStream.o obj/midi.o obj/pal.o obj/particles.o obj/qst.o obj/refInfo.o obj/room.o obj/save_gif.o obj/screenFreezeState.o obj/screenWipe.o obj/script_drawing.o $(SINGLE_INSTANCE_O) obj/sfxAllegro.o obj/sfxClass.o obj/sfxManager.o obj/sound.o obj/sprite.o obj/subscr.o obj/tab_ctl.o obj/tiles.o obj/title.o obj/weapons.o obj/zc_custom.o obj/zc_init.o obj/zc_items.o obj/zc_sprite.o obj/zc_subscr.o obj/zc_sys.o obj/zelda.o obj/zscriptversion.o obj/zsys.o \
obj/sequence/gameOver.o obj/sequence/ganonIntro.o obj/sequence/getBigTriforce.o obj/sequence/getTriforce.o obj/sequence/potion.o obj/sequence/sequence.o obj/sequence/whistle.o \
obj/win32.o \
$(ZC_ICON)

ZQUEST_OBJECTS = obj/zquest.o obj/colors.o obj/defdata.o obj/editbox.o obj/EditboxModel.o obj/EditboxView.o obj/encryption.o obj/ffc.o obj/gamedata.o obj/gui.o obj/init.o obj/items.o obj/jwin.o obj/jwinfsel.o obj/load_gif.o obj/md5.o obj/messageList.o obj/midi.o obj/particles.o obj/qst.o obj/questReport.o obj/refInfo.o obj/save_gif.o obj/sprite.o obj/subscr.o obj/tab_ctl.o obj/tiles.o obj/zc_custom.o obj/zq_class.o obj/zq_cset.o obj/zq_custom.o obj/zq_doors.o obj/zq_files.o obj/zq_items.o obj/zq_init.o obj/zq_misc.o obj/zq_sprite.o obj/zq_strings.o obj/zq_subscr.o obj/zq_tiles.o obj/zqscale.o obj/zsys.o obj/ffasm.o obj/parser/AST.o obj/parser/BuildVisitors.o obj/parser/ByteCode.o obj/parser/DataStructs.o obj/parser/GlobalSymbols.o obj/parser/lex.yy.o obj/parser/ParseError.o obj/parser/ScriptParser.o obj/parser/SymbolVisitors.o obj/parser/TypeChecker.o obj/parser/UtilVisitors.o obj/parser/y.tab.o \
obj/guiBitmapRenderer.o \
obj/gui/alert.o obj/gui/contents.o obj/gui/controller.o obj/gui/dialog.o obj/gui/manager.o \
obj/win32.o \
$(ALLEGRO_GUI_OBJECTS) \
obj/dialog/bitmap/tilePreview.o obj/dialog/bitmap/tileSelector.o \
obj/dialog/zquest/cheatEditor.o obj/dialog/zquest/infoShopEditor.o obj/dialog/zquest/paletteViewer.o obj/dialog/zquest/questRules.o obj/dialog/zquest/shopEditor.o obj/dialog/zquest/simpleListSelector.o obj/dialog/zquest/tileSelector.o obj/dialog/zquest/tileSelectorBackend.o obj/dialog/zquest/zscriptEditor.o obj/dialog/zquest/zscriptMain.o \
$(ZQ_ICON)

ROMVIEW_OBJECTS = obj/editbox.o obj/EditboxModel.o obj/EditboxView.o obj/gui.o obj/jwin.o obj/jwinfsel.o obj/load_gif.o obj/romview.o obj/save_gif.o obj/tab_ctl.o obj/zqscale.o obj/zsys.o \
$(RV_ICON)

ZCSOUND_OBJECTS = obj/zcmusic.o obj/zcmusicd.o

.PHONY: default veryclean clean all msg dos win windows linux gp2x test done

default: all
msg:
	@echo Compiling Zelda Classic for $(ZC_PLATFORM)...
done:
	@echo Done!
clean:
	rm -f $(ZELDA_OBJECTS) $(ZQUEST_OBJECTS) $(ROMVIEW_OBJECTS) $(ZCSOUND_OBJECTS)
	rm -rf ./obj
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
linux: obj_dir
	@echo COMPILE_FOR_LINUX=1 > makefile.inc
	@echo "#define ALLEGRO_UNIX=1" > ./allegro/allegro/platform/alplatf.h
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
obj/zcmusic.o: src/zcmusic.cpp src/mutex.h src/zc_alleg.h src/zcmusic.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -fpic -c src/zcmusic.cpp -o obj/zcmusic.o $(SFLAG) $(WINFLAG)
obj/zcmusicd.o: src/zcmusicd.cpp src/zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -fpic -c src/zcmusicd.cpp -o obj/zcmusicd.o $(SFLAG) $(WINFLAG)

$(ZELDA_EXE): $(ZELDA_OBJECTS) $(ZCSOUND_SO)
	$(CC) $(LINKOPTS) -o $(ZELDA_EXE) $(ZELDA_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(ZCSOUND_LIB) $(ALLEG_LIB) $(AS_LIB) $(STDCXX_LIB) $(ZC_ICON) $(SFLAG) $(WINFLAG)
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

$(ZQUEST_EXE): $(ZQUEST_OBJECTS) $(ZCSOUND_SO)
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

obj_dir:
	@if [ ! -d "obj/gui/allegro" ]; then \
		mkdir -p "obj/gui/allegro"; \
	fi; \
	if [ ! -d "obj/parser" ]; then \
		mkdir -p "obj/parser"; \
	fi; \

obj/aglogo.o: src/aglogo.cpp src/gamedata.h src/zc_alleg.h src/zdefs.h src/zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c src/aglogo.cpp -o obj/aglogo.o $(SFLAG) $(WINFLAG)
obj/colors.o: src/colors.cpp src/colors.h src/gamedata.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/colors.cpp -o obj/colors.o $(SFLAG) $(WINFLAG)
obj/debug.o: src/debug.cpp src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/debug.cpp -o obj/debug.o $(SFLAG) $(WINFLAG)
obj/decorations.o: src/decorations.cpp src/decorations.h src/gamedata.h src/jwin.h src/maps.h src/sfx.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/decorations.cpp -o obj/decorations.o $(SFLAG) $(WINFLAG)
obj/defdata.o: src/defdata.cpp src/defdata.h src/gamedata.h src/guys.h src/items.h src/sfx.h src/sprite.h src/weapons.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/defdata.cpp -o obj/defdata.o $(SFLAG) $(WINFLAG)
obj/editbox.o: src/editbox.cpp src/EditboxNew.h src/jwin.h src/tab_ctl.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/editbox.cpp -o obj/editbox.o $(SFLAG) $(WINFLAG)
obj/EditboxModel.o: src/EditboxModel.cpp src/editbox.h src/EditboxNew.h src/gamedata.h src/gui.h src/jwin.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/EditboxModel.cpp -o obj/EditboxModel.o $(SFLAG) $(WINFLAG)
obj/EditboxView.o: src/EditboxView.cpp src/EditboxNew.h src/jwin.h src/tab_ctl.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/EditboxView.cpp -o obj/EditboxView.o $(SFLAG) $(WINFLAG)
obj/encryption.o: src/$(ENCRYPTION_CPP) src/encryption.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/$(ENCRYPTION_CPP) -o obj/encryption.o $(SFLAG) $(WINFLAG)
obj/ending.o: src/ending.cpp src/aglogo.h src/colors.h src/ending.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h src/init.h
	$(CC) $(OPTS) $(CFLAG) -c src/ending.cpp -o obj/ending.o $(SFLAG) $(WINFLAG)
obj/enemyAttack.o: src/enemyAttack.cpp src/enemyAttack.h src/guys.h src/link.h src/zdefs.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/enemyAttack.cpp -o obj/enemyAttack.o $(SFLAG) $(WINFLAG)
obj/ffasm.o: src/ffasm.cpp src/ffasm.h src/ffscript.h src/gamedata.h src/jwin.h src/jwinfsel.h src/midi.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/ffasm.cpp -o obj/ffasm.o $(SFLAG) $(WINFLAG)
obj/ffc.o: src/ffc.cpp src/ffc.h src/refInfo.h src/types.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/ffc.cpp -o obj/ffc.o $(SFLAG) $(WINFLAG)
obj/ffscript.o: src/ffscript.cpp src/aglogo.h src/colors.h src/ffc.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_init.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h src/rendertarget.h
	$(CC) $(OPTS) $(CFLAG) -c src/ffscript.cpp -o obj/ffscript.o $(SFLAG) $(WINFLAG)
obj/font.o: src/allegro/tools/datedit.h src/font.cpp src/font.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/font.cpp -o obj/font.o $(SFLAG) $(WINFLAG)
obj/fontClass.o: src/fontClass.cpp src/fontClass.h
	$(CC) $(OPTS) $(CFLAG) -c src/fontClass.cpp -o obj/fontClass.o $(SFLAG) $(WINFLAG)
obj/gamedata.o: src/gamedata.cpp src/gamedata.h src/items.h src/jwin.h src/sfx.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/gamedata.cpp -o obj/gamedata.o $(SFLAG) $(WINFLAG)
obj/gui.o: src/gui.cpp src/colors.h src/debug.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui.cpp -o obj/gui.o $(SFLAG) $(WINFLAG)
obj/guys.o: src/guys.cpp src/aglogo.h src/colors.h src/defdata.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/room.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/guys.cpp -o obj/guys.o $(SFLAG) $(WINFLAG)
obj/init.o: src/init.cpp src/gamedata.h src/gui.h src/init.h src/jwin.h src/sfx.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(CFLAG) -c src/init.cpp -o obj/init.o $(SFLAG) $(WINFLAG)
obj/inventory.o: src/inventory.cpp src/inventory.h src/guys.h src/link.h src/messageManager.h src/sequence.h src/sound.h src/weapons.h src/zc_sys.h src/zdefs.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/inventory.cpp -o obj/inventory.o $(SFLAG) $(WINFLAG)
obj/items.o: src/items.cpp src/bind.h src/gamedata.h src/items.h src/jwin.h src/maps.h src/sfx.h src/sound.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/items.cpp -o obj/items.o $(SFLAG) $(WINFLAG)
obj/jmenu.o: src/jmenu.cpp src/gamedata.h src/jwin.h src/sfx.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/jmenu.cpp -o obj/jmenu.o $(SFLAG) $(WINFLAG)
obj/jwin.o: src/jwin.cpp src/editbox.h src/gamedata.h src/jwin.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/jwin.cpp -o obj/jwin.o $(SFLAG) $(WINFLAG)
obj/jwinfsel.o: src/jwinfsel.cpp src/jwin.h src/jwinfsel.h src/tab_ctl.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/jwinfsel.cpp -o obj/jwinfsel.o $(SFLAG) $(WINFLAG)
obj/link.o: src/link.cpp src/aglogo.h src/colors.h src/decorations.h src/ffc.h src/gamedata.h src/guys.h src/inventory.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/screenWipe.h src/sfx.h src/sound.h src/sprite.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_subscr.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/link.cpp -o obj/link.o $(SFLAG) $(WINFLAG)
#obj/linkHandler.o: src/linkHandler.cpp src/linkHandler.h src/decorations.h src/guys.h src/link.h src/sequence.h src/sfxManager.h src/zc_sys.h src/zelda.h
obj/load_gif.o: src/load_gif.cpp src/load_gif.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/load_gif.cpp -o obj/load_gif.o $(SFLAG) $(WINFLAG)
obj/maps.o: src/maps.cpp src/aglogo.h src/colors.h src/ffc.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/particles.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_subscr.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h src/rendertarget.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c src/maps.cpp -o obj/maps.o $(SFLAG) $(WINFLAG)
obj/matrix.o: src/matrix.cpp src/gamedata.h src/matrix.h src/zc_alleg.h src/zc_sys.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/matrix.cpp -o obj/matrix.o $(SFLAG) $(WINFLAG)
obj/md5.o: src/md5.cpp src/md5.h
	$(CC) $(OPTS) $(CFLAG) -c src/md5.cpp -o obj/md5.o $(SFLAG) $(WINFLAG)
obj/message.o: src/message.cpp src/message.h src/fontClass.h src/link.h src/messageManager.h src/messageStream.h src/pal.h src/sound.h src/zc_sys.h src/zdefs.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/message.cpp -o obj/message.o $(SFLAG) $(WINFLAG)
obj/messageList.o: src/messageList.cpp src/messageList.h src/types.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/messageList.cpp -o obj/messageList.o $(SFLAG) $(WINFLAG)
obj/messageManager.o: src/messageManager.cpp src/messageManager.h src/bind.h src/link.h src/message.h src/weapons.h src/zc_sys.h src/zdefs.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/messageManager.cpp -o obj/messageManager.o $(SFLAG) $(WINFLAG)
obj/messageRenderer.o: src/messageRenderer.cpp src/messageRenderer.h src/subscr.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/messageRenderer.cpp -o obj/messageRenderer.o $(SFLAG) $(WINFLAG)
obj/messageStream.o: src/messageStream.cpp src/messageStream.h
	$(CC) $(OPTS) $(CFLAG) -c src/messageStream.cpp -o obj/messageStream.o $(SFLAG) $(WINFLAG)
obj/midi.o: src/midi.cpp src/gamedata.h src/jwin.h src/midi.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/midi.cpp -o obj/midi.o $(SFLAG) $(WINFLAG)
obj/pal.o: src/pal.cpp src/aglogo.h src/colors.h src/gamedata.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/pal.cpp -o obj/pal.o $(SFLAG) $(WINFLAG)
obj/particles.o: src/particles.cpp src/gamedata.h src/particles.h src/sprite.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/particles.cpp -o obj/particles.o $(SFLAG) $(WINFLAG)
obj/qst.o: src/qst.cpp src/colors.h src/defdata.h src/encryption.h src/ffc.h src/font.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/md5.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zcmusic.h src/zdefs.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/qst.cpp -o obj/qst.o $(SFLAG) $(WINFLAG)
obj/questReport.o: src/questReport.cpp src/questReport.h src/editbox.h src/EditboxNew.h src/gui.h src/jwin.h src/mem_debug.h src/tiles.h src/zc_alleg.h src/zdefs.h src/zsys.h src/zq_class.h src/zq_misc.h src/zquest.h
	$(CC) $(OPTS) $(CFLAG) -c src/questReport.cpp -o obj/questReport.o $(SFLAG) $(WINFLAG)
obj/refInfo.o: src/refInfo.cpp src/refInfo.h src/types.h
	$(CC) $(OPTS) $(CFLAG) -c src/refInfo.cpp -o obj/refInfo.o $(SFLAG) $(WINFLAG)
obj/romview.o: src/romview.cpp src/fontsdat.h src/gamedata.h src/jwin.h src/jwinfsel.h src/load_gif.h src/save_gif.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h src/zqscale.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/romview.cpp -o obj/romview.o $(SFLAG) $(WINFLAG)
obj/room.o: src/room.cpp src/room.h src/gamedata.h src/guys.h src/items.h src/link.h src/maps.h src/messageManager.h src/sound.h src/zc_sys.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/room.cpp -o obj/room.o $(SFLAG) $(WINFLAG)
obj/rv_icon.o: src/$(RV_ICON_DEPS)
	$(RV_ICON_CMD)
obj/save_gif.o: src/save_gif.cpp src/save_gif.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/save_gif.cpp -o obj/save_gif.o $(SFLAG) $(WINFLAG)
obj/screenFreezeState.o: src/screenFreezeState.cpp src/screenFreezeState.h src/maps.h src/messageManager.h src/tiles.h src/zdefs.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/screenFreezeState.cpp -o obj/screenFreezeState.o $(SFLAG) $(WINFLAG)
obj/screenWipe.o: src/screenWipe.cpp src/screenWipe.h src/zc_alleg.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/screenWipe.cpp -o obj/screenWipe.o $(SFLAG) $(WINFLAG)
obj/script_drawing.o: src/script_drawing.cpp src/ffscript.h src/maps.h src/rendertarget.h src/script_drawing.h src/tiles.h src/zc_alleg.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/script_drawing.cpp -o obj/script_drawing.o $(SFLAG) $(WINFLAG)
#obj/sequence.o: src/sequence.cpp src/sequence.h src/link.h src/sfx.h src/sfxManager.h src/sequence/sequence.h src/sequence/gameOver.h src/sequence/ganonIntro.h src/sequence/getBigTriforce.h src/sequence/getTriforce.h src/sequence/potion.h src/sequence/whistle.h
#	$(CC) $(OPTS) $(CFLAG) -c src/sequence.cpp -o obj/sequence.o $(SFLAG) $(WINFLAG)
obj/sfxAllegro.o: src/sfxAllegro.cpp src/sfxAllegro.h src/sound.h
	$(CC) $(OPTS) $(CFLAG) -c src/sfxAllegro.cpp -o obj/sfxAllegro.o $(SFLAG) $(WINFLAG)
obj/sfxClass.o: src/sfxClass.cpp src/sfxClass.h src/sfxAllegro.h
	$(CC) $(OPTS) $(CFLAG) -c src/sfxClass.cpp -o obj/sfxClass.o $(SFLAG) $(WINFLAG)
obj/sfxManager.o: src/sfxManager.cpp src/sfxManager.h src/sfxAllegro.h src/sfxClass.h
	$(CC) $(OPTS) $(CFLAG) -c src/sfxManager.cpp -o obj/sfxManager.o $(SFLAG) $(WINFLAG)
$(SINGLE_INSTANCE_O): src/single_instance_unix.cpp src/single_instance.h
	$(CC) $(OPTS) $(CFLAG) -c src/single_instance_unix.cpp -o $(SINGLE_INSTANCE_O) $(SFLAG) $(WINFLAG)
obj/sound.o: src/sound.cpp src/sound.h src/zc_alleg.h src/zc_sys.h src/zelda.h src/zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c src/sound.cpp -o obj/sound.o $(SFLAG) $(WINFLAG)
obj/sprite.o: src/sprite.cpp src/sprite.h src/entityPtr.h src/gamedata.h src/tiles.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/sprite.cpp -o obj/sprite.o $(SFLAG) $(WINFLAG)
obj/subscr.o: src/subscr.cpp src/aglogo.h src/colors.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/subscr.cpp -o obj/subscr.o $(SFLAG) $(WINFLAG)
obj/tab_ctl.o: src/tab_ctl.cpp src/tab_ctl.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/tab_ctl.cpp -o obj/tab_ctl.o $(SFLAG) $(WINFLAG)
obj/tiles.o: src/tiles.cpp src/gamedata.h src/jwin.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c src/tiles.cpp -o obj/tiles.o $(SFLAG) $(WINFLAG)
obj/title.o: src/title.cpp src/aglogo.h src/colors.h src/encryption.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/title.cpp -o obj/title.o $(SFLAG) $(WINFLAG)
obj/weapons.o: src/weapons.cpp src/aglogo.h src/colors.h src/gamedata.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/weapons.cpp -o obj/weapons.o $(SFLAG) $(WINFLAG)
obj/win32.o: src/win32.cpp src/win32.h 
	$(CC) $(OPTS) $(CFLAG) -c src/win32.cpp -o obj/win32.o $(SFLAG) $(WINFLAG)
obj/zc_custom.o: src/zc_custom.cpp src/gamedata.h src/jwin.h src/sfx.h src/tab_ctl.h src/zc_alleg.h src/zc_custom.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_custom.cpp -o obj/zc_custom.o $(SFLAG) $(WINFLAG)
obj/zc_icon.o: src/$(ZC_ICON_DEPS)
	$(ZC_ICON_CMD)
obj/zc_init.o: src/zc_init.cpp src/aglogo.h src/colors.h src/gamedata.h src/gui.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/midi.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zc_init.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zq_init.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_init.cpp -o obj/zc_init.o $(SFLAG) $(WINFLAG)
obj/zc_items.o: src/zc_items.cpp src/gamedata.h src/guys.h src/jwin.h src/maps.h src/sfx.h src/sprite.h src/tab_ctl.h src/weapons.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_items.cpp -o obj/zc_items.o $(SFLAG) $(WINFLAG)
obj/zc_sprite.o: src/zc_sprite.cpp src/gamedata.h src/jwin.h src/maps.h src/sfx.h src/sound.h src/sprite.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_sprite.cpp -o obj/zc_sprite.o $(SFLAG) $(WINFLAG)
obj/zc_subscr.o: src/zc_subscr.cpp src/aglogo.h src/colors.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_subscr.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_subscr.cpp -o obj/zc_subscr.o $(SFLAG) $(WINFLAG)
obj/zc_sys.o: src/zc_sys.cpp src/aglogo.h src/colors.h src/debug.h src/gamedata.h src/gui.h src/guys.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/midi.h src/pal.h src/particles.h src/qst.h src/screenWipe.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_init.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_sys.cpp -o obj/zc_sys.o $(SFLAG) $(WINFLAG)
obj/zelda.o: src/zelda.cpp src/aglogo.h src/colors.h src/ending.h src/ffc.h src/ffscript.h src/fontsdat.h src/gamedata.h src/guys.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/load_gif.h src/maps.h src/matrix.h src/pal.h src/particles.h src/qst.h src/save_gif.h src/sfx.h src/sound.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h src/rendertarget.h
	$(CC) $(OPTS) $(CFLAG) -c src/zelda.cpp -o obj/zelda.o $(SFLAG) $(WINFLAG)
obj/zq_class.o: src/zq_class.cpp src/colors.h src/encryption.h src/ffc.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/maps.h src/md5.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zq_subscr.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_class.cpp -o obj/zq_class.o $(SFLAG) $(WINFLAG)
obj/zq_cset.o: src/zq_cset.cpp src/colors.h src/gamedata.h src/gfxpal.h src/gui.h src/jwin.h src/jwinfsel.h src/midi.h src/pal.h src/sfx.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_cset.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_cset.cpp -o obj/zq_cset.o $(SFLAG) $(WINFLAG)
obj/zq_custom.o: src/zq_custom.cpp src/defdata.h src/ffasm.h src/ffscript.h src/gamedata.h src/gui.h src/guys.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/sfx.h src/sprite.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zcmusic.h src/zdefs.h src/zq_custom.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_custom.cpp -o obj/zq_custom.o $(SFLAG) $(WINFLAG)
obj/zq_doors.o: src/zq_doors.cpp src/gamedata.h src/gui.h src/jwin.h src/jwinfsel.h src/midi.h src/sfx.h src/sprite.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_doors.cpp -o obj/zq_doors.o $(SFLAG) $(WINFLAG)
obj/zq_files.o: src/zq_files.cpp src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_custom.h src/zq_files.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_files.cpp -o obj/zq_files.o $(SFLAG) $(WINFLAG)
obj/zq_icon.o: src/$(ZQ_ICON_DEPS)
	$(ZQ_ICON_CMD)
obj/zq_init.o: src/zq_init.cpp src/gamedata.h src/gui.h src/init.h src/jwin.h src/jwinfsel.h src/midi.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_init.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_init.cpp -o obj/zq_init.o $(SFLAG) $(WINFLAG)
obj/zq_items.o: src/zq_items.cpp src/gamedata.h src/zc_alleg.h src/zdefs.h src/zq_class.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_items.cpp -o obj/zq_items.o $(SFLAG) $(WINFLAG)
obj/zq_misc.o: src/zq_misc.cpp src/colors.h src/gamedata.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_misc.cpp -o obj/zq_misc.o $(SFLAG) $(WINFLAG)
obj/zq_sprite.o: src/zq_sprite.cpp src/gamedata.h src/sprite.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_sprite.cpp -o obj/zq_sprite.o $(SFLAG) $(WINFLAG)
obj/zq_strings.o: src/zq_strings.cpp src/zq_strings.h src/zquest.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_strings.cpp -o obj/zq_strings.o $(SFLAG) $(WINFLAG)
obj/zq_subscr.o: src/zq_subscr.cpp src/gamedata.h src/gui.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_misc.h src/zq_subscr.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_subscr.cpp -o obj/zq_subscr.o $(SFLAG) $(WINFLAG)
obj/zq_tiles.o: src/zq_tiles.cpp src/colors.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zq_tiles.h src/zqscale.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c src/zq_tiles.cpp -o obj/zq_tiles.o $(SFLAG) $(WINFLAG)
obj/zqscale.o: src/zqscale.cpp
	$(CC) $(OPTS) $(CFLAG) -c src/zqscale.cpp -o obj/zqscale.o $(SFLAG) $(WINFLAG)
obj/zquest.o: src/zquest.cpp src/bind.h src/colors.h src/editbox.h src/EditboxNew.h src/ffasm.h src/ffc.h src/ffscript.h src/fontsdat.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/load_gif.h src/midi.h src/parser/Compiler.h src/qst.h src/save_gif.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_cset.h src/zq_custom.h src/zq_doors.h src/zq_files.h src/zq_init.h src/zq_misc.h src/zq_subscr.h src/zq_tiles.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c src/zquest.cpp -o obj/zquest.o $(SFLAG) $(WINFLAG)
obj/zscriptversion.o: src/zscriptversion.cpp src/zelda.h src/link.h
	$(CC) $(OPTS) $(CFLAG) -c src/zscriptversion.cpp -o obj/zscriptversion.o $(SFLAG) $(WINFLAG)
obj/zsys.o: src/zsys.cpp src/gamedata.h src/jwin.h src/tab_ctl.h src/zc_alleg.h src/zc_sys.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zsys.cpp -o obj/zsys.o $(SFLAG) $(WINFLAG)

obj/parser/AST.o: src/parser/AST.cpp src/parser/AST.h src/parser/Compiler.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/AST.cpp -o obj/parser/AST.o $(SFLAG) $(WINFLAG)
obj/parser/BuildVisitors.o: src/parser/BuildVisitors.cpp src/zsyssimple.h src/parser/AST.h src/parser/BuildVisitors.h src/parser/ByteCode.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/ParseError.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/BuildVisitors.cpp -o obj/parser/BuildVisitors.o $(SFLAG) $(WINFLAG)
obj/parser/ByteCode.o: src/parser/ByteCode.cpp src/zsyssimple.h src/parser/AST.h src/parser/ByteCode.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/ParseError.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/ByteCode.cpp -o obj/parser/ByteCode.o $(SFLAG) $(WINFLAG)
obj/parser/DataStructs.o: src/parser/DataStructs.cpp src/zsyssimple.h src/parser/AST.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/DataStructs.cpp -o obj/parser/DataStructs.o $(SFLAG) $(WINFLAG)
obj/parser/GlobalSymbols.o: src/parser/GlobalSymbols.cpp src/zsyssimple.h src/parser/AST.h src/parser/ByteCode.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/GlobalSymbols.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/GlobalSymbols.cpp -o obj/parser/GlobalSymbols.o $(SFLAG) $(WINFLAG)
obj/parser/lex.yy.o: src/parser/lex.yy.cpp src/zsyssimple.h src/parser/AST.h src/parser/Compiler.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/lex.yy.cpp -o obj/parser/lex.yy.o $(SFLAG) $(WINFLAG)
obj/parser/ParseError.o: src/parser/ParseError.cpp src/zsyssimple.h src/parser/AST.h src/parser/Compiler.h src/parser/ParseError.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/ParseError.cpp -o obj/parser/ParseError.o $(SFLAG) $(WINFLAG)
obj/parser/ScriptParser.o: src/parser/ScriptParser.cpp src/zsyssimple.h src/parser/AST.h src/parser/BuildVisitors.h src/parser/ByteCode.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/GlobalSymbols.h src/parser/ParseError.h src/parser/SymbolVisitors.h src/parser/TypeChecker.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/ScriptParser.cpp -o obj/parser/ScriptParser.o $(SFLAG) $(WINFLAG)
obj/parser/SymbolVisitors.o: src/parser/SymbolVisitors.cpp src/parser/AST.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/ParseError.h src/parser/SymbolVisitors.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/SymbolVisitors.cpp -o obj/parser/SymbolVisitors.o $(SFLAG) $(WINFLAG)
obj/parser/TypeChecker.o: src/parser/TypeChecker.cpp src/zsyssimple.h src/parser/AST.h src/parser/Compiler.h src/parser/DataStructs.h src/parser/GlobalSymbols.h src/parser/ParseError.h src/parser/TypeChecker.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/TypeChecker.cpp -o obj/parser/TypeChecker.o $(SFLAG) $(WINFLAG)
obj/parser/UtilVisitors.o: src/parser/UtilVisitors.cpp src/zsyssimple.h src/parser/AST.h src/parser/Compiler.h src/parser/ParseError.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/UtilVisitors.cpp -o obj/parser/UtilVisitors.o $(SFLAG) $(WINFLAG)
obj/parser/y.tab.o: src/parser/y.tab.cpp src/zsyssimple.h src/parser/AST.h src/parser/Compiler.h src/parser/UtilVisitors.h src/parser/y.tab.hpp
	$(CC) $(OPTS) $(CFLAG) -c src/parser/y.tab.cpp -o obj/parser/y.tab.o $(SFLAG) $(WINFLAG)

obj/guiBitmapRenderer.o: src/guiBitmapRenderer.cpp src/guiBitmapRenderer.h
	$(CC) $(OPTS) $(CFLAG) -c src/guiBitmapRenderer.cpp -o obj/guiBitmapRenderer.o $(SFLAG) $(WINFLAG)

obj/gui/alert.o: src/gui/alert.cpp src/gui/alert.h src/bind.h src/gui/dialog.h src/gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/alert.cpp -o obj/gui/alert.o $(SFLAG) $(WINFLAG)
obj/gui/contents.o: src/gui/contents.cpp src/gui/contents.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/contents.cpp -o obj/gui/contents.o $(SFLAG) $(WINFLAG)
obj/gui/controller.o: src/gui/controller.cpp src/gui/controller.h src/gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/controller.cpp -o obj/gui/controller.o $(SFLAG) $(WINFLAG)
obj/gui/dialog.o: src/gui/dialog.cpp src/gui/dialog.h src/gui/controller.h src/gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/dialog.cpp -o obj/gui/dialog.o $(SFLAG) $(WINFLAG)
obj/gui/manager.o: src/gui/manager.cpp src/gui/manager.h src/gui/alert.h src/gui/dialog.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/manager.cpp -o obj/gui/manager.o $(SFLAG) $(WINFLAG)

obj/gui/allegro/bitmap.o: src/gui/allegro/bitmap.cpp src/gui/allegro/bitmap.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/bitmap.cpp -o obj/gui/allegro/bitmap.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/button.o: src/gui/allegro/button.cpp src/gui/allegro/button.h src/bind.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/button.cpp -o obj/gui/allegro/button.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/checkbox.o: src/gui/allegro/checkbox.cpp src/gui/allegro/checkbox.h src/bind.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/checkbox.cpp -o obj/gui/allegro/checkbox.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/column.o: src/gui/allegro/column.cpp src/gui/allegro/column.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/column.cpp -o obj/gui/allegro/column.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/comboBox.o: src/gui/allegro/comboBox.cpp src/gui/allegro/comboBox.h src/bind.h src/gui/allegro/list.h src/gui/list.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/comboBox.cpp -o obj/gui/allegro/comboBox.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/common.o: src/gui/allegro/common.cpp src/gui/allegro/common.h src/gui/allegro/scrollbar.h src/gui/key.h src/gui/mouse.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/common.cpp -o obj/gui/allegro/common.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/controller.o: src/gui/allegro/controller.cpp src/gui/allegro/controller.h src/gui/allegro/common.h src/gui/allegro/widget.h src/gui/key.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/controller.cpp -o obj/gui/allegro/controller.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/dummy.o: src/gui/allegro/dummy.cpp src/gui/allegro/dummy.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/dummy.cpp -o obj/gui/allegro/dummy.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/editableText.o: src/gui/allegro/editableText.cpp src/gui/allegro/editableText.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/editableText.cpp -o obj/gui/allegro/editableText.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/factory.o: src/gui/allegro/factory.cpp src/gui/allegro/factory.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/factory.cpp -o obj/gui/allegro/factory.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/frame.o: src/gui/allegro/frame.cpp src/gui/allegro/frame.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/frame.cpp -o obj/gui/allegro/frame.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/list.o: src/gui/allegro/list.cpp src/gui/allegro/list.h src/bind.h src/gui/allegro/scrollingPane.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/list.cpp -o obj/gui/allegro/list.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/renderer.o: src/gui/allegro/renderer.cpp src/gui/allegro/renderer.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/renderer.cpp -o obj/gui/allegro/renderer.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/row.o: src/gui/allegro/row.cpp src/gui/allegro/row.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/row.cpp -o obj/gui/allegro/row.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/scrollbar.o: src/gui/allegro/scrollbar.cpp src/gui/allegro/scrollbar.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/scrollbar.cpp -o obj/gui/allegro/scrollbar.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/scrollingPane.o: src/gui/allegro/scrollingPane.cpp src/gui/allegro/scrollingPane.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/scrollingPane.cpp -o obj/gui/allegro/scrollingPane.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/serialContainer.o: src/gui/allegro/serialContainer.cpp src/gui/allegro/serialContainer.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/serialContainer.cpp -o obj/gui/allegro/serialContainer.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/standardWidget.o: src/gui/allegro/standardWidget.cpp src/gui/allegro/standardWidget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/standardWidget.cpp -o obj/gui/allegro/standardWidget.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/tab.o: src/gui/allegro/tab.cpp src/gui/allegro/tab.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/tab.cpp -o obj/gui/allegro/tab.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/tabBar.o: src/gui/allegro/tabBar.cpp src/gui/allegro/tabBar.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/tabBar.cpp -o obj/gui/allegro/tabBar.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/tabPanel.o: src/gui/allegro/tabPanel.cpp src/gui/allegro/tabPanel.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/tabPanel.cpp -o obj/gui/allegro/tabPanel.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/text.o: src/gui/allegro/text.cpp src/gui/allegro/text.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/text.cpp -o obj/gui/allegro/text.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/textField.o: src/gui/allegro/textField.cpp src/gui/allegro/textField.h src/gui/allegro/editableText.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/textField.cpp -o obj/gui/allegro/textField.o $(SFLAG) $(WINFLAG)
obj/gui/allegro/window.o: src/gui/allegro/window.cpp src/gui/allegro/window.h src/bind.h src/gui/allegro/common.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/allegro/window.cpp -o obj/gui/allegro/window.o $(SFLAG) $(WINFLAG)

obj/gui/gtk/bitmap.o: src/gui/gtk/bitmap.cpp src/gui/gtk/bitmap.h src/guiBitmapRenderer.h src/gui/mouse.h src/gui/widget.h src/gui/bitmap.h src/gui/gtk/util.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/bitmap.cpp -o obj/gui/gtk/bitmap.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/button.o: src/gui/gtk/button.cpp src/gui/gtk/button.h src/gui/gtk/util.h src/gui/widget.h src/gui/button.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/button.cpp -o obj/gui/gtk/button.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/buttonRow.o: src/gui/gtk/buttonRow.cpp src/gui/gtk/buttonRow.h src/gui/gtk/util.h src/gui/widget.h src/gui/serialContainer.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/buttonRow.cpp -o obj/gui/gtk/buttonRow.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/checkbox.o: src/gui/gtk/checkbox.cpp src/gui/gtk/checkbox.h src/bind.h src/gui/gtk/util.h src/gui/widget.h src/gui/checkbox.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/checkbox.cpp -o obj/gui/gtk/checkbox.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/controller.o: src/gui/gtk/controller.cpp src/gui/gtk/controller.h src/gui/controller.h src/gui/widget.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/controller.cpp -o obj/gui/gtk/controller.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/factory.o: src/gui/gtk/factory.cpp src/gui/gtk/factory.h src/bind.h src/gui/factory.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/factory.cpp -o obj/gui/gtk/factory.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/frame.o: src/gui/gtk/frame.cpp src/gui/gtk/frame.h src/gui/gtk/util.h src/gui/widget.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/frame.cpp -o obj/gui/gtk/frame.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/manager.o: src/gui/gtk/manager.cpp src/gui/gtk/manager.h src/gui/manager.h src/gui/gtk/factory.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/manager.cpp -o obj/gui/gtk/manager.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/serialContainer.o: src/gui/gtk/serialContainer.cpp src/gui/gtk/factory.h src/gui/gtk/util.h src/gui/widget.h src/gui/serialContainer.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/serialContainer.cpp -o obj/gui/gtk/serialContainer.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/spinner.o: src/gui/gtk/spinner.cpp src/gui/gtk/spinner.h src/bind.h src/gui/widget.h src/gui/spinner.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/spinner.cpp -o obj/gui/gtk/spinner.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/tabPanel.o: src/gui/gtk/tabPanel.cpp src/gui/gtk/tabPanel.h src/gui/gtk/util.h src/gui/widget.h src/gui/tabPanel.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/tabPanel.cpp -o obj/gui/gtk/tabPanel.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/text.o: src/gui/gtk/text.cpp src/gui/gtk/text.h src/gui/gtk/util.h src/gui/widget.h src/gui/text.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/text.cpp -o obj/gui/gtk/text.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/textBox.o: src/gui/gtk/textBox.cpp src/gui/gtk/textBox.h src/gui/gtk/util.h src/gui/widget.h src/gui/textBox.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/textBox.cpp -o obj/gui/gtk/textBox.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/textField.o: src/gui/gtk/textField.cpp src/gui/gtk/window.h src/gui/gtk/util.h src/gui/widget.h src/gui/textField.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/textField.cpp -o obj/gui/gtk/textField.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/util.o: src/gui/gtk/util.cpp src/gui/gtk/util.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/util.cpp -o obj/gui/gtk/util.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/widget.o: src/gui/gtk/widget.cpp src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/widget.cpp -o obj/gui/gtk/widget.o $(SFLAG) $(WINFLAG)
obj/gui/gtk/window.o: src/gui/gtk/window.cpp src/gui/gtk/window.h src/bind.h src/gui/key.h src/gui/gtk/util.h src/gui/widget.h src/gui/window.h src/gui/gtk/widget.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui/gtk/window.cpp -o obj/gui/gtk/window.o $(SFLAG) $(WINFLAG)

obj/dialog/bitmap/tilePreview.o: src/dialog/bitmap/tilePreview.cpp src/dialog/bitmap/tilePreview.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/bitmap/tilePreview.cpp -o obj/dialog/bitmap/tilePreview.o $(SFLAG) $(WINFLAG)
obj/dialog/bitmap/tileSelector.o: src/dialog/bitmap/tileSelector.cpp src/dialog/bitmap/tileSelector.h src/dialog/zquest/tileSelector.h src/gui/bitmap.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/bitmap/tileSelector.cpp -o obj/dialog/bitmap/tileSelector.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/cheatEditor.o: src/dialog/zquest/cheatEditor.cpp src/dialog/zquest/cheatEditor.h src/gui/dialog.h src/zdefs.h src/gui/factory.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/cheatEditor.cpp -o obj/dialog/zquest/cheatEditor.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/infoShopEditor.o: src/dialog/zquest/infoShopEditor.cpp src/dialog/zquest/infoShopEditor.h src/gui/dialog.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/infoShopEditor.cpp -o obj/dialog/zquest/infoShopEditor.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/paletteViewer.o: src/dialog/zquest/paletteViewer.cpp src/dialog/zquest/paletteViewer.h src/gui/dialog.h src/gui/bitmap.h src/gui/factory.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/paletteViewer.cpp -o obj/dialog/zquest/paletteViewer.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/questRules.o: src/dialog/zquest/questRules.cpp src/dialog/zquest/questRules.h src/zdefs.h src/zsys.h src/gui/dialog.h src/gui/factory.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/questRules.cpp -o obj/dialog/zquest/questRules.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/shopEditor.o: src/dialog/zquest/shopEditor.cpp src/dialog/zquest/shopEditor.h src/gui/dialog.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/shopEditor.cpp -o obj/dialog/zquest/shopEditor.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/simpleListSelector.o: src/dialog/zquest/simpleListSelector.cpp src/dialog/zquest/simpleListSelector.h src/gui/dialog.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/simpleListSelector.cpp -o obj/dialog/zquest/simpleListSelector.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/tileSelector.o: src/dialog/zquest/tileSelector.cpp src/dialog/zquest/tileSelector.h src/dialog/bitmap/tileSelector.h src/gui/dialog.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/tileSelector.cpp -o obj/dialog/zquest/tileSelector.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/tileSelectorBackend.o: src/dialog/zquest/tileSelectorBackend.cpp src/dialog/zquest/tileSelectorBackend.cpp src/dialog/zquest/tileSelector.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/tileSelectorBackend.cpp -o obj/dialog/zquest/tileSelectorBackend.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/zscriptEditor.o: src/dialog/zquest/zscriptEditor.cpp src/dialog/zquest/zscriptEditor.h src/gui/dialog.h src/gui/factory.h src/gui/manager.h src/gui/text.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/zscriptEditor.cpp -o obj/dialog/zquest/zscriptEditor.o $(SFLAG) $(WINFLAG)
obj/dialog/zquest/zscriptMain.o: src/dialog/zquest/zscriptMain.cpp src/dialog/zquest/zscriptMain.h src/dialog/zquest/zscriptEditor.h src/gui/dialog.h src/gui/factory.h src/gui/manager.h src/gui/text.h src/bind.h
	$(CC) $(OPTS) $(CFLAG) -c src/dialog/zquest/zscriptMain.cpp -o obj/dialog/zquest/zscriptMain.o $(SFLAG) $(WINFLAG)

obj/sequence/gameOver.o: src/sequence/gameOver.cpp src/sequence/gameOver.h src/link.h src/sfx.h src/sound.h src/zelda.h src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/gameOver.cpp -o obj/sequence/gameOver.o $(SFLAG) $(WINFLAG)
obj/sequence/ganonIntro.o: src/sequence/ganonIntro.cpp src/sequence/ganonIntro.h src/guys.h src/link.h src/sfx.h src/sound.h src/zc_sys.h src/zelda.h src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/ganonIntro.cpp -o obj/sequence/ganonIntro.o $(SFLAG) $(WINFLAG)
obj/sequence/getBigTriforce.o: src/sequence/getBigTriforce.cpp src/sequence/getBigTriforce.h src/link.h src/sound.h src/zelda.h src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/getBigTriforce.cpp -o obj/sequence/getBigTriforce.o $(SFLAG) $(WINFLAG)
obj/sequence/getTriforce.o: src/sequence/getTriforce.cpp src/sequence/getTriforce.h src/link.h src/sound.h src/zelda.h src/zc_alleg.h src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/getTriforce.cpp -o obj/sequence/getTriforce.o $(SFLAG) $(WINFLAG)
obj/sequence/potion.o: src/sequence/potion.cpp src/sequence/potion.h src/link.h src/sfxClass.h src/zelda.h src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/potion.cpp -o obj/sequence/potion.o $(SFLAG) $(WINFLAG)
obj/sequence/sequence.o: src/sequence/sequence.cpp src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/sequence.cpp -o obj/sequence/sequence.o $(SFLAG) $(WINFLAG)
obj/sequence/whistle.o: src/sequence/whistle.cpp src/sequence/whistle.h src/link.h src/sfxClass.h src/weapons.h src/zc_sys.h src/zelda.h src/sequence/sequence.h
	$(CC) $(OPTS) $(CFLAG) -c src/sequence/whistle.cpp -o obj/sequence/whistle.o $(SFLAG) $(WINFLAG)

