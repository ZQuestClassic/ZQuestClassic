-include makefile.inc

AUDIO_LIBS = -lgme -lalogg -lalmp3 -laldmb -ldumb
IMAGE_LIBS = -ljpgal -lldpng -lpng -lz
#LINKOPTS = -pg -g
#OPTS = -pg -g
#OPTS = -O3
#COMPRESS = 1

CFLAG = -pedantic -Wno-long-long -Wall -W -Wshadow -Wpointer-arith

ifdef COMPILE_FOR_WIN
  ALLEG_LIB = -lalleg
  SFLAG = -s
  WINFLAG = -mwindows
  PLATEXT = -w
  EXEEXT = .exe
  ZC_ICON = zc_icon$(PLATEXT).o
  ZQ_ICON = zq_icon$(PLATEXT).o
  ZELDA_PREFIX = zelda
  ZQUEST_PREFIX = zquest
  ZC_PLATFORM = Windows
  CC = g++
  LIBDIR = -L./libs/mingw
else
ifdef COMPILE_FOR_LINUX
  PLATEXT = -l
  ALLEG_LIB = `allegro-config --libs --static`
  ZC_PLATFORM = Linux
  ZELDA_PREFIX = zelda
  ZQUEST_PREFIX = zquest
  CC = g++
else
ifdef COMPILE_FOR_DOS
  ALLEG_LIB = -lalleg
  STDCXX_LIB = -lstdcxx
  EXEEXT = .exe
  ZC_PLATFORM = DOS
  ZELDA_PREFIX = zelda
  ZQUEST_PREFIX = zquest
  CC = g++
else
ifdef COMPILE_FOR_MACOSX
  PLATEXT = -m
  ALLEG_LIB = -framework Cocoa -framework Allegro -lalleg-main
  ZC_PLATFORM = Mac OS X
  ZELDA_PREFIX = zelda
  ZQUEST_PREFIX = zquest
  CFLAG = -pedantic -Wno-long-long -Wall -Wno-long-double
  CC = g++
  LIBDIR= -L./libs/osx
else
ifdef COMPILE_FOR_GP2X
  PLATEXT = -g
  EXEEXT = .gpe
  ALLEG_LIB = -lalleg -lpthread -static
  ZC_PLATFORM = GP2X
  ZELDA_PREFIX = zelda
  ZQUEST_PREFIX = zquest
  #CFLAG = -pedantic -Wno-long-long -Wall -I/devkitGP2X/include
  CFLAG = -Wno-long-long -Wall -I/devkitGP2X/include
  CC = arm-linux-g++
  AUDIO_LIBS = -L/devkitGP2X/lib -lalspc -lalogg -lalmp3 -laldmb -ldumb
  IMAGE_LIBS = -L/devkitGP2X/lib -ljpgal -lldpng -lpng -lz
endif
endif
endif
endif
endif

ZELDA_EXE = $(ZELDA_PREFIX)$(PLATEXT)$(EXEEXT)
ZQUEST_EXE = $(ZQUEST_PREFIX)$(PLATEXT)$(EXEEXT)

