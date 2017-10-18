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
  ALLEG_LIB = -lalleg -lpthreadGC2 -lws2_32
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
  CFLAG = $(CFLAGBASE) -pedantic -I./include/dumb/ -I./include/alogg/ -I./include/almp3 -I./include/lpng1212/ -I./include/loadpng/ -I./include/lpng1212/ -I./include/jpgalleg-2.5/ -I./include/zlib123/  -I./include/pthread/ -I./gme
  LIBDIR = -L./libs/mingw
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
  CFLAG = -I./include -I../include -I./include/alogg -I./include/almp3
  LIBDIR = -L./libs/linux
  SINGLE_INSTANCE_O = single_instance.o
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

ZELDA_OBJECTS = aglogo.o colors.o debug.o decorations.o defdata.o editbox.o EditboxModel.o EditboxView.o ending.o ffscript.o gamedata.o gui.o guys.o init.o items.o jwin.o jwinfsel.o link.o load_gif.o maps.o matrix.o md5.o midi.o pal.o particles.o qst.o save_gif.o script_drawing.o SINGLE_INSTANCE_O sprite.o subscr.o tab_ctl.o tiles.o title.o weapons.o win32.o zc_custom.o zc_init.o zc_items.o zc_sprite.o zc_subscr.o zc_sys.o zcmusic.o zelda.o zscriptversion.o zsys.o $(ZC_ICON)
ZQUEST_OBJECTS = zquest.o colors.o defdata.o editbox.o EditboxModel.o EditboxView.o gamedata.o gui.o init.o items.o jwin.o jwinfsel.o load_gif.o md5.o midi.o particles.o qst.o questReport.o save_gif.o sprite.o subscr.o tab_ctl.o tiles.o win32.o zc_custom.o zcmusic.o zcmusicd.o zq_class.o zq_cset.o zq_custom.o zq_doors.o zq_files.o zq_items.o zq_init.o zq_misc.o zq_rules.o zq_sprite.o zq_strings.o zq_subscr.o zq_tiles.o zqscale.o zsys.o ffasm.o parser/AST.o parser/BuildVisitors.o parser/ByteCode.o parser/DataStructs.o parser/GlobalSymbols.o parser/lex.yy.o parser/ParseError.o parser/ScriptParser.o parser/SymbolVisitors.o parser/TypeChecker.o parser/UtilVisitors.o parser/y.tab.o $(ZQ_ICON)
ROMVIEW_OBJECTS = editbox.o EditboxModel.o EditboxView.o gui.o jwin.o jwinfsel.o load_gif.o romview.o save_gif.o tab_ctl.o zqscale.o zsys.o $(RV_ICON)

.PHONY: default veryclean clean all msg dos win windows linux gp2x test done

default: all
msg:
	@echo Compiling Zelda Classic for $(ZC_PLATFORM)...
done:
	@echo Done!
clean:
	rm -f $(ZELDA_OBJECTS) $(ZQUEST_OBJECTS) $(ROMVIEW_OBJECTS)
veryclean: clean
	rm -f $(ZELDA_EXE) $(ZQUEST_EXE) $(ROMVIEW_EXE)

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

all: test msg $(ZELDA_EXE) $(ZQUEST_EXE) $(ROMVIEW_EXE) done

