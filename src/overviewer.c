#include "overviewer.h"
#include "reader.h"
#include "render.h"

int main(void) {
	read_region_file(REGION_PATH, CHUNK_X, CHUNK_Z);
	return 0;
}
