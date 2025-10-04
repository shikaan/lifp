#pragma once

#include "value.h"
#include <stddef.h>

typedef struct environment_t {
  struct environment_t *parent;
  value_map_t values;
  size_t refcount;
} environment_t;

typedef struct {
  environment_t *global;
} vm_t;

typedef Result(vm_t *) result_vm_ref_t;
typedef Result(environment_t *) result_environment_ref_t;

result_environment_ref_t environmentCreate(environment_t *);
void environmentDestroy(environment_t **);
void environmentForceDestroy(environment_t **);

const value_t *environmentResolveSymbol(const environment_t *, const char *);
result_void_t environmentRegisterSymbol(environment_t *, const char *,
                                        const value_t *);

result_vm_ref_t vmCreate(void);
void vmDestroy(vm_t **);
