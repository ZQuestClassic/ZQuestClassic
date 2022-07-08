CFLAGS = -Isrc -Wall -Wextra
LDLIBS = `pkg-config allegro_monolith-debug-static-5 --libs --static`
.PHONY: all
all: examples/example
examples/example: examples/example.o src/gif.o src/lzw.o src/bitmap.o src/algif.o
index.html: readme.md
	cp readme.md index.html
algif5.zip: index.html
	mkdir algif5
	cp Makefile index.html algif5/
	mkdir algif5/examples
	cp examples/*.c algif5/examples
	cp examples/*.gif algif5/examples
	mkdir algif5/src
	cp src/*.c algif5/src
	cp src/*.h algif5/src
	zip -r algif5.zip algif5
	rm -r algif5

clean:
	${RM} src/*.o
	${RM} examples/*.o
	${RM} examples/example
	${RM} allegro.log
	${RM} index.html
	${RM} algif5.zip
