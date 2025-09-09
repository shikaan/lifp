#include "../../lib/result.h"
#include "../error.h"
#include "../evaluate.h"
#include "../value.h"
#include <stdint.h>

// List count function - counts elements in a list
const char *LIST_COUNT = "list.count";
result_void_position_t listCount(value_t *result, const value_list_t *arguments,
                                 arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (arguments->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", LIST_COUNT,
          arguments->count);
  }

  value_t list_value = listGet(value_t, arguments, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list. Got type %u", LIST_COUNT, list_value.type);
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = (number_t)list_value.value.list.count;

  return ok(result_void_position_t);
}

// List from function - creates a list from the given arguments
const char *LIST_FROM = "list.from";
result_void_position_t listFrom(value_t *result, const value_list_t *arguments,
                                arena_t *arena, environment_t *environment) {
  (void)environment;
  result->type = VALUE_TYPE_LIST;

  value_list_t *new_list = nullptr;
  tryWithMeta(result_void_position_t,
              listCreate(value_t, arena, arguments->count), result->position,
              new_list);
  result->value.list = *new_list;

  if (arguments->count > 0) {
    // Copy all arguments to the new list
    for (size_t i = 0; i < arguments->count; i++) {
      value_t source = listGet(value_t, arguments, i);
      value_t duplicated;
      tryWithMeta(result_void_position_t,
                  valueCopy(&source, &duplicated, arena), result->position);
      tryWithMeta(result_void_position_t,
                  listAppend(value_t, &result->value.list, &duplicated),
                  source.position);
    }
  } else {
    new_list->data = nullptr;
  }

  return ok(result_void_position_t);
}

// List nth function - returns the nth element of a list, or nil if out of
// bounds
const char *LIST_NTH = "list.nth";
result_void_position_t listNth(value_t *result, const value_list_t *arguments,
                               arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;

  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LIST_NTH,
          arguments->count);
  }

  value_t index_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (index_value.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          index_value.position, "%s requires an number index. Got type %u",
          LIST_NTH, index_value.type);
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list. Got type %u", LIST_NTH, list_value.type);
  }

  number_t index = index_value.value.number;
  value_list_t *list = &list_value.value.list;

  if (index < 0 || (size_t)index >= list->count ||
      index != (number_t)(size_t)index) {
    result->type = VALUE_TYPE_NIL;
    result->value.nil = nullptr;
  } else {
    *result = listGet(value_t, list, (size_t)index);
  }

  return ok(result_void_position_t);
}

const char *LIST_MAP = "list.map";
result_void_position_t listMap(value_t *result, const value_list_t *arguments,
                               arena_t *arena, environment_t *environment) {
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LIST_MAP,
          arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          closure_value.position,
          "%s requires a function as first argument. Got type %u", LIST_MAP,
          closure_value.type);
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list as second argument. Got type %u", LIST_MAP,
          list_value.type);
  }

  value_list_t *input_list = &list_value.value.list;
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
        invokeClosure(&mapped, closure, closure_args, arena, environment));

    tryWithMeta(result_void_position_t,
                listAppend(value_t, mapped_list, &mapped), result->position);
  }

  result->type = VALUE_TYPE_LIST;
  result->value.list = *mapped_list;

  return ok(result_void_position_t);
}

const char *LIST_EACH = "list.each";
result_void_position_t listEach(value_t *result, const value_list_t *arguments,
                                arena_t *arena, environment_t *environment) {
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LIST_EACH,
          arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          closure_value.position,
          "%s requires a function as first argument. Got type %u", LIST_EACH,
          closure_value.type);
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list as second argument. Got type %u", LIST_EACH,
          list_value.type);
  }

  value_list_t *input_list = &list_value.value.list;
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
        invokeClosure(&mapped, closure, closure_args, arena, environment));
  }

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}

const char *LIST_FILTER = "list.filter";
result_void_position_t listFilter(value_t *result,
                                  const value_list_t *arguments, arena_t *arena,
                                  environment_t *environment) {
  if (arguments->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LIST_FILTER,
          arguments->count);
  }

  value_t closure_value = listGet(value_t, arguments, 0);
  value_t list_value = listGet(value_t, arguments, 1);

  if (closure_value.type != VALUE_TYPE_CLOSURE) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          closure_value.position,
          "%s requires a function as first argument. Got type %u", LIST_FILTER,
          closure_value.type);
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list as second argument. Got type %u", LIST_FILTER,
          list_value.type);
  }

  value_list_t *input_list = &list_value.value.list;
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
        invokeClosure(&filtered, closure, closure_args, arena, environment));

    if (filtered.type != VALUE_TYPE_BOOLEAN) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
            list_value.position,
            "%s requires a function returning a boolean as first argument. Got "
            "return type %u",
            LIST_FILTER, filtered.type);
    }

    if (filtered.value.boolean) {
      tryWithMeta(result_void_position_t,
                  listAppend(value_t, filtered_list, &input), result->position);
    }
  }

  result->type = VALUE_TYPE_LIST;
  result->value.list = *filtered_list;

  return ok(result_void_position_t);
}