ZELDA_OBJECTS = aglogo$(PLATEXT).o colors$(PLATEXT).o debug$(PLATEXT).o decorations$(PLATEXT).o defdata$(PLATEXT).o editbox$(PLATEXT).o EditboxModel$(PLATEXT).o EditboxView$(PLATEXT).o ending$(PLATEXT).o ffscript$(PLATEXT).o gamedata$(PLATEXT).o gui$(PLATEXT).o guys$(PLATEXT).o init$(PLATEXT).o items$(PLATEXT).o jwin$(PLATEXT).o jwinfsel$(PLATEXT).o link$(PLATEXT).o load_gif$(PLATEXT).o maps$(PLATEXT).o matrix$(PLATEXT).o md5$(PLATEXT).o midi$(PLATEXT).o pal$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o subscr$(PLATEXT).o tab_ctl$(PLATEXT).o tiles$(PLATEXT).o title$(PLATEXT).o weapons$(PLATEXT).o zc_custom$(PLATEXT).o zc_init$(PLATEXT).o zc_items$(PLATEXT).o zc_sprite$(PLATEXT).o zc_subscr$(PLATEXT).o zc_sys$(PLATEXT).o zcmusic$(PLATEXT).o zelda$(PLATEXT).o zsys$(PLATEXT).o $(ZC_ICON)
ifdef COMPILE_FOR_DOS
	ZQUEST_OBJECTS = colors$(PLATEXT).o defdata$(PLATEXT).o editbox$(PLATEXT).o EditboxModel$(PLATEXT).o EditboxView$(PLATEXT).o gamedata$(PLATEXT).o gui$(PLATEXT).o init$(PLATEXT).o items$(PLATEXT).o jwin$(PLATEXT).o jwinfsel$(PLATEXT).o load_gif$(PLATEXT).o md5$(PLATEXT).o midi$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o subscr$(PLATEXT).o tab_ctl$(PLATEXT).o tiles$(PLATEXT).o zc_custom$(PLATEXT).o zcmusic$(PLATEXT).o zcmusicd$(PLATEXT).o zq_class$(PLATEXT).o zq_cset$(PLATEXT).o zq_custom$(PLATEXT).o zq_doors$(PLATEXT).o zq_files$(PLATEXT).o zq_items$(PLATEXT).o zq_init$(PLATEXT).o zq_misc$(PLATEXT).o zq_rules$(PLATEXT).o zq_sprite$(PLATEXT).o zq_subscr$(PLATEXT).o zq_tiles$(PLATEXT).o zquest$(PLATEXT).o zsys$(PLATEXT).o ffasm$(PLATEXT).o parser/AST$(PLATEXT).o parser/BuildVisitors$(PLATEXT).o parser/ByteCode$(PLATEXT).o parser/DataStructs$(PLATEXT).o parser/GlobalSymbols$(PLATEXT).o parser/lex.yy$(PLATEXT).o parser/ParseError$(PLATEXT).o parser/ScriptParser$(PLATEXT).o parser/SymbolVisitors$(PLATEXT).o parser/TypeChecker$(PLATEXT).o parser/UtilVisitors$(PLATEXT).o parser/y.tab$(PLATEXT).o $(ZQ_ICON)
else
	ZQUEST_OBJECTS = colors$(PLATEXT).o defdata$(PLATEXT).o editbox-zq$(PLATEXT).o EditboxModel$(PLATEXT).o EditboxView$(PLATEXT).o gamedata$(PLATEXT).o gui$(PLATEXT).o init$(PLATEXT).o items$(PLATEXT).o jwin-zq$(PLATEXT).o jwinfsel-zq$(PLATEXT).o load_gif$(PLATEXT).o md5$(PLATEXT).o midi$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o subscr$(PLATEXT).o tab_ctl-zq$(PLATEXT).o tiles$(PLATEXT).o zc_custom$(PLATEXT).o zcmusic$(PLATEXT).o zcmusicd$(PLATEXT).o zq_class$(PLATEXT).o zq_cset-zq$(PLATEXT).o zq_custom$(PLATEXT).o zq_doors$(PLATEXT).o zq_files$(PLATEXT).o zq_items$(PLATEXT).o zq_init$(PLATEXT).o zq_misc$(PLATEXT).o zq_rules$(PLATEXT).o zq_sprite$(PLATEXT).o zq_subscr$(PLATEXT).o zq_tiles$(PLATEXT).o zqscale$(PLATEXT).o zquest$(PLATEXT).o zsys-zq$(PLATEXT).o ffasm$(PLATEXT).o parser/AST$(PLATEXT).o parser/BuildVisitors$(PLATEXT).o parser/ByteCode$(PLATEXT).o parser/DataStructs$(PLATEXT).o parser/GlobalSymbols$(PLATEXT).o parser/lex.yy$(PLATEXT).o parser/ParseError$(PLATEXT).o parser/ScriptParser$(PLATEXT).o parser/SymbolVisitors$(PLATEXT).o parser/TypeChecker$(PLATEXT).o parser/UtilVisitors$(PLATEXT).o parser/y.tab$(PLATEXT).o $(ZQ_ICON)
endif

.PHONY: default veryclean clean all msg dos win windows linux gp2x test done

