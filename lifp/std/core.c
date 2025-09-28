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

void debugValue(value_t *value) {
  char buf[256];
  int offset = 0;
  formatValue(value, 256, buf, &offset);
  puts(buf);
}

/**
 * Sums arguments.
 * @name +
 * @param {...number} arguments - The numbers to sum.
 * @returns {number} The sum of the arguments.
 * @example
 *   (+ 1 2 3) ; returns 6
 */
const char *SUM = "+";
result_value_ref_t sum(const value_array_t *arguments, position_t pos) {
  number_t total_sum = 0;
  for (size_t i = 0; i < arguments->count; i++) {

    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", SUM,
            formatValueType(current.type));
    }

    total_sum += current.as.number;
  }

  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = total_sum}, pos);
}

/**
 * Subtracts arguments from the first argument.
 * @name -
 * @param {number} argument - The initial number.
 * @param {...number} arguments - The numbers to subtract.
 * @returns {number} The result of the subtraction.
 * @example
 *   (- 6 3 2) ; returns 1
 */
const char *SUB = "-";
result_value_ref_t subtract(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 1 argument. Got %zu", SUB, arguments->count);
  }

  value_t first_value = listGet(value_t, arguments, 0);
  if (first_value.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first_value.position, "%s requires numbers. Got %s.", SUB,
          formatValueType(first_value.type));
  }

  number_t result_value = first_value.as.number;

  for (size_t i = 1; i < arguments->count; i++) {
    value_t current_value = listGet(value_t, arguments, i);
    if (current_value.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current_value.position, "%s requires numbers. Got %s.", SUB,
            formatValueType(current_value.type));
    }

    result_value -= current_value.as.number;
  }

  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = result_value},
                     pos);
}

/**
 * Multiplies arguments.
 * @name *
 * @param {...number} arguments - The numbers to multiply.
 * @returns {number} The product of the arguments.
 * @example
 *   (* 1 2 3) ; returns 6
 */
const char *MUL = "*";
result_value_ref_t multiply(const value_array_t *arguments, position_t pos) {
  number_t total_product = 1;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current_value = listGet(value_t, arguments, i);
    if (current_value.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current_value.position, "%s requires numbers. Got %s.", MUL,
            formatValueType(current_value.type));
    }

    total_product *= current_value.as.number;
  }

  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = total_product},
                     pos);
}

/**
 * Divides the first argument by the rest.
 * @name /
 * @param {number} argument - The initial number.
 * @param {...number} arguments - The numbers to divide by.
 * @returns {number} The result of the division.
 * @example
 *   (/ 6 3 2) ; returns 1
 */
const char *DIV = "/";
result_value_ref_t divide(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 1 argument. Got %zu", DIV, arguments->count);
  }

  value_t first_value = listGet(value_t, arguments, 0);
  if (first_value.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first_value.position, "%s requires numbers. Got %s.", DIV,
          formatValueType(first_value.type));
  }

  number_t result_value = first_value.as.number;

  for (size_t i = 1; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", DIV,
            formatValueType(current.type));
    }
    if (current.as.number == 0) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s division by zero", DIV);
    }
    result_value /= current.as.number;
  }

  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = result_value},
                     pos);
}

/**
 * Performs a modulo division.
 * @name %
 * @param {number} argument - The number to divide.
 * @param {number} argument - The number to divide by.
 * @returns {number} The remainder after division.
 * @example
 *   (% 6 3) ; returns 0
 */
const char *MOD = "%";
result_value_ref_t modulo(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", MOD, arguments->count);
  }

  value_t first_value = listGet(value_t, arguments, 0);
  if (first_value.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          first_value.position, "%s requires numbers. Got %s.", MOD,
          formatValueType(first_value.type));
  }

  value_t second = listGet(value_t, arguments, 1);
  if (second.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          second.position, "%s requires numbers. Got %s.", MOD,
          formatValueType(second.type));
  }

  if (second.as.number == 0) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, second.position,
          "%s modulo by zero", MOD);
  }

  number_t remainder_value = fmod(first_value.as.number, second.as.number);
  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = remainder_value},
                     pos);
}

/**
 * Checks if two arguments are equal.
 * @name =
 * @param {any} argument - The first argument.
 * @param {any} argument - The second argument.
 * @returns {boolean} True if the arguments are equal, false otherwise.
 * @example
 *   (= 6 6) ; returns true
 */
