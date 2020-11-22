CC = gcc
CFLAGS = -w
BIN = overviewer
LIB = -lz -lcairo

all: $(BIN)

$(BIN): overviewer.o reader.o render.o nbt.o
	$(CC) $(CFLAGS) $(LIB) overviewer.o reader.o render.o nbt.o -o $(BIN)

overviewer.o: src/overviewer.c src/overviewer.h
	$(CC) $(CFLAGS) -c src/overviewer.c

reader.o: src/reader.c src/reader.h
	$(CC) $(CFLAGS) -c src/reader.c

render.o: src/render.c src/render.h
	$(CC) $(CFLAGS) -c src/render.c -o render.o

nbt.o: src/nbt.c src/nbt.h
	$(CC) $(CFLAGS) -c src/nbt.c

clean:
	rm $(BIN) *.o

install:
	git clone -b assets --single-branch https://github.com/samipourquoi/overviewer.git assets