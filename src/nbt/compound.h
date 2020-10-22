#ifndef OVERVIEWER_COMPOUND_H
#define OVERVIEWER_COMPOUND_H

#define COMPOUND_LENGTH 50
#include "nbt.h"

struct compound_tag {
	int size;
	struct nbt_tag* to_tag;
	struct nbt_tag* values[5]; // TODO: dynamic allocation
};
typedef struct compound_tag compound_tag;

struct nbt_tag* get_tag_from_name(compound_tag* compound, char* name);
void append_tag(compound_tag* compound, struct nbt_tag* tag);

#endif
