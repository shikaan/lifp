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

result_void_t execute(value_t *result, const char *input) {
  token_list_t *tokens = nullptr;
  tryAssert(tokenize(test_arena, input), tokens);

  size_t offset = 0;
  size_t depth = 0;
  node_t *ast;
  tryAssert(parse(test_arena, tokens, &offset, &depth), ast);
  tryWithMeta(result_void_t,
              evaluate(result, test_arena, test_arena, ast, environment),
              nullptr);
  return ok(result_void_t);
}

void defSpecialForm() {
  value_t result;
  result_void_t exec;

  tryAssert(execute(&result, "(def! num 1.2)"));
  value_t *value = mapGet(value_t, environment->values, "num");
  expectEqlDouble(value->value.number, 1.2, "defines number");

  tryAssert(execute(&result, "(def! str \"string\")"));
  value = mapGet(value_t, environment->values, "str");
  expectEqlString(value->value.string, "string", 7, "defines string");

  tryAssert(execute(&result, "(def! bool true)"));
  value = mapGet(value_t, environment->values, "bool");
  expectTrue(value->value.boolean, "defines boolean");

  tryAssert(execute(&result, "(def! null nil)"));
  value = mapGet(value_t, environment->values, "null");
  expectNull(value->value.nil, "defines null");

  tryAssert(execute(&result, "(def! list (1 2))"));
  value = mapGet(value_t, environment->values, "list");
  expectTrue((value->value.list.count == 2 &&
              value->value.list.data[0].value.number == 1 &&
              value->value.list.data[1].value.number == 2) != 0,
             "defines list");

  tryAssert(execute(&result, "(def! fun (fn (a b) (+ a b)))"));
  value = mapGet(value_t, environment->values, "fun");
  expectTrue(
      (value->type == VALUE_TYPE_CLOSURE &&
       value->value.closure.arguments.count == 2 &&
       strcmp(value->value.closure.arguments.data[0].value.symbol, "a") == 0 &&
       strcmp(value->value.closure.arguments.data[1].value.symbol, "b") == 0 &&
       value->value.closure.form.value.list.count == 3) != 0,
      "defines function");

  tryFail(execute(&result, "(def! num 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding globals");

  tryFail(execute(&result, "(def! cond 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding specials");

  tryFail(execute(&result, "(def! and 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding builtins");

  tryFail(execute(&result, "(let ((foo 1)) (def! foo 2))"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overriding locals");

  value = mapGet(value_t, environment->values, "num");
  expectEqlDouble(value->value.number, 1.2, "original value remains unchanged");
}

void fnSpecialForm() {
  value_t result;
  result_void_t exec;

  tryAssert(execute(&result, "(fn (x y) (+ x y))"));

  expectEqlUint(result.type, VALUE_TYPE_CLOSURE, "creates closure");
  expectEqlSize(result.value.closure.arguments.count, 2,
                "with correct argument count");
  expectEqlUint(result.value.closure.form.type, NODE_TYPE_LIST,
                "with correct form type");

  tryFail(execute(&result, "(fn (x 1) (+ x y))"), exec);
  expectIncludeString(exec.message, "requires a binding list of symbols",
                      "prevents non-symbols as bindings");

  tryFail(execute(&result, "(fn 1 (+ x y))"), exec);
  expectIncludeString(exec.message, "requires a binding list and a form",
                      "requires a binding list");

  tryAssert(execute(&result, "(def! x 1)"));
  tryFail(execute(&result, "(fn (x) (+ x 1))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing globals");

  tryFail(execute(&result, "(fn (cond) (+ cond 1))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing specials");

  tryFail(execute(&result, "(fn (and) (+ and 1))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing builtins");

  tryFail(execute(&result, "(let ((a 1)) (fn (a) (+ a 1)))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing locals");
}

void letSpecialForm() {
  value_t result;
  result_void_t exec;

  tryAssert(execute(&result, "(let ((a 5) (b 10)) (+ a b))"));
  expectEqlDouble(result.value.number, 15, "evaluates last form");

  tryAssert(execute(&result, "(let ((a 5)) a)"));
  expectEqlDouble(result.value.number, 5, "defines numbers");

  tryAssert(execute(&result, "(let ((a \"lol\")) a)"));
  expectEqlString(result.value.string, "lol", 4, "defines strings");

  tryAssert(execute(&result, "(let ((a true)) a)"));
  expectTrue(result.value.boolean, "defines booleans");

  tryAssert(execute(&result, "(let ((a nil)) a)"));
  expectEqlUint(result.type, VALUE_TYPE_NIL, "defines null");

  tryAssert(execute(&result, "(let ((l (1 \"2\"))) l)"));
  expectTrue((result.value.list.count == 2 &&
              result.value.list.data[0].value.number == 1 &&
              strcmp(result.value.list.data[1].value.string, "2") == 0) != 0,
             "defines lists");

  tryAssert(execute(&result, "(let ((f (fn (x y) (+ x y)))) f)"));
  closure_t closure = result.value.closure;
  expectTrue((closure.arguments.count == 2 &&
              strcmp(closure.arguments.data[0].value.symbol, "x") == 0 &&
              strcmp(closure.arguments.data[1].value.symbol, "y") == 0 &&
              closure.form.value.list.count == 3 &&
              strcmp(closure.form.value.list.data[0].value.symbol, "+") == 0 &&
              strcmp(closure.form.value.list.data[1].value.symbol, "x") == 0 &&
              strcmp(closure.form.value.list.data[2].value.symbol, "y") == 0) !=
                 0,
             "defines functions");

  value_t *leaked_a = mapGet(value_t, environment->values, "a");
  value_t *leaked_b = mapGet(value_t, environment->values, "b");
  expectNull(leaked_a, "doesn't leak binding to outer scope");
  expectNull(leaked_b, "doesn't leak binding to outer scope");

  tryAssert(execute(&result, "(let ((a 5) (b (+ a 1))) (+ a b))"));
  expectEqlDouble(result.value.number, 11,
                  "bindings can depend on previously defined");

  tryFail(execute(&result, "(let ((a (+ 5 b)) (b 1)) (+ a b))"), exec);
  expectIncludeString(exec.message, "cannot be found",
                      "bindings cannot depend on not yet defined");

  tryFail(execute(&result, "(let (a 1) a)"), exec);
  expectIncludeString(exec.message,
                      "requires a list of symbol-form assignments",
                      "requires binding couples");

  tryFail(execute(&result, "(let ((a 1)))"), exec);
  expectIncludeString(exec.message,
                      "requires a list of symbol-form assignments",
                      "requires form");

  tryAssert(execute(&result, "(def! x 1)"));
  tryFail(execute(&result, "(let ((x 1)) x)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing globals");

  tryFail(execute(&result, "(let ((def! 1)) def!)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing specials");

  tryFail(execute(&result, "(let ((and 1)) and)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing builtins");

  tryFail(execute(&result, "(let ((y 1)) (let ((y 2)) y))"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents shadowing locals");
}

void condSpecialForm() {
  value_t result;
  result_void_t exec;

  tryAssert(execute(&result, "(cond (true 42) 99)"));
  expectEqlUint(result.type, VALUE_TYPE_NUMBER, "returns number");
  expectEqlDouble(result.value.number, 42, "evaluates true clause");

  tryAssert(execute(&result, "(cond (false 22) 99)"));
  expectEqlUint(result.type, VALUE_TYPE_NUMBER, "returns number");
  expectEqlDouble(result.value.number, 99, "evaluates fallback clause");

  tryAssert(execute(&result, "(cond (false 42) (true 42) 99)"));
  expectEqlUint(result.type, VALUE_TYPE_NUMBER, "returns number");
  expectEqlDouble(result.value.number, 42, "evaluates the first true clause");

  tryFail(execute(&result, "(cond (1 2) 3)"), exec);
  expectIncludeString(exec.message, "should resolve to a boolean",
                      "prevents non-boolean conditions");

  tryFail(execute(&result, "(cond 1 3)"), exec);
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
