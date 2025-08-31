#include <string.h>
#include "list.h"


int create_list(List *list, size_t element_size, size_t capacity) {
    list->element_size = element_size;
    list->size = 0;
    list->capacity = capacity;
    list->data = calloc(capacity, element_size);
    if (list->data == NULL) {
        return -1;
    }
    return 0;
}


void free_list(const List *list) {
    free(list->data);
}


int append_list_value(List *list, void *value) {
    if (list->size >= list->capacity) {
        return -1;
    }

    void *dest = list->data + list->size * list->element_size;
    memcpy(dest, value, list->element_size);

    list->size++;
    if (list->size == list->capacity) {
        list->capacity *= 2;
        void *new_data = realloc(list->data, list->capacity * list->element_size);
        if (new_data == NULL) {
            return -2;
        }
        list->data = new_data;
    }

    return 0;
}


void* get_list_value(const List *list, size_t index) {
    if (index >= list->size) {
        return NULL;
    }
    return list->data + index * list->element_size;
}
