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

typedef struct {
  arena_t *arena;
  environment_t *global;
  vm_opts_t options;
  size_t enviroment_count;
} virtual_machine_t;

result_ref_t vmInit(vm_opts_t);
void vmStop(void);
result_ref_t environmentCreate(arena_t *, environment_t *);
result_void_t environmentAddSymbol(environment_t *, const char *,
                                   const value_t *);
const value_t *environmentResolveSymbol(const environment_t *, const char *);
result_ref_t environmentClone(const environment_t *, arena_t *);
