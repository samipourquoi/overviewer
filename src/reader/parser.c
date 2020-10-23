#include "parser.h"
#include <stdio.h>

compound_tag* parse_chunk(unsigned char* data, int length) {
/*
	nbt_node* chunk = nbt_parse(data, length);
	nbt_node* sections = nbt_find_by_name(chunk, "Sections");

	// Loops through sub-chunks, 'sections'
	struct list_head* section;
	list_for_each(section, &sections->payload.tag_list->entry) {
		nbt_node* nbt_section = list_entry(section, struct nbt_list, entry)->data;
		nbt_node* nbt_palette = nbt_find_by_name(nbt_section, "Palette");

		if (nbt_palette == NULL) continue;

		struct list_head* block_type;
		list_for_each(block_type, &nbt_palette->payload.tag_compound->entry) {
			nbt_node* nbt_block_type = list_entry(block_type, struct nbt_list, entry)->data;
			printf("%s\n", );
		}
		printf("\n");
	}
 */
	// nbt_parse_tree(data, length);

    // Debugging
	FILE* file = fopen("test.nbt", "rb");
	unsigned char buf[0x4c];
	fread(buf, 1, sizeof buf, file);
	nbt_parse_tree(buf, sizeof buf);
	fclose(file);
	return NULL;
}