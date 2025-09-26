#pragma once

#include "../lib/arena.h"
#include "../lib/list.h"
#include "../lib/result.h"
#include "position.h"
#include "types.h"
#include <stddef.h>

typedef struct node_t node_t;
typedef union node_value_t node_value_t;
typedef List(node_t) node_list_t;

typedef enum {
  NODE_TYPE_LIST,
  NODE_TYPE_NUMBER,
  NODE_TYPE_SYMBOL,
  NODE_TYPE_STRING,
  NODE_TYPE_BOOLEAN,
  NODE_TYPE_NIL,
} node_type_t;

typedef union node_value_t {
  node_list_t list;
  number_t number;
  string_t symbol;
  string_t string;
  bool boolean;
  nullptr_t nil;
} node_value_t;

typedef struct node_t {
  position_t position;
  node_type_t type;
  node_value_t value;
} node_t;

typedef struct {
  size_t count;
  node_t *data;
} node_array_t;

result_ref_t nodeCreate(arena_t *, node_type_t);
result_void_t nodeInit(node_t *, arena_t *);
result_ref_t nodeCopy(const node_t *);
