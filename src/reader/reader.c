#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "reader.h"

/**
 * Reads a chunk data in a given region file.
 *
 * @param path Path to the .mca region file
 * @param chunkX X coordinates of the chunk
 * @param chunkZ Z coordinates of the chunk
 */
int read_region_file(char* path, int chunkX, int chunkZ) {
	FILE* region = fopen(path, "rb");

	const int offset_location = 4 * ((chunkX % 32) + (chunkZ % 32) * 32);
	const int offset_timestamp = offset_location + 4096;

	unsigned char location_header[4];
	fseek(region, offset_location, SEEK_SET);
	fread(location_header, sizeof(location_header), 1, region);
	for (int i = 0; i < sizeof(location_header); i++) printf("%x ", location_header[i]);
	printf("\noffset: %d\n\n", offset_location);

	int offset_chunk = (65536*location_header[0] + 256*location_header[1] + 1*location_header[2]) * 4096;
	unsigned char chunk_header[5];
	fseek(region, offset_chunk, SEEK_SET);
	fread(chunk_header, sizeof(chunk_header), 1, region);
	for (int i = 0; i < sizeof(chunk_header); i++) printf("%x ", chunk_header[i]);
	printf("\nchunk: %x\n\n", offset_chunk);

	unsigned char chunk_data[16777216*chunk_header[0] + 65536*chunk_header[1] + 256*chunk_header[2] + 1*chunk_header[3]-1];
	fread(chunk_data, sizeof(chunk_data), 1, region);
	// for (int i = 0; i < sizeof(chunk_data); i++) printf("%x ", chunk_data[i]);

	unsigned char* uncompressed_data = read_chunk_data(chunk_data);
	for (int i = 0; i < sizeof(uncompressed_data); i++) printf("%x ", uncompressed_data[i]);

	fclose(region);
	return 0;
}

unsigned char* read_chunk_data(unsigned char* data) {
	// TODO: Decompression
	return NULL;
}
