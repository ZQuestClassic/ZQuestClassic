-include makefile.inc

AUDIO_LIBS = -lalspc -lalogg -lalmp3 -laldmb -ldumb
#LINKOPTS = -pg -g
#OPTS = -pg -g
#OPTS = -O3
#COMPRESS = 1

ifdef COMPILE_FOR_WIN
  ALLEG_LIB = -lalleg
  SFLAG = -s
  WINFLAG = -mwindows
  PLATEXT = -w
  EXEEXT = .exe
  ZC_ICON = zc_icon$(PLATEXT).o
  ZQ_ICON = zq_icon$(PLATEXT).o
  ZC_PLATFORM = Windows
  MUSIC_FILES = zcmusic$(PLATEXT).o zcmusicd$(PLATEXT).o zcmusic.def
else
ifdef COMPILE_FOR_LINUX
  PLATEXT = -l
  ALLEG_LIB = `allegro-config --libs`
  ZC_PLATFORM = Linux
else
ifdef COMPILE_FOR_DOS
  ALLEG_LIB = -lalleg
  STDCXX_LIB = -lstdcxx
  EXEEXT = .exe
  ZC_PLATFORM = DOS
else
ifdef COMPILE_FOR_MACOSX
  PLATEXT = -m
  ALLEG_LIB = `allegro-config --libs`
  ZC_PLATFORM = Mac OS X
  LDFLAG = -Wno-long-double
endif
endif
endif
endif

ZELDA_EXE = zelda$(PLATEXT)$(EXEEXT)
ZQUEST_EXE = zquest$(PLATEXT)$(EXEEXT)

ZELDA_OBJECTS = aglogo$(PLATEXT).o colors$(PLATEXT).o debug$(PLATEXT).o decorations$(PLATEXT).o defdata$(PLATEXT).o editbox$(PLATEXT).o ending$(PLATEXT).o gui$(PLATEXT).o guys$(PLATEXT).o items$(PLATEXT).o jwin$(PLATEXT).o jwinfsel$(PLATEXT).o link$(PLATEXT).o maps$(PLATEXT).o matrix$(PLATEXT).o midi$(PLATEXT).o pal$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o sprite$(PLATEXT).o subscr$(PLATEXT).o tab_ctl$(PLATEXT).o tiles$(PLATEXT).o title$(PLATEXT).o weapons$(PLATEXT).o zc_custom$(PLATEXT).o zc_items$(PLATEXT).o zc_sprite$(PLATEXT).o zc_sys$(PLATEXT).o zcmusic$(PLATEXT).o zelda$(PLATEXT).o zsys$(PLATEXT).o $(ZC_ICON)
ifdef COMPILE_FOR_DOS
	ZQUEST_OBJECTS = colors$(PLATEXT).o defdata$(PLATEXT).o editbox$(PLATEXT).o gui$(PLATEXT).o items$(PLATEXT).o jwin$(PLATEXT).o jwinfsel$(PLATEXT).o load_gif$(PLATEXT).o midi$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o tab_ctl$(PLATEXT).o tiles$(PLATEXT).o zc_custom$(PLATEXT).o zcmusicd$(PLATEXT).o zq_class$(PLATEXT).o zq_cset$(PLATEXT).o zq_custom$(PLATEXT).o zq_doors$(PLATEXT).o zq_files$(PLATEXT).o zq_items$(PLATEXT).o zq_init$(PLATEXT).o zq_misc$(PLATEXT).o zq_rules$(PLATEXT).o zq_sprite$(PLATEXT).o zq_tiles$(PLATEXT).o zquest$(PLATEXT).o zsys$(PLATEXT).o
else
	ZQUEST_OBJECTS = colors$(PLATEXT).o defdata$(PLATEXT).o editbox-zq$(PLATEXT).o gui$(PLATEXT).o items$(PLATEXT).o jwin-zq$(PLATEXT).o jwinfsel-zq$(PLATEXT).o load_gif$(PLATEXT).o midi$(PLATEXT).o particles$(PLATEXT).o qst$(PLATEXT).o save_gif$(PLATEXT).o sprite$(PLATEXT).o tab_ctl-zq$(PLATEXT).o tiles$(PLATEXT).o zc_custom$(PLATEXT).o zcmusicd$(PLATEXT).o zq_class$(PLATEXT).o zq_cset$(PLATEXT).o zq_custom$(PLATEXT).o zq_doors$(PLATEXT).o zq_files$(PLATEXT).o zq_items$(PLATEXT).o zq_init$(PLATEXT).o zq_misc$(PLATEXT).o zq_rules$(PLATEXT).o zq_sprite$(PLATEXT).o zq_tiles$(PLATEXT).o zqscale$(PLATEXT).o zquest$(PLATEXT).o zsys-zq$(PLATEXT).o $(ZQ_ICON)
