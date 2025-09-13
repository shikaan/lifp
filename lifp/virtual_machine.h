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
  size_t environment_size;
  size_t vm_size;
} vm_options_t;

typedef struct {
  vm_options_t options;
  arena_t *arena;
  environment_t *global;
} vm_t;

typedef Result(vm_t *) result_vm_ref_t;
typedef Result(environment_t *) result_environment_ref_t;

result_vm_ref_t vmCreate(vm_options_t);
void vmDestroy(vm_t **);

result_environment_ref_t environmentCreate(arena_t *, environment_t *);
result_void_t environmentRegisterSymbol(environment_t *, const char *,
                                        const value_t *);
result_void_t environmentUnsafeRegisterSymbol(environment_t *, const char *,
                                              const value_t *);
const value_t *environmentResolveSymbol(const environment_t *, const char *);
result_environment_ref_t environmentClone(const environment_t *, arena_t *);
