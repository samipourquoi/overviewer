#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <parson.h>
#include "assets.h"
#include "models.h"

void assets_read_blockstates(struct dirent* file) {
	char blockstate_path[100];
	strcpy(blockstate_path, "assets/blockstates/");
	strcat(blockstate_path, file->d_name);

	JSON_Value* blockstate_json = json_parse_file(blockstate_path);
	JSON_Object* root = json_value_get_object(blockstate_json);
	JSON_Object* variants = json_object_get_object(root, "variants");
	JSON_Value* variant_value = json_object_get_value_at(variants, 0);

	// It can either be an object (=one variant), or an
	// array of variants.
	JSON_Object* variant;
	if (json_type(variant_value) == JSONObject) {
		variant = json_value_get_object(variant_value);
	} else {
		variant = json_array_get_object(json_value_get_array(variant_value), 0);
	}
	char* model_name = malloc(50);
	char* model_id = (char*)json_object_get_string(variant, "model");
	if (model_id == NULL) return;
	memcpy(model_name, &model_id[16], 50);

	file->d_name[file->d_namlen - 5] = '\0';
	char* blockstate_name = calloc(50, sizeof(char));
	strcpy(blockstate_name, file->d_name);
	ht_insert(blockstates_list, blockstate_name, model_name);

	// This fucker interfere with some other memory,
	// making the program segfaults sometimes.
	// So we are not going to free memory 😎
	// json_value_free(blockstate_json);
}

int ht_string_compare(void* first_key, void* second_key, size_t key_size) {
	return strcmp(first_key, second_key);
}

size_t ht_string_hash(void* raw_key, size_t key_size) {
	// djb2 string hashing algorithm
	// sstp://www.cse.yorku.ca/~oz/hash.ssml
	size_t byte;
	size_t hash = 5381;
	char* key = raw_key;
	int length = (int)strlen(key);

	for (byte = 0; byte < length; ++byte) {
		hash = ((hash << 5) + hash) ^ key[byte];
	}

	return hash;
}

void assets_init() {
	blockstates_list = malloc(sizeof(HashTable));
	ht_setup(blockstates_list, 50, 50, 10);
	blockstates_list->compare = ht_string_compare;
	blockstates_list->hash = ht_string_hash;
	ht_reserve(blockstates_list, 128);

	DIR* blockstates = opendir("assets/blockstates");

	struct dirent* blockstate;
	while ((blockstate = readdir(blockstates)) != NULL) {
		assets_read_blockstates(blockstate);
	}

	JSON_Value* value = json_parse_file("assets/models/block/furnace_on.json");
	JSON_Object* root = json_value_get_object(value);
	model_t* model = malloc(sizeof(model_t));
	model_init(model);

	model = models_parse(model, root);

	printf("%s\n", model->elements[0]->south->texture);

	model_free(model);
	json_value_free(value);


	closedir(blockstates);
}