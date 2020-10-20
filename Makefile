CC = clang
CFLAGS = -Wall
SRC = 	src/overviewer.c \
		src/reader/reader.c
BIN = overviewer
LIB = -lz -Ilibs/cNBT -Llibs/cNBT -lnbt

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIB) -o $(BIN)

clean:
	rm $(BIN)

install:
	git submodule update --init --recursive
	make -C libs/cNBT libnbt.a