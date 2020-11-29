CC = gcc
CFLAGS = -w

libs: clean parson.o strmap.o assets
	ar rvs libdeps.a parson.o strmap.o

install: parson_install strmap_install assets

parson.o: parson_install
	$(CC) $(CFLAGS) -c parson/parson.c

strmap.o: strmap_install
	$(CC) $(CFLAGS) -c hashtable/strmap.c

assets:
	git clone -b assets --single-branch https://github.com/samipourquoi/overviewer.git ../assets

parson_install:
	git clone https://github.com/kgabis/parson.git

strmap_install:
	mkdir hashtable
	curl http://pokristensson.com/code/strmap/strmap.c > hashtable/strmap.c
	curl http://pokristensson.com/code/strmap/strmap.h > hashtable/strmap.h

clean:
	rm -f *.o *.a
	rm -rf assets parson hashtable