const char *EQUAL = "=";
result_value_ref_t equal(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", EQUAL, arguments->count);
  }

  value_t left_value = listGet(value_t, arguments, 0);
  value_t right_value = listGet(value_t, arguments, 1);

  if (left_value.type != right_value.type) {
    return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false}, pos);
  }

  bool is_equal = false;
  switch (left_value.type) {
  case VALUE_TYPE_NUMBER:
    is_equal = left_value.as.number == right_value.as.number;
    break;
  case VALUE_TYPE_BOOLEAN:
    is_equal = left_value.as.boolean == right_value.as.boolean;
    break;
  case VALUE_TYPE_NIL:
    is_equal = true;
    break;
  case VALUE_TYPE_BUILTIN:
    is_equal = left_value.as.builtin == right_value.as.builtin;
    break;
  case VALUE_TYPE_SPECIAL:
    is_equal = left_value.as.special == right_value.as.special;
    break;
  case VALUE_TYPE_STRING:
    is_equal = strcmp(left_value.as.string, right_value.as.string) == 0;
    break;
  case VALUE_TYPE_CLOSURE:
  case VALUE_TYPE_LIST:
  default:
    is_equal = false;
    break;
  }

  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = is_equal},
                     pos);
}

/**
 * Checks if two arguments are not equal.
 * @name <>
 * @param {any} argument - The first argument.
 * @param {any} argument - The second argument.
 * @returns {boolean} True if the arguments are not equal, false otherwise.
 * @example
 *   (<> 6 6) ; returns false
 */
const char *NEQ = "<>";
result_value_ref_t notEqual(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", NEQ, arguments->count);
  }

  value_t first = listGet(value_t, arguments, 0);
  value_t second = listGet(value_t, arguments, 1);

  if (first.type != second.type) {
    return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true}, pos);
  }

  bool are_equal = false;
  switch (first.type) {
  case VALUE_TYPE_NUMBER:
    are_equal = first.as.number == second.as.number;
    break;
  case VALUE_TYPE_BOOLEAN:
    are_equal = first.as.boolean == second.as.boolean;
    break;
  case VALUE_TYPE_NIL:
    are_equal = true;
    break;
  case VALUE_TYPE_BUILTIN:
    are_equal = first.as.builtin == second.as.builtin;
    break;
  case VALUE_TYPE_SPECIAL:
    are_equal = first.as.special == second.as.special;
    break;
  case VALUE_TYPE_STRING:
    are_equal = strcmp(first.as.string, second.as.string) == 0;
    break;
  case VALUE_TYPE_CLOSURE:
  case VALUE_TYPE_LIST:
  default:
    are_equal = false;
    break;
  }

  return valueCreate(VALUE_TYPE_BOOLEAN,
                     (value_as_t){.boolean = (!are_equal) != 0}, pos);
}

/**
 * Checks if the first argument is less than the second.
 * @name <
 * @param {number} argument - The first argument.
 * @param {number} argument - The second argument.
 * @returns {boolean} True if the first argument is less than the second, false
 * otherwise.
 * @example
 *   (< 1 6) ; returns true
 */
const char *LESS_THAN = "<";
result_value_ref_t lessThan(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu", LESS_THAN,
          arguments->count);
  }

  for (size_t i = 0; i + 1 < arguments->count; i++) {
    value_t left_number = listGet(value_t, arguments, i);
    value_t right_number = listGet(value_t, arguments, i + 1);
    if (left_number.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            left_number.position, "%s requires numbers. Got %s.", LESS_THAN,
            formatValueType(left_number.type));
    }
    if (right_number.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            right_number.position, "%s requires numbers. Got %s.", LESS_THAN,
            formatValueType(right_number.type));
    }
    if (!(left_number.as.number < right_number.as.number)) {
      return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false},
                         pos);
    }
  }
  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true}, pos);
}

/**
 * Checks if the first argument is greater than the second.
 * @name >
 * @param {number} argument - The first argument.
 * @param {number} argument - The second argument.
 * @returns {boolean} True if the first argument is greater than the second,
 * false otherwise.
 * @example
 *   (> 1 6) ; returns false
 */
