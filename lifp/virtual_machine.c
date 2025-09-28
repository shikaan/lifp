// This is for the CI compiler
#define _POSIX_C_SOURCE 200809L
#include "virtual_machine.h"
#include "specials.h"
#include "value.h"

// NOLINTBEGIN - intentionally including .c files
#include "std/core.c"
#include "std/flow.c"
#include "std/io.c"
#include "std/list.c"
#include "std/math.c"
#include "std/str.c"
// NOLINTEND

#include <assert.h>
#include <stddef.h>

value_map_t *builtins;
value_map_t *specials;

result_vm_ref_t vmCreate(vm_options_t opts) {
  vm_t *machine = nullptr;
  try(result_vm_ref_t, allocSafe(sizeof(vm_t)), machine);

  machine->global = nullptr;
  machine->options = opts;

  try(result_vm_ref_t, environmentCreate(nullptr), machine->global);
  try(result_vm_ref_t, valueMapCreate(1), builtins);

#define setBuiltin(Label, Builtin)                                             \
  builtin.type = VALUE_TYPE_BUILTIN;                                           \
  builtin.as.builtin = (Builtin);                                              \
  try(result_vm_ref_t, valueMapSet(builtins, (Label), &builtin));

  value_t builtin;
  setBuiltin(SUM, sum);
  setBuiltin(SUB, subtract);
  setBuiltin(MUL, multiply);
  setBuiltin(DIV, divide);
  setBuiltin(MOD, modulo);
  setBuiltin(EQUAL, equal);
  setBuiltin(LESS_THAN, lessThan);
  setBuiltin(GREATER_THAN, greaterThan);
  setBuiltin(NEQ, notEqual);
  setBuiltin(LEQ, lessEqual);
  setBuiltin(GEQ, greaterEqual);
  setBuiltin(LOGICAL_AND, logicalAnd);
  setBuiltin(LOGICAL_OR, logicalOr);
  setBuiltin(FLOW_SLEEP, flowSleep);
  setBuiltin(IO_STDOUT, ioStdout);
  setBuiltin(IO_STDERR, ioStderr);
  setBuiltin(IO_PRINTF, ioPrintf);
  setBuiltin(IO_READLINE, ioReadline);
  setBuiltin(IO_CLEAR, ioClear);
  setBuiltin(LIST_COUNT, listCount);
  setBuiltin(LIST_FROM, listFrom);
  setBuiltin(LIST_NTH, listNth);
  setBuiltin(LIST_MAP, listMap);
  setBuiltin(LIST_EACH, listEach);
  setBuiltin(LIST_FILTER, listFilter);
  setBuiltin(LIST_TIMES, listTimes);
  setBuiltin(LIST_REDUCE, listReduce);
  setBuiltin(MATH_MAX, mathMax);
  setBuiltin(MATH_MIN, mathMin);
  setBuiltin(MATH_CEIL, mathCeil);
  setBuiltin(MATH_FLOOR, mathFloor);
  setBuiltin(MATH_RANDOM, mathRandom);
  setBuiltin(STR_LENGTH, strLength);
  setBuiltin(STR_JOIN, strJoin);
  setBuiltin(STR_SLICE, strSlice);
  setBuiltin(STR_INCLUDE, strInclude);
  setBuiltin(STR_TRIM_LEFT, strTrimLeft);
  setBuiltin(STR_TRIM_RIGHT, strTrimRight);
#undef setBuiltin

  try(result_vm_ref_t, valueMapCreate(4), specials);
#define setSpecial(Label, Special)                                             \
  special.type = VALUE_TYPE_SPECIAL;                                           \
  special.as.special = (Special);                                              \
  try(result_vm_ref_t, valueMapSet(specials, (Label), &special));

  value_t special;
  setSpecial(DEFINE, define);
  setSpecial(LET, let);
  setSpecial(COND, cond);
  setSpecial(FUNCTION, function);
#undef setSpecial

  return ok(result_vm_ref_t, machine);
}

// Merged from environment.c
result_environment_ref_t environmentCreate(environment_t *parent) {
  environment_t *environment = nullptr;
  try(result_environment_ref_t, allocSafe(sizeof(environment_t)), environment);

  value_map_t *values = nullptr;
  try(result_environment_ref_t, valueMapCreate(8), values);

  environment->values = *values;
  environment->parent = parent;

  return ok(result_environment_ref_t, environment);
}

void environmentDestroy(environment_t **self) {
  valueMapDestroyInner(&(*self)->values);
  deallocSafe(self);
}

result_void_t environmentRegisterSymbol(environment_t *self, const char *key,
                                        const value_t *value) {
  if (!value)
    return ok(result_void_t);

  if (environmentResolveSymbol(self, key)) {
    throw(result_void_t, ERROR_CODE_REFERENCE_SYMBOL_ALREADY_DEFINED, nullptr,
          "Identifier '%s' has already been declared", key);
  }

  try(result_void_t, valueMapSet(&self->values, key, value));
  return ok(result_void_t);
}

const value_t *environmentResolveSymbol(const environment_t *self,
                                        const char *symbol) {
  assert(self);

  const value_t *special = valueMapGet(specials, symbol);
  if (special) {
    return special;
  }

  const value_t *builtin = valueMapGet(builtins, symbol);
  if (builtin) {
    return builtin;
  }

  const value_t *result = valueMapGet(&self->values, symbol);
  if (!result && self->parent) {
    return environmentResolveSymbol(self->parent, symbol);
  }

  return result;
}

void vmDestroy(vm_t **self) {
  if (!self || !*self)
    return;
  environmentDestroy(&(*self)->global);
  deallocSafe(self);
}
