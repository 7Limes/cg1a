#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include <stdint.h>


typedef struct {
    char *key;
    void *value;
} MapNode;


typedef struct {
    size_t size, capacity;
    MapNode *data;
} Map;


int create_map(Map *map_dest, size_t capacity);

void free_map(const Map *map);

int add_map_value(Map *map, char *key, void *value);

int get_map_value(void **dest, const Map *map, char *key);


#endif