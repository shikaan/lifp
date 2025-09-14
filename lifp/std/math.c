#include "../../lib/result.h"
#include "../error.h"
#include "../value.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Math max function - returns the maximum value in a list of numbers
const char *MATH_MAX = "math:max";
result_void_position_t mathMax(value_t *result, const value_list_t *values,
                               arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", MATH_MAX, values->count);
  }

  value_t list_value = listGet(value_t, values, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list. Got type %u", MATH_MAX, list_value.type);
  }

  value_list_t *list = &list_value.value.list;
  if (list->count == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a non-empty list", MATH_MAX);
  }

  // Find the maximum value
  number_t max_value = DBL_MIN;
  for (size_t i = 0; i < list->count; i++) {
    value_t current = listGet(value_t, list, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list of numbers. Got type %u", MATH_MAX,
            current.type);
    }

    if (current.value.number > max_value) {
      max_value = current.value.number;
    }
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = max_value;

  return ok(result_void_position_t);
}

// Math min function - returns the minimum value in a list of numbers
const char *MATH_MIN = "math:min";
result_void_position_t mathMin(value_t *result, const value_list_t *values,
                               arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", MATH_MIN, values->count);
  }

  value_t list_value = listGet(value_t, values, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list. Got type %u", MATH_MIN, list_value.type);
  }

  value_list_t *list = &list_value.value.list;
  if (list->count == 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a non-empty list", MATH_MIN);
  }

  // Find the minimum value
  number_t min_value = DBL_MAX;
  for (size_t i = 0; i < list->count; i++) {
    value_t current = listGet(value_t, list, i);
    if (current.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list of numbers. Got type %u", MATH_MIN,
            current.type);
    }

    if (current.value.number < min_value) {
      min_value = current.value.number;
    }
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = min_value;

  return ok(result_void_position_t);
}

// Math random function - returns a random number between 0 and 1
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

const char *MATH_CEIL = "math:ceil";
result_void_position_t mathCeil(value_t *result, const value_list_t *values,
                                arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", MATH_CEIL, values->count);
  }

  value_t number = listGet(value_t, values, 0);
  if (number.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, number.position,
          "%s requires a number. Got type %u", MATH_CEIL, number.type);
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = ceil(number.value.number);

  return ok(result_void_position_t);
}

const char *MATH_FLOOR = "math:floor";
result_void_position_t mathFloor(value_t *result, const value_list_t *values,
                                 arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", MATH_FLOOR, values->count);
  }

  value_t number = listGet(value_t, values, 0);
  if (number.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, number.position,
          "%s requires a number. Got type %u", MATH_FLOOR, number.type);
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = floor(number.value.number);

  return ok(result_void_position_t);
}
