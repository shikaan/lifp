// String manipulation functions for lifp.
//
// ```lisp
// (str:length "hello") ; returns 5
// (str:join "," ("a" "b" "c")) ; returns "a,b,c"
// (str:slice "abcdef" 1 4) ; returns "bcde"
// (str:include "hello world" "world") ; returns true
// (str:trimLeft "   foo") ; returns "foo"
// (str:trimRight "foo   ") ; returns "foo"
// ```
// ___HEADER_END___

#include "../../lib/result.h"
#include "../../lib/string.h"
#include "../error.h"
#include "../fmt.h"
#include "../value.h"
#include <ctype.h>
#include <stddef.h>
#include <string.h>

#define tryCreateBuffer(Buffer, Length)                                        \
  tryWithMeta(result_void_position_t,                                          \
              arenaAllocate(arena, (Length) * sizeof(char)), result->position, \
              Buffer);                                                         \
  memset(Buffer, 0, Length);

/**
 * Returns the length of a string.
 * @name str:length
 * @param {string} str - The string to measure.
 * @returns {number} The length of the string.
 * @example
 *   (str:length "hello") ; returns 5
 */
const char *STR_LENGTH = "str:length";
result_void_position_t strLength(value_t *result, const value_list_t *values,
                                 arena_t *arena) {
  (void)arena;
  if (values->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", STR_LENGTH, values->count);
  }

  value_t string_value = listGet(value_t, values, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position, "%s requires a string. Got %s.", STR_LENGTH,
          formatValueType(string_value.type));
  }

  result->type = VALUE_TYPE_NUMBER;
  result->value.number = (double)strlen(string_value.value.string);

  return ok(result_void_position_t);
}

/**
 * Joins a list of strings using a separator.
 * @name str:join
 * @param {string} separator - The separator string.
 * @param {list} strings - The list of strings to join.
 * @returns {string} The joined string.
 * @example
 *   (str:join "," ("a" "b" "c")) ; returns "a,b,c"
 */
const char *STR_JOIN = "str:join";
result_void_position_t strJoin(value_t *result, const value_list_t *values,
                               arena_t *arena) {
  if (values->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", STR_JOIN, values->count);
  }

  value_t separator_value = listGet(value_t, values, 0);
  if (separator_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          separator_value.position,
          "%s requires a string as first argument. Got %s.", STR_JOIN,
          formatValueType(separator_value.type));
  }

  value_t list_value = listGet(value_t, values, 1);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position,
          "%s requires a list of strings as second argument. Got %s.", STR_JOIN,
          formatValueType(list_value.type));
  }

  if (list_value.value.list->count == 0) {
    string_t buffer;
    tryCreateBuffer(buffer, 1);
    result->type = VALUE_TYPE_STRING;
    result->value.string = buffer;
    return ok(result_void_position_t);
  }

  size_t separator_length = strlen(separator_value.value.string);

  size_t total_length = 0;
  for (size_t i = 0; i < list_value.value.list->count; i++) {
    value_t current = listGet(value_t, list_value.value.list, i);
    if (current.type != VALUE_TYPE_STRING) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires a list of strings. Got %s.",
            STR_JOIN, formatValueType(current.type));
    }
    total_length += strlen(current.value.string);
  }
  total_length += separator_length * (list_value.value.list->count - 1);

  string_t buffer;
  tryCreateBuffer(buffer, total_length + 1);

  for (size_t i = 0; i < list_value.value.list->count - 1; i++) {
    value_t current = listGet(value_t, list_value.value.list, i);
    strcat(buffer, current.value.string);
    strcat(buffer, separator_value.value.string);
  }

  value_t last =
      listGet(value_t, list_value.value.list, list_value.value.list->count - 1);
  strcat(buffer, last.value.string);

  result->type = VALUE_TYPE_STRING;
  result->value.string = buffer;

  return ok(result_void_position_t);
}

/**
 * Returns a substring from start to end (end not inclusive).
 * Negative indices count from the end of the string.
 * @name str:slice
 * @param {string} str - The string to slice.
 * @param {number} start - The start index.
 * @param {number} [end] - The end index (optional).
 * @returns {string} The sliced substring.
 * @example
 *   (str:slice "abcdef" 1 4) ; returns "bcde"
 *   (str:slice "abcdef" 2) ; returns "cdef"
 */