endif

.PHONY: default veryclean clean all msg dos win windows linux test done

default: all
msg:
	@echo Compiling Zelda Classic for $(ZC_PLATFORM)...
done:
	@echo Done!
clean:
	rm -f $(ZELDA_OBJECTS) $(ZCMUSIC_O) $(ZQUEST_OBJECTS)
veryclean: clean
	rm -f $(ZELDA_EXE) $(ZQUEST_EXE) $(MUSIC_LIB)

test:
ifndef COMPILE_FOR_WIN
ifndef COMPILE_FOR_DOS
ifndef COMPILE_FOR_LINUX
ifndef COMPILE_FOR_MACOSX
	#change this if you want to change the default platform
	@make macosx
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

all: test msg $(ZELDA_EXE) $(ZQUEST_EXE) done

$(ZELDA_EXE): $(ZELDA_OBJECTS) $(ZCMUSIC_O)
	g++ $(LINKOPTS) -o $(ZELDA_EXE) $(ZELDA_OBJECTS) $(ZCMUSIC_O) $(AUDIO_LIBS) $(ALLEG_LIB) $(STDCXX_LIB) $(ZC_ICON) $(SFLAG) $(WINFLAG) $(LDFLAG)
ifdef COMPRESS
	upx --best $(ZELDA_EXE)
endif
ifdef COMPILE_FOR_MACOSX
	fixbundle $(ZELDA_EXE)
	cp Info1.plist $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>$(ZELDA_EXE)</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZELDA_EXE).app/Contents/tempinfo
	echo '	<string>$(ZELDA_EXE).icns</string>' >> $(ZELDA_EXE).app/Contents/tempinfo
	cat $(ZELDA_EXE).app/Contents/tempinfo Info2.plist > $(ZELDA_EXE).app/Contents/Info.plist
	rm $(ZELDA_EXE).app/Contents/tempinfo
	cp $(ZELDA_EXE).icns $(ZELDA_EXE).app/Contents/Resources/
endif

$(ZQUEST_EXE): $(ZQUEST_OBJECTS) zquest.dat
	g++ $(LINKOPTS) -o $(ZQUEST_EXE) $(ZQUEST_OBJECTS) $(ZCMUSICD_O) $(ALLEG_LIB) $(STDCXX_LIB) $(ZQ_ICON) $(SFLAG) $(WINFLAG) $(LDFLAG)
ifdef COMPRESS
	upx --best $(ZQUEST_EXE)
endif
ifdef COMPILE_FOR_MACOSX
	fixbundle $(ZQUEST_EXE) -d zquest.dat
	chmod 755 $(ZQUEST_EXE)
	cp Info1.plist $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleExecutable</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<string>$(ZQUEST_EXE)</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<key>CFBundleIconFile</key>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	echo '	<string>$(ZQUEST_EXE).icns</string>' >> $(ZQUEST_EXE).app/Contents/tempinfo
	cat $(ZQUEST_EXE).app/Contents/tempinfo Info2.plist > $(ZQUEST_EXE).app/Contents/Info.plist
	rm $(ZQUEST_EXE).app/Contents/tempinfo
	cp $(ZQUEST_EXE).icns $(ZQUEST_EXE).app/Contents/Resources/
endif


aglogo$(PLATEXT).o: aglogo.cpp zc_alleg.h zdefs.h zeldadat.h	
	g++ $(OPTS) -Wall -c aglogo.cpp -o aglogo$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
colors$(PLATEXT).o: colors.cpp colors.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c colors.cpp -o colors$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
debug$(PLATEXT).o: debug.cpp zc_alleg.h	
	g++ $(OPTS) -Wall -c debug.cpp -o debug$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
decorations$(PLATEXT).o: decorations.cpp decorations.h jwin.h maps.h sfx.h sprite.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c decorations.cpp -o decorations$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
defdata$(PLATEXT).o: defdata.cpp defdata.h guys.h items.h sprite.h weapons.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c defdata.cpp -o defdata$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
editbox$(PLATEXT).o: editbox.c eb_intern.h editbox.h zc_alleg.h	
	g++ $(OPTS) -Wall -c editbox.c -o editbox$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
editbox-zq$(PLATEXT).o: editbox.c eb_intern.h editbox.h zc_alleg.h	
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c editbox.c -o editbox-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
ending$(PLATEXT).o: ending.cpp aglogo.h colors.h ending.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c ending.cpp -o ending$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
font$(PLATEXT).o: font.c font.h zc_alleg.h	
	g++ $(OPTS) -Wall -c font.cpp -o font$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
