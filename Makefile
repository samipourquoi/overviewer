CC = clang
CFLAGS = -Wall
SRC = 	src/overviewer.c \
		src/nbt/nbt.c
BIN = overviewer
LIB = -lz

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIB) -o $(BIN)

clean:
	rm $(BIN)

install:
	git submodule update --init --recursive
	make -C libs/cNBT libnbt.a