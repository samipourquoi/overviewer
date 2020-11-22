CC = clang
CFLAGS = -Wall
SRC = 	src/overviewer.c \
		src/nbt/nbt.c \
		src/nbt/compound.c \
		src/reader/reader.c \
		src/reader/parser.c \
		src/render.c
BIN = overviewer
LIB = -lz -lcairo

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIB) -o $(BIN)

clean:
	rm $(BIN)

install:
	git clone -b assets --single-branch https://github.com/samipourquoi/overviewer.git assets