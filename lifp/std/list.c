#include "../../lib/result.h"
#include "../error.h"
#include "../value.h"
#include <stdint.h>

// List count function - counts elements in a list
const char *LIST_COUNT = "list.count";
result_void_position_t listCount(value_t *result, value_list_t *values) {
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", LIST_COUNT, values->count);
  }

  value_t list_value = listGet(value_t, values, 0);
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
result_void_position_t listFrom(value_t *result, value_list_t *values) {
  result->type = VALUE_TYPE_LIST;

  value_list_t *new_list = nullptr;
  tryWithMeta(result_void_position_t,
              listCreate(value_t, values->arena, values->count),
              result->position, new_list);
  result->value.list = *new_list;

  if (values->count > 0) {
    // Copy all values to the new list
    for (size_t i = 0; i < values->count; i++) {
      value_t source = listGet(value_t, values, i);
      tryWithMeta(result_void_position_t,
                  listAppend(value_t, &result->value.list, &source),
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
result_void_position_t listNth(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 2 arguments. Got %zu", LIST_NTH, values->count);
  }

  value_t index_value = listGet(value_t, values, 0);
  value_t list_value = listGet(value_t, values, 1);

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

  // Check bounds
  if (index < 0 || (size_t)index >= list->count) {
    result->type = VALUE_TYPE_NIL;
    result->value.nil = nullptr;
  } else {
    *result = listGet(value_t, list, (size_t)index);
  }

  return ok(result_void_position_t);
}
