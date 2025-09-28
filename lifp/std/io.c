// Input/output utilities for lifp. These functions provide basic console IO.
//
// ```lisp
// (io:stdout! "hello") ; prints to stdout
// (io:stderr! "error") ; prints to stderr
// (io:printf! "Hello, {}!" ["world"]) ; prints formatted string
// (io:readline! "Enter your name: ") ; reads a line from stdin
// (io:clear!) ; clears the terminal
// ```
// ___HEADER_END___

#include "../../lib/result.h"
#include "../error.h"
#include "../fmt.h"
#include "../value.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

constexpr size_t INTERMEDIATE_BUFFER_SIZE = 1024;

static void streamPrint(FILE *stream, value_t *value) {
  char buffer[INTERMEDIATE_BUFFER_SIZE];

  if (value->type != VALUE_TYPE_STRING) {
    int offset = 0;
    formatValue(value, INTERMEDIATE_BUFFER_SIZE, buffer, &offset);
    fprintf(stream, "%s\n", buffer);
  } else {
    // This prevents printing quotes in the formatted string
    fprintf(stream, "%s\n", value->as.string);
  }
}

/**
 * Prints an argument to standard output.
 * @name io:stdout!
 * @param {any} value - The value to print.
 * @returns {nil} Returns nil.
 * @example
 *   (io:stdout! "hello")
 */
const char *IO_STDOUT = "io:stdout!";
result_value_ref_t ioStdout(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", IO_STDOUT, arguments->count);
  }

  value_t value = listGet(value_t, arguments, 0);
  streamPrint(stdout, &value);

  return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, pos);
}

/**
 * Prints a value to standard error.
 * @name io:stderr!
 * @param {any} value - The value to print.
 * @returns {nil} Returns nil.
 * @example
 *   (io:stderr! "error")
 */
const char *IO_STDERR = "io:stderr!";
result_value_ref_t ioStderr(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", IO_STDERR, arguments->count);
  }

  value_t value = listGet(value_t, arguments, 0);
  streamPrint(stderr, &value);

  return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, pos);
}

/**
 * Prints a formatted string to standard output, replacing each '{}' in the
 * format string with the corresponding value from the list.
 * @name io:printf!
 * @param {string} format - The format string containing '{}' placeholders.
 * @param {list} values - The list of values to insert into the format string.
 * @returns {nil} Returns nil.
 * @example
 *   (io:printf! "Hello, {}!" ("world")) ; prints "Hello, world!"
 */
const char *IO_PRINTF = "io:printf!";
result_value_ref_t ioPrintf(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 2) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires at least 2 arguments. Got %zu", IO_PRINTF,
          arguments->count);
  }

  value_t format_value = listGet(value_t, arguments, 0);
  value_t inputs_value = listGet(value_t, arguments, 1);

  if (format_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          format_value.position,
          "%s requires a format string as the first argument. Got %s.",
          IO_PRINTF, formatValueType(format_value.type));
  }
  if (inputs_value.type != VALUE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          inputs_value.position,
          "%s requires a list as the second argument. Got %s.", IO_PRINTF,
          formatValueType(inputs_value.type));
  }

  value_array_t *inputs = inputs_value.as.list;
  char *format = format_value.as.string;

  size_t placeholder_count = 0;
  const char *placeholder = format;
  while ((placeholder = strstr(placeholder, "{}")) != nullptr) {
    placeholder++;
    placeholder_count++;
  }

  if (placeholder_count > inputs->count) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, format_value.position,
          "Cannot have more placeholders than values. "
          "Got %lu placeholders and %lu values.",
          placeholder_count, inputs->count);
  }

  size_t index = 0;
  const char *current = format;
  while (*current) {
    if (*current == '{' && *(current + 1) == '}') {
      value_t value = listGet(value_t, inputs, index);
      if (value.type != VALUE_TYPE_STRING) {
        char buffer[INTERMEDIATE_BUFFER_SIZE];
        int offset = 0;
        formatValue(&value, INTERMEDIATE_BUFFER_SIZE, buffer, &offset);
        fputs(buffer, stdout);
      } else {
        // This prevents printing quotes in the formatted string
        fputs(value.as.string, stdout);
      }
      index++;
      current += 2;
      continue;
    }
    putchar(*current);
    current++;
  }

  return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, pos);
}

/**
 * Prints a prompt and returns the answer as a string.
 * @name io:readline!
 * @param {string} prompt - The prompt to the question
 * @returns {string} Returns the user-input string.
 * @example
 *   (io:readline! "What's your name?") ; returns user input
 */
const char *IO_READLINE = "io:readline!";
result_value_ref_t ioReadline(const value_array_t *arguments, position_t pos) {
  if (arguments->count < 1) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires 1 argument. Got %zu", IO_READLINE, arguments->count);
  }

  value_t question_value = listGet(value_t, arguments, 0);

  if (question_value.type != VALUE_TYPE_STRING) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR_UNEXPECTED_TYPE,
          question_value.position, "%s requires a string. Got %s.", IO_READLINE,
          formatValueType(question_value.type));
  }

  printf("%s", question_value.as.string);

  char buffer[INTERMEDIATE_BUFFER_SIZE];
  if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
    return valueCreate(VALUE_TYPE_STRING, (value_as_t){.string = strdup("")},
                       pos);
  }

  // Remove trailing newline if present
  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
  }

  return valueCreate(VALUE_TYPE_STRING,
                     (value_as_t){
                         .string = strdup(buffer),
                     },
                     pos);
}

/**
 * Clears the terminal screen.
 * @name io:clear!
 * @returns {nil} Returns nil.
 * @example
 *   (io:clear!)
 */
const char *IO_CLEAR = "io:clear!";
result_value_ref_t ioClear(const value_array_t *arguments, position_t pos) {
  if (arguments->count != 0) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, pos,
          "%s requires no arguments. Got %zu", IO_CLEAR, arguments->count);
  }
  puts("\e[1;1H\e[2J");
  return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, pos);
}
