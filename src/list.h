#ifndef LIST_H
#define LIST_H

#include <stdlib.h>


typedef struct {
    size_t element_size, size, capacity;
    void *data;
} List;


// Create a new list.
int create_list(List *list, size_t element_size, size_t capacity);

// Free `list`.
void free_list(const List *list);

// Add a copy of `value` to the end of `list`.
int append_list_value(List *list, void *value);

// Returns a value from `list` at `index`.
void* get_list_value(const List *list, size_t index);


#endif