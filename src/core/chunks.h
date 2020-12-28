#ifndef OVERVIEWER_CHUNKS_H
#define OVERVIEWER_CHUNKS_H

#include <stdio.h>
#include <lmdb.h>
#include "nbt.h"
#include "models.h"


///=================================///
///            CHUNK DATA           ///
///=================================///

#define POS_MAX_VALUE 0xFFFF

#define POS_GET_X(POS) ( ((POS) & 0x000F) )
#define POS_GET_Y(POS) ( ((POS) & 0xFF00) >> 8 )
#define POS_GET_Z(POS) ( ((POS) & 0x00F0) >> 4 )

#define POS_ADD_X(POS) ( (POS) + 0x0001 )
#define POS_ADD_Y(POS) ( (POS) + 0x0100 )
#define POS_ADD_Z(POS) ( (POS) + 0x0010 )

typedef struct {
	model_t** blocks;
	blockstate_t*** blockstates; /* Array of arrays of pointers blockstate_t, for each block. */
} chunk_t;

/**
 * `short` with bytes ordered like that:
 * 0x	00				0				0
 * Y coordinate	| Z coordinate | X coordinate
 *
 * That means for coordinate (X=15; Y=134; Z=3),
 * its pos_t representation will be `0x86F3`.
 *
 * @note
 * Cool thing about storing coordinates like that is that
 * loop trough each coordinate very easily:
 * ```
 * for (pos_t pos = 0; pos < 16*16*256; pos++) {
 * 	// do your things here...
 * }
 * ```
 */
typedef unsigned short pos_t;

chunk_t* chunk_init();

void chunk_free(chunk_t* chunk);

///=================================///
///           CHUNK READING         ///
///=================================///

compound_tag* parse_chunk(unsigned char* data, int length);

int read_region_file(char* path, int chunkX, int chunkZ);

unsigned char* read_chunk_data(FILE* region, int c_length, int* unc_length);

///=================================///
///           	DATABASE	        ///
///=================================///

extern MDB_env* env;

void chunks_init_db();

void chunks_set_at(int x, int z);

void chunks_get_at(int x, int z);


#endif
