#include "../lifp/fmt.h"
#include "../lib/list.h"
#include "../lifp/node.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;

void values() {
  const int size = 128;
  char buffer[size];
  int offset = 0;

  value_t number;
  tryAssert(valueInit(&number, test_arena, 123.0));
  formatValue(&number, size, buffer, &offset);
  expectEqlString(buffer, "123", 3, "formats numbers");

  offset = 0;
  value_t nil;
  tryAssert(valueInit(&nil, test_arena, nullptr));
  formatValue(&nil, size, buffer, &offset);
  expectEqlString(buffer, "nil", 3, "formats nil");

  offset = 0;
  value_t true_value;
  tryAssert(valueInit(&true_value, test_arena, true));
  formatValue(&true_value, size, buffer, &offset);
  expectEqlString(buffer, "true", 4, "formats true");

  offset = 0;
  value_t false_value;
  tryAssert(valueInit(&false_value, test_arena, false));
  formatValue(&false_value, size, buffer, &offset);
  expectEqlString(buffer, "false", 5, "formats false");

  offset = 0;
  const value_t builtin = {.type = VALUE_TYPE_BUILTIN};
  formatValue(&builtin, size, buffer, &offset);
  expectEqlString(buffer, "#<builtin>", 11, "formats builtin");

  offset = 0;
  const value_t special = {.type = VALUE_TYPE_SPECIAL};
  formatValue(&special, size, buffer, &offset);
  expectEqlString(buffer, "#<special>", 11, "formats specials");

  offset = 0;
  value_t string_value;
  tryAssert(valueInit(&string_value, test_arena, "test"));
  formatValue(&string_value, size, buffer, &offset);
  expectEqlString(buffer, "\"test\"", 7, "formats strings");

  offset = 0;
  arenaReset(test_arena);
  value_t list_value;
  tryAssert(valueInit(&list_value, test_arena, (size_t)2));
  tryAssert(listAppend(value_t, &list_value.value.list, &number));
  tryAssert(listAppend(value_t, &list_value.value.list, &nil));

  formatValue(&list_value, size, buffer, &offset);
  expectEqlString(buffer, "(123 nil)", 10, "formats lists");

  offset = 0;
  arenaReset(test_arena);
  value_t closure_value;
  tryAssert(
      valueInit(&closure_value, test_arena, (node_type_t)(NODE_TYPE_LIST)));

  node_t symbol = nSym(test_arena, "a");

  tryAssert(
      listAppend(node_t, &closure_value.value.closure.arguments, &symbol));
  tryAssert(listAppend(node_t, &closure_value.value.closure.form.value.list,
                       &symbol));

  formatValue(&closure_value, size, buffer, &offset);
  expectEqlString(buffer, "(fn (a) (a))", 13, "formats lambdas");
}

void errors() {
  const int size = 128;
  char buffer[size];
  int offset = 0;
  position_t position = {1, 10};
  message_t message = "message";

  const char list_buffer[23] = "(1 2 3 4 not-found 10)";
  formatErrorMessage(message, position, "file.lifp", list_buffer, size, buffer,
                     &offset);
  expectEqlString(buffer,
                  "Error: message\n"
                  "\n"
                  "  (1 2 3 4 not-found 10)\n"
                  "           ^\n"
                  "  at file.lifp:1:10",
                  95, "puts caret in the right place (list)");

  offset = 0;
  position = (position_t){1, 1};
  const char atom_buffer[10] = "not-found";
  formatErrorMessage(message, position, "file.lifp", atom_buffer, size, buffer,
                     &offset);
  expectEqlString(buffer,
                  "Error: message\n"
                  "\n"
                  "  not-found\n"
                  "  ^\n"
                  "  at file.lifp:1:1",
                  72, "puts caret in the right place (atom)");
  offset = 0;
  position = (position_t){1, 2};
  const char init_list_buffer[12] = "(not-found)";
  formatErrorMessage(message, position, "file.lifp", init_list_buffer, size,
                     buffer, &offset);
  expectEqlString(buffer,
                  "Error: message\n"
                  "\n"
                  "  (not-found)\n"
                  "   ^\n"
                  "  at file.lifp:1:2",
                  72, "puts caret in the right place (init list)");
}

int main() {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);
  suite(values);
  suite(errors);
  return report();
}
