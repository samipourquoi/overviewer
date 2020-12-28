#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <math.h>
#include <string.h>
#include <lmdb.h>
#include "chunks.h"
#include "nbt.h"
#include "render.h"
#include "assets.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CHUNK_SIZE 4096
#define DEBUG

///=================================///
///            CHUNK DATA           ///
///=================================///

chunk_t* chunk_init() {
	chunk_t* chunk = malloc(sizeof(chunk_t));
	// Uses `calloc` in case some sections are missing
	chunk->blocks = calloc(16 * 16 * 256, sizeof(model_t*));
	chunk->blockstates = calloc(16 * 16 * 256, sizeof(blockstate_t*));
	return chunk;
}

void chunk_free(chunk_t* chunk) {
	free(chunk->blocks);
	free(chunk);
}

///=================================///
///           CHUNK READING         ///
///=================================///

/**
 * Reads a chunk data in a given region file.
 *
 * @param path Path to the .mca region file
 * @param chunkX X coordinates of the chunk
 * @param chunkZ Z coordinates of the chunk
 */
int read_region_file(char* path, int chunkX, int chunkZ) {
	FILE* region = fopen(path, "rb");

	const int offset_location = 4 * ((chunkX & 31) + (chunkZ & 31) * 32);
	const int offset_timestamp = offset_location + 4096;

	unsigned char location_header[4];
	fseek(region, offset_location, SEEK_SET);
	fread(location_header, sizeof(location_header), 1, region);

	int offset_chunk = (65536 * location_header[0] + 256 * location_header[1] + 1 * location_header[2]) * 4096;
	unsigned char chunk_header[5];
	fseek(region, offset_chunk, SEEK_SET);
	fread(chunk_header, sizeof(chunk_header), 1, region);

	int final_length;
	int data_length = 16777216 * chunk_header[0] + 65536 * chunk_header[1] + 256 * chunk_header[2] + 1 * chunk_header[3] - 1;
	unsigned char* uncompressed_data = read_chunk_data(region, data_length, &final_length);

	parse_chunk(uncompressed_data, final_length);

#ifdef DEBUG
	FILE* foo = fopen("chunk.nbt", "wb+");
	fwrite(uncompressed_data, 1, final_length, foo);
	fclose(foo);
#endif

	fclose(region);
	return 0;
}

/**
 * Decompressed the chunk data from a region file.
 *
 * @see
 * Zlib decompress bytes with unknown compressed length in C:<ul>
 * <li>https://stackoverflow.com/questions/64409280/zlib-decompress-bytes-with-unknown-compressed-length-in-c</li>
 * <li>https://stackoverflow.com/questions/34323361/how-to-return-how-many-bytes-were-actually-consumed-to-return-the-decompressed-d?rq=1</li>
 * <li>https://zlib.net/zlib_how.html</li>
 * <li>https://chromium.googlesource.com/native_client/nacl-gcc/+/master/zlib/examples/zpipe.c</li>
 * </ul>
 *
 * @param region Pointer to the region file
 * @param c_length Length of the compressed chunk data
 * @param unc_length Pointer to an int, which will be assigned to the uncompressed chunk data length
 * @return Pointer to a byte array of the uncompressed chunk data
 */
unsigned char* read_chunk_data(FILE* region, int c_length, int* unc_length) {
	int ret;
	int size = CHUNK_SIZE;
	unsigned char in[CHUNK_SIZE];
	unsigned char out[CHUNK_SIZE];
	unsigned char* uncompressed_data = malloc(size);
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;
	ret = inflateInit(&stream);
	if (ret != Z_OK) return NULL;

	int total = 0;
	int readable_chunk = c_length % CHUNK_SIZE;
	do {
		// Reads a chunk of data until it reaches the end of the chunk data
		stream.avail_in = fread(in, 1, readable_chunk, region);
		if (stream.avail_in == 0) break;
		stream.next_in = in;

		// Decompresses that data
		do {
			stream.avail_out = CHUNK_SIZE;
			stream.next_out = out;
			ret = inflate(&stream, Z_NO_FLUSH);

			// Loops through the relevant bytes
			for (int i = 0; i < CHUNK_SIZE - stream.avail_out; i++) {
				// Expands the array if it's not large enough
				if (total >= size) {
					size += CHUNK_SIZE;
					uncompressed_data = realloc(uncompressed_data, size);
				}

				// Copies the uncompressed byte to the dynamic array
				uncompressed_data[total] = out[i];
				total++;
			}
		} while (stream.avail_out == 0);

		readable_chunk = CHUNK_SIZE;
	} while (ret != Z_STREAM_END);

	*unc_length = total;
	inflateEnd(&stream);
	return uncompressed_data;
}


