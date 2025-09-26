#pragma once

#include "value.h"

typedef enum {
  MAP_ERROR_ALLOCATION = ARENA_ERROR_OUT_OF_SPACE,
  MAP_ERROR_INVALID_KEY,
} map_error_t;

typedef struct {
  size_t capacity;
  bool *used;
  char **keys;
  value_t *data;
} value_map_t;

typedef struct environment_t {
  struct environment_t *parent;
  value_map_t values;
} environment_t;

typedef Result(value_map_t *) result_value_map_ref_t;
typedef Result(environment_t *) result_environment_ref_t;

result_environment_ref_t environmentCreate(environment_t *);
void environmentDestroy(environment_t **);

result_value_map_ref_t valueMapCreate(size_t);
void valueMapDestroy(value_map_t **);
result_void_t valueMapSet(value_map_t *, const char *, const value_t *);
void *valueMapGet(const value_map_t *, const char *);
