#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <parson.h>
#include <stdbool.h>
#include "assets.h"

blockstate_t** assets_parse_variants_states(char* variant_name) {
	const char* delim = ",=";
	size_t current_size = 1;
	blockstate_t** blockstates = malloc((current_size + 1) * sizeof(blockstate_t*));
	blockstate_t* blockstate;
	char* token = strtok(variant_name, delim);
	int i;
	for (i = 0; token != NULL; i++) {
		if (i % 2 == 0) {
			// When it's a key
			blockstate = malloc(sizeof(blockstate_t));
			blockstate->key = malloc(strlen(token) + 1);
			strcpy(blockstate->key, token);
		} else {
			// When it's a value
			blockstate->value = malloc(strlen(token) + 1);
			strcpy(blockstate->value, token);
			if (i - 1 > current_size) {
				current_size *= 2;
				blockstates = realloc(blockstates, (current_size + 1) * sizeof(blockstate_t*));
			}
			blockstates[(i - 1) / 2] = blockstate;
		}
		token = strtok(NULL, delim);
	}
	// NULL-terminates the array
	blockstates[i / 2] = NULL;
	return blockstates;
}

model_t** assets_read_variants(JSON_Object* variants) {
	size_t variants_amount = json_object_get_count(variants);
	model_t** models = malloc((variants_amount + 1) * sizeof(model_t*));
	// We NULL-terminate the array, hence the +1.
	for (int i = 0; i < variants_amount; i++) {
		char* variant_name = (char*) json_object_get_name(variants, i);
		blockstate_t** blockstates = assets_parse_variants_states(variant_name);

		// It can either be an object (=one variant), or an
		// array of variants.
		JSON_Value* variant_value = json_object_get_value_at(variants, i);
		JSON_Object* variant;
		if (json_type(variant_value) == JSONObject) {
			variant = json_value_get_object(variant_value);
		} else {
			variant = json_array_get_object(json_value_get_array(variant_value), 0);
		}
		char* model_id = (char*) json_object_get_string(variant, "model");
		if (model_id == NULL) return NULL;
		char* model_path = malloc(26 + strlen(model_id));
		strcpy(model_path, "assets/models/block/");
		strcat(model_path, &model_id[16]);
		strcat(model_path, ".json");

		JSON_Value* value = json_parse_file(model_path);
		JSON_Object* root = json_value_get_object(value);
		model_t* model = calloc(1, sizeof(model_t));
		model_init(model);
		models_parse(model, root);
		model->blockstates = blockstates;
		models[i] = model;

		json_value_free(value);
		free(model_path);
	}

	models[variants_amount] = NULL;
	return models;
}

void assets_read_blockstates(struct dirent* file) {
	char blockstate_path[100];
	strcpy(blockstate_path, "assets/blockstates/");
	strcat(blockstate_path, file->d_name);

	JSON_Value* blockstate_json = json_parse_file(blockstate_path);
	JSON_Object* root = json_value_get_object(blockstate_json);
	JSON_Object* variants = json_object_get_object(root, "variants");

	model_t** models = assets_read_variants(variants);

	file->d_name[file->d_namlen - 5] = '\0';
	char* blockstate_name = calloc(50, sizeof(char));
	strcpy(blockstate_name, file->d_name);
	ht_insert(blockstates_list, blockstate_name, &models);

	json_value_free(blockstate_json);
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
	int length = (int) strlen(key);

	for (byte = 0; byte < length; ++byte) {
		hash = ((hash << 5) + hash) ^ key[byte];
	}

	return hash;
}

void assets_init() {
	blockstates_list = malloc(sizeof(HashTable));
	ht_setup(blockstates_list, 50, sizeof(model_t***), 10);
	blockstates_list->compare = ht_string_compare;
	blockstates_list->hash = ht_string_hash;
	ht_reserve(blockstates_list, 128);

	DIR* blockstates = opendir("assets/blockstates");

	struct dirent* blockstate;
	while ((blockstate = readdir(blockstates)) != NULL) {
		assets_read_blockstates(blockstate);
	}

	closedir(blockstates);
}

model_t* assets_get_model(char* blockstate_name, blockstate_t** states) {
	model_t** models = *(model_t***) ht_lookup(blockstates_list, blockstate_name);
	if (states == NULL) return models[0];

	// All arrays are NULL-terminated
	for (int i = 0; models[i] != NULL; i++) {
		model_t* model = models[i];

		// Will be set to `false` if it doesn't correspond.
		bool corresponds = true;
		for (int j = 0; states[j] != NULL; j++) {
			bool temp_corresponds = false;
			blockstate_t* given_bs = states[j];
			for (int k = 0; model->blockstates[k] != NULL; k++) {
				blockstate_t* model_bs = model->blockstates[k];

				if (strcmp(given_bs->key, model_bs->key) == 0 &&
				    strcmp(given_bs->value, model_bs->value) == 0) {

					temp_corresponds = true;
					break;
				}
			}
			if (!temp_corresponds) corresponds = false;
		}
		if (corresponds) return model;
	}

	return models[0];
}