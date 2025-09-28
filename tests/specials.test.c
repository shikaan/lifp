#include "../lifp/evaluate.h"
#include "../lifp/node.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"

#include "test.h"
#include "utils.h"
#include <string.h>

static arena_t *test_arena;
static environment_t *environment;

result_value_ref_t execute(const char *input) {
  token_list_t *tokens = nullptr;
  tryAssert(tokenize(test_arena, input), tokens);

  size_t offset = 0;
  size_t depth = 0;
  node_t *ast;
  tryAssert(parse(test_arena, tokens, &offset, &depth), ast);
  return evaluate(ast, environment);
}

void defSpecialForm() {
  value_t *result;
  result_value_ref_t exec;

  tryAssert(execute("(def! num 1.2)"), result);
  value_t *value = valueMapGet(&environment->values, "num");
  expectEqlDouble(value->as.number, 1.2, "defines number");
  valueDestroy(&result);

  tryAssert(execute("(def! str \"string\")"), result);
  value = valueMapGet(&environment->values, "str");
  expectEqlString(value->as.string, "string", 7, "defines string");
  valueDestroy(&result);

  tryAssert(execute("(def! bool true)"), result);
  value = valueMapGet(&environment->values, "bool");
  expectTrue(value->as.boolean, "defines boolean");
  valueDestroy(&result);

  tryAssert(execute("(def! null nil)"), result);
  value = valueMapGet(&environment->values, "null");
  expectEqlUint(value->type, VALUE_TYPE_NIL, "defines null");
  valueDestroy(&result);

  tryAssert(execute("(def! list (1 2))"), result);
  value = valueMapGet(&environment->values, "list");
  expectTrue((value->as.list->count == 2 &&
              value->as.list->data[0].as.number == 1 &&
              value->as.list->data[1].as.number == 2) != 0,
             "defines list");
  valueDestroy(&result);

  tryAssert(execute("(def! fun (fn (a b) (+ a b)))"), result);
  value = valueMapGet(&environment->values, "fun");
  expectTrue((value->type == VALUE_TYPE_CLOSURE &&
              value->as.closure.arguments->count == 2 &&
              strcmp(value->as.closure.arguments->data[0], "a") == 0 &&
              strcmp(value->as.closure.arguments->data[1], "b") == 0 &&
              value->as.closure.form->value.list.count == 3) != 0,
             "defines function");
  valueDestroy(&result);

  tryFail(execute("(def! num 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding globals");

  tryFail(execute("(def! cond 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding specials");

  tryFail(execute("(def! and 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding builtins");

  tryFail(execute("(def! lo:l 2)"), exec);
  expectIncludeString(exec.message, "Unexpected namespace delimiter",
                      "bindings cannot include namespace symbol");

  tryFail(execute("(let ((foo 1)) (def! foo 2))"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding locals");

  value = valueMapGet(&environment->values, "num");
  expectEqlDouble(value->as.number, 1.2, "original value remains unchanged");
}

void fnSpecialForm() {
  value_t *result;
  result_value_ref_t exec;

  tryAssert(execute("(fn (x y) (+ x y))"), result);

  expectEqlUint(result->type, VALUE_TYPE_CLOSURE, "creates closure");
  expectEqlSize(result->as.closure.arguments->count, 2,
                "with correct argument count");
  expectEqlUint(result->as.closure.form->type, NODE_TYPE_LIST,
                "with correct form type");
  valueDestroy(&result);

  tryFail(execute("(fn (x 1) (+ x y))"), exec);
  expectIncludeString(exec.message, "requires a binding list of symbols",
                      "prevents non-symbols as bindings");

  tryFail(execute("(fn 1 (+ x y))"), exec);
  expectIncludeString(exec.message, "requires a binding list and a form",
                      "requires a binding list");

  tryAssert(execute("(def! x 1)"), result);
  tryFail(execute("(fn (x) (+ x 1))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing globals");
  valueDestroy(&result);

  tryFail(execute("(fn (cond) (+ cond 1))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing specials");

  tryFail(execute("(fn (and) (+ and 1))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing builtins");

  tryFail(execute("(let ((a 1)) (fn (a) (+ a 1)))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing locals");
}

void letSpecialForm() {
  value_t *result;
  result_value_ref_t exec;

  tryAssert(execute("(let ((a 5) (b 10)) (+ a b))"), result);
  expectEqlDouble(result->as.number, 15, "evaluates last form");
  valueDestroy(&result);

  tryAssert(execute("(let ((a 5)) a)"), result);
  expectEqlDouble(result->as.number, 5, "defines numbers");
  valueDestroy(&result);

  tryAssert(execute("(let ((a \"lol\")) a)"), result);
  expectEqlString(result->as.string, "lol", 4, "defines strings");
  valueDestroy(&result);

  tryAssert(execute("(let ((a true)) a)"), result);
  expectTrue(result->as.boolean, "defines booleans");
  valueDestroy(&result);

  tryAssert(execute("(let ((a nil)) a)"), result);
  expectEqlUint(result->type, VALUE_TYPE_NIL, "defines null");
  valueDestroy(&result);

  tryAssert(execute("(let ((l (1 \"2\"))) l)"), result);
  expectTrue((result->as.list->count == 2 &&
              result->as.list->data[0].as.number == 1 &&
              strcmp(result->as.list->data[1].as.string, "2") == 0) != 0,
             "defines lists");
  valueDestroy(&result);

  tryAssert(execute("(let ((f (fn (x y) (+ x y)))) f)"), result);
  expectTrue((result->type == VALUE_TYPE_CLOSURE &&
              result->as.closure.arguments->count == 2 &&
              strcmp(result->as.closure.arguments->data[0], "x") == 0 &&
              strcmp(result->as.closure.arguments->data[1], "y") == 0 &&
              result->as.closure.form->value.list.count == 3 &&
              strcmp(result->as.closure.form->value.list.data[0].value.symbol,
                     "+") == 0 &&
              strcmp(result->as.closure.form->value.list.data[1].value.symbol,
                     "x") == 0 &&
              strcmp(result->as.closure.form->value.list.data[2].value.symbol,
                     "y") == 0) != 0,
             "defines functions");
  valueDestroy(&result);

  value_t *leaked_a = valueMapGet(&environment->values, "a");
  value_t *leaked_b = valueMapGet(&environment->values, "b");
  expectNull(leaked_a, "doesn't leak binding to outer scope");
  expectNull(leaked_b, "doesn't leak binding to outer scope");

  tryAssert(execute("(let ((a 5) (b (+ a 1))) (+ a b))"), result);
  expectEqlDouble(result->as.number, 11,
                  "bindings can depend on previously defined");
  valueDestroy(&result);

  tryFail(execute("(let ((a (+ 5 b)) (b 1)) (+ a b))"), exec);
  expectIncludeString(exec.message, "cannot be found",
                      "bindings cannot depend on not yet defined");

  tryFail(execute("(let (a 1) a)"), exec);
  expectIncludeString(exec.message,
                      "requires a list of symbol-form assignments",
                      "requires binding couples");

  tryFail(execute("(let ((a 1)))"), exec);
  expectIncludeString(exec.message,
                      "requires a list of symbol-form assignments",
                      "requires form");

  tryAssert(execute("(def! x 1)"), result);
  valueDestroy(&result);
  tryFail(execute("(let ((x 1)) x)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing globals");

  tryFail(execute("(let ((def! 1)) def!)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing specials");

  tryFail(execute("(let ((and 1)) and)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing builtins");

  tryFail(execute("(let ((y 1)) (let ((y 2)) y))"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing locals");

  tryFail(execute("(let ((lo:l 1)) lo:l)"), exec);
  expectIncludeString(exec.message, "Unexpected namespace delimiter",
                      "bindings cannot include namespace symbol");
}

void condSpecialForm() {
  value_t *result;
  result_value_ref_t exec;

  tryAssert(execute("(cond (true 42) 99)"), result);
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns number");
  expectEqlDouble(result->as.number, 42, "evaluates true clause");
  valueDestroy(&result);

  tryAssert(execute("(cond (false 22) 99)"), result);
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns number");
  expectEqlDouble(result->as.number, 99, "evaluates fallback clause");
  valueDestroy(&result);

  tryAssert(execute("(cond (false 42) (true 42) 99)"), result);
  expectEqlUint(result->type, VALUE_TYPE_NUMBER, "returns number");
  expectEqlDouble(result->as.number, 42, "evaluates the first true clause");
  valueDestroy(&result);

  tryFail(execute("(cond (1 2) 3)"), exec);
  expectIncludeString(exec.message, "should resolve to a boolean",
                      "prevents non-boolean conditions");

  tryFail(execute("(cond 1 3)"), exec);
  expectIncludeString(exec.message, "requires a list of condition-form",
                      "requires a list of conditions");
}

int main(void) {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);
  vm_t *machine;

  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);
  environment = machine->global;
  suite(defSpecialForm);
  vmDestroy(&machine);

  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);
  environment = machine->global;
  suite(fnSpecialForm);
  vmDestroy(&machine);

  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);
  environment = machine->global;
  suite(letSpecialForm);
  vmDestroy(&machine);

  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);
  environment = machine->global;
  suite(condSpecialForm);
  vmDestroy(&machine);

  arenaDestroy(&test_arena);
  return report();
}
