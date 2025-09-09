#include "../../lib/result.h"
#include "../error.h"
#include "../value.h"
#include <math.h>
#include <stdint.h>

const char *SUM = "+";
result_void_position_t sum(value_t *result, const value_list_t *arguments,
                           arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  number_t sum = 0;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list of numbers. Got type %u", SUM, current.type);
    }

    sum += current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = sum;

  return ok(result_void_position_t);
}

const char *SUB = "-";
result_void_position_t subtract(value_t *result, const value_list_t *arguments,
                                arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires a non-empty list of numbers", SUB);
  }

  value_t first = listGet(value_t, arguments, 0);
  if (first.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list numbers. Got type %u", SUB, first.type);
  }

  number_t result_value = first.value.number;

  for (size_t i = 1; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
            "%s requires a list numbers. Got type %u", SUB, current.type);
    }

    result_value -= current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = result_value;

  return ok(result_void_position_t);
}

const char *MUL = "*";
result_void_position_t multiply(value_t *result, const value_list_t *arguments,
                                arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  number_t product = 1;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list numbers. Got type %u", MUL, current.type);
    }

    product *= current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = product;

  return ok(result_void_position_t);
}

const char *DIV = "/";
result_void_position_t divide(value_t *result, const value_list_t *arguments,
                              arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires a non-empty list of numbers", DIV);
  }

  value_t first = listGet(value_t, arguments, 0);
  if (first.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list numbers. Got type %u", DIV, first.type);
  }

  number_t result_value = first.value.number;

  for (size_t i = 1; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list numbers. Got type %u", DIV, current.type);
    }

    if (current.value.number == 0) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s division by zero", DIV);
    }

    result_value /= current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = result_value;

  return ok(result_void_position_t);
}

const char *MOD = "%";
result_void_position_t modulo(value_t *result, const value_list_t *arguments,
                              arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", MOD, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list numbers. Got type %u", MOD, first.type);
  }

  if (second.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, second.position,
          "%s requires a list numbers. Got type %u", MOD, second.type);
  }

  if (second.value.number == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, second.position,
          "%s modulo by zero", MOD);
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = fmod(first.value.number, second.value.number);

  return ok(result_void_position_t);
}

const char *EQUAL = "=";
result_void_position_t equal(value_t *result, const value_list_t *arguments,
                             arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", EQUAL, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  bool are_equal = false;
  if (first.type == second.type) {
    switch (first.type) {
    case VALUE_TYPE_NUMBER:
      // NOTE: this is imprecise equality (due to floats) but it's how most
      // programming languages handle it
      are_equal = first.value.number == second.value.number;
      break;
    case VALUE_TYPE_BOOLEAN:
      are_equal = first.value.boolean == second.value.boolean;
      break;
    case VALUE_TYPE_NIL:
      are_equal = true;
      break;
    case VALUE_TYPE_BUILTIN:
    case VALUE_TYPE_SPECIAL:
    case VALUE_TYPE_CLOSURE:
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_LIST:
    default:
      are_equal = false;
      break;
    }
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = are_equal;

  return ok(result_void_position_t);
}

const char *LESS_THAN = "<";
result_void_position_t lessThan(value_t *result, const value_list_t *arguments,
                                arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LESS_THAN,
          arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, error_pos,
          "%s requires a list numbers. Got type %u", LESS_THAN, error_type);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number < second.value.number;

  return ok(result_void_position_t);
}

const char *GREATER_THAN = ">";
result_void_position_t greaterThan(value_t *result,
                                   const value_list_t *arguments,
                                   arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", GREATER_THAN,
          arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, error_pos,
          "%s requires a list numbers. Got type %u", GREATER_THAN, error_type);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number > second.value.number;

  return ok(result_void_position_t);
}

const char *NEQ = "!=";
result_void_position_t notEqual(value_t *result, const value_list_t *arguments,
                                arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", NEQ, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  bool are_equal = false;
  if (first.type == second.type) {
    switch (first.type) {
    case VALUE_TYPE_NUMBER:
      are_equal = first.value.number == second.value.number;
      break;
    case VALUE_TYPE_BOOLEAN:
      are_equal = first.value.boolean == second.value.boolean;
      break;
    case VALUE_TYPE_NIL:
      are_equal = true;
      break;
    case VALUE_TYPE_BUILTIN:
    case VALUE_TYPE_SPECIAL:
    case VALUE_TYPE_CLOSURE:
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_LIST:
    default:
      are_equal = false;
      break;
    }
  }

  result->type = VALUE_TYPE_BOOLEAN;
  // Logical NOT operation on are_equal - flip the bit
  result->value.boolean = (bool)(!are_equal);

  return ok(result_void_position_t);
}

const char *LEQ = "<=";
result_void_position_t lessEqual(value_t *result, const value_list_t *arguments,
                                 arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LEQ, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, error_pos,
          "%s requires a list numbers. Got type %u", LEQ, error_type);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number <= second.value.number;

  return ok(result_void_position_t);
}

const char *GEQ = ">=";
result_void_position_t greaterEqual(value_t *result,
                                    const value_list_t *arguments,
                                    arena_t *arena,
                                    environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", GEQ, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, error_pos,
          "%s requires a list numbers. Got type %u", GEQ, error_type);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number >= second.value.number;

  return ok(result_void_position_t);
}

const char *LOGICAL_AND = "and";
result_void_position_t logicalAnd(value_t *result,
                                  const value_list_t *arguments, arena_t *arena,
                                  environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LOGICAL_AND,
          arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list booleans. Got type %u", LOGICAL_AND, first.type);
  }

  if (second.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, second.position,
          "%s requires a list booleans. Got type %u", LOGICAL_AND, second.type);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  bool both_true = first.value.boolean;
  if (both_true) {
    both_true = second.value.boolean;
  }
  result->value.boolean = both_true;

  return ok(result_void_position_t);
}

const char *LOGICAL_OR = "or";
result_void_position_t logicalOr(value_t *result, const value_list_t *arguments,
                                 arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LOGICAL_OR,
          arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list booleans. Got type %u", LOGICAL_OR, first.type);
  }

  if (second.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, second.position,
          "%s requires a list booleans. Got type %u", LOGICAL_OR, second.type);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  bool either_true = first.value.boolean;
  if (!either_true) {
    either_true = second.value.boolean;
  }
  result->value.boolean = either_true;

  return ok(result_void_position_t);
}