const char *STR_SLICE = "str:slice";
result_void_position_t strSlice(value_t *result, const value_list_t *values,
                                arena_t *arena) {
  if (values->count < 2 || values->count > 3) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 or 3 arguments. Got %zu", STR_SLICE, values->count);
  }

  value_t string_value = listGet(value_t, values, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position,
          "%s requires a string as first argument. Got %s.", STR_SLICE,
          formatValueType(string_value.type));
  }

  value_t start_value = listGet(value_t, values, 1);
  if (start_value.type != VALUE_TYPE_NUMBER) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          start_value.position,
          "%s requires a number as second argument. Got %s.", STR_SLICE,
          formatValueType(start_value.type));
  }

  size_t str_len = strlen(string_value.value.string);
  number_t start_num = start_value.value.number;
  size_t start = (start_num < 0) ? (size_t)((int)str_len + (int)start_num)
                                 : (size_t)start_num;
  if (start > str_len)
    start = str_len;

  size_t end = str_len;
  if (values->count == 3) {
    value_t end_value = listGet(value_t, values, 2);
    if (end_value.type != VALUE_TYPE_NUMBER) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            end_value.position,
            "%s requires a number as third argument. Got %s.", STR_SLICE,
            formatValueType(end_value.type));
    }
    number_t end_num = end_value.value.number;
    end =
        (end_num < 0) ? (size_t)((int)str_len + (int)end_num) : (size_t)end_num;
    if (end > str_len)
      end = str_len;
  }

  if (start > end)
    start = end;
  size_t slice_len = (end > start) ? (end - start + 1) : 0;

  string_t buffer;
  tryCreateBuffer(buffer, slice_len);

  if (slice_len > 0) {
    stringCopy(buffer, string_value.value.string + start, slice_len);
  }

  result->type = VALUE_TYPE_STRING;
  result->value.string = buffer;
  return ok(result_void_position_t);
}

/**
 * Checks if a string contains a substring.
 * @name str:include
 * @param {string} str - The string to search in.
 * @param {string} search - The substring to search for.
 * @returns {boolean} True if the substring is found, false otherwise.
 * @example
 *   (str:include "hello world" "world") ; returns true
 */
const char *STR_INCLUDE = "str:include";
result_void_position_t strInclude(value_t *result, const value_list_t *values,
                                  arena_t *arena) {
  (void)arena;

  if (values->count < 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 2 arguments. Got %zu", STR_INCLUDE, values->count);
  }

  value_t string_value = listGet(value_t, values, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position,
          "%s requires a string as first argument. Got %s.", STR_INCLUDE,
          formatValueType(string_value.type));
  }

  value_t search_value = listGet(value_t, values, 1);
  if (search_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          search_value.position,
          "%s requires a string as second argument. Got %s.", STR_INCLUDE,
          formatValueType(search_value.type));
  }

  bool found =
      strstr(string_value.value.string, search_value.value.string) != nullptr;

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = found;

  return ok(result_void_position_t);
}

/**
 * Removes whitespace from the start of a string.
 * @name str:trimLeft
 * @param {string} str - The string to trim.
 * @returns {string} The trimmed string.
 * @example
 *   (str:trimLeft "   foo") ; returns "foo"
 */
const char *STR_TRIM_LEFT = "str:trimLeft";
result_void_position_t strTrimLeft(value_t *result, const value_list_t *values,
                                   arena_t *arena) {
  (void)arena;

  if (values->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", STR_TRIM_LEFT, values->count);
  }

  value_t string_value = listGet(value_t, values, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position, "%s requires a string. Got %s.", STR_TRIM_LEFT,
          formatValueType(string_value.type));
  }

  char *start = string_value.value.string;
  while (isspace(*start)) {
    start++;
  }

  size_t len = strlen(start);
  string_t buffer;
  tryCreateBuffer(buffer, len + 1);
  stringCopy(buffer, start, len + 1);

  result->type = VALUE_TYPE_STRING;
  result->value.string = buffer;

  return ok(result_void_position_t);
}

/**
 * Removes whitespace from the end of a string.
 * @name str:trimRight
 * @param {string} str - The string to trim.
 * @returns {string} The trimmed string.
 * @example
 *   (str:trimRight "foo   ") ; returns "foo"
 */
const char *STR_TRIM_RIGHT = "str:trimRight";
result_void_position_t strTrimRight(value_t *result, const value_list_t *values,
                                    arena_t *arena) {
  (void)arena;

  if (values->count < 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", STR_TRIM_RIGHT, values->count);
  }

  value_t string_value = listGet(value_t, values, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position, "%s requires a string. Got %s.",
          STR_TRIM_RIGHT, formatValueType(string_value.type));
  }

  size_t len = strlen(string_value.value.string);
  char *end = string_value.value.string + len - 1;
  while (isspace(*end)) {
    len--;
    end--;
  }

  string_t buffer;
  tryCreateBuffer(buffer, len + 1);
  stringCopy(buffer, string_value.value.string, len + 1);

  result->type = VALUE_TYPE_STRING;
  result->value.string = buffer;

  return ok(result_void_position_t);
}

#undef tryCreateBuffer
