#ifndef G1_UTIL_H
#define G1_UTIL_H


#include <stdbool.h>
#include <stdint.h>


// Based on https://stackoverflow.com/a/3464656
// Reads data from `file_path` into `output_buffer` and stores the length in `length`.
int read_file_bytes(char **output_buffer, size_t *length, const char *file_path);

bool file_exists(char* path);

bool safecat(char* dest, char* src, int size);

// Write a 16 bit integer to `file` in big endian format.
int write_i16_big(FILE* file, uint16_t value);

// Write a 32 bit integer to `file` in big endian format.
int write_i32_big(FILE* file, uint32_t value);


#endif
