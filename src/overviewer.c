#include <stdio.h>
#include <stdlib.h>
#include "overviewer.h"
#include "parse.h"
#include "render.h"
#include "models.h"

int main(void) {
	srand(0xDEADBEEF);
	models_init();
	read_region_file(REGION_PATH, CHUNK_X, CHUNK_Z);
	return 0;
}
