#pragma once

#include "../lib/arena.h"
#include "../lib/list.h"
#include "../lib/result.h"
#include "node.h"
#include "position.h"
#include "types.h"
#include <stddef.h>
#include <stdint.h>

typedef struct value_t value_t;
typedef struct environment_t environment_t;

typedef List(value_t) value_list_t;
typedef Result(value_t *, position_t) result_value_ref_t;
typedef ResultVoid(position_t) result_void_position_t;
typedef result_void_position_t (*builtin_t)(arena_t *, value_t *,
                                            value_list_t *);
typedef result_void_position_t (*special_form_t)(value_t *, arena_t *,
                                                 environment_t *,
                                                 const node_list_t *);

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
  node_t form;
  node_list_t arguments;
} closure_t;

typedef struct value_t {
  value_type_t type;
  position_t position;
  union {
    bool boolean;
    number_t number;
    closure_t closure;
    builtin_t builtin;
    nullptr_t nil;
    value_list_t list;
    special_form_t special;
    string_t string;
  } value;
} value_t;

constexpr size_t VALUE_LIST_INITIAL_SIZE = 8;

result_void_t valueCopy(value_t *source, value_t *destination,
                        arena_t *destination_arena);
result_ref_t valueCreate(arena_t *arena);

#define valueInit(Self, Arena, Value)                                          \
  _Generic((Value),                                                            \
      bool: valueInitBoolean,                                                  \
      number_t: valueInitNumber,                                               \
      nullptr_t: valueInitNil,                                                 \
      size_t: valueInitList,                                                   \
      node_type_t: valueInitClosure,                                           \
      builtin_t: valueInitBuiltin,                                             \
      special_form_t: valueInitSpecial,                                        \
      string_t: valueInitString,                                               \
      const char *: valueInitString)(Self, Arena, Value)

result_void_t valueInitBoolean(value_t *self, arena_t *arena, bool value);
result_void_t valueInitNumber(value_t *self, arena_t *arena, number_t value);
result_void_t valueInitBuiltin(value_t *self, arena_t *arena, builtin_t value);
result_void_t valueInitClosure(value_t *self, arena_t *arena,
                               node_type_t form_type);
result_void_t valueInitNil(value_t *self, arena_t *arena, nullptr_t value);
result_void_t valueInitList(value_t *self, arena_t *arena, size_t size);
result_void_t valueInitSpecial(value_t *self, arena_t *arena,
                               special_form_t value);
result_void_t valueInitString(value_t *self, arena_t *arena, const char *value);
