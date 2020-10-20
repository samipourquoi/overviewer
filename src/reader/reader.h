#ifndef OVERVIEWER_READER_H
#define OVERVIEWER_READER_H

int read_region_file(char* path, int chunkX, int chunkZ);
unsigned char* read_chunk_data(FILE* region, int c_length, int* unc_length);

#endif
