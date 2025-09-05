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
  tryAssert(vmInit(), global_environment);

  while (line != NULL) {
    arenaReset(ast_arena);
    arenaReset(temp_arena);
    token_list_t *tokens = nullptr;
    tryAssert(tokenize(ast_arena, line), tokens);

    size_t offset = 0;
    size_t depth = 0;
    node_t *ast = nullptr;
    tryAssert(parse(ast_arena, tokens, &offset, &depth), ast);

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
  tryAssert(arenaCreate((size_t)(1024 * 1024)), ast_arena);
  tryAssert(arenaCreate((size_t)(1024 * 1024)), temp_arena);

  case("number");
  value_t number;
  execute(&number, "1");
  expectEqlDouble(number.value.number, 1, "returns correct value");
  
  case("symbol");
  value_t symbol;
  execute(&symbol, "+");
  expectNotNull(symbol.value.builtin, "returns correct value");
  
  case("list");
  value_t list;
  execute(&list, "(1 2)");
  expectEqlUint((unsigned int)list.value.list.count, 2, "returns a list"); 
  value_t first = listGet(value_t, &list.value.list,  0); 
  expectEqlDouble(first.value.number, 1, "correct first item"); 
  value_t second = listGet(value_t, &list.value.list, 1);
  expectEqlDouble(second.value.number, 2, "correct second item");
  
  case("nested list");
  value_t nested_list;
  execute(&nested_list, "((1) 2)");
  expectEqlUint(nested_list.type, VALUE_TYPE_LIST, "returns a list");
  expectEqlUint(nested_list.value.list.data[0].type, VALUE_TYPE_LIST, "with nested list");

  case("immediate invocation");
  value_t immediate_invocation;
  execute(&immediate_invocation, "((fn (a) a) 2)");
  expectEqlUint(immediate_invocation.type, VALUE_TYPE_NUMBER, "returns a number");
  expectEqlDouble(immediate_invocation.value.number, 2, "with correct value");

  case("simple form");
  value_t simple;
  execute(&simple, "(+ 1 2)");
  expectEqlDouble(simple.value.number, 3, "returns correct value");
  
  case("nested form");
  value_t nested;
  execute(&nested, "(+ 1 (+ 2 4))");
  expectEqlDouble(nested.value.number, 7, "returns correct value");
  
  case("declare function");
  value_t fun;
  execute(&fun, "(def! sum (fn (a b) (+ a b)))\n(sum 1 2)");
  expectEqlUint(fun.type, VALUE_TYPE_NUMBER, "returns correct type");
  expectEqlDouble(fun.value.number, 3, "returns correct value");
  
  case("let");
  value_t let;
  execute(&let, "(let ((plus (fn (x y) (+ x y))) (a 1)) (plus a 1))");
  expectEqlUint(let.type, VALUE_TYPE_NUMBER, "returns correct type");
  expectEqlDouble(let.value.number, 2, "returns correct value");

  case("conditional - cond special form");
  value_t cond_test;
  execute(&cond_test, "(cond ((< 5 3) 1) ((> 5 3) 2) (3))");
  expectEqlDouble(cond_test.value.number, 2, "evaluates correct branch");

  case("boolean operations");
  value_t bool_test;
  execute(&bool_test, "(and (= 1 1) (> 5 3))");
  expectEqlUint(bool_test.type, VALUE_TYPE_BOOLEAN, "returns boolean type");
  expectTrue(bool_test.value.boolean, "logical and works");

  case("recursive function calls");
  value_t factorial;
  execute(&factorial, "(def! fact (fn (n) (cond ((< n 1) 1) (* n (fact (- n 1))))))\n(fact 5)");
  expectEqlDouble(factorial.value.number, 120, "recursive factorial works");

  case("empty list and nil handling");
  value_t empty_list;
  execute(&empty_list, "()");
  expectEqlUint(empty_list.type, VALUE_TYPE_LIST, "empty list has correct type");
  expectEqlUint((unsigned int)empty_list.value.list.count, 0, "empty list has zero elements");

  arenaDestroy(&ast_arena);
  arenaDestroy(&temp_arena);
  return report();
}
