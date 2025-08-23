#include "test.h"
#include "utils.h"

#include "../lib/arena.h"
#include "../lifp/evaluate.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include <assert.h>
#include <stddef.h>

static arena_t *ast_arena;
static arena_t *temp_arena;

void execute(value_t *result, const char *input) {
  char input_copy[1024];
  strcpy(input_copy, input);

  char *line = strtok(input_copy, "\n");

  environment_t *global_environment = nullptr;
  tryAssertAssign(vmInit(), global_environment);

  while (line != NULL) {
    arenaReset(ast_arena);
    arenaReset(temp_arena);
    token_list_t *tokens = nullptr;
    tryAssertAssign(tokenize(ast_arena, line), tokens);

    size_t offset = 0;
    size_t depth = 0;
    node_t *ast = nullptr;
    tryAssertAssign(parse(ast_arena, tokens, &offset, &depth), ast);

    value_t res;
    auto reduction = evaluate(&res, temp_arena, ast, global_environment);
    if (reduction.code != RESULT_OK) {
      printf("Reduction error: %s\n", reduction.message);
      assert(reduction.code == RESULT_OK);
    }

    line = strtok(nullptr, "\n");
    *result = res;
  }

  environmentDestroy(&global_environment);
}

int main() {
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), ast_arena);
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), temp_arena);

  case("number");
  value_t number;
  execute(&number, "1");
  expectEqlInt(number.value.integer, 1, "returns correct value");
  
  case("symbol");
  value_t symbol;
  execute(&symbol, "+");
  expectNotNull(symbol.value.builtin, "returns correct value");
  
  case("list");
  value_t list;
  execute(&list, "(1 2)");
  expectEqlUint((unsigned int)list.value.list.count, 2, "returns a list"); 
  value_t first = listGet(value_t, &list.value.list,  0); 
  expectEqlInt(first.value.integer, 1, "correct first item"); 
  value_t second = listGet(value_t, &list.value.list, 1);
  expectEqlInt(second.value.integer, 2, "correct second item");
  
  case("simple form");
  value_t simple;
  execute(&simple, "(+ 1 2)");
  expectEqlInt(simple.value.integer, 3, "returns correct value");
  
  case("nested form");
  value_t nested;
  execute(&nested, "(+ 1 (+ 2 4))");
  expectEqlInt(nested.value.integer, 7, "returns correct value");
  
  case("declare function");
  value_t fun;
  execute(&fun, "(def! sum (fn (a b) (+ a b)))\n(sum 1 2)");
  expectEqlUint(fun.type, VALUE_TYPE_INTEGER, "returns correct type");
  expectEqlInt(fun.value.integer, 3, "returns correct value");
  
  case("let");
  value_t let;
  execute(&let, "(let ((plus (fn (x y) (+ x y))) (a 1)) (plus a 1))");
  expectEqlUint(let.type, VALUE_TYPE_INTEGER, "returns correct type");
  expectEqlInt(let.value.integer, 2, "returns correct value");

  arenaDestroy(&ast_arena);
  arenaDestroy(&temp_arena);
  return report();
}
