#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

template <typename T>
struct List {
    size_t count;
    size_t capacity;

    T *elements;

    T &operator[](size_t index) {
        return elements[index];
    }
};

template <typename T>
T *begin(List<T> &list) {
    return list.elements;
}

template <typename T>
T *end(List<T> &list) {
    return list.elements + list.count;
}

template <typename T>
size_t append(List<T> *list, T element) {
    const size_t initial_capacity = 16;

    if(list->capacity == 0) {
        list->capacity = initial_capacity;

        list->elements = (T*)malloc(initial_capacity * sizeof(T));
    } else if(list->count == list->capacity) {
        auto new_capacity = list->capacity * 2;

        auto new_elements = (T*)realloc((void*)(list->elements), new_capacity * sizeof(T));

        list->capacity = new_capacity;
        list->elements = new_elements;
    }

    auto index = list->count;

    list->elements[index] = element;

    list->count += 1;

    return index;
}

template <typename T>
void remove_at(List<T> *list, size_t index) {
    for(size_t i = index; i < list->count - 1; i += 1) {
        list->elements[i] = list->elements[i + 1];
    }

    list->count -= 1;
}