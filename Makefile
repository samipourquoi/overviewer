CC = gcc
CFLAGS = -Wfatal-errors -g $(HEADERS)
HEADERS = -Ithirdparty/parson -Ithirdparty/hashtable
LIB = -lz -lcairo -Lthirdparty/ -ldeps
CORE = build/core
SERVER = build/server
OS = $(shell uname -s | tr A-Z a-z)
LIBEXT = so

ifeq ($(OS),darwin)
	LIBEXT = dylib
else ifeq ($(OS),mac)
	LIBEXT = dylib
endif

all: builddir core server

builddir:
	mkdir -p $(CORE)
	mkdir -p $(SERVER)

server:
	npx tsc

core: assets.o models.o nbt.o overviewer.o parse.o render.o
	$(CC) $(CFLAGS) $(LIB) $(CORE)/*.o -shared -o $(CORE)/libcore.$(LIBEXT)

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
