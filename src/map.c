#include <string.h>
#include "map.h"

#define FNV_PRIME 1099511628211UL
#define FNV_OFFSET 14695981039346656037UL

const size_t NODE_SIZE = sizeof(MapNode);


size_t fnv_hash(const char *s) {
    size_t hash = FNV_OFFSET;
    for (size_t i = 0;; i++) {
        const char c = s[i];
        if (c == '\0') {
            break;
        }
        hash = (hash ^ c) * FNV_PRIME;
    }
    return hash;
}


int create_map(Map *map_dest, size_t capacity) {
    map_dest->size = 0;
    map_dest->capacity = capacity;
    map_dest->data = calloc(capacity, NODE_SIZE);
    if (map_dest->data == NULL) {
        return -1;
    }
    return 0;
}


void free_map(const Map *map) {
    free(map->data);
}


int resize_map(Map *map, size_t new_capacity) {
    Map new_map;
    int create_result = create_map(&new_map, new_capacity);
    if (create_result != 0) {
        return -1;
    } 

    for (size_t i = 0; i < map->capacity; i++) {
        MapNode* node = &map->data[i];
        if (node->key != NULL) {
            int add_result = add_map_value(&new_map, node->key, node->value);
            if (add_result != 0) {
                free_map(&new_map);
                return -2;
            }
        }
    }

    free(map->data);
    map->capacity = new_capacity;
    map->data = new_map.data;

    return 0;
}


int add_map_value(Map *map, const char *key, void *value) {
    if (map->size >= map->capacity) {
        return -1;
    }
    size_t index = fnv_hash(key) % map->capacity;
    MapNode *node = &map->data[index];

    // Linear probe to find empty node
    while (node->key != NULL) {
        index = (index + 1) % map->capacity;
        node = &map->data[index];
    }

    node->key = key;
    node->value = value;

    map->size++;

    if (map->size > map->capacity / 2) {
        int resize_result = resize_map(map, map->capacity * 2);
        if (resize_result != 0) {
            return -2;
        }
    }

    return 0;
}


int get_map_value(void **dest, const Map *map, const char *key) {
    size_t index = fnv_hash(key) % map->capacity;
    MapNode *node = &map->data[index];
    
    size_t iterations = 0;
    while (node->key != NULL && strcmp(node->key, key) != 0) {
        index = (index + 1) % map->capacity;
        node = &map->data[index];
        if (iterations++ >= map->capacity) {
            // Could not find key
            return -1;
        }
    }

    if (node->key == NULL) {
        // Could not find key
        return -1;
    }

    if (dest != NULL) {
        *dest = node->value;
    }
    return 0;
}
