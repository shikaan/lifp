// List manipulation operators for lifp. These functions provide core list
// operations such as counting, creating, accessing, mapping, filtering, and
// iterating over lists.
//
// ```lisp
// (list:count (list:from 1 2 3)) ; returns 3
// (list:nth 1 (list:from 10 20 30)) ; returns 20
// (list:map (fn (x i) (* x 2)) (list:from 1 2 3)) ; returns (2 4 6)
// ```
// ___HEADER_END___

#include "../../lib/result.h"
#include "../error.h"
#include "../evaluate.h"
#include "../fmt.h"
#include "../value.h"
#include <stddef.h>

/**
 * Counts the number of elements in a list.
 * @name list:count
 * @param {list} list - The list to count elements of.
 * @returns {number} The number of elements in the list.
 * @example
 *   (list:count (1 2 3)) ; returns 3
 */
const char *LIST_COUNT = "list:count";
result_value_ref_t listCount(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", LIST_COUNT, arguments->count);
  }

  value_t list_value = listGet(value_t, arguments, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list. Got %s.", LIST_COUNT,
          formatValueType(list_value.type));
  }

  return valueCreate(
      VALUE_TYPE_NUMBER,
      (value_as_t){.number = (number_t)list_value.as.list->count}, pos);
}

/**
 * Creates a list from the given arguments.
 * @name list:from
 * @param {...any} values - The values to include in the list.
 * @returns {list} A new list containing the provided values.
 * @example
 *   (list:from 1 2 3) ; returns (1 2 3)
 */
const char *LIST_FROM = "list:from";
result_value_ref_t listFrom(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 1 argument. Got %zu", LIST_FROM,
          arguments->count);
  }

  value_array_t *value_list = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(arguments->count), pos,
              value_list);

  value_t *result = nullptr;
  try(result_value_ref_t,
      valueCreate(VALUE_TYPE_LIST, (value_as_t){.list = value_list}, pos))
      result->type = VALUE_TYPE_LIST;

  // Copy all arguments to the new list
  for (size_t i = 0; i < arguments->count; i++) {
    value_t source = listGet(value_t, arguments, i);
    value_t *duplicated;
    tryWithMeta(result_value_ref_t, valueDeepCopy(&source), pos, duplicated);
    value_list->data[i] = *duplicated;
    deallocSafe(&duplicated);
  }

  return ok(result_value_ref_t);
}

/**
 * Returns the nth element of a list, or nil if out of bounds.
 * @name list:nth
 * @param {number} index - The index of the element to retrieve.
 * @param {list} list - The list to access.
 * @returns {any} The nth element, or nil if out of bounds.
 * @example
 *   (list:nth 1 (10 20 30)) ; returns 20
 */
const char *LIST_NTH = "list:nth";
result_value_ref_t listNth(const value_array_t *arguments, position_t pos) {

  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", LIST_NTH, arguments->count);
  }

  value_t index_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (index_value.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          index_value.position,
          "%s requires a number as first argument. Got %s.", LIST_NTH,
          formatValueType(index_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_NTH, formatValueType(list_value.type));
  }

  number_t index = index_value.as.number;
  value_array_t *list = list_value.as.list;

  if (index < 0 || (size_t)index >= list->count ||
      index != (number_t)(size_t)index) {
    return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, pos);
  }

  value_t value = listGet(value_t, list, (size_t)index);
  return valueDeepCopy(&value);
}

/**
 * Maps a function over a list, returning a new list of results.
 * The function receives each element and its index.
 * @name list:map
 * @param {function} fn - The function to apply (fn element index).
 * @param {list} list - The list to map over.
 * @returns {list} A new list with mapped values.
 * @example
 *   (list:map (fn (x i) (* x 2)) (1 2 3)) ; returns (2 4 6)
 */
const char *LIST_MAP = "list:map";
result_value_ref_t listMap(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", LIST_MAP, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_MAP,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_MAP, formatValueType(list_value.type));
  }

  value_array_t *input_list = list_value.as.list;

  value_array_t *mapped_list = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(input_list->count), pos,
              mapped_list);

  value_array_t *closure_args = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(2), pos, closure_args);

  for (size_t i = 0; i < input_list->count; i++) {
    value_t input = listGet(value_t, input_list, i);
    value_t index = {
        .type = VALUE_TYPE_NUMBER,
        .as.number = (number_t)i,
        .position = input.position,
    };

    closure_args->data[0] = input;
    closure_args->data[1] = index;

    value_t *mapped;
    try(result_value_ref_t, invokeClosure(&closure_value, closure_args),
        mapped);
    mapped_list->data[i] = *mapped;
    deallocSafe(&mapped);
  }

  valueArrayDestroy(&closure_args);
  return valueCreate(VALUE_TYPE_LIST, (value_as_t){.list = mapped_list}, pos);
}

/**
 * Applies a function to each element of a list for side effects.
 * The function receives each element and its index.
 * @name list:each
 * @param {function} fn - The function to apply (fn element index).
 * @param {list} list - The list to iterate over.
 * @returns {nil} Always returns nil.
 * @example
 *   (list:each (fn (x i) (print x)) (1 2 3))
 */
const char *LIST_EACH = "list:each";
result_value_ref_t listEach(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", LIST_EACH, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_EACH,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_EACH, formatValueType(list_value.type));
  }

  value_array_t *input_list = list_value.as.list;

  value_array_t *closure_args = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(2), pos, closure_args);

  for (size_t i = 0; i < input_list->count; i++) {
    value_t input = listGet(value_t, input_list, i);
    value_t index = {
        .type = VALUE_TYPE_NUMBER,
        .as.number = (number_t)i,
        .position = input.position,
    };

    closure_args->data[0] = input;
    closure_args->data[1] = index;

    value_t *ignored;
    try(result_value_ref_t, invokeClosure(&closure_value, closure_args),
        ignored);
    valueDestroy(&ignored);
  }

  valueArrayDestroy(&closure_args);
  return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, pos);
}

