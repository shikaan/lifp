#include "test.h"
#include "utils.h"

#include "../lib/arena.h"
#include "../lifp/evaluate.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include <assert.h>
#include <stddef.h>

static arena_t *ast_arena;

value_t *execute(const char *input) {
  char input_copy[1024];
  strcpy(input_copy, input);

  char *line = strtok(input_copy, "\n");

  vm_t *machine;
  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);

  value_t *intermediate_result;
  while (line != NULL) {
    valueDestroy(&intermediate_result);
    arenaReset(ast_arena);
    token_list_t *tokens = nullptr;
    tryAssert(tokenize(ast_arena, line), tokens);

    size_t offset = 0;
    size_t depth = 0;
    node_t *node = nullptr;
    tryAssert(parse(ast_arena, tokens, &offset, &depth), node);
    tryAssert(evaluate(node, machine->global), intermediate_result);

    line = strtok(nullptr, "\n");
  }

  vmDestroy(&machine);
  return intermediate_result;
}

int main() {
  tryAssert(arenaCreate((size_t)(64 * 1024)), ast_arena);

  case("number");
  value_t *number = execute("1");
  expectEqlDouble(number->as.number, 1, "returns correct value");
  valueDestroy(&number);

  case("symbol");
  value_t *symbol = execute("+");
  expectEqlUint(symbol->type, VALUE_TYPE_BUILTIN, "returns builtin type");
  valueDestroy(&symbol);

  case("list");
  value_t *list = execute("(1 2)");
  expectEqlUint((unsigned int)list->as.list->count, 2, "returns a list");
  value_t first = list->as.list->data[0];
  expectEqlDouble(first.as.number, 1, "correct first item");
  value_t second = list->as.list->data[1];
  expectEqlDouble(second.as.number, 2, "correct second item");
  valueDestroy(&list);

  case("nested list");
  value_t *nested_list = execute("((1) 2)");
  expectEqlUint(nested_list->type, VALUE_TYPE_LIST, "returns a list");
  expectEqlUint(nested_list->as.list->data[0].type, VALUE_TYPE_LIST, "with nested list");
  expectEqlUint(nested_list->as.list->data[0].as.list->data->type,
  VALUE_TYPE_NUMBER, "with correct type"); valueDestroy(&nested_list);

  case("immediate invocation");
  value_t *immediate_invocation = execute("((fn (a b) (list:from a b)) 2 3)"); 
  expectEqlUint(immediate_invocation->type, VALUE_TYPE_LIST, "returns a list"); 
  expectTrue((immediate_invocation->as.list->data[0].as.number == 2 &&
              immediate_invocation->as.list->data[1].as.number == 3) != 0,
             "with correct value");
  valueDestroy(&immediate_invocation);

  case("simple form");
  value_t *simple = execute("(+ 1 2)");
  expectEqlDouble(simple->as.number, 3, "returns correct value");
  valueDestroy(&simple);

  case("nested form");
  value_t *nested = execute("(+ 1 (+ 2 4))");
  expectEqlDouble(nested->as.number, 7, "returns correct value");
  valueDestroy(&nested);

  // case("declare function");
  // value_t *fun = execute("(def! sum (fn (a b) (+ a b)))\n(sum 1 2)");
  // expectEqlUint(fun->type, VALUE_TYPE_NUMBER, "returns correct type");
  // expectEqlDouble(fun->as.number, 3, "returns correct value");
  // valueDestroy(&fun);
  
  // case("functions with non-inline types");
  // value_t *fun2 = execute("(def! comma (fn (s) (str:join \", \" s)))\n(comma (\"1\" \"2\"))");
  // expectEqlUint(fun2->type, VALUE_TYPE_STRING, "returns correct type");

  // case("let");
  // value_t *let = execute("(let ((plus (fn (x y) (+ x y))) (a 1)) (plus a 1))");
  // expectEqlUint(let->type, VALUE_TYPE_NUMBER, "returns correct type");
  // expectEqlDouble(let->as.number, 2, "returns correct value");

  // case("let with escape");
  // value_t *let_with_escape = execute("(+ 1 (let ((l 1)) l))");
  // expectEqlUint(let_with_escape->type, VALUE_TYPE_NUMBER, "returns correct type");
  // expectEqlDouble(let_with_escape->as.number, 2, "returns correct value");

  // case("conditional - cond special form");
  // value_t *cond_test = execute("(cond ((< 5 3) 1) ((> 5 3) 2) (3))");
  // expectEqlDouble(cond_test->as.number, 2, "evaluates correct branch");

  // case("boolean operations");
  // value_t *bool_test = execute("(and (= 1 1) (> 5 3))");
  // expectEqlUint(bool_test->type, VALUE_TYPE_BOOLEAN, "returns boolean type");
  // expectTrue(bool_test->as.boolean, "logical and works");

  // case("recursive function calls");
  // value_t *factorial = execute("(def! fact (fn (n) (cond ((< n 1) 1) (* n (fact (- n 1))))))\n(fact 5)");
  // expectEqlDouble(factorial->as.number, 120, "recursive factorial works");

  // case("empty list and nil handling");
  // value_t *empty_list = execute("()");
  // expectEqlUint(empty_list->type, VALUE_TYPE_LIST, "empty list has correct type");
  // expectEqlUint((unsigned int)empty_list->as.list->count, 0, "empty list has zero elements");

  // case("currying");
  // value_t *currying = execute("(def! make-add (fn (a) (fn (b) (+ a b))))\n((make-add 4) 1)");
  // expectEqlUint(currying->type, VALUE_TYPE_NUMBER, "returns a number");
  // expectEqlDouble(currying->as.number, 5, "it has correct value");

  // case("expanding environment");
  // value_t *environment = execute(
  //     "(def! aa 1)\n"
  //     "(def! ab 1)\n"
  //     "(def! ac 1)\n"
  //     "(def! ad 1)\n"
  //     "(def! ae 1)\n"
  //     "(def! af 1)\n"
  //     "(def! ag 1)\n"
  //     "(def! ah 1)\n"
  //     "(def! ai 1)\n");
  // expectEqlUint(environment->type, VALUE_TYPE_NIL, "allows environment growth");

  arenaDestroy(&ast_arena);

  return report();
}
