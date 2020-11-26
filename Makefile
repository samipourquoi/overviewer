CC = gcc
CFLAGS = -Wfatal-errors -g
BIN = overviewer
LIB = -lz -lcairo

all: $(BIN)

$(BIN): overviewer.o reader.o render.o nbt.o parson.o
	$(CC) $(CFLAGS) $(LIB) *.o -o $(BIN)

overviewer.o: src/overviewer.c src/overviewer.h
	$(CC) $(CFLAGS) -c src/overviewer.c

reader.o: src/reader.c src/reader.h
	$(CC) $(CFLAGS) -c src/reader.c

render.o: src/render.c src/render.h
	$(CC) $(CFLAGS) -c src/render.c -o render.o

nbt.o: src/nbt.c src/nbt.h
	$(CC) $(CFLAGS) -c src/nbt.c

parson.o: parson/parson.c parson/parson.h
	$(CC) $(CFLAGS) -c parson/parson.c

clean:
	rm -f $(BIN) *.o

install:
	git clone -b assets --single-branch https://github.com/samipourquoi/overviewer.git assets
	git clone https://github.com/kgabis/parson.git

uninstall:
	rm -rf ./assets
	rm -rf ./parson