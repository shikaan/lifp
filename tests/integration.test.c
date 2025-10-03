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

  value_t *intermediate_result = nullptr;
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

void number() {
  value_t *result = execute("1");
  expectEqlDouble(result->as.number, 1, "returns correct value");
  valueDestroy(&result);
}

void symbol() {
  value_t *result = execute("+");
  expectEqlUint(result->type, VALUE_TYPE_BUILTIN, "returns builtin type");
  valueDestroy(&result);
}

void list() {
  value_t *result = execute("(1 2)");
  expectEqlUint((unsigned int)result->as.list->count, 2, "returns a list");
  value_t first = result->as.list->data[0];
  expectEqlDouble(first.as.number, 1, "correct first item");
  value_t second = result->as.list->data[1];
  expectEqlDouble(second.as.number, 2, "correct second item");
  valueDestroy(&result);
}

void nestedList() {
  value_t *result = execute("((1) 2)");
  expectEqlUint(result->type, VALUE_TYPE_LIST, "returns a list");
  expectEqlUint(result->as.list->data[0].type, VALUE_TYPE_LIST,
                "with nested list");
  expectEqlUint(result->as.list->data[0].as.list->data->type, VALUE_TYPE_NUMBER,
                "with correct type");
  valueDestroy(&result);
}

void immediateInvocation() {
  value_t *result = execute("((fn (a b) (list:from a b)) 2 3)");
  expectEqlUint(result->type, VALUE_TYPE_LIST, "returns a list");
  expectTrue((result->as.list->data[0].as.number == 2 &&
              result->as.list->data[1].as.number == 3) != 0,
             "with correct value");
  valueDestroy(&result);
}

void simpleForm() {
  value_t *result = execute("(+ 1 2)");
  expectEqlDouble(result->as.number, 3, "returns correct value");
  valueDestroy(&result);
}

void nestedForm() {
  value_t *result = execute("(+ 1 (+ 2 4))");
  expectEqlDouble(result->as.number, 7, "returns correct value");
  valueDestroy(&result);
}

void functionDeclaration() {
  value_t *result = execute("(def! sum (fn (a b) (+ a b)))\n(sum 1 2)");
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns correct type");
  expectEqlDouble(result->as.number, 3, "returns correct value");
  valueDestroy(&result);
}

void functionWithAllocations() {
  value_t *result = execute(
      "(def! comma (fn (s) (str:join \", \" s)))\n(comma (\"1\" \"2\"))");
  expectEqlUint(result->type, VALUE_TYPE_STRING, "returns correct type");
  valueDestroy(&result);
}

void basicLet() {
  value_t *result =
      execute("(let ((plus (fn (x y) (+ x y))) (a 1)) (plus a 1))");
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns correct type");
  expectEqlDouble(result->as.number, 2, "returns correct value");
  valueDestroy(&result);
}

void nestedLet() {
  value_t *result =
      execute("(let ((plus (fn (x y) (+ x y)))) (let ((a 1)) (plus a 1)))");
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns correct type");
  expectEqlDouble(result->as.number, 2, "returns correct value");
  valueDestroy(&result);
}

void letWithEscapedValue() {
  value_t *result = execute("(+ 1 (let ((l 1)) l))");
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns correct type");
  expectEqlDouble(result->as.number, 2, "returns correct value");
  valueDestroy(&result);
}

void basicCond() {
  value_t *result = execute("(cond ((< 5 3) 1) ((> 5 3) 2) (3))");
  expectEqlDouble(result->as.number, 2, "evaluates correct branch");
  valueDestroy(&result);
}

void booleanOperations() {
  value_t *result = execute("(and (= 1 1) (> 5 3))");
  expectEqlUint(result->type, VALUE_TYPE_BOOLEAN, "returns correct type");
  expectTrue(result->as.boolean, "returns correct value");
  valueDestroy(&result);
}

void recursiveCalls() {
  value_t *result = execute(
      "(def! fact (fn (n) (cond ((< n 1) 1) (* n (fact (- n 1))))))\n(fact 5)");
  expectEqlDouble(result->as.number, 120, "returns correct value");
  valueDestroy(&result);
}

void emptyList() {
  value_t *result = execute("()");
  expectEqlUint(result->type, VALUE_TYPE_LIST, "returns correct type");
  expectEqlSize(result->as.list->count, 0, "has zero elements");
  valueDestroy(&result);
}

void currying() {
  value_t *result =
      execute("(def! make-add (fn (a) (fn (b) (+ a b))))\n((make-add 4) 1)");
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns a number");
  expectEqlDouble(result->as.number, 5, "returns correct value");
  valueDestroy(&result);
}

void expandingEnvironment() {
  value_t *result = execute("(def! aa 1)\n"
                            "(def! ab 1)\n"
                            "(def! ac 1)\n"
                            "(def! ad 1)\n"
                            // "(def! ae 1)\n"
                            // "(def! af 1)\n"
                            // "(def! ag 1)\n"
                            // "(def! ah 1)\n"
                            "(def! ai 1)\n");
  expectEqlUint(result->type, VALUE_TYPE_NIL, "allows environment growth");
  valueDestroy(&result);
}

int main() {
  tryAssert(arenaCreate((size_t)(64 * 1024)), ast_arena);

  suite(number);
  suite(symbol);
  suite(list);
  suite(nestedList);
  suite(immediateInvocation);
  suite(simpleForm);
  suite(nestedForm);
  suite(functionDeclaration);
  suite(functionWithAllocations);
  suite(basicLet);
  suite(letWithEscapedValue);
  suite(nestedLet);
  suite(basicCond);
  suite(booleanOperations);
  suite(recursiveCalls);
  suite(emptyList);
  suite(currying);
  suite(expandingEnvironment);

  arenaDestroy(&ast_arena);

  return report();
}
