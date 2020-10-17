CC = clang
CFLAGS = -Wall
SRC = src/overviewer.c
BIN = overviewer

all: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(BIN)

clean:
	rm $(BIN)