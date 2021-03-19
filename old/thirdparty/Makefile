CC = gcc
CFLAGS = -w

libs: clean parson.o hashtable.o assets
	ar rvs libdeps.a parson.o hashtable.o

install: parson_install hashtable_install assets

parson.o: parson_install
	$(CC) $(CFLAGS) -c parson/parson.c

hashtable.o: hashtable_install
	$(CC) $(CFLAGS) -c hashtable/hashtable.c

assets:
	git clone -b assets --single-branch https://github.com/samipourquoi/overviewer.git ../assets

parson_install:
	git clone https://github.com/kgabis/parson.git

hashtable_install:
	git clone https://github.com/goldsborough/hashtable.git

clean:
	rm -f *.o *.a
	rm -rf ../assets parson hashtable