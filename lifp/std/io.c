#include "../../lib/result.h"
#include "../error.h"
#include "../fmt.h"
#include "../value.h"

const char *IO_PRINT = "io.print!";
result_void_position_t ioPrint(value_t *result, const value_list_t *values,
                               arena_t *arena, environment_t *environment) {
  (void)arena;
  (void)environment;
  if (values->count != 1) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, result->position,
          "%s requires 1 argument. Got %zu", IO_PRINT, values->count);
  }

  value_t value = listGet(value_t, values, 0);

  constexpr static size_t BUFFER_SIZE = 1024;
  char buffer[BUFFER_SIZE];
  int offset = 0;
  formatValue(&value, BUFFER_SIZE, buffer, &offset);
  printf("%s\n", buffer);

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return ok(result_void_position_t);
}
