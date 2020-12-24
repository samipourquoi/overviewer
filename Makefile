CC = gcc
CFLAGS = -Wfatal-errors -g $(HEADERS)
HEADERS = -Ithirdparty/parson -Ithirdparty/hashtable
LIB = -lz -lcairo -Lthirdparty/ -ldeps
CORE = build/core

all: builddir core

builddir:
	mkdir -p $(CORE)

core: assets.o models.o nbt.o overviewer.o parse.o render.o
	$(CC) $(CFLAGS) $(LIB) $(CORE)/*.o -o overviewer

%.o: src/core/%.c src/core/%.h
	$(CC) -c $(CFLAGS) $< -o $(CORE)/$@

install:
	cd thirdparty && $(MAKE)

uninstall:
	cd thirdparty && $(MAKE) clean

clean:
	rm -f overviewer
	rm -rf build
