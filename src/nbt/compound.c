#include "compound.h"
#include "nbt.h"
#include <stdio.h>

nbt_tag* get_tag_from_name(compound_tag* tag, char* name) {
	return NULL;
}

void append_tag(compound_tag* compound, nbt_tag* tag) {
	compound->values[compound->size] = tag;
	compound->size++;
}