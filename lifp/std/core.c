// Core lifp operators. For all intents and purposes, these should be thought as
// language keywords.
//
// ```lisp
// (and true false) ; returns false
// ```
// ___HEADER_END___

#include "../../lib/result.h"
#include "../error.h"
#include "../fmt.h"
#include "../value.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

/**
 * Sums a sequence of numbers.
 * @name +
 * @param {...number} numbers - The numbers to sum.
 * @returns {number} The sum of the numbers.
 * @example
 *   (+ 1 2 3) ; returns 6
 */
const char *SUM = "+";
result_void_position_t sum(value_t *result, const value_list_t *arguments,
                           arena_t *arena) {
  (void)arena;
  number_t sum = 0;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", SUM,
            formatValueType(current.type));
    }

    sum += current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = sum;

  return ok(result_void_position_t);
}

/**
 * Subtracts a sequence of numbers from the first.
 * @name -
 * @param {number} minuend - The initial number.
 * @param {...number} subtrahends - The numbers to subtract.
 * @returns {number} The result of the subtraction.
 * @example
 *   (- 6 3 2) ; returns 1
 */
const char *SUB = "-";
result_void_position_t subtract(value_t *result, const value_list_t *arguments,
                                arena_t *arena) {
  (void)arena;
  if (arguments->count == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires a non-empty list of numbers", SUB);
  }

  value_t first = listGet(value_t, arguments, 0);
  if (first.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first.position, "%s requires numbers. Got %s.", SUB,
          formatValueType(first.type));
  }

  number_t result_value = first.value.number;

  for (size_t i = 1; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", SUB,
            formatValueType(current.type));
    }

    result_value -= current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = result_value;

  return ok(result_void_position_t);
}

/**
 * Multiplies a sequence of numbers.
 * @name *
 * @param {...number} numbers - The numbers to multiply.
 * @returns {number} The product of the numbers.
 * @example
 *   (* 1 2 3) ; returns 6
 */
const char *MUL = "*";
result_void_position_t multiply(value_t *result, const value_list_t *arguments,
                                arena_t *arena) {
  (void)arena;
  number_t product = 1;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", MUL,
            formatValueType(current.type));
    }

    product *= current.value.number;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = product;

  return ok(result_void_position_t);
}

/**
 * Divides the first number by the rest.
 * @name /
 * @param {number} dividend - The initial number.
 * @param {...number} divisors - The numbers to divide by.
 * @returns {number} The result of the division.
 * @example
 *   (/ 6 3 2) ; returns 1
 */
const char *DIV = "/";
result_void_position_t divide(value_t *result, const value_list_t *arguments,
                              arena_t *arena) {
  (void)arena;
  if (arguments->count == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires at least 1 number.", DIV);
  }

  value_t first = listGet(value_t, arguments, 0);
  if (first.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first.position, "%s requires numbers. Got %s.", DIV,
          formatValueType(first.type));
  }

  number_t result_value = first.value.number;

  for (size_t i = 1; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", DIV,
            formatValueType(current.type));
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

/**
 * Performs a modulo division.
 * @name %
 * @param {number} dividend - The number to divide.
 * @param {number} divisor - The number to divide by.
 * @returns {number} The remainder after division.
 * @example
 *   (% 6 3) ; returns 0
 */
const char *MOD = "%";
result_void_position_t modulo(value_t *result, const value_list_t *arguments,
                              arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", MOD, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first.position, "%s requires numbers. Got %s.", MOD,
          formatValueType(first.type));
  }

  if (second.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          second.position, "%s requires numbers. Got %s.", MOD,
          formatValueType(second.type));
  }

  if (second.value.number == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, second.position,
          "%s modulo by zero", MOD);
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = fmod(first.value.number, second.value.number);

  return ok(result_void_position_t);
}

/**
 * Checks if two values are equal.
 * @name =
 * @param {any} a - The first value.
 * @param {any} b - The second value.
 * @returns {boolean} True if the values are equal, false otherwise.
 * @example
 *   (= 6 6) ; returns true
 */
const char *EQUAL = "=";
result_void_position_t equal(value_t *result, const value_list_t *arguments,
                             arena_t *arena) {
  (void)arena;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", EQUAL, arguments->count);
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
      are_equal = first.value.builtin == second.value.builtin;
      break;
    case VALUE_TYPE_SPECIAL:
      are_equal = first.value.special == second.value.special;
      break;
    case VALUE_TYPE_STRING:
      are_equal = strcmp(first.value.string, second.value.string) == 0;
    case VALUE_TYPE_CLOSURE:
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

/**
 * Checks if the first value is less than the second.
 * @name <
 * @param {number} a - The first number.
 * @param {number} b - The second number.
 * @returns {boolean} True if a is less than b, false otherwise.
 * @example
 *   (< 1 6) ; returns true
 */
const char *LESS_THAN = "<";
result_void_position_t lessThan(value_t *result, const value_list_t *arguments,
                                arena_t *arena) {
  (void)arena;
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LESS_THAN, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          error_pos, "%s requires numbers. Got %s.", LESS_THAN,
          formatValueType(error_type));
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number < second.value.number;

  return ok(result_void_position_t);
}

