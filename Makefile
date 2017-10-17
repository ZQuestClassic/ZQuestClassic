-include makefile.inc

AUDIO_LIBS = -lgme -lalogg -lalmp3 -laldmb -ldumb
IMAGE_LIBS = -ljpgal -lldpng -lpng -lz
LINKOPTS = -pg -g
OPTS = -pg -g
#OPTS = -O3
#COMPRESS = 1

CFLAG = -pedantic -Wno-long-long -Wall
#-W -Wshadow -Wpointer-arith

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
  LIBDIR = -L./libs/linux
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

ZELDA_EXE = zelda$(PLATEXT)$(EXEEXT)
ZQUEST_EXE = zquest$(PLATEXT)$(EXEEXT)

ZELDA_OBJECTS = aglogo$(PLATEXT).o colors$(PLATEXT).o debug$(PLATEXT).o decorations$(PLATEXT).o defdata$(PLATEXT).o editbox$(PLATEXT).o EditboxModel$(PLATEXT).o EditboxView$(PLATEXT).o ending$(PLATEXT).o gui$(PLATEXT).o guys$(PLATEXT).o items$(PLATEXT).o jwin$(PLATEXT).o jwinfsel$(PLATEXT).o link$(PLATEXT).o maps$(PLATEXT).o matrix$(PLATEXT).o midi$(PLATEXT).o pal$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o sprite$(PLATEXT).o subscr$(PLATEXT).o tab_ctl$(PLATEXT).o tiles$(PLATEXT).o title$(PLATEXT).o weapons$(PLATEXT).o zc_custom$(PLATEXT).o zc_items$(PLATEXT).o zc_sprite$(PLATEXT).o zc_sys$(PLATEXT).o zcmusic$(PLATEXT).o zelda$(PLATEXT).o zsys$(PLATEXT).o $(ZC_ICON)
ifdef COMPILE_FOR_DOS
	ZQUEST_OBJECTS = colors$(PLATEXT).o defdata$(PLATEXT).o editbox$(PLATEXT).o EditboxModel$(PLATEXT).o EditboxView$(PLATEXT).o gui$(PLATEXT).o items$(PLATEXT).o jwin$(PLATEXT).o jwinfsel$(PLATEXT).o load_gif$(PLATEXT).o midi$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o tab_ctl$(PLATEXT).o tiles$(PLATEXT).o zc_custom$(PLATEXT).o zcmusicd$(PLATEXT).o zq_class$(PLATEXT).o zq_cset$(PLATEXT).o zq_custom$(PLATEXT).o zq_doors$(PLATEXT).o zq_files$(PLATEXT).o zq_items$(PLATEXT).o zq_init$(PLATEXT).o zq_misc$(PLATEXT).o zq_rules$(PLATEXT).o zq_sprite$(PLATEXT).o zq_tiles$(PLATEXT).o zquest$(PLATEXT).o zsys$(PLATEXT).o
else
	ZQUEST_OBJECTS = colors$(PLATEXT).o defdata$(PLATEXT).o editbox-zq$(PLATEXT).o EditboxModel$(PLATEXT).o EditboxView$(PLATEXT).o gui$(PLATEXT).o items$(PLATEXT).o jwin-zq$(PLATEXT).o jwinfsel-zq$(PLATEXT).o load_gif$(PLATEXT).o midi$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o tab_ctl-zq$(PLATEXT).o tiles$(PLATEXT).o zc_custom$(PLATEXT).o zcmusicd$(PLATEXT).o zq_class$(PLATEXT).o zq_cset$(PLATEXT).o zq_custom$(PLATEXT).o zq_doors$(PLATEXT).o zq_files$(PLATEXT).o zq_items$(PLATEXT).o zq_init$(PLATEXT).o zq_misc$(PLATEXT).o zq_rules$(PLATEXT).o zq_sprite$(PLATEXT).o zq_tiles$(PLATEXT).o zqscale$(PLATEXT).o zquest$(PLATEXT).o zsys-zq$(PLATEXT).o $(ZQ_ICON)
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
	$(CC) $(OPTS) $(CFLAG) -Wall -c aglogo.cpp -o aglogo$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
