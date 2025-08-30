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

  const value_t number = {.type = VALUE_TYPE_NUMBER, .value.number = 123};
  formatValue(&number, size, buffer, &offset);
  expectEqlString(buffer, "123", 3, "formats numbers");

  offset = 0;
  const value_t nil = {.type = VALUE_TYPE_NIL};
  formatValue(&nil, size, buffer, &offset);
  expectEqlString(buffer, "nil", 3, "formats nil");

  offset = 0;
  const value_t v_true = {.type = VALUE_TYPE_BOOLEAN, .value.boolean = true};
  formatValue(&v_true, size, buffer, &offset);
  expectEqlString(buffer, "true", 4, "formats true");

  offset = 0;
  const value_t v_false = {.type = VALUE_TYPE_BOOLEAN, .value.boolean = false};
  formatValue(&v_false, size, buffer, &offset);
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
  arenaReset(test_arena);
  value_t *list = nullptr;
  tryAssert(valueCreateInit(test_arena, VALUE_TYPE_LIST, 0), list);

  tryAssert(listAppend(value_t, &list->value.list, &number));
  tryAssert(listAppend(value_t, &list->value.list, &nil));

  formatValue(list, size, buffer, &offset);
  expectEqlString(buffer, "(123 nil)", 10, "formats lists");

  offset = 0;
  arenaReset(test_arena);
  value_t *closure = nullptr;
  tryAssert(valueCreateInit(test_arena, VALUE_TYPE_CLOSURE, NODE_TYPE_LIST),
            closure);

  node_t symbol = nSym(test_arena, "a");

  tryAssert(listAppend(node_t, &closure->value.closure.arguments, &symbol));

  closure->value.closure.form.type = NODE_TYPE_LIST;

  tryAssert(
      listAppend(node_t, &closure->value.closure.form.value.list, &symbol));

  formatValue(list, size, buffer, &offset);
  expectEqlString(buffer, "(fn (a) (a))", 10, "formats lambdas");
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
