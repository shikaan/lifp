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
 * Returns the maximum of the arguments.
 * @name math:max
 * @param {...number} arguments - The arguments to compare.
 * @returns {number} The maximum value.
 * @example
 *   (math:max 1 2 3) ; returns 3
 */
const char *MATH_MAX = "math:max";
result_value_ref_t mathMax(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 1 argument. Got %zu", MATH_MAX,
          arguments->count);
  }

  number_t max_value = DBL_MIN;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", MATH_MAX,
            formatValueType(current.type));
    }
    if (current.as.number > max_value) {
      max_value = current.as.number;
    }
  }

  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = max_value}, pos);
}

/**
 * Returns the minimum of the arguments.
 * @name math:min
 * @param {...number} arguments - The arguments to compare.
 * @returns {number} The minimum value.
 * @example
 *   (math:min 1 2 3) ; returns 1
 */
const char *MATH_MIN = "math:min";
result_value_ref_t mathMin(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 1 argument. Got %zu", MATH_MIN,
          arguments->count);
  }

  number_t min_value = DBL_MAX;
  for (size_t i = 0; i < arguments->count; i++) {
    value_t current = listGet(value_t, arguments, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires numbers. Got %s.", MATH_MIN,
            formatValueType(current.type));
    }
    if (current.as.number < min_value) {
      min_value = current.as.number;
    }
  }

  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = min_value}, pos);
}

/**
 * Returns a random number between 0 and 1.
 * @name math:random!
 * @returns {number} A random number in [0, 1].
 * @example
 *   (math:random!) ; returns a random number between 0 and 1
 */
const char *MATH_RANDOM = "math:random!";
result_value_ref_t mathRandom(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 0) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires no arguments. Got %zu", MATH_RANDOM, arguments->count);
  }

  static bool initialized = false;
  if (!initialized) {
    srand((unsigned int)time(nullptr));
    initialized = true;
  }

  number_t rand_value = (number_t)rand() / RAND_MAX;
  return valueCreate(VALUE_TYPE_NUMBER, (value_as_t){.number = rand_value},
                     pos);
}

/**
 * Returns the smallest integer greater than or equal to the argument.
 * @name math:ceil
 * @param {number} argument - The number to ceil.
 * @returns {number} The smallest integer >= argument.
 * @example
 *   (math:ceil 2.3) ; returns 3
 */
const char *MATH_CEIL = "math:ceil";
result_value_ref_t mathCeil(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", MATH_CEIL, arguments->count);
  }

  value_t number = listGet(value_t, arguments, 0);
  if (number.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          number.position, "%s requires a number. Got %s.", MATH_CEIL,
          formatValueType(number.type));
  }

  return valueCreate(VALUE_TYPE_NUMBER,
                     (value_as_t){.number = ceil(number.as.number)}, pos);
}

/**
 * Returns the largest integer less than or equal to the argument.
 * @name math:floor
 * @param {number} argument - The number to floor.
 * @returns {number} The largest integer <= argument.
 * @example
 *   (math:floor 2.7) ; returns 2
 */
const char *MATH_FLOOR = "math:floor";
result_value_ref_t mathFloor(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", MATH_FLOOR, arguments->count);
  }

  value_t number = listGet(value_t, arguments, 0);
  if (number.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          number.position, "%s requires a number. Got %s.", MATH_FLOOR,
          formatValueType(number.type));
  }

  return valueCreate(VALUE_TYPE_NUMBER,
                     (value_as_t){.number = floor(number.as.number)}, pos);
}
