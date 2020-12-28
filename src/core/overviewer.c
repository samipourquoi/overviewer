#include <stdio.h>
#include <stdlib.h>
#include "overviewer.h"
#include "chunks.h"
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
	chunks_init_db();
	chunks_set_at(0, 0, "sami", 5);
}

void core_end() {
	ht_destroy(blockstates_list);
	free(blockstates_list);
}