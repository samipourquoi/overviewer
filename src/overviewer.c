#include <stdlib.h>
#include "overviewer.h"
#include "parse.h"
#include "render.h"

int main(void) {
	srand(0xDEADBEEF);
	read_region_file(REGION_PATH, CHUNK_X, CHUNK_Z);
	return 0;
}