void parse_palette_index(nbt_tag* palette, int index, chunk_t* chunk, pos_t coords) {
	compound_tag* block = palette->value->list_value->values[index]->value->compound_value;
	char* name = cmpd_get_from_name(block, "Name")->value->string_value;
	int shortened_length = (int) strlen(name) - 10 + 1;
	char* shortened_name = malloc(shortened_length);
	memcpy(shortened_name, &name[10], shortened_length);

	nbt_tag* properties_tag = cmpd_get_from_name(block, "Properties");
	model_t* model;
	if (properties_tag != NULL) {
		compound_tag* properties = properties_tag->value->list_value;
		blockstate_t** blockstates = malloc((properties->size + 1) * sizeof(blockstate_t));

		for (int i = 0; i < properties->size; i++) {
			char* key = properties->values[i]->name;
			char* value = properties->values[i]->value->string_value;

			/* Not bullshit */
			blockstate_t* bs = malloc(sizeof(blockstate_t));
			bs->key = malloc(strlen(key) + 1);
			bs->value = malloc(strlen(value) + 1);
			strcpy(bs->key, key);
			strcpy(bs->value, value);
			blockstates[i] = bs;
		}
		blockstates[properties->size] = NULL;
		model = assets_get_model(shortened_name, blockstates);

		// Frees the blockstates
		for (int i = 0; i < properties->size; i++) {
			blockstate_t* bs = blockstates[i];
			free(bs->key);
			free(bs->value);
			free(bs);
		}
		free(blockstates);
	} else {
		model = assets_get_model(shortened_name, NULL);
	}

	chunk->blocks[coords] = model;

	free(shortened_name);
}

void parse_section(compound_tag* section, chunk_t* chunk, pos_t* coords) {
	nbt_tag* palette = cmpd_get_from_name(section, "Palette");
	if (palette == NULL) return;
	unsigned int block_length = ceil(log2(palette->value->list_value->size));
	block_length = MAX(block_length, 4);
	unsigned int per_line = floor(64.0 / block_length);
	unsigned int bits_range = (unsigned int) pow(2, block_length) - 1;

	// Overwrites the Y value of the coords.
	int y = cmpd_get_from_name(section, "Y")->value->int_value * 16;
	*coords = (*coords & (~(0xFF00))) | y << 8;

	// for (int x = 0; x < section->size; x++) {
	nbt_tag* blockstates = cmpd_get_from_path(section, "BlockStates");
	for (int i = 0; i < blockstates->value->array_length; i++) {
		unsigned long line = blockstates->value->long_array_value[i];
		for (int j = 0; j < per_line; j++) {
			unsigned int shift = (j * block_length);
			int palette_index = (line & ((unsigned long) bits_range << shift)) >> shift;

			parse_palette_index(palette, palette_index, chunk, *coords);
			(*coords)++;
		}
	}
	// }
}

compound_tag* parse_chunk(unsigned char* data, int length) {
	// Better go read that! https://minecraft.gamepedia.com/Chunk_format
	compound_tag* tree = nbt_parse_tree(data, length);
	nbt_tag* sections = cmpd_get_from_path(tree, "Level.Sections");

	chunk_t* chunk = chunk_init();
	pos_t coords = 0x0000;

	// There is an empty section at index 0,
	// so we start at index 1.
	for (int y = 1; y < sections->value->list_value->size; y++) {
		nbt_tag* section = sections->value->list_value->values[y];
		parse_section(section->value->compound_value, chunk, &coords);
	}

	render(chunk);

	chunk_free(chunk);
	return NULL;
}


MDB_env* env;
void chunks_init_db() {
	mdb_env_create(&env);
	mdb_env_set_maxdbs(env, 1);
	mdb_env_set_mapsize(env, 10485760);
	mdb_env_open(env, "chunks", MDB_NOTLS, 0664);
}

#define GENERATE_KEY(POS) \
	(MDB_val) { \
		.mv_data = &(POS), \
		.mv_size = 2 * sizeof(int) \
	}

void chunks_set_at(int x, int z, unsigned char* data, int data_length) {
	MDB_txn* txn;
	MDB_dbi dbi;
	int pos[2] = { x, z };
	MDB_val key = GENERATE_KEY(pos);
	MDB_val value = {
			.mv_data = data,
			.mv_size = data_length
	};

	mdb_txn_begin(env, NULL, 0, &txn);
	mdb_dbi_open(txn, NULL, 0, &dbi);
	mdb_put(txn, dbi, &key, &value, 0);
	mdb_dbi_close(env, dbi);
	mdb_txn_commit(txn);
}

unsigned char* chunks_get_at(int x, int z) {
	MDB_txn* txn;
	MDB_dbi dbi;
	int pos[2] = { x, z };
	MDB_val key = GENERATE_KEY(pos);
	MDB_val value = {};

	mdb_txn_begin(env, NULL, 0, &txn);
	mdb_dbi_open(txn, NULL, 0, &dbi);
	mdb_get(txn, dbi, &key, &value);
	mdb_dbi_close(env, dbi);
	mdb_txn_abort(txn);

	return value.mv_data;
}