/**
 * Checks if the first value is greater than the second.
 * @name >
 * @param {number} a - The first number.
 * @param {number} b - The second number.
 * @returns {boolean} True if a is greater than b, false otherwise.
 * @example
 *   (> 1 6) ; returns false
 */
const char *GREATER_THAN = ">";
result_void_position_t
greaterThan(value_t *result, const value_list_t *arguments, arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", GREATER_THAN, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          error_pos, "%s requires numbers. Got %s.", GREATER_THAN,
          formatValueType(error_type));
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number > second.value.number;

  return ok(result_void_position_t);
}

/**
 * Checks if two values are not equal.
 * @name <>
 * @param {any} a - The first value.
 * @param {any} b - The second value.
 * @returns {boolean} True if the values are not equal, false otherwise.
 * @example
 *   (<> 6 6) ; returns false
 */
const char *NEQ = "<>";
result_void_position_t notEqual(value_t *result, const value_list_t *arguments,
                                arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", NEQ, arguments->count);
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
      are_equal = first.value.builtin == second.value.builtin;
      break;
    case VALUE_TYPE_SPECIAL:
      are_equal = first.value.special == second.value.special;
      break;
    case VALUE_TYPE_STRING:
      are_equal = strcmp(first.value.string, second.value.string) == 0;
    case VALUE_TYPE_CLOSURE:
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

/**
 * Checks if the first value is less than or equal to the second.
 * @name <=
 * @param {number} a - The first number.
 * @param {number} b - The second number.
 * @returns {boolean} True if a is less than or equal to b, false otherwise.
 * @example
 *   (<= 1 6) ; returns true
 */
const char *LEQ = "<=";
result_void_position_t lessEqual(value_t *result, const value_list_t *arguments,
                                 arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu.", LEQ, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          error_pos, "%s requires numbers. Got %s.", LEQ,
          formatValueType(error_type));
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number <= second.value.number;

  return ok(result_void_position_t);
}

/**
 * Checks if the first value is greater than or equal to the second.
 * @name >=
 * @param {number} a - The first number.
 * @param {number} b - The second number.
 * @returns {boolean} True if a is greater than or equal to b, false otherwise.
 * @example
 *   (>= 6 1) ; returns true
 */
const char *GEQ = ">=";
result_void_position_t
greaterEqual(value_t *result, const value_list_t *arguments, arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu.", GEQ, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_NUMBER || second.type != VALUE_TYPE_NUMBER) {
    position_t error_pos =
        first.type != VALUE_TYPE_NUMBER ? first.position : second.position;
    value_type_t error_type =
        first.type != VALUE_TYPE_NUMBER ? first.type : second.type;
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          error_pos, "%s requires numbers. Got %s.", GEQ,
          formatValueType(error_type));
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.number >= second.value.number;

  return ok(result_void_position_t);
}

/**
 * Logical AND operation on two boolean values.
 * @name and
 * @param {boolean} a - The first boolean value.
 * @param {boolean} b - The second boolean value.
 * @returns {boolean} True if both values are true, false otherwise.
 * @example
 *   (and true false) ; returns false
 */
const char *LOGICAL_AND = "and";
result_void_position_t
logicalAnd(value_t *result, const value_list_t *arguments, arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu.", LOGICAL_AND, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first.position, "%s requires booleans. Got %s.", LOGICAL_AND,
          formatValueType(first.type));
  }

  if (second.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          second.position, "%s requires booleans. Got %s.", LOGICAL_AND,
          formatValueType(second.type));
  }

  result->type = VALUE_TYPE_BOOLEAN;
  bool both_true = first.value.boolean;
  if (both_true) {
    both_true = second.value.boolean;
  }
  result->value.boolean = both_true;

  return ok(result_void_position_t);
}

/**
 * Logical OR operation on two boolean values.
 * @name or
 * @param {boolean} a - The first boolean value.
 * @param {boolean} b - The second boolean value.
 * @returns {boolean} True if either value is true, false otherwise.
 * @example
 *   (or true false) ; returns true
 */
const char *LOGICAL_OR = "or";
result_void_position_t logicalOr(value_t *result, const value_list_t *arguments,
                                 arena_t *arena) {
  (void)arena;
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LOGICAL_OR, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first.position, "%s requires booleans. Got %s.", LOGICAL_OR,
          formatValueType(first.type));
  }

  if (second.type != VALUE_TYPE_BOOLEAN) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          second.position, "%s requires booleans. Got %s.", LOGICAL_OR,
          formatValueType(second.type));
  }

  result->type = VALUE_TYPE_BOOLEAN;
  bool either_true = first.value.boolean;
  if (!either_true) {
    either_true = second.value.boolean;
  }
  result->value.boolean = either_true;

  return ok(result_void_position_t);
}
