#include "compound.h"
#include "nbt.h"
#include <stdio.h>
#include <string.h>

nbt_tag* get_tag_from_name(compound_tag* compound, char* name) {
	printf("i think i love tina... %s\n", compound->to_tag->name);
	compound->to_tag->name = "bonk";
	for (int i = 0; i < compound->size; i++) {
		// printf("i%d\n", i);
		printf("bonk? %d\n", compound->values[compound->size-1]->value);
		if (strcmp((compound->values[i]->name), name) == 0) {
			return compound->values[i];
		}
	}
	printf("not found...\n");
	return NULL;
}

void append_tag(compound_tag* compound, nbt_tag* tag) {
	printf("appending %s to compound %s with index %d\n", tag->name, compound->to_tag->name, compound->size);
	compound->values[compound->size] = tag;
	printf("is it %s?\n", compound->values[compound->size]->name);
	compound->size++;
}