# -------- variables and definitions --------------
LIB = lib/libgif.a
INC = src/load_gif.h src/save_gif.h
OBJ = obj/

OBJS = obj/load_gif.o obj/save_gif.o
SRC = src/

LIBDEST = $(DJDIR)/$(LIB)
INCDEST = $(DJDIR)/include/
CPLIBDEST = $(subst /,\,$(LIBDEST))
CPINCDEST = $(subst /,\,$(INCDEST))
RM = rm -v -f
FLAGS =-m386 -O3

# -------- Rules start here -----------------------

all : msg $(LIB) test
	@echo .
	@echo Fine, now type make install to install the library and header files to your
	@echo DJGPPs subdirectories.
	@echo .
	@echo You may now enter the test directory and type test.exe too.

install : $(LIBDEST) $(INCDEST)
	@echo .
	@echo If you would like to delete this lib, type first: make uninstall to remove all
	@echo files copied to your djgpp directory, then erase the gif directory
	@echo .

$(LIBDEST) : $(LIB)
	@echo Copying lib to DJGPPs directory...
	copy lib\libgif.a $(CPLIBDEST)

$(INCDEST): $(INC)
	copy src\load_gif.h $(CPINCDEST)load_gif.h
	copy src\save_gif.h $(CPINCDEST)save_gif.h

uninstall :
	$(RM) $(LIBDEST)
	$(RM) $(INCDEST)load_gif.h
	$(RM) $(INCDEST)save_gif.h

$(LIB) : $(OBJS)
	@echo .
	@echo Linking all the lib together in one file...
	ar rs $(LIB) $(OBJS)

$(OBJ)%.o : $(SRC)%.c
	gcc $(FLAGS) -c $< -o $@

test : test/test.exe

test/test.exe : test/test.c $(LIB)
	gcc $(FLAGS) -o test/test.exe test/test.c $(LIB) -lalleg -s

clean:
	$(RM) obj/*.o lib/*.a test/*.exe

msg :
	@echo Ok, I will try to create the library. Please wait...