/**
 * Filters a list using a predicate function.
 * The function receives each element and its index, and should return a
 * boolean.
 * @name list:filter
 * @param {function} fn - The predicate function (fn element index).
 * @param {list} list - The list to filter.
 * @returns {list} A new list with elements for which the predicate returned
 * true.
 * @example
 *   (list:filter (fn (x i) (> x 1)) (1 2 3)) ; returns (2 3)
 */
const char *LIST_FILTER = "list:filter";
result_value_ref_t listFilter(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", LIST_FILTER, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s", LIST_FILTER,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s",
          LIST_FILTER, formatValueType(list_value.type));
  }

  value_array_t *input_list = list_value.as.list;

  value_array_t *filtered_list = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(input_list->count), pos,
              filtered_list);

  value_array_t *closure_args = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(2), pos, closure_args);

  size_t filtered_count = 0;
  for (size_t i = 0; i < input_list->count; i++) {
    value_t input = listGet(value_t, input_list, i);
    value_t index = {
        .type = VALUE_TYPE_NUMBER,
        .as.number = (number_t)i,
        .position = input.position,
    };

    closure_args->data[0] = input;
    closure_args->data[1] = index;

    value_t *predicate_result;
    try(result_value_ref_t, invokeClosure(&closure_value, closure_args),
        predicate_result);

    if (predicate_result->type != VALUE_TYPE_BOOLEAN) {
      valueDestroy(&predicate_result);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            list_value.position,
            "%s requires a function returning a boolean as first argument. Got "
            "return type %s.",
            LIST_FILTER, formatValueType(predicate_result->type));
    }

    if (predicate_result->as.boolean) {
      value_t *duplicate;
      tryWithMeta(result_value_ref_t, valueDeepCopy(&index), pos, duplicate);
      filtered_list->data[filtered_count++] = *duplicate;
      deallocSafe(&duplicate);
    }

    valueDestroy(&predicate_result);
  }

  filtered_list->count = filtered_count;
  valueArrayDestroy(&closure_args);
  return valueCreate(VALUE_TYPE_LIST, (value_as_t){.list = filtered_list}, pos);
}

/**
 * Calls a function a given number of times, collecting the results in a list.
 * The function receives the current index.
 * @name list:times
 * @param {function} fn - The function to call (fn index).
 * @param {number} count - The number of times to call the function.
 * @returns {list} A new list with the results.
 * @example
 *   (list:times (fn (i) (* i 2)) 3) ; returns (0 2 4)
 */
const char *LIST_TIMES = "list:times";
result_value_ref_t listTimes(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", LIST_TIMES, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t repeats_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_TIMES,
          formatValueType(closure_value.type));
  }

  if (repeats_value.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          repeats_value.position,
          "%s requires a number as second argument. Got %s.", LIST_TIMES,
          formatValueType(repeats_value.type));
  }

  size_t repeats = (size_t)repeats_value.as.number;

  value_array_t *repeated_list = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(repeats), pos,
              repeated_list);

  value_array_t *closure_args = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(1), pos, closure_args);

  for (size_t i = 0; i < repeats; i++) {
    value_t index = {
        .type = VALUE_TYPE_NUMBER,
        .as.number = (number_t)i,
        .position = pos,
    };
    closure_args->data[0] = index;

    value_t *mapped;
    try(result_value_ref_t, invokeClosure(&closure_value, closure_args),
        mapped);
    repeated_list->data[i] = *mapped;
    deallocSafe(&mapped);
  }

  valueArrayDestroy(&closure_args);
  return valueCreate(VALUE_TYPE_LIST, (value_as_t){.list = repeated_list}, pos);
}

/**
 * Reduces a list to a single value by repeatedly applying a reducer function.
 * The function receives the accumulated value, the current element, and its
 * index.
 * @name list:reduce
 * @param {function} fn - The reducer function (fn previous current index).
 * @param {any} initial - The initial value to accumulate over.
 * @param {list} list - The list to reduce.
 * @returns {any} The final reduced value.
 * @example
 *   (list:reduce (fn (p c i) (+ p c)) 0 (1 2 3)) ; returns 6
 */
const char *LIST_REDUCE = "list:reduce";
result_value_ref_t listReduce(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 3 arguments. Got %zu", LIST_REDUCE, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t initial_value = listGet(value_t, arguments, 1);
  value_t list_value = listGet(value_t, arguments, 2);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_REDUCE,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as third argument. Got %s.",
          LIST_REDUCE, formatValueType(list_value.type));
  }

  value_array_t *input_list = list_value.as.list;

  value_t *accum = nullptr;
  tryWithMeta(result_value_ref_t, valueDeepCopy(&initial_value), pos, accum);

  value_array_t *closure_args = nullptr;
  tryWithMeta(result_value_ref_t, valueArrayCreate(3), pos, closure_args);

  for (size_t i = 0; i < input_list->count; i++) {
    value_t current = listGet(value_t, input_list, i);
    value_t index = {
        .type = VALUE_TYPE_NUMBER,
        .as.number = (number_t)i,
        .position = current.position,
    };

    closure_args->data[0] = *accum;
    closure_args->data[1] = current;
    closure_args->data[2] = index;

    value_t *result = nullptr;
    try(result_value_ref_t, invokeClosure(&closure_value, closure_args),
        result);
    valueDestroy(&accum);
    accum = result;
  }

  valueArrayDestroy(&closure_args);
  return ok(result_value_ref_t, accum);
}
