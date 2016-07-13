-include makefile.inc

AUDIO_LIBS = -lgme -lalogg -lalmp3 -laldmb -ldumb
IMAGE_LIBS = -ljpgal -lldpng -lpng -lz
# -lSDLmain -lSDL
#LINKOPTS = -pg -g
#LINKOPTS = -pg
#OPTS = -pg -g
#OPTS = -pg
#OPTS = -O3
OPTS = -DPTW32_STATIC_LIB -O3
#COMPRESS = 1

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
  ZC_ICON_CMD = windres --use-temp-file -J rc -o coff -i zc_icon.rc -o zc_icon.o
  ZQ_ICON = zq_icon.o
  ZQ_ICON_DEPS = zq_icon.rc zq_icon.ico
  ZQ_ICON_CMD = windres --use-temp-file -J rc -o coff -i zq_icon.rc -o zq_icon.o
  RV_ICON = rv_icon.o
  RV_ICON_DEPS = rv_icon.rc rv_icon.ico
  RV_ICON_CMD = windres --use-temp-file -J rc -o coff -i rv_icon.rc -o rv_icon.o
  ZC_PLATFORM = Windows
  CC = g++
  CFLAG = $(CFLAGBASE) -pedantic -I./include/dumb/ -I./include/alogg/ -I./include/almp3 -I./include/lpng1212/ -I./include/loadpng/ -I./include/lpng1212/ -I./include/jpgalleg-2.5/ -I./include/zlib123/ -I./gme
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
  CFLAG = -I./include -I../include -I./include/alogg -I./include/almp3
  LIBDIR = -L./libs/linux
  
  SINGLE_INSTANCE_CPP = src/single_instance_unix.cpp
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
  CFLAG = $(CFLAGBASE) -pedantic -arch i386 -I./include/dumb/ -I./include/alogg/ -I./include/almp3/ -I./include/libjpgal/
  CC = g++
  LIBDIR= -L./libs/osx
  DATA = output/common/
  SINGLE_INSTANCE_O = single_instance.o
  
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
  SINGLE_INSTANCE_O = single_instance.o
  
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
  SINGLE_INSTANCE_O = single_instance.o
  
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

ZELDA_OBJECTS = aglogo.o colors.o debug.o decorations.o defdata.o editbox.o EditboxModel.o EditboxView.o ending.o ffscript.o gamedata.o gui.o guys.o init.o items.o jwin.o jwinfsel.o link.o load_gif.o maps.o matrix.o md5.o midi.o pal.o particles.o qst.o save_gif.o script_drawing.o $(SINGLE_INSTANCE_O) sprite.o subscr.o tab_ctl.o tiles.o title.o weapons.o win32.o zc_custom.o zc_init.o zc_items.o zc_sprite.o zc_subscr.o zc_sys.o zelda.o zscriptversion.o zsys.o
ZELDA_OBJECTS := $(addprefix obj/,$(ZELDA_OBJECTS)) $(ZC_ICON)

ZQUEST_OBJECTS = zquest.o colors.o defdata.o editbox.o EditboxModel.o EditboxView.o gamedata.o gui.o init.o items.o jwin.o jwinfsel.o load_gif.o md5.o midi.o particles.o qst.o questReport.o save_gif.o sprite.o subscr.o tab_ctl.o tiles.o win32.o zc_custom.o zq_class.o zq_cset.o zq_custom.o zq_doors.o zq_files.o zq_items.o zq_init.o zq_misc.o zq_rules.o zq_sprite.o zq_strings.o zq_subscr.o zq_tiles.o zqscale.o zsys.o ffasm.o parser/AST.o parser/BuildVisitors.o parser/ByteCode.o parser/DataStructs.o parser/GlobalSymbols.o parser/lex.yy.o parser/ParseError.o parser/ScriptParser.o parser/SymbolVisitors.o parser/TypeChecker.o parser/UtilVisitors.o parser/y.tab.o
ZQUEST_OBJECTS := $(addprefix obj/,$(ZQUEST_OBJECTS)) $(ZQ_ICON)

ROMVIEW_OBJECTS = editbox.o EditboxModel.o EditboxView.o gui.o jwin.o jwinfsel.o load_gif.o romview.o save_gif.o tab_ctl.o zqscale.o zsys.o
ROMVIEW_OBJECTS := $(addprefix obj/,$(ROMVIEW_OBJECTS)) $(RV_ICON)

