#ifndef OVERVIEWER_COMPOUND_H
#define OVERVIEWER_COMPOUND_H

struct compound_tag {
	int size;
	char* keys;
	struct nbt_tag* values;
};
typedef struct compound_tag compound_tag;

struct nbt_tag* cp_get_tag_from_name(struct compound_tag* tag, char* name);

#endif