$(ZELDA_EXE): $(ZELDA_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ZELDA_EXE) $(ZELDA_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(AUDIO_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(ZC_ICON) $(SFLAG) $(WINFLAG)
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

	mv $(ZELDA_EXE).app/Contents/MacOS/$(ZELDA_EXE) "$(ZELDA_EXE).app/Contents/MacOS/Zelda Classic"
	mv $(ZELDA_EXE).app "Zelda Classic.app"
endif

$(ZQUEST_EXE): $(ZQUEST_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ZQUEST_EXE) $(ZQUEST_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(AUDIO_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(ZQ_ICON) $(SFLAG) $(WINFLAG)
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
	cp $(DATA)string.zh $(ZQUEST_EXE).app/Contents/Resources/
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
	cp $(DATA)string.zh $(ZQUEST_EXE).app/Contents/Resources/
	mv $(ZQUEST_EXE).app/Contents/MacOS/$(ZQUEST_EXE) "$(ZQUEST_EXE).app/Contents/MacOS/ZQuest Editor"
	mv $(ZQUEST_EXE).app "ZQuest Editor.app"
endif

$(ROMVIEW_EXE): $(ROMVIEW_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ROMVIEW_EXE) $(ROMVIEW_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(AUDIO_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(RV_ICON) $(SFLAG) $(WINFLAG)
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
editbox.o: editbox.cpp EditboxNew.h jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c editbox.cpp -o editbox.o $(SFLAG) $(WINFLAG)
EditboxModel.o: EditboxModel.cpp editbox.h EditboxNew.h gamedata.h gui.h jwin.h tab_ctl.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxModel.cpp -o EditboxModel.o $(SFLAG) $(WINFLAG)
EditboxView.o: EditboxView.cpp EditboxNew.h jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxView.cpp -o EditboxView.o $(SFLAG) $(WINFLAG)
ending.o: ending.cpp aglogo.h colors.h ending.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h init.h
	$(CC) $(OPTS) $(CFLAG) -c ending.cpp -o ending.o $(SFLAG) $(WINFLAG)
ffasm.o: ffasm.cpp ffasm.h ffscript.h gamedata.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c ffasm.cpp -o ffasm.o $(SFLAG) $(WINFLAG)
ffscript.o: ffscript.cpp aglogo.h colors.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_init.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c ffscript.cpp -o ffscript.o $(SFLAG) $(WINFLAG)
font.o: /allegro/tools/datedit.h font.cpp font.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c font.cpp -o font.o $(SFLAG) $(WINFLAG)
gamedata.o: gamedata.cpp gamedata.h items.h jwin.h sfx.h sprite.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c gamedata.cpp -o gamedata.o $(SFLAG) $(WINFLAG)
gui.o: gui.cpp colors.h debug.h gamedata.h gui.h items.h jwin.h jwinfsel.h midi.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c gui.cpp -o gui.o $(SFLAG) $(WINFLAG)
guys.o: guys.cpp aglogo.h colors.h defdata.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c guys.cpp -o guys.o $(SFLAG) $(WINFLAG)
init.o: init.cpp gamedata.h gui.h init.h jwin.h sfx.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(CFLAG) -c init.cpp -o init.o $(SFLAG) $(WINFLAG)
items.o: items.cpp gamedata.h items.h jwin.h maps.h sfx.h sprite.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c items.cpp -o items.o $(SFLAG) $(WINFLAG)
jmenu.o: jmenu.cpp gamedata.h jwin.h sfx.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c jmenu.cpp -o jmenu.o $(SFLAG) $(WINFLAG)
jwin.o: jwin.cpp editbox.h gamedata.h jwin.h tab_ctl.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c jwin.cpp -o jwin.o $(SFLAG) $(WINFLAG)
jwinfsel.o: jwinfsel.cpp jwin.h jwinfsel.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c jwinfsel.cpp -o jwinfsel.o $(SFLAG) $(WINFLAG)
link.o: link.cpp aglogo.h colors.h decorations.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c link.cpp -o link.o $(SFLAG) $(WINFLAG)
load_gif.o: load_gif.cpp load_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c load_gif.cpp -o load_gif.o $(SFLAG) $(WINFLAG)
maps.o: maps.cpp aglogo.h colors.h ffscript.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h particles.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c maps.cpp -o maps.o $(SFLAG) $(WINFLAG)
matrix.o: matrix.cpp gamedata.h matrix.h zc_alleg.h zc_sys.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c matrix.cpp -o matrix.o $(SFLAG) $(WINFLAG)
md5.o: md5.cpp md5.h
	$(CC) $(OPTS) $(CFLAG) -c md5.cpp -o md5.o $(SFLAG) $(WINFLAG)
midi.o: midi.cpp gamedata.h jwin.h midi.h tab_ctl.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c midi.cpp -o midi.o $(SFLAG) $(WINFLAG)
pal.o: pal.cpp aglogo.h colors.h gamedata.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c pal.cpp -o pal.o $(SFLAG) $(WINFLAG)
particles.o: particles.cpp gamedata.h particles.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c particles.cpp -o particles.o $(SFLAG) $(WINFLAG)
qst.o: qst.cpp colors.h defdata.h font.h gamedata.h guys.h items.h jwin.h jwinfsel.h md5.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c qst.cpp -o qst.o $(SFLAG) $(WINFLAG)
questReport.o: questReport.cpp questReport.h editbox.h EditboxNew.h gui.h jwin.h mem_debug.h tiles.h zc_alleg.h zdefs.h zsys.h zq_class.h zq_misc.h zquest.h
	$(CC) $(OPTS) $(CFLAG) -c questReport.cpp -o questReport.o $(SFLAG) $(WINFLAG)
romview.o: romview.cpp fontsdat.h gamedata.h jwin.h jwinfsel.h load_gif.h save_gif.h tab_ctl.h zc_alleg.h zdefs.h zqscale.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c romview.cpp -o romview.o $(SFLAG) $(WINFLAG)
rv_icon.o: $(RV_ICON_DEPS)
	$(RV_ICON_CMD)
save_gif.o: save_gif.cpp save_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c save_gif.cpp -o save_gif.o $(SFLAG) $(WINFLAG)
script_drawing.o: script_drawing.cpp ffscript.h maps.h rendertarget.h script_drawing.h tiles.h zc_alleg.h zelda.h
	$(CC) $(OPTS) $(CFLAG) -c script_drawing.cpp -o script_drawing.o $(SFLAG) $(WINFLAG)
single_instance.o: single_instance_unix.cpp single_instance.h
	$(CC) $(OPTS) $(CFLAG) -c $(SINGLE_INSTANCE_CPP) -o single_instance.o $(SFLAG) $(WINFLAG)
sprite.o: sprite.cpp gamedata.h sprite.h tiles.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c sprite.cpp -o sprite.o $(SFLAG) $(WINFLAG)
subscr.o: subscr.cpp aglogo.h colors.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c subscr.cpp -o subscr.o $(SFLAG) $(WINFLAG)
tab_ctl.o: tab_ctl.cpp tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c tab_ctl.cpp -o tab_ctl.o $(SFLAG) $(WINFLAG)
tiles.o: tiles.cpp gamedata.h jwin.h tab_ctl.h tiles.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c tiles.cpp -o tiles.o $(SFLAG) $(WINFLAG)
title.o: title.cpp aglogo.h colors.h gamedata.h gui.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c title.cpp -o title.o $(SFLAG) $(WINFLAG)
weapons.o: weapons.cpp aglogo.h colors.h gamedata.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c weapons.cpp -o weapons.o $(SFLAG) $(WINFLAG)
zc_custom.o: zc_custom.cpp gamedata.h jwin.h sfx.h tab_ctl.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_custom.cpp -o zc_custom.o $(SFLAG) $(WINFLAG)
zc_icon.o: $(ZC_ICON_DEPS)
	$(ZC_ICON_CMD)
zc_init.o: zc_init.cpp aglogo.h colors.h gamedata.h gui.h init.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_init.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zq_init.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_init.cpp -o zc_init.o $(SFLAG) $(WINFLAG)
zc_items.o: zc_items.cpp gamedata.h guys.h jwin.h maps.h sfx.h sprite.h tab_ctl.h weapons.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_items.cpp -o zc_items.o $(SFLAG) $(WINFLAG)
zc_sprite.o: zc_sprite.cpp gamedata.h jwin.h maps.h sfx.h sprite.h tab_ctl.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_sprite.cpp -o zc_sprite.o $(SFLAG) $(WINFLAG)
zc_subscr.o: zc_subscr.cpp aglogo.h colors.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_subscr.cpp -o zc_subscr.o $(SFLAG) $(WINFLAG)
zc_sys.o: zc_sys.cpp aglogo.h colors.h debug.h gamedata.h gui.h guys.h init.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h particles.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_init.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_sys.cpp -o zc_sys.o $(SFLAG) $(WINFLAG)
zcmusic.o: zcmusic.cpp nothread.h zc_alleg.h zcmusic.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zcmusic.cpp -o zcmusic.o $(SFLAG) $(WINFLAG)
zcmusicd.o: zcmusicd.cpp zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -c zcmusicd.cpp -o zcmusicd.o $(SFLAG) $(WINFLAG)
zelda.o: zelda.cpp aglogo.h colors.h ending.h ffscript.h fontsdat.h gamedata.h guys.h init.h items.h jwin.h jwinfsel.h link.h load_gif.h maps.h matrix.h pal.h particles.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zelda.cpp -o zelda.o $(SFLAG) $(WINFLAG)
zscriptversion.o: zscriptversion.cpp zscriptversion.h zelda.h link.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c zscriptversion.cpp -o zscriptversion.o $(SFLAG) $(WINFLAG)
win32.o: win32.cpp win32.h
	$(CC) $(OPTS) $(CFLAG) -c win32.cpp -o win32.o $(SFLAG) $(WINFLAG)
zq_class.o: zq_class.cpp colors.h gamedata.h gui.h items.h jwin.h jwinfsel.h maps.h md5.h midi.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zq_class.h zq_misc.h zq_subscr.h zquest.h zquestdat.h zsys.h
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
zq_rules.o: zq_rules.cpp gamedata.h gui.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_custom.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_rules.cpp -o zq_rules.o $(SFLAG) $(WINFLAG)
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
zquest.o: zquest.cpp colors.h editbox.h EditboxNew.h ffasm.h ffscript.h fontsdat.h gamedata.h gui.h items.h jwin.h jwinfsel.h load_gif.h midi.h parser/Compiler.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_cset.h zq_custom.h zq_doors.h zq_files.h zq_init.h zq_misc.h zq_rules.h zq_subscr.h zq_tiles.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zquest.cpp -o zquest.o $(SFLAG) $(WINFLAG)
zsys.o: zsys.cpp gamedata.h jwin.h tab_ctl.h zc_alleg.h zc_sys.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zsys.cpp -o zsys.o $(SFLAG) $(WINFLAG)
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