const char *GREATER_THAN = ">";
result_value_ref_t greaterThan(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu", GREATER_THAN,
          arguments->count);
  }

  for (size_t i = 0; i + 1 < arguments->count; i++) {
    value_t left_number = listGet(value_t, arguments, i);
    value_t right_number = listGet(value_t, arguments, i + 1);
    if (left_number.type != VALUE_TYPE_NUMBER) {
      debugValue(&left_number);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            left_number.position, "%s requires numbers. Got %s.", GREATER_THAN,
            formatValueType(left_number.type));
    }
    if (right_number.type != VALUE_TYPE_NUMBER) {
      debugValue(&right_number);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            right_number.position, "%s requires numbers. Got %s.", GREATER_THAN,
            formatValueType(right_number.type));
    }
    if (!(left_number.as.number > right_number.as.number)) {
      return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false},
                         pos);
    }
  }
  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true}, pos);
}

/**
 * Checks if the first argument is less than or equal to the second.
 * @name <=
 * @param {number} argument - The first argument.
 * @param {number} argument - The second argument.
 * @returns {boolean} True if the first argument is less than or equal to the
 * second, false otherwise.
 * @example
 *   (<= 1 6) ; returns true
 */
const char *LEQ = "<=";
result_value_ref_t lessEqual(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu.", LEQ, arguments->count);
  }

  for (size_t i = 0; i + 1 < arguments->count; i++) {
    value_t left_number = listGet(value_t, arguments, i);
    value_t right_number = listGet(value_t, arguments, i + 1);
    if (left_number.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            left_number.position, "%s requires numbers. Got %s.", LEQ,
            formatValueType(left_number.type));
    }
    if (right_number.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            right_number.position, "%s requires numbers. Got %s.", LEQ,
            formatValueType(right_number.type));
    }
    if (!(left_number.as.number <= right_number.as.number)) {
      return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false},
                         pos);
    }
  }
  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true}, pos);
}

/**
 * Checks if the first argument is greater than or equal to the second.
 * @name >=
 * @param {number} argument - The first argument.
 * @param {number} argument - The second argument.
 * @returns {boolean} True if the first argument is greater than or equal to the
 * second, false otherwise.
 * @example
 *   (>= 6 1) ; returns true
 */
const char *GEQ = ">=";
result_value_ref_t greaterEqual(const value_array_t *arguments,
                                position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu.", GEQ, arguments->count);
  }

  for (size_t i = 0; i + 1 < arguments->count; i++) {
    value_t left_number = listGet(value_t, arguments, i);
    value_t right_number = listGet(value_t, arguments, i + 1);
    if (left_number.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            left_number.position, "%s requires numbers. Got %s.", GEQ,
            formatValueType(left_number.type));
    }
    if (right_number.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            right_number.position, "%s requires numbers. Got %s.", GEQ,
            formatValueType(right_number.type));
    }
    if (!(left_number.as.number >= right_number.as.number)) {
      return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false},
                         pos);
    }
  }
  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true}, pos);
}

/**
 * Logical AND operation on boolean arguments.
 * @name and
 * @param {...boolean} arguments - The boolean arguments.
 * @returns {boolean} True if all arguments are true, false otherwise.
 * @example
 *   (and true false) ; returns false
 */
const char *LOGICAL_AND = "and";
result_value_ref_t logicalAnd(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu.", LOGICAL_AND,
          arguments->count);
  }

  for (size_t i = 0; i < arguments->count; i++) {
    value_t current_boolean = listGet(value_t, arguments, i);
    if (current_boolean.type != VALUE_TYPE_BOOLEAN) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current_boolean.position, "%s requires booleans. Got %s.",
            LOGICAL_AND, formatValueType(current_boolean.type));
    }
    if (!current_boolean.as.boolean) {
      return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false},
                         pos);
    }
  }

  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true}, pos);
}

/**
 * Logical OR operation on boolean arguments.
 * @name or
 * @param {...boolean} arguments - The boolean arguments.
 * @returns {boolean} True if any argument is true, false otherwise.
 * @example
 *   (or true false) ; returns true
 */
const char *LOGICAL_OR = "or";
result_value_ref_t logicalOr(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu", LOGICAL_OR,
          arguments->count);
  }

  for (size_t i = 0; i < arguments->count; i++) {
    value_t current_boolean = listGet(value_t, arguments, i);
    if (current_boolean.type != VALUE_TYPE_BOOLEAN) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current_boolean.position, "%s requires booleans. Got %s.",
            LOGICAL_OR, formatValueType(current_boolean.type));
    }
    if (current_boolean.as.boolean) {
      return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = true},
                         pos);
    }
  }
  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = false}, pos);
}
