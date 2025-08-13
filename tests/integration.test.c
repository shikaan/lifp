#include "test.h"
#include "utils.h"

#include "../lib/arena.h"
#include "../lifp/evaluate.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_ast_arena;
static arena_t *test_temp_arena;

result_value_ref_t execute(const char *input) {
  char input_copy[1024];
  strcpy(input_copy, input);

  char *line = strtok(input_copy, "\n");
  result_value_ref_t last_result;

  environment_t *env = nullptr;
  tryAssertAssign(vmInit(), env);

  while (line != NULL) {
    token_list_t *tokens = nullptr;
    tryAssertAssign(tokenize(test_ast_arena, line), tokens);

    size_t offset = 0;
    size_t depth = 0;
    node_t *syntax_tree = nullptr;
    tryAssertAssign(parse(test_ast_arena, tokens, &offset, &depth),
                    syntax_tree);

    result_value_ref_t reduction = evaluate(test_temp_arena, syntax_tree, env);
    assert(reduction.code == RESULT_OK);
    last_result = reduction;

    line = strtok(nullptr, "\n");
    arenaReset(test_ast_arena);
  }

  environmentDestroy(&env);
  arenaReset(test_temp_arena);
  return last_result;
}

int main() {
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), test_ast_arena);
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), test_temp_arena);

  case("number");
  result_value_ref_t reduction = execute("1");
  expectEqlInt(reduction.value->value.integer, 1, "returns correct value");

  case("symbol");
  reduction = execute("+");
  expectNotNull(reduction.value->value.builtin, "returns correct value");

  case("list");
  reduction = execute("(1 2)");
  expectEqlUint((unsigned int)reduction.value->value.list.count, 2, "returns a list"); 
  value_t first = listGet(value_t, &reduction.value->value.list,  0); 
  expectEqlInt(first.value.integer, 1, "correct first item"); 
  value_t second = listGet(value_t, &reduction.value->value.list, 1);
  expectEqlInt(second.value.integer, 2, "correct second item");

  case("simple form");
  reduction = execute("(+ 1 2)");
  expectEqlInt(reduction.value->value.integer, 3, "returns correct value");

  case("nested form");
  reduction = execute("(+ 1 (+ 2 4))");
  expectEqlInt(reduction.value->value.integer, 7, "returns correct value");

  case("declare function");
  reduction = execute("(def! sum (fn (a b) (+ a b)))\n(sum 1 2)");
  expectEqlInt(reduction.value->value.integer, 3, "returns correct value");

  arenaDestroy(&test_ast_arena);
  arenaDestroy(&test_temp_arena);
  return report();
}
