// Math operators for lifp. Provides mathematical utilities such as min, max,
// random, ceil, and floor for working with numbers and lists of numbers.
//
// ```lisp
// (math:max (list:from 1 2 3)) ; returns 3
// (math:min (list:from 1 2 3)) ; returns 1
// (math:random!) ; returns a random number between 0 and 1
// (math:ceil 2.3) ; returns 3
// (math:floor 2.7) ; returns 2
// ```
// ___HEADER_END___

#include "../../lib/result.h"
#include "../error.h"
#include "../fmt.h"
#include "../value.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/**
 * Returns the maximum value in a sequence of numbers.
 * @name math:max
 * @param {...number} numbers - The list of numbers.
 * @returns {number} The maximum value.
 * @example
 *   (math:max 1 2 3) ; returns 3
 */
const char *MATH_MAX = "math:max";
result_void_position_t mathMax(value_t *result, const value_list_t *values,
                               arena_t *arena) {
  (void)arena;
  if (values->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", MATH_MAX, values->count);
  }

  // Find the maximum value
  number_t max_value = DBL_MIN;
  for (size_t i = 0; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", MATH_MAX,
            formatValueType(current.type));
    }

    if (current.value.number > max_value) {
      max_value = current.value.number;
    }
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = max_value;

  return ok(result_void_position_t);
}

/**
 * Returns the minimum value in a sequence of numbers.
 * @name math:min
 * @param {...number} numbers - The list of numbers.
 * @returns {number} The minimum value.
 * @example
 *   (math:min 1 2 3) ; returns 1
 */
const char *MATH_MIN = "math:min";
result_void_position_t mathMin(value_t *result, const value_list_t *values,
                               arena_t *arena) {
  (void)arena;
  if (values->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", MATH_MIN, values->count);
  }

  // Find the minimum value
  number_t min_value = DBL_MAX;
  for (size_t i = 0; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", MATH_MIN,
            formatValueType(current.type));
    }

    if (current.value.number < min_value) {
      min_value = current.value.number;
    }
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = min_value;

  return ok(result_void_position_t);
}

/**
 * Returns a random number between 0 and 1.
 * @name math:random!
 * @returns {number} A random number in [0, 1].
 * @example
 *   (math:random!) ; returns a random number between 0 and 1
 */
const char *MATH_RANDOM = "math:random!";
result_void_position_t mathRandom(value_t *result, const value_list_t *values,
                                  arena_t *arena) {
  (void)arena;
  if (values->count != 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires no arguments. Got %zu", MATH_RANDOM, values->count);
  }

  // Initialize random number generator on first call
  static bool initialized = false;
  if (!initialized) {
    srand((unsigned int)time(nullptr));
    initialized = true;
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = (number_t)rand() / RAND_MAX;

  return ok(result_void_position_t);
}

/**
 * Returns the smallest integer greater than or equal to the given number.
 * @name math:ceil
 * @param {number} n - The number to ceil.
 * @returns {number} The smallest integer >= n.
 * @example
 *   (math:ceil 2.3) ; returns 3
 */
const char *MATH_CEIL = "math:ceil";
result_void_position_t mathCeil(value_t *result, const value_list_t *values,
                                arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", MATH_CEIL, values->count);
  }

  value_t number = listGet(value_t, values, 0);
  if (number.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          number.position, "%s requires a number. Got %s.", MATH_CEIL,
          formatValueType(number.type));
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = ceil(number.value.number);

  return ok(result_void_position_t);
}

/**
 * Returns the largest integer less than or equal to the given number.
 * @name math:floor
 * @param {number} n - The number to floor.
 * @returns {number} The largest integer <= n.
 * @example
 *   (math:floor 2.7) ; returns 2
 */
const char *MATH_FLOOR = "math:floor";
result_void_position_t mathFloor(value_t *result, const value_list_t *values,
                                 arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", MATH_FLOOR, values->count);
  }

  value_t number = listGet(value_t, values, 0);
  if (number.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          number.position, "%s requires a number. Got %s.", MATH_FLOOR,
          formatValueType(number.type));
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = floor(number.value.number);

  return ok(result_void_position_t);
}
