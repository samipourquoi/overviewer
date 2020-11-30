CC = gcc
CFLAGS = -Wfatal-errors -g $(HEADERS)
HEADERS = -I../thirdparty/parson -I../thirdparty/hashtable
LIB = -lz -lcairo -L../thirdparty/ -ldeps

all: overviewer.o parse.o render.o models.o assets.o nbt.o
	$(CC) $(CFLAGS) $(LIB) *.o -o ../overviewer

overviewer.o: overviewer.c overviewer.h
	$(CC) $(CFLAGS) -c overviewer.c

parse.o: parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

render.o: render.c render.h
	$(CC) $(CFLAGS) -c render.c

models.o: models.c models.h
	$(CC) $(CFLAGS) -c models.c

assets.o: assets.c assets.h
	$(CC) $(CFLAGS) -c assets.c

nbt.o: nbt.c nbt.h
	$(CC) $(CFLAGS) -c nbt.c

clean:
	rm -f *.o