colors$(PLATEXT).o: colors.cpp colors.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c colors.cpp -o colors$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
debug$(PLATEXT).o: debug.cpp zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c debug.cpp -o debug$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
decorations$(PLATEXT).o: decorations.cpp decorations.h jwin.h maps.h sfx.h sprite.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c decorations.cpp -o decorations$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
defdata$(PLATEXT).o: defdata.cpp defdata.h guys.h items.h sprite.h weapons.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c defdata.cpp -o defdata$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
editbox$(PLATEXT).o: editbox.cpp editbox.h zc_alleg.h EditboxNew.h
	$(CC) $(OPTS) $(CFLAG) -c editbox.cpp -o editbox$(PLATEXT).o $(SFLAG) $(WINFLAG)
editbox-zq$(PLATEXT).o: editbox.cpp editbox.h zc_alleg.h EditboxNew.h
	$(CC) $(OPTS) -D_ZQUEST_SCALE_ $(CFLAG) -c editbox.cpp -o editbox-zq$(PLATEXT).o
EditboxModel$(PLATEXT).o: EditboxModel.cpp EditboxNew.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxModel.cpp -o EditboxModel$(PLATEXT).o $(SFLAG) $(WINFLAG)
EditboxView$(PLATEXT).o: EditboxView.cpp EditboxNew.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -c EditboxView.cpp -o EditboxView$(PLATEXT).o $(SFLAG) $(WINFLAG)
ending$(PLATEXT).o: ending.cpp aglogo.h colors.h ending.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c ending.cpp -o ending$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
font$(PLATEXT).o: font.c font.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c font.cpp -o font$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
gui$(PLATEXT).o: gui.cpp colors.h debug.h gui.h jwin.h jwinfsel.h pal.h qst.h sfx.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c gui.cpp -o gui$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
guys$(PLATEXT).o: guys.cpp aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c guys.cpp -o guys$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
items$(PLATEXT).o: items.cpp items.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c items.cpp -o items$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jmenu$(PLATEXT).o: jmenu.cpp jwin.h sfx.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c jmenu.cpp -o jmenu$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwin$(PLATEXT).o: jwin.cpp jwin.h editbox.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c jwin.cpp -o jwin$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwin-zq$(PLATEXT).o: jwin.cpp jwin.h editbox.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -D_ZQUEST_SCALE_ -Wall -c jwin.cpp -o jwin-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwinfsel$(PLATEXT).o: jwinfsel.cpp jwin.h jwinfsel.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c jwinfsel.cpp -o jwinfsel$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwinfsel-zq$(PLATEXT).o: jwinfsel.cpp jwin.h jwinfsel.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -D_ZQUEST_SCALE_ -Wall -c jwinfsel.cpp -o jwinfsel-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
link$(PLATEXT).o: link.cpp aglogo.h colors.h decorations.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c link.cpp -o link$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
load_gif$(PLATEXT).o: load_gif.c load_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c load_gif.c -o load_gif$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
maps$(PLATEXT).o: maps.cpp aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -O3 -Wall -c maps.cpp -o maps$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
matrix$(PLATEXT).o: matrix.cpp matrix.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c matrix.cpp -o matrix$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
midi$(PLATEXT).o: midi.cpp midi.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c midi.cpp -o midi$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
pal$(PLATEXT).o: pal.cpp colors.h items.h jwin.h maps.h pal.h sfx.h sprite.h subscr.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c pal.cpp -o pal$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
particles$(PLATEXT).o: particles.cpp particles.h sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c particles.cpp -o particles$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
qst$(PLATEXT).o: qst.cpp colors.h defdata.h font.h guys.h items.h jwin.h midi.h qst.h sprite.h tiles.h weapons.h zc_alleg.h zdefs.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c qst.cpp -o qst$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
save_gif$(PLATEXT).o: save_gif.c save_gif.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c save_gif.c -o save_gif$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
sprite$(PLATEXT).o: sprite.cpp sprite.h tiles.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c sprite.cpp -o sprite$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
subscr$(PLATEXT).o: subscr.cpp aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c subscr.cpp -o subscr$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
tab_ctl$(PLATEXT).o: tab_ctl.c tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c tab_ctl.c -o tab_ctl$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
tab_ctl-zq$(PLATEXT).o: tab_ctl.c tab_ctl.h zc_alleg.h
	$(CC) $(OPTS) $(CFLAG) -D_ZQUEST_SCALE_ -Wall -c tab_ctl.c -o tab_ctl-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
