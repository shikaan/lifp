// This is for the CI compiler
#define _POSIX_C_SOURCE 200809L
#include "virtual_machine.h"
#include "error.h"
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

static Map(value_t) * builtins;
static Map(value_t) * specials;

result_vm_ref_t vmCreate(vm_options_t opts) {
  arena_t *arena = nullptr;
  try(result_vm_ref_t, arenaCreate(opts.vm_size), arena);

  vm_t *machine = nullptr;
  try(result_vm_ref_t, arenaAllocate(arena, sizeof(vm_t)), machine);

  machine->global = nullptr;
  machine->options = opts;
  machine->arena = arena;

  try(result_vm_ref_t, environmentCreate(arena, nullptr), machine->global);

  try(result_vm_ref_t, mapCreate(value_t, arena, 64), builtins);

#define setBuiltin(Label, Builtin)                                             \
  builtin.type = VALUE_TYPE_BUILTIN;                                           \
  builtin.value.builtin = (Builtin);                                           \
  try(result_vm_ref_t, mapSet(value_t, builtins, (Label), &builtin));

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
  setBuiltin(LIST_COUNT, listCount);
  setBuiltin(LIST_FROM, listFrom);
  setBuiltin(LIST_NTH, listNth);
  setBuiltin(LIST_MAP, listMap);
  setBuiltin(LIST_EACH, listEach);
  setBuiltin(LIST_FILTER, listFilter);
  setBuiltin(LIST_TIMES, listTimes);
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
  setBuiltin(IO_STDOUT, ioStdout);
  setBuiltin(IO_STDERR, ioStderr);
  setBuiltin(IO_PRINTF, ioPrintf);
  setBuiltin(IO_READLINE, ioReadline);
  setBuiltin(IO_CLEAR, ioClear);
#undef setBuiltin

  try(result_vm_ref_t, mapCreate(value_t, arena, 4), specials);
#define setSpecial(Label, Special)                                             \
  special.type = VALUE_TYPE_SPECIAL;                                           \
  special.value.special = (Special);                                           \
  try(result_vm_ref_t, mapSet(value_t, specials, (Label), &special));

  value_t special;
  setSpecial(DEFINE, define);
  setSpecial(LET, let);
  setSpecial(COND, cond);
  setSpecial(FUNCTION, function);
#undef setSpecial

  return ok(result_vm_ref_t, machine);
}

result_environment_ref_t environmentCreate(arena_t *arena,
                                           environment_t *parent) {
  assert(arena);

  environment_t *environment = nullptr;
  try(result_environment_ref_t, arenaAllocate(arena, sizeof(environment_t)),
      environment);

  environment->arena = arena;
  environment->parent = parent;

  try(result_environment_ref_t, mapCreate(value_t, arena, 4),
      environment->values);

  return ok(result_environment_ref_t, environment);
}

result_void_t environmentRegisterSymbol(environment_t *self, const char *key,
                                        const value_t *value) {
  if (!value)
    return ok(result_void_t);

  if (environmentResolveSymbol(self, key)) {
    throw(result_void_t, ERROR_CODE_REFERENCE_SYMBOL_ALREADY_DEFINED, nullptr,
          "Identifier '%s' has already been declared", key);
  }

  value_t copied;
  try(result_void_t, valueCopy(value, &copied, self->arena));
  try(result_void_t, mapSet(value_t, self->values, key, &copied));
  return ok(result_void_t);
}

result_void_t environmentUnsafeRegisterSymbol(environment_t *self,
                                              const char *key,
                                              const value_t *value) {
  if (!value) {
    return ok(result_void_t);
  }

  value_t copied;
  try(result_void_t, valueCopy(value, &copied, self->arena));
  try(result_void_t, mapSet(value_t, self->values, key, &copied));
  return ok(result_void_t);
}

result_environment_ref_t environmentClone(const environment_t *source,
                                          arena_t *arena) {
  environment_t *result;
  try(result_environment_ref_t, environmentCreate(arena, source->parent),
      result);

  for (size_t i = 0; i < source->values->capacity; i++) {
    if (source->values->used[i]) {
      const char *key = source->values->keys[i];
      const value_t *value = &source->values->values[i];

      if (!environmentResolveSymbol(result, key)) {
        try(result_environment_ref_t,
            environmentUnsafeRegisterSymbol(result, key, value));
      }
    }
  }

  return ok(result_environment_ref_t, result);
}

const value_t *environmentResolveSymbol(const environment_t *self,
                                        const char *symbol) {
  assert(self);

  const value_t *special = mapGet(value_t, specials, symbol);
  if (special) {
    return special;
  }

  const value_t *builtin = mapGet(value_t, builtins, symbol);
  if (builtin) {
    return builtin;
  }

  const value_t *result = mapGet(value_t, self->values, symbol);
  if (!result && self->parent) {
    return environmentResolveSymbol(self->parent, symbol);
  }

  return result;
}

void vmDestroy(vm_t **self) {
  if (!self || !*self)
    return;
  arena_t *arena = (*self)->arena;
  arenaDestroy(&arena);
  *(self) = nullptr;
}
