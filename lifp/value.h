#pragma once

#include "../lib/arena.h"
#include "../lib/list.h"
#include "../lib/result.h"
#include "node.h"
#include "position.h"
#include <stdint.h>

typedef struct value_t value_t;
typedef List(value_t) value_list_t;
typedef Result(value_t *, position_t) result_value_ref_t;
typedef ResultVoid(position_t) result_void_position_t;
typedef result_void_position_t (*builtin_t)(value_t *result,
                                            value_list_t *nodes);

typedef enum {
  VALUE_TYPE_BOOLEAN,
  VALUE_TYPE_INTEGER,
  VALUE_TYPE_BUILTIN,
  VALUE_TYPE_CLOSURE,
  VALUE_TYPE_NIL,
  VALUE_TYPE_LIST,
} value_type_t;

typedef struct {
  node_t form;
  node_list_t arguments;
} closure_t;

typedef struct value_t {
  value_type_t type;
  position_t position;
  union {
    bool boolean;
    int32_t integer;
    closure_t closure;
    builtin_t builtin;
    nullptr_t nil;
    value_list_t list;
  } value;
} value_t;

constexpr size_t VALUE_LIST_INITIAL_SIZE = 8;

result_ref_t valueCreateInit(arena_t *arena, value_type_t type,
                             node_type_t form_type);
result_void_t valueCopy(value_t *source, value_t *destination,
                        arena_t *destination_arena);
result_void_t valueInit(value_t *value, arena_t *arena, value_type_t type,
                        node_type_t form_type);
result_ref_t valueCreate(arena_t *arena);
