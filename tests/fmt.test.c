#include "../lifp/fmt.h"
#include "../lifp/node.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;

void values() {
  position_t pos = {1, 1};
  const int size = 128;
  char buffer[size];
  int offset = 0;

  value_t number = {
      .type = VALUE_TYPE_NUMBER,
      .as.number = 123.0,
      .position = pos,
  };
  formatValue(&number, size, buffer, &offset);
  expectEqlString(buffer, "123", 3, "formats numbers");

  offset = 0;
  value_t nil = {
      .type = VALUE_TYPE_NIL,
      .position = pos,
  };
  formatValue(&nil, size, buffer, &offset);
  expectEqlString(buffer, "nil", 3, "formats nil");

  offset = 0;
  value_t true_value = {
      .type = VALUE_TYPE_BOOLEAN,
      .as.boolean = true,
      .position = pos,
  };
  formatValue(&true_value, size, buffer, &offset);
  expectEqlString(buffer, "true", 4, "formats true");

  offset = 0;
  value_t false_value = {
      .type = VALUE_TYPE_BOOLEAN,
      .as.boolean = false,
      .position = pos,
  };
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
  string_t string = strdup("test");
  value_t string_value = {
      .type = VALUE_TYPE_STRING,
      .as.string = string,
      .position = pos,
  };
  formatValue(&string_value, size, buffer, &offset);
  expectEqlString(buffer, "\"test\"", 7, "formats strings");
  deallocSafe(&string);

  offset = 0;
  value_t list_value = {
      .type = VALUE_TYPE_LIST,
      .position = pos,
  };
  value_array_t arr = {
      .count = 2,
      .data = (value_t[]){number, nil},
  };
  list_value.as.list = &arr;
  formatValue(&list_value, size, buffer, &offset);
  expectEqlString(buffer, "(123 nil)", 10, "formats lists");

  offset = 0;
  // Build a closure value inline: (fn (a) (a))
  string_t arg_a = strdup("a");
  arguments_t args = {
      .count = 1,
      .data = (string_t[]){arg_a},
  };
  node_t form_list_data[] = {nSym(test_arena, arg_a)}; // will overwrite
  node_t form = {
      .type = NODE_TYPE_LIST,
      .position = pos,
      .value.list = {.item_size = sizeof(node_t),
                     .count = 1,
                     .capacity = 1,
                     .data = form_list_data},
  };
  value_t closure_value = {
      .type = VALUE_TYPE_CLOSURE,
      .position = pos,
      .as.closure =
          {
              .form = &form,
              .arguments = &args,
              .environment = nullptr,
          },
  };

  formatValue(&closure_value, size, buffer, &offset);
  expectEqlString(buffer, "(fn (a) (a))", 13, "formats lambdas");
  deallocSafe(&arg_a);
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
                  "1 | (1 2 3 4 not-found 10)\n"
                  "             ^\n"
                  "  at file.lifp:1:10",
                  (size_t)offset, "puts caret in the right place (list)");

  offset = 0;
  position = (position_t){1, 1};
  const char atom_buffer[10] = "not-found";
  formatErrorMessage(message, position, "file.lifp", atom_buffer, size, buffer,
                     &offset);
  expectEqlString(buffer,
                  "Error: message\n"
                  "\n"
                  "1 | not-found\n"
                  "    ^\n"
                  "  at file.lifp:1:1",
                  (size_t)offset, "puts caret in the right place (atom)");
  offset = 0;
  position = (position_t){1, 2};
  const char init_list_buffer[12] = "(not-found)";
  formatErrorMessage(message, position, "file.lifp", init_list_buffer, size,
                     buffer, &offset);
  expectEqlString(buffer,
                  "Error: message\n"
                  "\n"
                  "1 | (not-found)\n"
                  "     ^\n"
                  "  at file.lifp:1:2",
                  (size_t)offset, "puts caret in the right place (init list)");

  offset = 0;
  position = (position_t){3, 14};
  const char complex_buffer[] = "(def! fun\n"
                                "; this is a function\n"
                                "  (fn (a b) (+ a b)))\n"
                                "\n"
                                "; invoking the function\n"
                                "(fun 1 2)";
  formatErrorMessage(message, position, "file.lifp", complex_buffer, size,
                     buffer, &offset);
  expectEqlString(buffer,
                  "Error: message\n"
                  "\n"
                  "3 |   (fn (a b) (+ a b)))\n"
                  "                 ^\n"
                  "  at file.lifp:3:14\n",
                  (size_t)offset, "puts caret in the right place (multiline)");
}

int main() {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);
  suite(values);
  suite(errors);
  return report();
}
