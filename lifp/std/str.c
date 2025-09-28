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

/**
 * Returns the length of a string.
 * @name str:length
 * @param {string} str - The string to measure.
 * @returns {number} The length of the string.
 * @example
 *   (str:length "hello") ; returns 5
 */
const char *STR_LENGTH = "str:length";
result_value_ref_t strLength(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", STR_LENGTH, arguments->count);
  }
  value_t string_value = listGet(value_t, arguments, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position, "%s requires a string. Got %s.", STR_LENGTH,
          formatValueType(string_value.type));
  }
  return valueCreate(
      VALUE_TYPE_NUMBER,
      (value_as_t){.number = (number_t)strlen(string_value.as.string)}, pos);
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
result_value_ref_t strJoin(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", STR_JOIN, arguments->count);
  }
  value_t separator_value = listGet(value_t, arguments, 0);
  if (separator_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          separator_value.position,
          "%s requires a string as first argument. Got %s.", STR_JOIN,
          formatValueType(separator_value.type));
  }
  value_t list_value = listGet(value_t, arguments, 1);
  if (list_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          list_value.position,
          "%s requires a list of strings as second argument. Got %s.", STR_JOIN,
          formatValueType(list_value.type));
  }
  value_array_t *input_list = list_value.as.list;
  if (input_list->count == 0) {
    return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = strdup("")},
                       pos);
  }
  size_t separator_length = strlen(separator_value.as.string);
  size_t total_length = 0;
  for (size_t i = 0; i < input_list->count; i++) {
    value_t current = listGet(value_t, input_list, i);
    if (current.type != VALUE_TYPE_STRING) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            current.position, "%s requires a list of strings. Got %s.",
            STR_JOIN, formatValueType(current.type));
    }
    total_length += strlen(current.as.string);
  }
  total_length += separator_length * (input_list->count - 1);
  char *buffer = nullptr;
  tryWithMeta(result_value_ref_t, allocSafe(total_length + 1), pos, buffer);

  for (size_t i = 0; i < input_list->count - 1; i++) {
    value_t current = listGet(value_t, input_list, i);
    strcat(buffer, current.as.string);
    strcat(buffer, separator_value.as.string);
  }
  value_t last = listGet(value_t, input_list, input_list->count - 1);
  strcat(buffer, last.as.string);
  return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = buffer}, pos);
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
result_value_ref_t strSlice(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2 && arguments->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 or 3 arguments. Got %zu", STR_SLICE, arguments->count);
  }
  value_t string_value = listGet(value_t, arguments, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position,
          "%s requires a string as first argument. Got %s.", STR_SLICE,
          formatValueType(string_value.type));
  }
  value_t start_value = listGet(value_t, arguments, 1);
  if (start_value.type != VALUE_TYPE_NUMBER) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          start_value.position,
          "%s requires a number as second argument. Got %s.", STR_SLICE,
          formatValueType(start_value.type));
  }
  size_t str_len = strlen(string_value.as.string);
  number_t start_num = start_value.as.number;
  size_t start = (start_num < 0) ? (size_t)((int)str_len + (int)start_num)
                                 : (size_t)start_num;
  if (start > str_len) {
    start = str_len;
  }

  size_t end = str_len;
  if (arguments->count == 3) {
    value_t end_value = listGet(value_t, arguments, 2);
    if (end_value.type != VALUE_TYPE_NUMBER) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
            end_value.position,
            "%s requires a number as third argument. Got %s.", STR_SLICE,
            formatValueType(end_value.type));
    }
    number_t end_num = end_value.as.number;
    end =
        (end_num < 0) ? (size_t)((int)str_len + (int)end_num) : (size_t)end_num;

    if (end > str_len) {
      end = str_len;
    }
  }

  if (start > end) {
    start = end;
  }

  size_t slice_len = (end > start) ? (end - start) : 0;
  char *buffer = nullptr;
  tryWithMeta(result_value_ref_t, allocSafe(slice_len + 1), pos, buffer);
  stringCopy(buffer, string_value.as.string + start, slice_len + 1);

  return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = buffer}, pos);
}

/**
 * Checks if a string contains a substring.
 * @name str:include?
 * @param {string} str - The string to search in.
 * @param {string} search - The substring to search for.
 * @returns {boolean} True if the substring is found, false otherwise.
 * @example
 *   (str:include? "hello world" "world") ; returns true
 */
const char *STR_INCLUDE = "str:include?";
result_value_ref_t strInclude(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 2 arguments. Got %zu", STR_INCLUDE, arguments->count);
  }
  value_t string_value = listGet(value_t, arguments, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position,
          "%s requires a string as first argument. Got %s.", STR_INCLUDE,
          formatValueType(string_value.type));
  }
  value_t search_value = listGet(value_t, arguments, 1);
  if (search_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          search_value.position,
          "%s requires a string as second argument. Got %s.", STR_INCLUDE,
          formatValueType(search_value.type));
  }
  bool found = strstr(string_value.as.string, search_value.as.string) != NULL;
  return valueCreate(VALUE_TYPE_BOOLEAN, (value_as_t){.boolean = found}, pos);
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
result_value_ref_t strTrimLeft(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", STR_TRIM_LEFT, arguments->count);
  }
  value_t string_value = listGet(value_t, arguments, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position, "%s requires a string. Got %s.", STR_TRIM_LEFT,
          formatValueType(string_value.type));
  }
  char *start = string_value.as.string;
  while (isspace(*start)) {
    start++;
  }
  size_t len = strlen(start);
  char *buffer = nullptr;
  tryWithMeta(result_value_ref_t, allocSafe(len + 1), pos, buffer);
  stringCopy(buffer, start, len + 1);
  return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = buffer}, pos);
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
result_value_ref_t strTrimRight(const value_array_t *arguments,
                                position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", STR_TRIM_RIGHT, arguments->count);
  }
  value_t string_value = listGet(value_t, arguments, 0);
  if (string_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          string_value.position, "%s requires a string. Got %s.",
          STR_TRIM_RIGHT, formatValueType(string_value.type));
  }
  size_t len = strlen(string_value.as.string);
  if (len == 0) {
    char *buffer = (char *)calloc(1, 1);
    return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = buffer}, pos);
  }
  char *end = string_value.as.string + len - 1;
  while (len > 0 && isspace(*end)) {
    len--;
    end--;
  }
  char *buffer = nullptr;
  tryWithMeta(result_value_ref_t, allocSafe(len + 1), pos, buffer);
  stringCopy(buffer, string_value.as.string, len + 1);
  return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = buffer}, pos);
}

#undef tryCreateBuffer
