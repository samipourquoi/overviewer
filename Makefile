CC = clang
CFLAGS = -Wall
SRC = 	src/overviewer.c \
		src/reader/reader.c
BIN = overviewer
LIB = -lz

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LIB) -o $(BIN)

clean:
	rm $(BIN)