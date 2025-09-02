#include "../../lib/result.h"
#include "../error.h"
#include "../value.h"
#include <float.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const char *STR_LENGTH = "str.length";
result_void_position_t strLength(arena_t *arena, value_t *result,
                                 value_list_t *values) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires exactly 1 argument. Got %zu", STR_LENGTH, values->count);
  }

  value_t string_value = listGet(value_t, values, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          string_value.position, "%s requires a string. Got type %u",
          STR_LENGTH, string_value.type);
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = (double)strlen(string_value.value.string);

  return ok(result_void_position_t);
}

const char *STR_JOIN = "str.join";
result_void_position_t strJoin(arena_t *arena, value_t *result,
                               value_list_t *values) {
  if (values->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", STR_JOIN, values->count);
  }

  value_t separator_value = listGet(value_t, values, 0);
  if (separator_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          separator_value.position,
          "%s requires a separator as first argument. Got type %u", STR_JOIN,
          separator_value.type);
  }

  value_t list_value = listGet(value_t, values, 1);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, list_value.position,
          "%s requires a list of strings as second argument. Got type %u",
          STR_JOIN, list_value.type);
  }

  if (list_value.value.list.count == 0) {
    string_t buffer;
    tryWithMeta(result_void_position_t, arenaAllocate(arena, sizeof(char)),
                result->position, buffer);
    buffer[0] = 0;
    result->type = VALUE_TYPE_STRING;
    result->value.string = buffer;
    return ok(result_void_position_t);
  }

  size_t separator_length = strlen(separator_value.value.string);

  size_t total_length = 0;
  for (size_t i = 0; i < list_value.value.list.count; i++) {
    value_t current = listGet(value_t, &list_value.value.list, i);
    if (current.type != VALUE_TYPE_STRING) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list of strings. Got type %u", STR_JOIN,
            current.type);
    }
    total_length += strlen(current.value.string);
  }
  total_length += separator_length * (list_value.value.list.count - 1);

  string_t buffer;
  tryWithMeta(result_void_position_t,
              arenaAllocate(arena, total_length * sizeof(char)),
              result->position, buffer);

  for (size_t i = 0; i < list_value.value.list.count - 1; i++) {
    value_t current = listGet(value_t, &list_value.value.list, i);
    if (current.type != VALUE_TYPE_STRING) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, current.position,
            "%s requires a list of strings. Got type %u", STR_JOIN,
            current.type);
    }
    strcat(buffer, current.value.string);
    strcat(buffer, separator_value.value.string);
  }
  value_t last =
      listGet(value_t, &list_value.value.list, list_value.value.list.count - 1);
  strcat(buffer, last.value.string);

  result->type = VALUE_TYPE_STRING;
  result->value.string = buffer;

  return ok(result_void_position_t);
}