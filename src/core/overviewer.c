#include <stdio.h>
#include <stdlib.h>
#include "overviewer.h"
#include "parse.h"
#include "render.h"
#include "models.h"
#include "assets.h"

int main(void) {
	srand(0xDEADBEEF);
	assets_init();
	read_region_file(REGION_PATH, CHUNK_X, CHUNK_Z);
	ht_destroy(blockstates_list);
	free(blockstates_list);
	return 0;
}

void core_start() {
	srand(0xDEADBEEF);
	assets_init();
}

void core_end() {
	ht_destroy(blockstates_list);
	free(blockstates_list);
}