#include "overviewer.h"
#include "reader/reader.h"

int main(void) {
	read_region_file(REGION_PATH, CHUNK_X, CHUNK_Z);
	return 0;
}
