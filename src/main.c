#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "assembler.h"


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("usage: g1a [--data_path DATA_PATH] input_path output_path\n");
        return 1;
    }
    
    // Parse flags
    char *data_file_path = NULL;
    if (argc > 3) {
        if (strcmp(argv[3], "-d") == 0) {
            if (argc < 5) {
                printf("Expected data file path.\n");
                return 2;
            }
            char *data_file_path = argv[4];
        }
        else {
            printf("Got unrecognized flag \"%s\".\n", argv[3]);
            return 2;
        }
    }
    
    if (!file_exists(argv[1])) {
        printf("File \"%s\" does not exist.\n", argv[1]);
        return 3;
    }

    return assemble_file(argv[1], argv[2]);
}