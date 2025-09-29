#pragma once

#include "../lib/result.h"
#include "node.h"
#include "position.h"
#include "types.h"
#include <stddef.h>
#include <stdint.h>

typedef struct value_t value_t;
typedef struct environment_t environment_t;
typedef struct {
  bool more;
  environment_t *environment;
  node_t *node;
} trampoline_t;

typedef Result(value_t *, position_t) result_value_ref_t;
typedef ResultVoid(position_t) result_void_position_t;

typedef struct {
  size_t count;
  value_t *data;
} value_array_t;

typedef result_value_ref_t (*builtin_t)(const value_array_t *, position_t);
typedef result_value_ref_t (*special_form_t)(const node_array_t *,
                                             environment_t *, trampoline_t *);

typedef enum {
  VALUE_TYPE_BOOLEAN,
  VALUE_TYPE_NUMBER,
  VALUE_TYPE_BUILTIN,
  VALUE_TYPE_CLOSURE,
  VALUE_TYPE_NIL,
  VALUE_TYPE_LIST,
  VALUE_TYPE_SPECIAL,
  VALUE_TYPE_STRING,
} value_type_t;

typedef struct {
  size_t count;
  string_t *data;
} arguments_t;

typedef struct {
  node_t *form;
  arguments_t *arguments;
  environment_t *environment;
} closure_t;

typedef union {
  bool boolean;
  number_t number;
  closure_t closure;
  builtin_t builtin;
  nullptr_t nil;
  value_array_t *list;
  special_form_t special;
  string_t string;
} value_as_t;

typedef struct value_t {
  value_type_t type;
  value_as_t as;
  position_t position;
} value_t;

typedef enum {
  VALUE_MAP_ERROR_ALLOCATION,
  VALUE_MAP_ERROR_INVALID_KEY,
} value_map_error_t;

typedef struct {
  size_t capacity;
  bool *used;
  char **keys;
  value_t *data;
} value_map_t;

typedef Result(value_map_t *) result_value_map_ref_t;

result_value_ref_t valueCreate(value_type_t, value_as_t, position_t);
result_value_ref_t valueDeepCopy(const value_t *);
void valueDestroy(value_t **);

result_ref_t valueArrayCreate(size_t);
void valueArrayDestroy(value_array_t **);
result_ref_t argumentsCreate(size_t);

result_value_map_ref_t valueMapCreate(size_t);
void valueMapDestroy(value_map_t **);
void valueMapDestroyInner(value_map_t *);
result_void_t valueMapSet(value_map_t *, const char *, const value_t *);
void *valueMapGet(const value_map_t *, const char *);
