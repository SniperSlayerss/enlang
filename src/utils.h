#ifndef UTILS_H
#define UTILS_H
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_ERR(fmt, ...)                          \
    fprintf(stderr, "ERROR [%s:%d %s]: " fmt "\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define da_init(T, name)  \
    struct {              \
        T* data;          \
        size_t size, cap; \
    } name = { NULL, 0, 0 }

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
    size_t capacity;
    size_t length;
} StringBuilder;

#define sb_init(name)     \
    struct {              \
        char* msg;        \
        size_t size, cap; \
    } name = { NULL, 0, 0 }

#define _sb_reserve(arr)                                        \
    do {                                                        \
        size_t new_cap = (arr).cap ? (arr).cap * 2 : 4;         \
        (arr).msg = realloc((arr).msg, new_cap * sizeof(char)); \
        if (!(arr).msg)                                         \
            exit(ENOMEM);                                       \
        (arr).cap = new_cap;                                    \
    } while (0)

#define sb_append(arr, value)              \
    do {                                   \
        if ((arr).size + 1 >= (arr).cap)   \
            _sb_reserve(arr);              \
        (arr).msg[(arr).size++] = (value); \
        (arr).msg[(arr).size] = '\0';      \
    } while (0)

#define sb_pop(arr)            \
    ((arr).msg[--(arr).size]); \
    (arr).msg[(arr).size] = '\0';

#define sb_free(arr)                \
    do {                            \
        free((arr).msg);            \
        (arr).msg = NULL;           \
        (arr).size = (arr).cap = 0; \
    } while (0)

// static inline void sb_append(StringBuilder* sb, char c)
// {
//     if (sb->length + 1 >= sb->capacity) {
//         sb->capacity *= 2;
//         char* tmp = (char*)realloc(sb->msg, (sizeof(char) * sb->capacity));
//
//         if (!tmp) {
//             if (sb->msg) {
//                 free(sb->msg);
//             }
//             LOG_ERR("Realloc failed: %s", strerror(errno));
//             exit(ENOMEM);
//         }
//         sb->msg = tmp;
//     }
//
//     sb->msg[sb->length++] = c;
//     sb->msg[sb->length] = '\0';
// }
// static inline void sb_init(StringBuilder* sb)
// {
//     sb->capacity = 16;
//     sb->length = 0;
//     sb->msg = (char*)malloc(sizeof(char) * sb->capacity);
//     if (!sb->msg) {
//         LOG_ERR("Malloc failed: %s", strerror(errno));
//         exit(ENOMEM);
//     }
//     sb->msg[0] = '\0';
// }

// static inline void sb_init_and_clear(StringBuilder* sb)
// {
//     if (sb->msg) {
//         sb->msg = NULL;
//     }
//     sb->capacity = 16;
//     sb->length = 0;
//     sb->msg = (char*)malloc(sizeof(char) * sb->capacity);
//     if (!sb->msg) {
//         LOG_ERR("Malloc failed: %s", strerror(errno));
//         exit(ENOMEM);
//     }
//     sb->msg[0] = '\0';
// }
//
// static inline void sb_destroy(StringBuilder* sb)
// {
//     if (sb->msg) {
//         free(sb->msg);
//         sb->msg = NULL;
//     }
//     sb->length = 0;
//     sb->capacity = 0;
// }
//
#endif
