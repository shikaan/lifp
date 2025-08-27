#include "../lifp/evaluate.h"
#include "../lifp/node.h"
#include "../lifp/parse.h"
#include "../lifp/tokenize.h"
#include "../lifp/virtual_machine.h"

#include "test.h"
#include "utils.h"

static arena_t *test_arena;
static environment_t *environment;

result_void_t execute(value_t *result, const char *input) {
  token_list_t *tokens = nullptr;
  tryAssert(tokenize(test_arena, input), tokens);

  size_t offset = 0;
  size_t depth = 0;
  node_t *ast;
  tryAssert(parse(test_arena, tokens, &offset, &depth), ast);
  tryWithMeta(result_void_t, evaluate(result, test_arena, ast, environment),
              nullptr);
  return ok(result_void_t);
}

void defSpecialForm() {
  arenaReset(test_arena);
  environmentReset(environment);

  value_t result;
  result_void_t exec;

  tryAssert(execute(&result, "(def! foo 1)"));

  value_t *val = mapGet(value_t, environment->values, "foo");
  expectNotNull(val, "environment is updated");
  expectEqlDouble(val->value.number, 1, "with correct value");

  tryFail(execute(&result, "(def! foo 2)"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents overrides");
  val = mapGet(value_t, environment->values, "foo");
  expectEqlDouble(val->value.number, 1, "original value remains unchanged");
}

void fnSpecialForm() {
  arenaReset(test_arena);
  environmentReset(environment);

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

  tryFail(execute(&result, "(let ((a 1)) (fn (a) (+ a 1)))"), exec);
  expectIncludeString(exec.message, "shadows a value",
                      "prevents shadowing locals");
}

void letSpecialForm() {
  arenaReset(test_arena);
  environmentReset(environment);

  value_t result;
  result_void_t exec;

  tryAssert(execute(&result, "(let ((a 5) (b 10)) (+ a b))"));

  expectEqlUint(result.type, VALUE_TYPE_NUMBER, "evaluates to number");
  expectEqlDouble(result.value.number, 15, "with correct result (5 + 10)");

  value_t *leaked_a = mapGet(value_t, environment->values, "a");
  value_t *leaked_b = mapGet(value_t, environment->values, "b");
  expectNull(leaked_a, "let binding 'a' doesn't leak to outer scope");
  expectNull(leaked_b, "let binding 'b' doesn't leak to outer scope");

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
                      "prevents global shadowing");

  tryFail(execute(&result, "(let ((y 1)) (let ((y 2)) y))"), exec);
  expectIncludeString(exec.message, "already been declared",
                      "prevents local shadowing");
}

void condSpecialForm() {
  arenaReset(test_arena);
  environmentReset(environment);

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
  tryAssert(vmInit(), environment);

  suite(defSpecialForm);
  suite(fnSpecialForm);
  suite(letSpecialForm);
  suite(condSpecialForm);

  environmentDestroy(&environment);
  arenaDestroy(&test_arena);
  return report();
}