ZCSOUND_OBJECTS = obj/zcmusic.o obj/zcmusicd.o

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
obj/zcmusic.o: src/zcmusic.cpp src/mutex.h src/zc_alleg.h src/zcmusic.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -fpic -c src/zcmusic.cpp -o obj/zcmusic.o $(SFLAG) $(WINFLAG)
obj/zcmusicd.o: src/zcmusicd.cpp src/zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -fpic -c src/zcmusicd.cpp -o obj/zcmusicd.o $(SFLAG) $(WINFLAG)

$(ZELDA_EXE): $(ZELDA_OBJECTS) $(ZCSOUND_SO)
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
	$(CC) $(LINKOPTS) -o obj/$(ROMVIEW_EXE) $(ROMVIEW_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(RV_ICON) $(SFLAG) $(WINFLAG)
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
obj/ending.o: src/ending.cpp src/aglogo.h src/colors.h src/ending.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h src/init.h
	$(CC) $(OPTS) $(CFLAG) -c src/ending.cpp -o obj/ending.o $(SFLAG) $(WINFLAG)
obj/ffasm.o: src/ffasm.cpp src/ffasm.h src/ffscript.h src/gamedata.h src/jwin.h src/jwinfsel.h src/midi.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/ffasm.cpp -o obj/ffasm.o $(SFLAG) $(WINFLAG)
obj/ffscript.o: src/ffscript.cpp src/aglogo.h src/colors.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_init.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/ffscript.cpp -o obj/ffscript.o $(SFLAG) $(WINFLAG)
obj/font.o: /allegro/tools/datedit.h src/font.cpp font.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/font.cpp -o obj/font.o $(SFLAG) $(WINFLAG)
obj/gamedata.o: src/gamedata.cpp src/gamedata.h src/items.h src/jwin.h src/sfx.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/gamedata.cpp -o obj/gamedata.o $(SFLAG) $(WINFLAG)
obj/gui.o: src/gui.cpp src/colors.h src/debug.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/gui.cpp -o obj/gui.o $(SFLAG) $(WINFLAG)
obj/guys.o: src/guys.cpp src/aglogo.h src/colors.h src/defdata.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/guys.cpp -o obj/guys.o $(SFLAG) $(WINFLAG)
obj/init.o: src/init.cpp src/gamedata.h src/gui.h src/init.h src/jwin.h src/sfx.h src/tab_ctl.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(CFLAG) -c src/init.cpp -o obj/init.o $(SFLAG) $(WINFLAG)
obj/items.o: src/items.cpp src/gamedata.h src/items.h src/jwin.h src/maps.h src/sfx.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/items.cpp -o obj/items.o $(SFLAG) $(WINFLAG)
obj/jmenu.o: src/jmenu.cpp src/gamedata.h src/jwin.h src/sfx.h src/tab_ctl.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/jmenu.cpp -o obj/jmenu.o $(SFLAG) $(WINFLAG)
obj/jwin.o: src/jwin.cpp src/editbox.h src/gamedata.h src/jwin.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/jwin.cpp -o obj/jwin.o $(SFLAG) $(WINFLAG)
obj/jwinfsel.o: src/jwinfsel.cpp src/jwin.h src/jwinfsel.h src/tab_ctl.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/jwinfsel.cpp -o obj/jwinfsel.o $(SFLAG) $(WINFLAG)
obj/link.o: src/link.cpp src/aglogo.h src/colors.h src/decorations.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_subscr.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/link.cpp -o obj/link.o $(SFLAG) $(WINFLAG)
obj/load_gif.o: src/load_gif.cpp src/load_gif.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/load_gif.cpp -o obj/load_gif.o $(SFLAG) $(WINFLAG)
obj/maps.o: src/maps.cpp src/aglogo.h src/colors.h src/ffscript.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/particles.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_subscr.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c src/maps.cpp -o obj/maps.o $(SFLAG) $(WINFLAG)
obj/matrix.o: src/matrix.cpp src/gamedata.h src/matrix.h src/zc_alleg.h src/zc_sys.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/matrix.cpp -o obj/matrix.o $(SFLAG) $(WINFLAG)
obj/md5.o: src/md5.cpp src/md5.h
	$(CC) $(OPTS) $(CFLAG) -c src/md5.cpp -o obj/md5.o $(SFLAG) $(WINFLAG)
obj/midi.o: src/midi.cpp src/gamedata.h src/jwin.h src/midi.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/midi.cpp -o obj/midi.o $(SFLAG) $(WINFLAG)
obj/pal.o: src/pal.cpp src/aglogo.h src/colors.h src/gamedata.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/pal.cpp -o obj/pal.o $(SFLAG) $(WINFLAG)
obj/particles.o: src/particles.cpp src/gamedata.h src/particles.h src/sprite.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/particles.cpp -o obj/particles.o $(SFLAG) $(WINFLAG)
obj/qst.o: src/qst.cpp src/colors.h src/defdata.h src/font.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/md5.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zcmusic.h src/zdefs.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/qst.cpp -o obj/qst.o $(SFLAG) $(WINFLAG)
obj/questReport.o: src/questReport.cpp src/questReport.h src/editbox.h src/EditboxNew.h src/gui.h src/jwin.h src/mem_debug.h src/tiles.h src/zc_alleg.h src/zdefs.h src/zsys.h src/zq_class.h src/zq_misc.h src/zquest.h
	$(CC) $(OPTS) $(CFLAG) -c src/questReport.cpp -o obj/questReport.o $(SFLAG) $(WINFLAG)
obj/romview.o: src/romview.cpp src/fontsdat.h src/gamedata.h src/jwin.h src/jwinfsel.h src/load_gif.h src/save_gif.h src/tab_ctl.h src/zc_alleg.h src/zdefs.h src/zqscale.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/romview.cpp -o obj/romview.o $(SFLAG) $(WINFLAG)
obj/rv_icon.o: $(RV_ICON_DEPS)
	$(RV_ICON_CMD)
obj/save_gif.o: src/save_gif.cpp src/save_gif.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/save_gif.cpp -o obj/save_gif.o $(SFLAG) $(WINFLAG)
obj/script_drawing.o: src/script_drawing.cpp src/ffscript.h src/maps.h src/rendertarget.h src/script_drawing.h src/tiles.h src/zc_alleg.h src/zelda.h
	$(CC) $(OPTS) $(CFLAG) -c src/script_drawing.cpp -o obj/script_drawing.o $(SFLAG) $(WINFLAG)
obj/single_instance.o: $(SINGLE_INSTANCE_CPP) src/single_instance.h
	$(CC) $(OPTS) $(CFLAG) -c $(SINGLE_INSTANCE_CPP) -o obj/single_instance.o $(SFLAG) $(WINFLAG)
obj/sprite.o: src/sprite.cpp src/gamedata.h src/sprite.h src/tiles.h src/zc_alleg.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/sprite.cpp -o obj/sprite.o $(SFLAG) $(WINFLAG)
obj/subscr.o: src/subscr.cpp src/aglogo.h src/colors.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/subscr.cpp -o obj/subscr.o $(SFLAG) $(WINFLAG)
obj/tab_ctl.o: src/tab_ctl.cpp src/tab_ctl.h src/zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c src/tab_ctl.cpp -o obj/tab_ctl.o $(SFLAG) $(WINFLAG)
obj/tiles.o: src/tiles.cpp src/gamedata.h src/jwin.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zdefs.h src/zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c src/tiles.cpp -o obj/tiles.o $(SFLAG) $(WINFLAG)
obj/title.o: src/title.cpp src/aglogo.h src/colors.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/title.cpp -o obj/title.o $(SFLAG) $(WINFLAG)
obj/weapons.o: src/weapons.cpp src/aglogo.h src/colors.h src/gamedata.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/weapons.cpp -o obj/weapons.o $(SFLAG) $(WINFLAG)
obj/zc_custom.o: src/zc_custom.cpp src/gamedata.h src/jwin.h src/sfx.h src/tab_ctl.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_custom.cpp -o obj/zc_custom.o $(SFLAG) $(WINFLAG)
obj/zc_icon.o: $(ZC_ICON_DEPS)
	$(ZC_ICON_CMD)
obj/zc_init.o: src/zc_init.cpp src/aglogo.h src/colors.h src/gamedata.h src/gui.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/midi.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zc_init.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zq_init.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_init.cpp -o obj/zc_init.o $(SFLAG) $(WINFLAG)
obj/zc_items.o: src/zc_items.cpp src/gamedata.h src/guys.h src/jwin.h src/maps.h src/sfx.h src/sprite.h src/tab_ctl.h src/weapons.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_items.cpp -o obj/zc_items.o $(SFLAG) $(WINFLAG)
obj/zc_sprite.o: src/zc_sprite.cpp src/gamedata.h src/jwin.h src/maps.h src/sfx.h src/sprite.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_sprite.cpp -o obj/zc_sprite.o $(SFLAG) $(WINFLAG)
obj/zc_subscr.o: src/zc_subscr.cpp src/aglogo.h src/colors.h src/gamedata.h src/guys.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/pal.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_subscr.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_subscr.cpp -o obj/zc_subscr.o $(SFLAG) $(WINFLAG)
obj/zc_sys.o: src/zc_sys.cpp src/aglogo.h src/colors.h src/debug.h src/gamedata.h src/gui.h src/guys.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/maps.h src/matrix.h src/midi.h src/pal.h src/particles.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_init.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zc_sys.cpp -o obj/zc_sys.o $(SFLAG) $(WINFLAG)
obj/zelda.o: src/zelda.cpp src/aglogo.h src/colors.h src/ending.h src/ffscript.h src/fontsdat.h src/gamedata.h src/guys.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/link.h src/load_gif.h src/maps.h src/matrix.h src/pal.h src/particles.h src/qst.h src/save_gif.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/title.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zelda.h src/zeldadat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zelda.cpp -o obj/zelda.o $(SFLAG) $(WINFLAG)
obj/zscriptversion.o: src/zscriptversion.cpp src/zscriptversion.h src/zelda.h src/link.h src/zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c src/zscriptversion.cpp -o obj/zscriptversion.o $(SFLAG) $(WINFLAG)
obj/win32.o: src/win32.cpp src/win32.h
	$(CC) $(OPTS) $(CFLAG) -c src/win32.cpp -o obj/win32.o $(SFLAG) $(WINFLAG)
obj/zq_class.o: src/zq_class.cpp src/colors.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/maps.h src/md5.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_custom.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zq_subscr.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_class.cpp -o obj/zq_class.o $(SFLAG) $(WINFLAG)
obj/zq_cset.o: src/zq_cset.cpp src/colors.h src/gamedata.h src/gfxpal.h src/gui.h src/jwin.h src/jwinfsel.h src/midi.h src/pal.h src/sfx.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_cset.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_cset.cpp -o obj/zq_cset.o $(SFLAG) $(WINFLAG)
obj/zq_custom.o: src/zq_custom.cpp src/defdata.h src/ffasm.h src/ffscript.h src/gamedata.h src/gui.h src/guys.h src/init.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/sfx.h src/sprite.h src/tab_ctl.h src/tiles.h src/weapons.h src/zc_alleg.h src/zc_custom.h src/zcmusic.h src/zdefs.h src/zq_custom.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_custom.cpp -o obj/zq_custom.o $(SFLAG) $(WINFLAG)
obj/zq_doors.o: src/zq_doors.cpp src/gamedata.h src/gui.h src/jwin.h src/jwinfsel.h src/midi.h src/sfx.h src/sprite.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zc_sys.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_doors.cpp -o obj/zq_doors.o $(SFLAG) $(WINFLAG)
obj/zq_files.o: src/zq_files.cpp src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_custom.h src/zq_files.h src/zq_misc.h src/zq_tiles.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_files.cpp -o obj/zq_files.o $(SFLAG) $(WINFLAG)
obj/zq_icon.o: $(ZQ_ICON_DEPS)
	$(ZQ_ICON_CMD)
obj/zq_init.o: src/zq_init.cpp src/gamedata.h src/gui.h src/init.h src/jwin.h src/jwinfsel.h src/midi.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_init.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_init.cpp -o obj/zq_init.o $(SFLAG) $(WINFLAG)
obj/zq_items.o: src/zq_items.cpp src/gamedata.h src/zc_alleg.h src/zdefs.h src/zq_class.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_items.cpp -o obj/zq_items.o $(SFLAG) $(WINFLAG)
obj/zq_misc.o: src/zq_misc.cpp src/colors.h src/gamedata.h src/items.h src/jwin.h src/jwinfsel.h src/midi.h src/qst.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_misc.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_misc.cpp -o obj/zq_misc.o $(SFLAG) $(WINFLAG)
obj/zq_rules.o: src/zq_rules.cpp src/gamedata.h src/gui.h src/jwin.h src/jwinfsel.h src/midi.h src/sprite.h src/tab_ctl.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_custom.h src/zquest.h src/zsys.h
	$(CC) $(OPTS) $(CFLAG) -c src/zq_rules.cpp -o obj/zq_rules.o $(SFLAG) $(WINFLAG)
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
obj/zquest.o: src/zquest.cpp src/colors.h src/editbox.h src/EditboxNew.h src/ffasm.h src/ffscript.h src/fontsdat.h src/gamedata.h src/gui.h src/items.h src/jwin.h src/jwinfsel.h src/load_gif.h src/midi.h src/parser/Compiler.h src/qst.h src/save_gif.h src/sfx.h src/sprite.h src/subscr.h src/tab_ctl.h src/tiles.h src/zc_alleg.h src/zcmusic.h src/zdefs.h src/zq_class.h src/zq_cset.h src/zq_custom.h src/zq_doors.h src/zq_files.h src/zq_init.h src/zq_misc.h src/zq_rules.h src/zq_subscr.h src/zq_tiles.h src/zquest.h src/zquestdat.h src/zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c src/zquest.cpp -o obj/zquest.o $(SFLAG) $(WINFLAG)
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
