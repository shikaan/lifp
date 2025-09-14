#include "../../lib/result.h"
#include "../../lib/string.h"
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
  int offset = 0;
  formatValue(value, INTERMEDIATE_BUFFER_SIZE, buffer, &offset);
  fprintf(stream, "%s\n", buffer);
}

const char *IO_STDOUT = "io:stdout!";
result_void_position_t ioStdout(value_t *result, const value_list_t *values,
                                arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires an argument. Got %zu", IO_STDOUT, values->count);
  }

  value_t value = listGet(value_t, values, 0);
  streamPrint(stdout, &value);

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}

const char *IO_STDERR = "io:stderr!";
result_void_position_t ioStderr(value_t *result, const value_list_t *values,
                                arena_t *arena) {
  (void)arena;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires an argument. Got %zu", IO_STDERR, values->count);
  }

  value_t value = listGet(value_t, values, 0);

  streamPrint(stderr, &value);

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}

const char *IO_PRINTF = "io:printf!";
result_void_position_t ioPrintf(value_t *result, const value_list_t *values,
                                arena_t *arena) {
  (void)arena;
  if (values->count != 2) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires two arguments. Got %zu", IO_PRINTF, values->count);
  }

  value_t format_value = listGet(value_t, values, 0);
  value_t inputs_value = listGet(value_t, values, 1);

  if (format_value.type != VALUE_TYPE_STRING ||
      inputs_value.type != VALUE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires a format string and a list of arguments. "
          "Got types %u and %u.",
          IO_PRINTF, format_value.type, inputs_value.type);
  }

  value_list_t inputs = inputs_value.value.list;
  char *format = format_value.value.string;

  size_t placeholder_count = 0;
  const char *placeholder = format;
  while ((placeholder = strstr(placeholder, "{}")) != nullptr) {
    placeholder++;
    placeholder_count++;
  }

  if (placeholder_count > inputs.count) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
          format_value.position,
          "Cannot have more placeholders than values. "
          "Got %lu placeholders and %lu values.",
          placeholder_count, inputs.count);
  }

  size_t index = 0;
  const char *current = format;
  while (*current) {
    if (*current == '{' && *(current + 1) == '}') {
      value_t value = listGet(value_t, &inputs, index);
      if (value.type != VALUE_TYPE_STRING) {
        char buffer[INTERMEDIATE_BUFFER_SIZE];
        int offset = 0;
        formatValue(&value, INTERMEDIATE_BUFFER_SIZE, buffer, &offset);
        fputs(buffer, stdout);
      } else {
        // This prevents printing quotes in the formatted string
        fputs(value.value.string, stdout);
      }
      index++;
      current += 2;
      continue;
    }
    putchar(*current);
    current++;
  }

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}

const char *IO_READLINE = "io:readline!";
result_void_position_t ioReadline(value_t *result, const value_list_t *values,
                                  arena_t *arena) {
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires an argument. Got %zu", IO_READLINE, values->count);
  }

  value_t question_value = listGet(value_t, values, 0);

  if (question_value.type != VALUE_TYPE_STRING) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires a string as a question. Got %u", IO_READLINE,
          question_value.type);
  }

  printf("%s", question_value.value.string);

  char buffer[INTERMEDIATE_BUFFER_SIZE];
  if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
    result->type = VALUE_TYPE_STRING;
    *result->value.string = 0;
    return ok(result_void_position_t);
  }

  // Remove trailing newline if present
  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
  }

  result->type = VALUE_TYPE_STRING;
  tryWithMeta(result_void_position_t, arenaAllocate(arena, len),
              question_value.position, result->value.string);

  stringCopy(result->value.string, buffer, len);

  return ok(result_void_position_t);
}

const char *IO_CLEAR = "io:clear!";
result_void_position_t ioClear(value_t *result, const value_list_t *values,
                               arena_t *arena) {
  (void)arena;
  if (values->count != 0) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s takes no arguments. Got %zu", IO_CLEAR, values->count);
  }

  puts("\e[1;1H\e[2J");

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}
