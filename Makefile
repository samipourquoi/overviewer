all: overviewer

.PHONY: overviewer
overviewer:
	cd src && $(MAKE)

install:
	cd thirdparty && $(MAKE)

uninstall:
	cd thirdparty && $(MAKE) clean

clean:
	rm -f overviewer
	cd src && $(MAKE) clean
