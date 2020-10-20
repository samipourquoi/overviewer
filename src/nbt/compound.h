#ifndef OVERVIEWER_COMPOUND_H
#define OVERVIEWER_COMPOUND_H

#define COMPOUND_LENGTH
#include "nbt.h"

struct compound_tag {
	int size;
	struct nbt_tag* values[COMPOUND_LENGTH];
};
typedef struct compound_tag compound_tag;

struct nbt_tag* get_tag_from_name(struct compound_tag* tag, char* name);
void append_tag(compound_tag* compound, struct nbt_tag* tag);

#endif
