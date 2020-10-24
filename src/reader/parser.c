#include "parser.h"
#include "../overviewer.h"
#include <stdio.h>

compound_tag* parse_chunk(unsigned char* data, int length) {
	compound_tag* tree = nbt_parse_tree(data, length);
	nbt_tag* data_version = cmpd_get_from_path(tree, "DataVersion");
	return NULL;
}