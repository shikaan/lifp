#include "../../lib/result.h"
#include "../error.h"
#include "../value.h"
#include <float.h>
#include <stdlib.h>
#include <string.h>

const char *STR_LENGTH = "str.length";
result_void_position_t strLength(value_t *result, value_list_t *values) {
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