#include "parser.h"
#include "../overviewer.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))

char* get_block_from_index(nbt_tag* palette, int index) {
	compound_tag* block = palette->value->list_value->values[index]->value->compound_value;
	return cmpd_get_from_name(block, "Name")->value->string_value;
}

compound_tag* parse_section(compound_tag* section) {
	nbt_tag* palette = cmpd_get_from_path(section, "Palette");
	if (palette == NULL) return NULL;
	unsigned int block_length = ceil(log2(palette->value->list_value->size));
	block_length = MAX(block_length, 4);
	unsigned int per_line = floor(64.0 / block_length);
	unsigned int bits_range = (unsigned int)pow(2, block_length) - 1;
	printf("b: %d p: %d r: %d\n", block_length, per_line, bits_range);

	for (int x = 0; x < section->size; x++) {
		nbt_tag* blockstates = cmpd_get_from_path(section, "BlockStates");
		for (int i = 0; i < blockstates->value->array_length; i++) {
			unsigned long line = blockstates->value->long_array_value[i];
			printf("\n%d\t", i);
			for (int j = 0; j < per_line; j++) {
				unsigned int shift = (j*block_length);
				int palette_index = (line & ((unsigned long)bits_range << shift)) >> shift;
				printf("%s ", get_block_from_index(palette, palette_index));
				// printf("%x", shift);
			}
		}
	}
}

compound_tag* parse_chunk(unsigned char* data, int length) {
	// Better go read that! https://minecraft.gamepedia.com/Chunk_format
	compound_tag* tree = nbt_parse_tree(data, length);
	nbt_tag* sections = cmpd_get_from_path(tree, "Level.Sections");
	assert(sections != NULL);

	// There is an empty section at index 0,
	// so we start at index 1.
	for (int y = 1; y < sections->value->list_value->size; y++) {
		nbt_tag* section = sections->value->list_value->values[y];
		parse_section(section->value->compound_value);

		/*{
			nbt_tag* block_name = cmpd_get_from_path(
				palette->value->compound_value->values[0]->value->compound_value,
				"Name");
			printf("%s\n", block_name->value->string_value);
		}*/

		int j = 0;
		// while (true)
	}

	return NULL;
}