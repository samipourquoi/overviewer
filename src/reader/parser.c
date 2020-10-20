#include "parser.h"

int nbt_has_tag(nbt_node* tree) {
	struct list_head* tag;
	list_for_each(tag, &tree->payload.tag_list->entry) {
		nbt_node* nbt_tag = list_entry(tag, struct nbt_list, entry)->data;
		printf("%s\n", nbt_tag->name);
	}
	return 0;
}

nbt_node* parse_chunk(unsigned char* data, int length) {
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
}