default: all
msg:
	@echo Compiling Zelda Classic for $(ZC_PLATFORM)...
done:
	@echo Done!
clean:
	rm -f $(ZELDA_OBJECTS) $(ZQUEST_OBJECTS)
veryclean: clean
	rm -f $(ZELDA_EXE) $(ZQUEST_EXE) $(MUSIC_LIB)

test:
ifndef COMPILE_FOR_WIN
ifndef COMPILE_FOR_DOS
ifndef COMPILE_FOR_LINUX
ifndef COMPILE_FOR_MACOSX
ifndef COMPILE_FOR_GP2X
	#change this if you want to change the default platform
	@make win
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

all: test msg $(ZELDA_EXE) $(ZQUEST_EXE) done

$(ZELDA_EXE): $(ZELDA_OBJECTS)
	$(CC) $(LINKOPTS) -o $(ZELDA_EXE) $(ZELDA_OBJECTS) $(LIBDIR) $(IMAGE_LIBS) $(AUDIO_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(ZC_ICON) $(SFLAG) $(WINFLAG)
ifdef COMPRESS
	upx --best $(ZELDA_EXE)
endif
ifdef COMPILE_FOR_MACOSX
	rm -rf "Zelda Classic.app"
	fixbundle $(ZELDA_EXE) -e
	cp Info1.plist $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>Zelda Classic</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>zc_icon.icns</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	cat $(ZELDA_EXE).app/Contents/tempinfo Info2.plist > $(ZELDA_EXE).app/Contents/Info.plist
	rm $(ZELDA_EXE).app/Contents/tempinfo
	cp "zc_icon.icns" $(ZELDA_EXE).app/Contents/Resources/
	cp zelda.dat $(ZELDA_EXE).app/
	cp sfx.dat $(ZELDA_EXE).app/
	cp fonts.dat $(ZELDA_EXE).app/
	cp qst.dat $(ZELDA_EXE).app/
	cp 1st.qst $(ZELDA_EXE).app/
	cp 2nd.qst $(ZELDA_EXE).app/
	cp 3rd.qst $(ZELDA_EXE).app/
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
	cat $(ZQUEST_EXE).app/Contents/tempinfo Info2.plist > $(ZQUEST_EXE).app/Contents/Info.plist
	rm $(ZQUEST_EXE).app/Contents/tempinfo
	cp "zq_icon.icns" $(ZQUEST_EXE).app/Contents/Resources/
	cp zquest.dat $(ZQUEST_EXE).app/
	cp sfx.dat $(ZQUEST_EXE).app/
	cp qst.dat $(ZQUEST_EXE).app/
	cp fonts.dat $(ZQUEST_EXE).app/
	cp zquest.txt $(ZQUEST_EXE).app/
	cp zscript.txt $(ZQUEST_EXE).app/
	cp std.zh $(ZQUEST_EXE).app/
	mv $(ZQUEST_EXE).app/Contents/MacOS/$(ZQUEST_EXE) "$(ZQUEST_EXE).app/Contents/MacOS/ZQuest Editor"
	mv $(ZQUEST_EXE).app "ZQuest Editor.app"
endif

aglogo$(PLATEXT).o: aglogo.cpp zc_alleg.h zdefs.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c aglogo.cpp -o aglogo$(PLATEXT).o $(SFLAG) $(WINFLAG)
colors$(PLATEXT).o: colors.cpp colors.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c colors.cpp -o colors$(PLATEXT).o $(SFLAG) $(WINFLAG)
debug$(PLATEXT).o: debug.cpp zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c debug.cpp -o debug$(PLATEXT).o $(SFLAG) $(WINFLAG)
decorations$(PLATEXT).o: decorations.cpp decorations.h jwin.h maps.h sfx.h sprite.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c decorations.cpp -o decorations$(PLATEXT).o $(SFLAG) $(WINFLAG)
defdata$(PLATEXT).o: defdata.cpp defdata.h guys.h items.h sfx.h sprite.h weapons.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c defdata.cpp -o defdata$(PLATEXT).o $(SFLAG) $(WINFLAG)
editbox$(PLATEXT).o: editbox.cpp editbox.h zc_alleg.h EditboxNew.h
	$(CC) $(OPTS) $(CFLAG) -c editbox.cpp -o editbox$(PLATEXT).o $(SFLAG) $(WINFLAG)
editbox-zq$(PLATEXT).o: editbox.cpp editbox.h zc_alleg.h EditboxNew.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c editbox.cpp -o editbox-zq$(PLATEXT).o 
EditboxModel$(PLATEXT).o: EditboxModel.cpp EditboxNew.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxModel.cpp -o EditboxModel$(PLATEXT).o $(SFLAG) $(WINFLAG)
EditboxView$(PLATEXT).o: EditboxView.cpp EditboxNew.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxView.cpp -o EditboxView$(PLATEXT).o $(SFLAG) $(WINFLAG)
ending$(PLATEXT).o: aglogo.h colors.h ending.cpp ending.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c ending.cpp -o ending$(PLATEXT).o $(SFLAG) $(WINFLAG)
ffscript$(PLATEXT).o:ffscript.cpp ffscript.h zelda.h link.h
	$(CC) $(OPTS) $(CFLAG) -c ffscript.cpp -o ffscript$(PLATEXT).o $(SFLAG) $(WINFLAG)
ffasm$(PLATEXT).o:ffasm.cpp ffasm.h
	$(CC) $(OPTS) $(CFLAG) -c ffasm.cpp -o ffasm$(PLATEXT).o $(SFLAG) $(WINFLAG)	
parser/ByteCode$(PLATEXT).o:parser/ByteCode.cpp parser/ByteCode.h parser/DataStructs.h parser/ParseError.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/ByteCode.cpp -o parser/ByteCode$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/GlobalSymbols$(PLATEXT).o:parser/GlobalSymbols.cpp parser/GlobalSymbols.h parser/ByteCode.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/GlobalSymbols.cpp -o parser/GlobalSymbols$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/lex.yy$(PLATEXT).o:parser/lex.yy.cpp parser/AST.h
	$(CC) $(OPTS) $(CFLAG) -c parser/lex.yy.cpp -o parser/lex.yy$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/ParseError$(PLATEXT).o:parser/ParseError.cpp parser/ParseError.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/ParseError.cpp -o parser/ParseError$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/ScriptParser$(PLATEXT).o:parser/ScriptParser.cpp parser/ParseError.h parser/y.tab.hpp parser/TypeChecker.h parser/GlobalSymbols.h parser/ByteCode.h parser/DataStructs.h parser/SymbolVisitors.h parser/UtilVisitors.h parser/AST.h parser/BuildVisitors.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/ScriptParser.cpp -o parser/ScriptParser$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/TypeChecker$(PLATEXT).o:parser/TypeChecker.cpp parser/TypeChecker.h parser/ParseError.h parser/GlobalSymbols.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/TypeChecker.cpp -o parser/TypeChecker$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/y.tab$(PLATEXT).o:parser/y.tab.cpp parser/AST.h
	$(CC) $(OPTS) $(CFLAG) -c parser/y.tab.cpp -o parser/y.tab$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/AST$(PLATEXT).o:parser/AST.cpp parser/AST.h
	$(CC) $(OPTS) $(CFLAG) -c parser/AST.cpp -o parser/AST$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/UtilVisitors$(PLATEXT).o:parser/UtilVisitors.h parser/ParseError.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/UtilVisitors.cpp -o parser/UtilVisitors$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/SymbolVisitors$(PLATEXT).o:parser/SymbolVisitors.cpp parser/SymbolVisitors.h parser/DataStructs.h parser/UtilVisitors.h parser/ParseError.h
	$(CC) $(OPTS) $(CFLAG) -c parser/SymbolVisitors.cpp -o parser/SymbolVisitors$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/DataStructs$(PLATEXT).o:parser/DataStructs.cpp parser/DataStructs.h zsyssimple.h
	$(CC) $(OPTS) $(CFLAG) -c parser/DataStructs.cpp -o parser/DataStructs$(PLATEXT).o $(SFLAG) $(WINFLAG)
parser/BuildVisitors$(PLATEXT).o:parser/BuildVisitors.cpp parser/BuildVisitors.h parser/ParseError.h
	$(CC) $(OPTS) $(CFLAG) -c parser/BuildVisitors.cpp -o parser/BuildVisitors$(PLATEXT).o $(SFLAG) $(WINFLAG)
font$(PLATEXT).o: font.c font.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c font.cpp -o font$(PLATEXT).o $(SFLAG) $(WINFLAG)
gamedata$(PLATEXT).o: gamedata.cpp zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c gamedata.cpp -o gamedata$(PLATEXT).o $(SFLAG) $(WINFLAG)
gui$(PLATEXT).o: colors.h debug.h gui.cpp gui.h jwin.h jwinfsel.h pal.h qst.h sfx.h tab_ctl.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c gui.cpp -o gui$(PLATEXT).o $(SFLAG) $(WINFLAG)
guys$(PLATEXT).o: aglogo.h colors.h gamedata.h guys.cpp guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c guys.cpp -o guys$(PLATEXT).o $(SFLAG) $(WINFLAG)
init$(PLATEXT).o: init.cpp jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(CFLAG) -c init.cpp -o init$(PLATEXT).o $(SFLAG) $(WINFLAG)
items$(PLATEXT).o: items.cpp items.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c items.cpp -o items$(PLATEXT).o $(SFLAG) $(WINFLAG)
jmenu$(PLATEXT).o: jmenu.cpp jwin.h sfx.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c jmenu.cpp -o jmenu$(PLATEXT).o $(SFLAG) $(WINFLAG)
jwin$(PLATEXT).o: editbox.h jwin.cpp jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c jwin.cpp -o jwin$(PLATEXT).o $(SFLAG) $(WINFLAG)
jwin-zq$(PLATEXT).o: editbox.h jwin.cpp jwin.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c jwin.cpp -o jwin-zq$(PLATEXT).o $(SFLAG) $(WINFLAG)
jwinfsel$(PLATEXT).o: jwin.h jwinfsel.cpp jwinfsel.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c jwinfsel.cpp -o jwinfsel$(PLATEXT).o $(SFLAG) $(WINFLAG)
jwinfsel-zq$(PLATEXT).o: jwin.h jwinfsel.cpp jwinfsel.h tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c jwinfsel.cpp -o jwinfsel-zq$(PLATEXT).o $(SFLAG) $(WINFLAG)
link$(PLATEXT).o: aglogo.h colors.h decorations.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.cpp link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c link.cpp -o link$(PLATEXT).o $(SFLAG) $(WINFLAG)
load_gif$(PLATEXT).o: load_gif.c load_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c load_gif.c -o load_gif$(PLATEXT).o $(SFLAG) $(WINFLAG)
maps$(PLATEXT).o: aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.cpp maps.h matrix.h pal.h particles.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c maps.cpp -o maps$(PLATEXT).o $(SFLAG) $(WINFLAG)
matrix$(PLATEXT).o: matrix.cpp matrix.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c matrix.cpp -o matrix$(PLATEXT).o $(SFLAG) $(WINFLAG)
md5$(PLATEXT).o: md5.c md5.h
	$(CC) $(OPTS) $(CFLAG) -c md5.c -o md5$(PLATEXT).o $(SFLAG) $(WINFLAG)
midi$(PLATEXT).o: midi.cpp midi.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c midi.cpp -o midi$(PLATEXT).o $(SFLAG) $(WINFLAG)
pal$(PLATEXT).o: colors.h items.h jwin.h maps.h pal.cpp pal.h sfx.h sprite.h subscr.h tab_ctl.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c pal.cpp -o pal$(PLATEXT).o $(SFLAG) $(WINFLAG)
particles$(PLATEXT).o: particles.cpp particles.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c particles.cpp -o particles$(PLATEXT).o $(SFLAG) $(WINFLAG)
qst$(PLATEXT).o: colors.h defdata.h font.h guys.h items.h jwin.h jwinfsel.h md5.h midi.h qst.cpp qst.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c qst.cpp -o qst$(PLATEXT).o $(SFLAG) $(WINFLAG)
save_gif$(PLATEXT).o: save_gif.c save_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c save_gif.c -o save_gif$(PLATEXT).o $(SFLAG) $(WINFLAG)
sprite$(PLATEXT).o: sprite.cpp sprite.h tiles.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -c sprite.cpp -o sprite$(PLATEXT).o $(SFLAG) $(WINFLAG)
subscr$(PLATEXT).o: aglogo.h colors.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.cpp subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c subscr.cpp -o subscr$(PLATEXT).o $(SFLAG) $(WINFLAG)
tab_ctl$(PLATEXT).o: tab_ctl.c tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c tab_ctl.c -o tab_ctl$(PLATEXT).o $(SFLAG) $(WINFLAG)
tab_ctl-zq$(PLATEXT).o: tab_ctl.c tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c tab_ctl.c -o tab_ctl-zq$(PLATEXT).o $(SFLAG) $(WINFLAG)
tiles$(PLATEXT).o: jwin.h tab_ctl.h tiles.cpp tiles.h zc_alleg.h zdefs.h zsys.h
	$(CC) $(OPTS) -O3 $(CFLAG) -c tiles.cpp -o tiles$(PLATEXT).o $(SFLAG) $(WINFLAG)
title$(PLATEXT).o: colors.h gamedata.h gui.h items.h jwin.h jwinfsel.h pal.h qst.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.cpp title.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c title.cpp -o title$(PLATEXT).o $(SFLAG) $(WINFLAG)
weapons$(PLATEXT).o: aglogo.h colors.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.cpp weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c weapons.cpp -o weapons$(PLATEXT).o $(SFLAG) $(WINFLAG)
zc_custom$(PLATEXT).o: jwin.h sfx.h tab_ctl.h zc_alleg.h zc_custom.cpp zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c zc_custom.cpp -o zc_custom$(PLATEXT).o $(SFLAG) $(WINFLAG)
zc_icon$(PLATEXT).o: zc_icon.rc
	windres --use-temp-file -I rc -O coff -i zc_icon.rc -o zc_icon$(PLATEXT).o
zc_init$(PLATEXT).o: gamedata.h gui.h init.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zc_init.cpp zcmusic.h zdefs.h zq_init.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_init.cpp -o zc_init$(PLATEXT).o $(SFLAG) $(WINFLAG)
zc_items$(PLATEXT).o: guys.h jwin.h sfx.h sprite.h tab_ctl.h weapons.h zc_alleg.h zc_items.cpp zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c zc_items.cpp -o zc_items$(PLATEXT).o $(SFLAG) $(WINFLAG)
zc_sprite$(PLATEXT).o: jwin.h maps.h sfx.h sprite.h tab_ctl.h tiles.h zc_alleg.h zc_sprite.cpp zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -c zc_sprite.cpp -o zc_sprite$(PLATEXT).o $(SFLAG) $(WINFLAG)
zc_subscr$(PLATEXT).o: aglogo.h colors.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_subscr.cpp zc_subscr.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_subscr.cpp -o zc_subscr$(PLATEXT).o $(SFLAG) $(WINFLAG)
zc_sys$(PLATEXT).o: aglogo.h colors.h debug.h gamedata.h gui.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h particles.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_init.h zc_sys.cpp zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zc_sys.cpp -o zc_sys$(PLATEXT).o $(SFLAG) $(WINFLAG)
zcmusic$(PLATEXT).o: zc_alleg.h zcmusic.cpp zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -c zcmusic.cpp -o zcmusic$(PLATEXT).o $(SFLAG) $(WINFLAG)
zcmusicd$(PLATEXT).o: zcmusic.h zcmusicd.cpp
	$(CC) $(OPTS) $(CFLAG) -c zcmusicd.cpp -o zcmusicd$(PLATEXT).o $(SFLAG) $(WINFLAG)
zelda$(PLATEXT).o: aglogo.h colors.h ending.h fontsdat.h gamedata.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h particles.h qst.h save_gif.h sfx.h sprite.h subscr.h tab_ctl.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.cpp zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zelda.cpp -o zelda$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_class$(PLATEXT).o: colors.h gui.h items.h jwin.h jwinfsel.h maps.h md5.h midi.h qst.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zq_class.cpp zq_class.h zq_misc.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_class.cpp -o zq_class$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_cset$(PLATEXT).o: colors.h gfxpal.h gui.h jwin.h jwinfsel.h midi.h pal.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_cset.cpp zq_cset.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_cset.cpp -o zq_cset$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_cset-zq$(PLATEXT).o: colors.h gfxpal.h gui.h jwin.h jwinfsel.h midi.h pal.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_cset.cpp zq_cset.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zq_cset.cpp -o zq_cset-zq$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_custom$(PLATEXT).o: gui.h items.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zq_custom.cpp zq_custom.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_custom.cpp -o zq_custom$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_doors$(PLATEXT).o: gui.h jwin.h tab_ctl.h tiles.h zc_alleg.h zc_sys.h zdefs.h zq_class.h zq_doors.cpp zq_misc.h zq_tiles.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_doors.cpp -o zq_doors$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_files$(PLATEXT).o: gui.h jwin.h jwinfsel.h midi.h qst.h sprite.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_files.cpp zq_files.h zq_misc.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_files.cpp -o zq_files$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_icon$(PLATEXT).o: zq_icon.rc
	windres --use-temp-file -I rc -O coff -i zq_icon.rc -o zq_icon$(PLATEXT).o
zq_init$(PLATEXT).o: gui.h init.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_init.cpp zq_init.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_init.cpp -o zq_init$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_items$(PLATEXT).o: zc_alleg.h zdefs.h zq_items.cpp
	$(CC) $(OPTS) $(CFLAG) -c zq_items.cpp -o zq_items$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_misc$(PLATEXT).o: colors.h jwin.h jwinfsel.h midi.h qst.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_misc.cpp zq_misc.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_misc.cpp -o zq_misc$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_rules$(PLATEXT).o: gui.h jwin.h jwinfsel.h midi.h sprite.h tab_ctl.h zc_alleg.h zcmusic.h zdefs.h zq_rules.cpp zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_rules.cpp -o zq_rules$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_sprite$(PLATEXT).o: sprite.h zc_alleg.h zdefs.h zq_sprite.cpp
	$(CC) $(OPTS) $(CFLAG) -c zq_sprite.cpp -o zq_sprite$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_subscr$(PLATEXT).o: gui.h items.h jwin.h jwinfsel.h midi.h sprite.h qst.h subscr.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_misc.h zq_subscr.cpp zq_subscr.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zq_subscr.cpp -o zq_subscr$(PLATEXT).o $(SFLAG) $(WINFLAG)
zq_tiles$(PLATEXT).o: colors.h gui.h jwin.h jwinfsel.h midi.h qst.h sprite.h tab_ctl.h tiles.h zc_alleg.h zc_custom.h zcmusic.h zdefs.h zq_class.h zq_misc.h zq_tiles.cpp zq_tiles.h zqscale.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zq_tiles.cpp -o zq_tiles$(PLATEXT).o $(SFLAG) $(WINFLAG)
zqscale$(PLATEXT).o: zqscale.cpp
	$(CC) $(OPTS) $(CFLAG) -c zqscale.cpp -o zqscale$(PLATEXT).o $(SFLAG) $(WINFLAG)
zquest$(PLATEXT).o: colors.h editbox.h fontsdat.h gui.h items.h jwin.h jwinfsel.h load_gif.h midi.h qst.h save_gif.h sprite.h subscr.h tab_ctl.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_cset.h zq_custom.h zq_doors.h zq_files.h zq_init.h zq_misc.h zq_rules.h zq_subscr.h zq_tiles.h zqscale.h zquest.cpp zquest.h zquestdat.h zsys.h parser/Compiler.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zquest.cpp -o zquest$(PLATEXT).o $(SFLAG) $(WINFLAG)
zsys$(PLATEXT).o: jwin.h tab_ctl.h zc_alleg.h zc_sys.h zdefs.h zsys.cpp zsys.h
	$(CC) $(OPTS) $(CFLAG) -c zsys.cpp -o zsys$(PLATEXT).o $(SFLAG) $(WINFLAG)
zsys-zq$(PLATEXT).o: jwin.h tab_ctl.h zc_alleg.h zc_sys.h zdefs.h zsys.cpp zsys.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c zsys.cpp -o zsys-zq$(PLATEXT).o $(SFLAG) $(WINFLAG)
