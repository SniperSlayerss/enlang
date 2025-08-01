#ifndef UTILS_H
#define UTILS_H
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_ERR(fmt, ...)                          \
    fprintf(stderr, "ERROR [%s:%d %s]: " fmt "\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define da_new(T, name)  \
    struct {              \
        T* data;          \
        size_t size, cap; \
    } name = { NULL, 0, 0 }

#define da_init(ptr)          \
    do {                      \
        (ptr).data = NULL;    \
        (ptr).size = 0;       \
        (ptr).cap = 0;        \
    } while (0)

#define _da_reserve(arr)                                                \
    do {                                                                \
        size_t new_cap = (arr).cap ? (arr).cap * 2 : 4;                 \
        (arr).data = realloc((arr).data, new_cap * sizeof *(arr).data); \
        if (!(arr).data)                                                \
            exit(ENOMEM);                                               \
        (arr).cap = new_cap;                                            \
    } while (0)

#define da_append(arr, value)               \
    do {                                    \
        if ((arr).size == (arr).cap)        \
            _da_reserve(arr);               \
        (arr).data[(arr).size++] = (value); \
    } while (0)

#define da_pop(arr) \
    ((arr).data[--(arr).size])

#define da_free(arr)                \
    do {                            \
        free((arr).data);           \
        (arr).data = NULL;          \
        (arr).size = (arr).cap = 0; \
    } while (0)

typedef struct {
    char* msg;
    size_t size;
    size_t capacity;
} StringBuilder;

static inline StringBuilder* sb_create(void) {
    StringBuilder* sb = malloc(sizeof(StringBuilder));
    if (!sb) return NULL;
    sb->msg = NULL;
    sb->size = 0;
    sb->capacity = 0;
    return sb;
}

static inline void sb_init(StringBuilder* sb) {
    sb->msg = NULL;
    sb->size = 0;
    sb->capacity = 0;
}

static inline void sb_reserve(StringBuilder* sb, size_t min_capacity) {
    if (sb->capacity >= min_capacity) return;
    
    size_t new_cap = sb->capacity ? sb->capacity * 2 : 8;
    while (new_cap < min_capacity) new_cap *= 2;
    
    char* new_msg = realloc(sb->msg, new_cap);
    if (!new_msg) {
        fprintf(stderr, "StringBuilder: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    sb->msg = new_msg;
    sb->capacity = new_cap;
}

static inline void sb_append_char(StringBuilder* sb, char c) {
    if (sb->size + 2 > sb->capacity) {
        sb_reserve(sb, sb->size + 2);
    }
    sb->msg[sb->size++] = c;
    sb->msg[sb->size] = '\0';
}

static inline void sb_append(StringBuilder* sb, const char* str) {
    if (!str) return;
    
    size_t str_len = strlen(str);
    if (str_len == 0) return;
    
    if (sb->size + str_len + 1 > sb->capacity) {
        sb_reserve(sb, sb->size + str_len + 1);
    }
    
    memcpy(&sb->msg[sb->size], str, str_len);
    sb->size += str_len;
    sb->msg[sb->size] = '\0';
}

static inline void sb_appendf(StringBuilder* sb, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // First, calculate required size
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (len < 0) {
        va_end(args);
        return; // encoding error
    }
    
    // Reserve space
    if (sb->size + len + 1 > sb->capacity) {
        sb_reserve(sb, sb->size + len + 1);
    }
    
    // Format into the buffer
    vsnprintf(&sb->msg[sb->size], len + 1, format, args);
    sb->size += len;
    
    va_end(args);
}

static inline void sb_free_contents(StringBuilder* sb) {
    free(sb->msg);
    sb->msg = NULL;
    sb->size = 0;
    sb->capacity = 0;
}

static inline void sb_free(StringBuilder* sb) {
    if (!sb) return;
    sb_free_contents(sb);
    free(sb);
}

#endif
