CC = gcc
CFLAGS = -Wfatal-errors -g $(HEADERS)
HEADERS = -Ithirdparty/parson -Ithirdparty/hashtable
LIB = -lz -lcairo -Lthirdparty/ -ldeps -lm -llmdb
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

core: $(CORE)/assets.o $(CORE)/models.o $(CORE)/nbt.o $(CORE)/overviewer.o $(CORE)/chunks.o $(CORE)/render.o
	$(CC) $(CFLAGS) $(LIB) $(CORE)/*.o -shared -o $(CORE)/libcore.$(LIBEXT)

$(CORE)/%.o: src/core/%.c src/core/%.h
	$(CC) -c $(CFLAGS) $< -o $@

install:
	cd thirdparty && $(MAKE)
	npm install

uninstall:
	cd thirdparty && $(MAKE) clean

clean:
	rm -f overviewer
	rm -rf build
