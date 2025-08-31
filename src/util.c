#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>


// Based on https://stackoverflow.com/a/3464656
// Reads data from `file_path` into `output_buffer` and stores the length in `length`.
int read_file_bytes(char **output_buffer, size_t *length, const char *file_path) {
    if (!output_buffer || !length || !file_path) {
        return -1;  // Invalid arguments
    }

    FILE *handler = fopen(file_path, "rb");
    if (!handler) {
        return -2;  // File cannot be opened
    }
    
    if (fseek(handler, 0, SEEK_END) != 0) {
        fclose(handler);
        return -3;  // Seek error
    }

    long file_size = ftell(handler);
    if (file_size < 0) {
        fclose(handler);
        return -4;  // Tell error
    }
    rewind(handler);

    char *buffer = malloc((size_t) file_size + 1);
    if (!buffer) {
        fclose(handler);
        return -5;  // Memory allocation failure
    }

    size_t read_size = fread(buffer, sizeof (char), (size_t) file_size, handler);
    if (read_size != (size_t) file_size) {
        free(buffer);
        fclose(handler);
        return -6;  // Read error
    }

    buffer[file_size] = '\0';
    fclose(handler);

    *output_buffer = buffer;
    *length = (size_t) file_size;

    return 0;  // Success
}


bool file_exists(char* path) {
    struct stat buffer;
    return stat(path, &buffer) == 0;
}


static uint16_t host_to_big_16(uint16_t host_16bits) {
    // Check if system is little endian
    uint16_t test = 1;
    if (*(uint8_t*)&test == 1) {
        // Little endian system - swap bytes
        return ((host_16bits & 0xFF) << 8) | ((host_16bits >> 8) & 0xFF);
    }
    // Big endian system - no conversion needed
    return host_16bits;
}


static uint32_t host_to_big_32(uint32_t host_32bits) {
    // Check if system is little endian
    uint16_t test = 1;
    if (*(uint8_t*)&test == 1) {
        // Little endian system - swap bytes
        return ((host_32bits & 0xFF) << 24) |
               ((host_32bits & 0xFF00) << 8) |
               ((host_32bits & 0xFF0000) >> 8) |
               ((host_32bits >> 24) & 0xFF);
    }
    // Big endian system - no conversion needed
    return host_32bits;
}

// Write a 16-bit integer to file in big endian format
int write_i16_big(FILE* file, uint16_t value) {
    if (!file) {
        return -1; // Invalid file pointer
    }
    
    uint16_t be_value = host_to_big_16(value);
    size_t written = fwrite(&be_value, sizeof(uint16_t), 1, file);
    
    return (written == 1) ? 0 : -1;
}

// Write a 32-bit integer to file in big endian format
int write_i32_big(FILE* file, uint32_t value) {
    if (!file) {
        return -1;
    }
    
    uint32_t be_value = host_to_big_32(value);
    size_t written = fwrite(&be_value, sizeof(uint32_t), 1, file);
    
    return (written == 1) ? 0 : -1;
}