#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compound.h"
#include "nbt.h"

// Windows lacks that function, used by cmpd_get_from_path(). This is quite literally copied
// from stackoverflow though...
#if !(defined(_WIN32) || defined(_WIN64))
extern char* strsep(char **stringp, const char *delim) {
	if (*stringp == NULL) { return NULL; }
	char *token_start = *stringp;
	*stringp = strpbrk(token_start, delim);
	if (*stringp) {
		**stringp = '\0';
		(*stringp)++;
	}
	return token_start;
}
#endif

nbt_tag* cmpd_get_from_name(compound_tag* compound, char* name) {
	for (int i = 0; i < compound->size; i++) {
		if (strcmp((compound->values[i]->name), name) == 0) {
			return compound->values[i];
		}
	}
	return NULL;
}

nbt_tag* cmpd_get_from_path(compound_tag* compound, char* path) {
	// https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
	char* path_copy = strdup(path);
	char* token;
	compound_tag* current_compound = compound;
	while ((token = strsep(&path_copy, "."))) {
		nbt_tag* tag = cmpd_get_from_name(current_compound, token);
		if (tag == NULL) {
			printf("Not found...\n");
			free(path_copy);
			return NULL;
		} if (tag->type == TAG_Compound) {
			current_compound = tag->value->compound_value;
		} else if (tag->type == TAG_List) {
			current_compound = tag->value->list_value;
		} else {
			free(path_copy);
			return tag;
		}
	}
	free(path_copy);
	return current_compound->to_tag;
}

void cmpd_append_entry(compound_tag* compound, nbt_tag* tag) {
	compound->values[compound->size] = tag;
	compound->size++;
}