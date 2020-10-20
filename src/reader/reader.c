#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "reader.h"

#define CHUNK_SIZE 4096
#define DEBUG

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
	for (int i = 0; i < sizeof(location_header); i++) printf("%x ", location_header[i]);
	printf("\noffset: %d\n\n", offset_location);

	int offset_chunk = (65536*location_header[0] + 256*location_header[1] + 1*location_header[2]) * 4096;
	unsigned char chunk_header[5];
	fseek(region, offset_chunk, SEEK_SET);
	fread(chunk_header, sizeof(chunk_header), 1, region);
	for (int i = 0; i < sizeof(chunk_header); i++) printf("%x ", chunk_header[i]);
	printf("\nchunk: %x\n\n", offset_chunk);

	int final_length;
	int data_length = 16777216*chunk_header[0] + 65536*chunk_header[1] + 256*chunk_header[2] + 1*chunk_header[3]-1;
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