gui$(PLATEXT).o: gui.cpp colors.h debug.h gui.h jwin.h jwinfsel.h pal.h qst.h sfx.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c gui.cpp -o gui$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
guys$(PLATEXT).o: guys.cpp aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c guys.cpp -o guys$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
items$(PLATEXT).o: items.cpp items.h sprite.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c items.cpp -o items$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jmenu$(PLATEXT).o: jmenu.cpp jwin.h sfx.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h	
	g++ $(OPTS) -Wall -c jmenu.cpp -o jmenu$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwin$(PLATEXT).o: jwin.c jwin.h zc_alleg.h	
	g++ $(OPTS) -Wall -c jwin.c -o jwin$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwin-zq$(PLATEXT).o: jwin.c jwin.h zc_alleg.h	
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c jwin.c -o jwin-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwinfsel$(PLATEXT).o: jwinfsel.c jwin.h jwinfsel.h zc_alleg.h
	g++ $(OPTS) -Wall -c jwinfsel.c -o jwinfsel$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
jwinfsel-zq$(PLATEXT).o: jwinfsel.c jwin.h jwinfsel.h zc_alleg.h
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c jwinfsel.c -o jwinfsel-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
link$(PLATEXT).o: link.cpp aglogo.h colors.h decorations.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c link.cpp -o link$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
load_gif$(PLATEXT).o: load_gif.c load_gif.h zc_alleg.h	
	g++ $(OPTS) -Wall -c load_gif.c -o load_gif$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
maps$(PLATEXT).o: maps.cpp aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -O3 -Wall -c maps.cpp -o maps$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
matrix$(PLATEXT).o: matrix.cpp matrix.h zc_alleg.h	
	g++ $(OPTS) -Wall -c matrix.cpp -o matrix$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
midi$(PLATEXT).o: midi.cpp midi.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c midi.cpp -o midi$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
pal$(PLATEXT).o: pal.cpp colors.h items.h jwin.h maps.h pal.h sfx.h sprite.h subscr.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c pal.cpp -o pal$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
particles$(PLATEXT).o: particles.cpp particles.h sprite.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c particles.cpp -o particles$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
qst$(PLATEXT).o: qst.cpp colors.h defdata.h font.h guys.h items.h jwin.h midi.h qst.h sprite.h tiles.h weapons.h zc_alleg.h zdefs.h zquest.h zsys.h	
	g++ $(OPTS) -Wall -c qst.cpp -o qst$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
save_gif$(PLATEXT).o: save_gif.c save_gif.h zc_alleg.h	
	g++ $(OPTS) -Wall -c save_gif.c -o save_gif$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
sprite$(PLATEXT).o: sprite.cpp sprite.h tiles.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c sprite.cpp -o sprite$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
subscr$(PLATEXT).o: subscr.cpp aglogo.h colors.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c subscr.cpp -o subscr$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
tab_ctl$(PLATEXT).o: tab_ctl.c tab_ctl.h zc_alleg.h
	g++ $(OPTS) -Wall -c tab_ctl.c -o tab_ctl$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
tab_ctl-zq$(PLATEXT).o: tab_ctl.c tab_ctl.h zc_alleg.h
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c tab_ctl.c -o tab_ctl-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
tiles$(PLATEXT).o: tiles.cpp tiles.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -O3 -Wall -c tiles.cpp -o tiles$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
title$(PLATEXT).o: title.cpp colors.h gui.h items.h jwin.h jwinfsel.h pal.h qst.h sfx.h sprite.h subscr.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c title.cpp -o title$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
weapons$(PLATEXT).o: weapons.cpp aglogo.h colors.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h tiles.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c weapons.cpp -o weapons$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_custom$(PLATEXT).o: zc_custom.cpp zc_alleg.h zc_custom.h zdefs.h	
	g++ $(OPTS) -Wall -c zc_custom.cpp -o zc_custom$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_icon$(PLATEXT).o: zc_icon.rc	
	windres --use-temp-file -I rc -O coff -i zc_icon.rc -o zc_icon$(PLATEXT).o
zc_items$(PLATEXT).o: zc_items.cpp guys.h jwin.h sfx.h sprite.h weapons.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h	
	g++ $(OPTS) -Wall -c zc_items.cpp -o zc_items$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
