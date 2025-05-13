#ifndef UTILS_H
#define UTILS_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char *msg;
  size_t capacity;
  size_t length;
} StringBuilder;

static inline void sb_init(StringBuilder *sb) {
  sb->capacity = 16;
  sb->length = 0;
  sb->msg = (char *)malloc(sizeof(char) * sb->capacity);
  if (!sb->msg) {
    printf("Could not malloc identifier");
    exit(EXIT_FAILURE);
  }
  sb->msg[0] = '\0';
}

static inline void sb_init_and_clear(StringBuilder *sb) {
  if (sb->msg) {
    sb->msg = NULL;
  }
  sb->capacity = 16;
  sb->length = 0;
  sb->msg = (char *)malloc(sizeof(char) * sb->capacity);
  if (!sb->msg) {
    printf("Could not malloc identifier");
    exit(EXIT_FAILURE);
  }
  sb->msg[0] = '\0';
}

static inline void sb_destroy(StringBuilder *sb) {
  if (sb->msg) {
    free(sb->msg);
    sb->msg = NULL;
  }
  sb->length = 0;
  sb->capacity = 0;
}

static inline void sb_append(StringBuilder *sb, char c) {
  if (sb->length + 1 >= sb->capacity) {
    sb->capacity *= 2;
    char *tmp = (char *)realloc(sb->msg, (sizeof(char) * sb->capacity));

    if (!tmp) {
      if (sb->msg) {
        free(sb->msg);
      }
      exit(EXIT_FAILURE);
    }
    sb->msg = tmp;
  }

  sb->msg[sb->length++] = c;
  sb->msg[sb->length] = '\0';
}

#endif