tiles$(PLATEXT).o: tiles.cpp tiles.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -O3 -Wall -c tiles.cpp -o tiles$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
title$(PLATEXT).o: title.cpp colors.h gui.h items.h jwin.h jwinfsel.h pal.h qst.h sfx.h sprite.h subscr.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c title.cpp -o title$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
weapons$(PLATEXT).o: weapons.cpp aglogo.h colors.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c weapons.cpp -o weapons$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_custom$(PLATEXT).o: zc_custom.cpp zc_alleg.h zc_custom.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zc_custom.cpp -o zc_custom$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_icon$(PLATEXT).o: zc_icon.rc
	windres --use-temp-file -I rc -O coff -i zc_icon.rc -o zc_icon$(PLATEXT).o
zc_items$(PLATEXT).o: zc_items.cpp guys.h jwin.h sfx.h sprite.h weapons.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zc_items.cpp -o zc_items$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_sprite$(PLATEXT).o: zc_sprite.cpp jwin.h maps.h sfx.h sprite.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zc_sprite.cpp -o zc_sprite$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_sys$(PLATEXT).o: zc_sys.cpp aglogo.h colors.h debug.h gui.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zc_sys.cpp -o zc_sys$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zcmusic$(PLATEXT).o: zcmusic.cpp zc_alleg.h zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zcmusic.cpp -o zcmusic$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zcmusicd$(PLATEXT).o: zcmusicd.cpp zcmusic.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zcmusicd.cpp -o zcmusicd$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zelda$(PLATEXT).o: zelda.cpp aglogo.h colors.h ending.h fontsdat.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zelda.cpp -o zelda$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_class$(PLATEXT).o: zq_class.cpp colors.h items.h jwin.h maps.h midi.h qst.h sprite.h tiles.h zc_alleg.h zc_sys.h zdefs.h zq_class.h zq_misc.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_class.cpp -o zq_class$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_cset$(PLATEXT).o: zq_cset.cpp zq_cset.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_cset.cpp -o zq_cset$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_custom$(PLATEXT).o: zq_custom.cpp gui.h items.h jwin.h midi.h sprite.h tiles.h zc_alleg.h zc_custom.h zdefs.h zq_custom.h zq_misc.h zq_tiles.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_custom.cpp -o zq_custom$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_doors$(PLATEXT).o: zq_doors.cpp zq_doors.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_doors.cpp -o zq_doors$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_files$(PLATEXT).o: zq_files.cpp jwin.h midi.h qst.h tiles.h zc_alleg.h zdefs.h zq_class.h zq_files.h zq_misc.h zquest.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_files.cpp -o zq_files$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_icon$(PLATEXT).o: zq_icon.rc
	windres --use-temp-file -I rc -O coff -i zq_icon.rc -o zq_icon$(PLATEXT).o
zq_items$(PLATEXT).o: zq_items.cpp zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_items.cpp -o zq_items$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_init$(PLATEXT).o: zq_init.cpp zq_init.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_init.cpp -o zq_init$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_misc$(PLATEXT).o: zq_misc.cpp colors.h jwin.h midi.h qst.h zc_alleg.h zdefs.h zq_misc.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_misc.cpp -o zq_misc$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_rules$(PLATEXT).o: zq_rules.cpp zq_rules.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_rules.cpp -o zq_rules$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_sprite$(PLATEXT).o: zq_sprite.cpp sprite.h zc_alleg.h zdefs.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zq_sprite.cpp -o zq_sprite$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_tiles$(PLATEXT).o: zq_tiles.cpp colors.h jwin.h midi.h qst.h tiles.h zc_alleg.h zdefs.h zq_class.h zq_misc.h zq_tiles.h zquest.h
	$(CC) $(OPTS) $(CFLAG) -D_ZQUEST_SCALE_ -Wall -c zq_tiles.cpp -o zq_tiles$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zqscale$(PLATEXT).o: zqscale.cpp zqscale.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zqscale.cpp -o zqscale$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zquest$(PLATEXT).o: zquest.cpp colors.h editbox.h fontsdat.h gfxpal.h items.h jwin.h jwinfsel.h load_gif.h midi.h qst.h save_gif.h sprite.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_custom.h zq_files.h zq_misc.h zq_tiles.h zquest.h zquestdat.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -D_ZQUEST_SCALE_ -Wall -c zquest.cpp -o zquest$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zsys$(PLATEXT).o: zsys.cpp jwin.h zc_alleg.h zc_sys.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -Wall -c zsys.cpp -o zsys$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zsys-zq$(PLATEXT).o: zsys.cpp jwin.h zc_alleg.h zc_sys.h zdefs.h zsys.h
	$(CC) $(OPTS) $(CFLAG) -D_ZQUEST_SCALE_ -Wall -c zsys.cpp -o zsys-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
