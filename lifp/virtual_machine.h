#pragma once

#include "../lib/arena.h"
#include "../lib/map.h"
#include "value.h"
#include <stddef.h>

typedef struct environment_t {
  arena_t *arena;
  Map(value_t) * values;
  struct environment_t *parent;
} environment_t;

typedef struct {
  size_t max_call_stack_size;
  size_t environment_size;
} vm_opts_t;

result_ref_t vmInit(vm_opts_t opts);

result_ref_t environmentCreate(environment_t *parent);
void environmentDestroy(environment_t **self_ref);
const value_t *environmentResolveSymbol(environment_t *self,
                                        const char *symbol);
void environmentReset(environment_t *self);
