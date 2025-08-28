// This is for the CI compiler
#define _POSIX_C_SOURCE 200809L

#include "virtual_machine.h"

// NOLINTBEGIN - intentionally including .c files
#include "std/core.c"
#include "std/flow.c"
#include "std/io.c"
#include "std/list.c"
#include "std/math.c"
// NOLINTEND

#include "specials.h"
#include "value.h"
#include <assert.h>

static Map(value_t) * builtins;
static Map(value_t) * specials;

constexpr size_t ENVIRONMENT_MAX_SIZE = (long)32 * 1024;

result_ref_t vmInit() {
  environment_t *global_environment = nullptr;
  try(result_ref_t, environmentCreate(nullptr), global_environment);

  try(result_ref_t, mapCreate(value_t, global_environment->arena, 32),
      builtins);

#define setBuiltin(Label, Builtin)                                             \
  builtin.type = VALUE_TYPE_BUILTIN;                                           \
  builtin.value.builtin = (Builtin);                                           \
  try(result_ref_t, mapSet(value_t, builtins, (Label), &builtin));

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
  setBuiltin(MATH_MAX, mathMax);
  setBuiltin(MATH_MIN, mathMin);
  setBuiltin(MATH_CEIL, mathCeil);
  setBuiltin(MATH_FLOOR, mathFloor);
  setBuiltin(MATH_RANDOM, mathRandom);
  setBuiltin(IO_PRINT, ioPrint);
#undef setBuiltin

  try(result_ref_t, mapCreate(value_t, global_environment->arena, 4), specials);
#define setSpecial(Label, Special)                                             \
  special.type = VALUE_TYPE_SPECIAL;                                           \
  special.value.special = (Special);                                           \
  try(result_ref_t, mapSet(value_t, specials, (Label), &special));

  value_t special;
  setSpecial(DEFINE, define);
  setSpecial(LET, let);
  setSpecial(COND, cond);
  setSpecial(FUNCTION, function);
#undef setSpecial

  return ok(result_ref_t, global_environment);
}

result_ref_t environmentCreate(environment_t *parent) {
  arena_t *arena = nullptr;
  try(result_ref_t, arenaCreate(ENVIRONMENT_MAX_SIZE), arena);

  environment_t *environment = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(environment_t)), environment);

  environment->arena = arena;
  environment->parent = parent;

  try(result_ref_t, mapCreate(value_t, arena, 32), environment->values);

  return ok(result_ref_t, environment);
}

void environmentDestroy(environment_t **self) {
  if (!self || !*self)
    return;

  arena_t *arena = (*self)->arena;
  // The environment is allocated on its own arena. This frees all the resources
  arenaDestroy(&arena);
  // Setting the reference to null for good measure
  *(self) = nullptr;
}

const value_t *environmentResolveSymbol(environment_t *self,
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

void environmentReset(environment_t *self) {
  assert(self);
  // The environment is allocated on its own arena. This resets its state
  arenaReset(self->arena);
}
