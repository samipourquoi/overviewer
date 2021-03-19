#include <stdlib.h>
#include "core.h"
#include "chunks.h"
#include "assets.h"

void core_start() {
	srand(0xDEADBEEF);
	assets_init();
	chunks_init_db();
}

void core_end() {
	ht_destroy(blockstates_list);
	free(blockstates_list);
}