#zc_music$(PLATEXT).o: zc_music.cpp zc_alleg.h zc_music.h	
#	g++ $(OPTS) -Wall -c zc_music.cpp -o zc_music$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_sprite$(PLATEXT).o: zc_sprite.cpp jwin.h maps.h sfx.h sprite.h tiles.h zc_alleg.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h	
	g++ $(OPTS) -Wall -c zc_sprite.cpp -o zc_sprite$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zc_sys$(PLATEXT).o: zc_sys.cpp aglogo.h colors.h debug.h gui.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h midi.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c zc_sys.cpp -o zc_sys$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zcmusic$(PLATEXT).o: zcmusic.cpp zc_alleg.h zcmusic.h	
	g++ $(OPTS) -Wall -c zcmusic.cpp -o zcmusic$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zcmusicd$(PLATEXT).o: zcmusicd.cpp zcmusic.h	
	g++ $(OPTS) -Wall -c zcmusicd.cpp -o zcmusicd$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zelda$(PLATEXT).o: zelda.cpp aglogo.h colors.h ending.h fontsdat.h guys.h items.h jwin.h jwinfsel.h link.h maps.h matrix.h pal.h qst.h save_gif.h sfx.h sprite.h subscr.h tiles.h title.h weapons.h zc_alleg.h zc_custom.h zc_sys.h zcmusic.h zdefs.h zelda.h zeldadat.h zsys.h	
	g++ $(OPTS) -Wall -c zelda.cpp -o zelda$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_class$(PLATEXT).o: zq_class.cpp colors.h items.h jwin.h maps.h midi.h qst.h sprite.h tiles.h zc_alleg.h zc_sys.h zdefs.h zq_class.h zq_misc.h zquest.h zquestdat.h zsys.h	
	g++ $(OPTS) -Wall -c zq_class.cpp -o zq_class$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_cset$(PLATEXT).o: zq_cset.cpp zq_cset.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c zq_cset.cpp -o zq_cset$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_custom$(PLATEXT).o: zq_custom.cpp gui.h items.h jwin.h midi.h sprite.h tiles.h zc_alleg.h zc_custom.h zdefs.h zq_custom.h zq_misc.h zq_tiles.h zquest.h zsys.h	
	g++ $(OPTS) -Wall -c zq_custom.cpp -o zq_custom$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_doors$(PLATEXT).o: zq_doors.cpp zq_doors.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c zq_doors.cpp -o zq_doors$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_files$(PLATEXT).o: zq_files.cpp jwin.h midi.h qst.h tiles.h zc_alleg.h zdefs.h zq_class.h zq_files.h zq_misc.h zquest.h zsys.h	
	g++ $(OPTS) -Wall -c zq_files.cpp -o zq_files$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_icon$(PLATEXT).o: zq_icon.rc	
	windres --use-temp-file -I rc -O coff -i zq_icon.rc -o zq_icon$(PLATEXT).o
zq_items$(PLATEXT).o: zq_items.cpp zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c zq_items.cpp -o zq_items$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_init$(PLATEXT).o: zq_init.cpp zq_init.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c zq_init.cpp -o zq_init$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_misc$(PLATEXT).o: zq_misc.cpp colors.h jwin.h midi.h qst.h zc_alleg.h zdefs.h zq_misc.h zquest.h zquestdat.h zsys.h	
	g++ $(OPTS) -Wall -c zq_misc.cpp -o zq_misc$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_rules$(PLATEXT).o: zq_rules.cpp zq_rules.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c zq_rules.cpp -o zq_rules$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_sprite$(PLATEXT).o: zq_sprite.cpp sprite.h zc_alleg.h zdefs.h	
	g++ $(OPTS) -Wall -c zq_sprite.cpp -o zq_sprite$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zq_tiles$(PLATEXT).o: zq_tiles.cpp colors.h jwin.h midi.h qst.h tiles.h zc_alleg.h zdefs.h zq_class.h zq_misc.h zq_tiles.h zquest.h	
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c zq_tiles.cpp -o zq_tiles$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zqscale$(PLATEXT).o: zqscale.cpp zqscale.h
	g++ $(OPTS) -Wall -c zqscale.cpp -o zqscale$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zquest$(PLATEXT).o: zquest.cpp colors.h fontsdat.h gfxpal.h items.h jwin.h jwinfsel.h load_gif.h midi.h qst.h save_gif.h sprite.h tiles.h zc_alleg.h zcmusic.h zdefs.h zq_class.h zq_custom.h zq_files.h zq_misc.h zq_tiles.h zquest.h zquestdat.h zsys.h	
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c zquest.cpp -o zquest$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zsys$(PLATEXT).o: zsys.cpp jwin.h zc_alleg.h zc_sys.h zdefs.h zsys.h	
	g++ $(OPTS) -Wall -c zsys.cpp -o zsys$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
zsys-zq$(PLATEXT).o: zsys.cpp jwin.h zc_alleg.h zc_sys.h zdefs.h zsys.h	
	g++ $(OPTS) -D_ZQUEST_SCALE_ -Wall -c zsys.cpp -o zsys-zq$(PLATEXT).o $(SFLAG) $(WINFLAG) $(LDFLAG)
