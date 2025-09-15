// Flow control utilities for lifp. These functions provide basic control over
// program execution, such as pausing execution for a specified duration.
//
// ```lisp
// (flow:sleep! 1000) ; pauses execution for ~1 second
// ```
// ___HEADER_END___

#include "../../lib/result.h"
#include "../error.h"
#include "../fmt.h"
#include "../value.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

/**
 * Suspends execution for a given number of milliseconds.
 * @name flow:sleep!
 * @param {number} milliseconds - The number of milliseconds to sleep.
 * @returns {nil} Returns nil after sleeping for the specified duration.
 * @example
 *   (flow:sleep! 1000) ; pauses for ~1 second
 */
const char *FLOW_SLEEP = "flow:sleep!";

result_void_position_t flowSleep(value_t *result, const value_list_t *arguments,
                                 arena_t *arena) {
  (void)arena;
  if (arguments->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", FLOW_SLEEP, arguments->count);
  }

  value_t ms_value = listGet(value_t, arguments, 0);
  if (ms_value.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          ms_value.position, "%s requires a number. Got %s.", FLOW_SLEEP,
          formatValueType(ms_value.type));
  }

  long milliseconds = lround(ms_value.value.number);
  if (milliseconds < 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, ms_value.position,
          "%s requires a non-negative number.", FLOW_SLEEP);
  }

  struct timespec timespec_val;
  timespec_val.tv_sec = milliseconds / 1000;
  timespec_val.tv_nsec = (milliseconds % 1000) * 1000000L;
  nanosleep(&timespec_val, nullptr);

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}
