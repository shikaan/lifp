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

/**
 * Counts the number of elements in a list.
 * @name list:count
 * @param {list} list - The list to count elements of.
 * @returns {number} The number of elements in the list.
 * @example
 *   (list:count (1 2 3)) ; returns 3
 */
const char *LIST_COUNT = "list:count";
result_void_position_t listCount(value_t *result, const value_list_t *arguments,
                                 arena_t *arena) {
  (void)arena;
  if (arguments->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", LIST_COUNT, arguments->count);
  }

  value_t list_value = listGet(value_t, arguments, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list. Got %s.", LIST_COUNT,
          formatValueType(list_value.type));
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = (number_t)list_value.value.list->count;

  return ok(result_void_position_t);
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
result_void_position_t listFrom(value_t *result, const value_list_t *arguments,
                                arena_t *arena) {
  result->type = VALUE_TYPE_LIST;

  value_list_t *new_list = nullptr;
  tryWithMeta(result_void_position_t,
              listCreate(value_t, arena, arguments->count), result->position,
              new_list);
  result->value.list = new_list;

  if (arguments->count > 0) {
    // Copy all arguments to the new list
    for (size_t i = 0; i < arguments->count; i++) {
      value_t source = listGet(value_t, arguments, i);
      value_t duplicated;
      tryWithMeta(result_void_position_t,
                  valueCopy(&source, &duplicated, arena), result->position);
      tryWithMeta(result_void_position_t,
                  listAppend(value_t, result->value.list, &duplicated),
                  source.position);
    }
  } else {
    new_list->data = nullptr;
  }

  return ok(result_void_position_t);
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
result_void_position_t listNth(value_t *result, const value_list_t *arguments,
                               arena_t *arena) {
  (void)arena;

  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LIST_NTH, arguments->count);
  }

  value_t index_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (index_value.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          index_value.position,
          "%s requires a number as first argument. Got %s.", LIST_NTH,
          formatValueType(index_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_NTH, formatValueType(list_value.type));
  }

  number_t index = index_value.value.number;
  value_list_t *list = list_value.value.list;

  if (index < 0 || (size_t)index >= list->count ||
      index != (number_t)(size_t)index) {
    result->type = VALUE_TYPE_NIL;
    result->value.nil = nullptr;
  } else {
    *result = listGet(value_t, list, (size_t)index);
  }

  return ok(result_void_position_t);
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
result_void_position_t listMap(value_t *result, const value_list_t *arguments,
                               arena_t *arena) {
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LIST_MAP, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_MAP,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_MAP, formatValueType(list_value.type));
  }

  value_list_t *input_list = list_value.value.list;
  closure_t closure = closure_value.value.closure;

  value_list_t *mapped_list = nullptr;
  tryWithMeta(result_void_position_t,
              listCreate(value_t, arena, input_list->count), result->position,
              mapped_list);

  for (size_t i = 0; i < input_list->count; i++) {
    value_t input = listGet(value_t, input_list, i);

    value_list_t *closure_args = nullptr;
    tryWithMeta(result_void_position_t, listCreate(value_t, arena, 2),
                result->position, closure_args);

    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &input), result->position);

    value_t index;
    tryWithMeta(result_void_position_t, valueInit(&index, arena, (number_t)i),
                result->position);
    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &index), result->position);

    value_t mapped;
    try(result_void_position_t,
        invokeClosure(&mapped, closure, closure_args, arena, nullptr));

    tryWithMeta(result_void_position_t,
                listAppend(value_t, mapped_list, &mapped), result->position);
  }

  result->type = VALUE_TYPE_LIST;
  result->value.list = mapped_list;

  return ok(result_void_position_t);
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
result_void_position_t listEach(value_t *result, const value_list_t *arguments,
                                arena_t *arena) {
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LIST_EACH, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_EACH,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_EACH, formatValueType(list_value.type));
  }

  value_list_t *input_list = list_value.value.list;
  closure_t closure = closure_value.value.closure;

  for (size_t i = 0; i < input_list->count; i++) {
    value_t input = listGet(value_t, input_list, i);

    value_list_t *closure_args = nullptr;
    tryWithMeta(result_void_position_t, listCreate(value_t, arena, 2),
                result->position, closure_args);

    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &input), result->position);

    value_t index;
    tryWithMeta(result_void_position_t, valueInit(&index, arena, (number_t)i),
                result->position);
    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &index), result->position);

    value_t mapped;
    try(result_void_position_t,
        invokeClosure(&mapped, closure, closure_args, arena, nullptr));
  }

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
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
result_void_position_t
listFilter(value_t *result, const value_list_t *arguments, arena_t *arena) {
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LIST_FILTER, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_FILTER,
          formatValueType(closure_value.type));
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position, "%s requires a list as second argument. Got %s.",
          LIST_FILTER, formatValueType(list_value.type));
  }

  value_list_t *input_list = list_value.value.list;
  closure_t closure = closure_value.value.closure;

  value_list_t *filtered_list = nullptr;
  tryWithMeta(result_void_position_t,
              listCreate(value_t, arena, input_list->count), result->position,
              filtered_list);

  for (size_t i = 0; i < input_list->count; i++) {
    value_t input = listGet(value_t, input_list, i);

    value_list_t *closure_args = nullptr;
    tryWithMeta(result_void_position_t, listCreate(value_t, arena, 2),
                result->position, closure_args);

    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &input), result->position);

    value_t index;
    tryWithMeta(result_void_position_t, valueInit(&index, arena, (number_t)i),
                result->position);
    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &index), result->position);

    value_t filtered;
    try(result_void_position_t,
        invokeClosure(&filtered, closure, closure_args, arena, nullptr));

    if (filtered.type != VALUE_TYPE_BOOLEAN) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            list_value.position,
            "%s requires a function returning a boolean as first argument. Got "
            "return type %s.",
            LIST_FILTER, formatValueType(filtered.type));
    }

    if (filtered.value.boolean) {
      tryWithMeta(result_void_position_t,
                  listAppend(value_t, filtered_list, &input), result->position);
    }
  }

  result->type = VALUE_TYPE_LIST;
  result->value.list = filtered_list;

  return ok(result_void_position_t);
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
result_void_position_t listTimes(value_t *result, const value_list_t *arguments,
                                 arena_t *arena) {
  if (arguments->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", LIST_TIMES, arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t repeats_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          closure_value.position,
          "%s requires a function as first argument. Got %s.", LIST_TIMES,
          formatValueType(closure_value.type));
  }

  if (repeats_value.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          repeats_value.position,
          "%s requires a number as second argument. Got %s.", LIST_TIMES,
          formatValueType(repeats_value.type));
  }

  size_t repeats = (size_t)repeats_value.value.number;
  closure_t closure = closure_value.value.closure;

  value_list_t *repeated_list = nullptr;
  tryWithMeta(result_void_position_t, listCreate(value_t, arena, repeats),
              result->position, repeated_list);

  for (size_t i = 0; i < repeats; i++) {
    value_list_t *closure_args = nullptr;
    tryWithMeta(result_void_position_t, listCreate(value_t, arena, 1),
                result->position, closure_args);

    value_t index;
    tryWithMeta(result_void_position_t, valueInit(&index, arena, (number_t)i),
                result->position);
    tryWithMeta(result_void_position_t,
                listAppend(value_t, closure_args, &index), result->position);

    value_t mapped;
    try(result_void_position_t,
        invokeClosure(&mapped, closure, closure_args, arena, nullptr));

    tryWithMeta(result_void_position_t,
                listAppend(value_t, repeated_list, &mapped), result->position);
  }

  result->type = VALUE_TYPE_LIST;
  result->value.list = repeated_list;

  return ok(result_void_position_t);
}
