#pragma once

#include "value.h"
#include <stddef.h>

typedef struct environment_t {
  struct environment_t *parent;
  value_map_t values;
} environment_t;

typedef struct {
  size_t environment_size;
  size_t vm_size;
} vm_options_t;

typedef struct {
  vm_options_t options;
  environment_t *global;
} vm_t;

typedef Result(vm_t *) result_vm_ref_t;
typedef Result(environment_t *) result_environment_ref_t;

result_environment_ref_t environmentCreate(environment_t *);
void environmentDestroy(environment_t **);

const value_t *environmentResolveSymbol(const environment_t *, const char *);
result_void_t environmentRegisterSymbol(environment_t *, const char *,
                                        const value_t *);

result_vm_ref_t vmCreate(vm_options_t);
void vmDestroy(vm_t **);

result_environment_ref_t environmentClone(environment_t *original);