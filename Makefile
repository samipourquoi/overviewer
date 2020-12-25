CC = gcc
CFLAGS = -Wfatal-errors -g $(HEADERS)
HEADERS = -Ithirdparty/parson -Ithirdparty/hashtable
LIB = -lz -lcairo -Lthirdparty/ -ldeps
CORE = build/core
SERVER = build/server

all: builddir core server

builddir:
	mkdir -p $(CORE)
	mkdir -p $(SERVER)

server:
	npx tsc

core: assets.o models.o nbt.o overviewer.o parse.o render.o
	# TODO: Make the shared library file extension not OS-dependent
	$(CC) $(CFLAGS) $(LIB) $(CORE)/*.o -shared -o $(CORE)/libcore.dylib

%.o: src/core/%.c src/core/%.h
	$(CC) -c $(CFLAGS) $< -o $(CORE)/$@

install:
	cd thirdparty && $(MAKE)
	npm install

uninstall:
	cd thirdparty && $(MAKE) clean

clean:
	rm -f overviewer
	rm -rf build
