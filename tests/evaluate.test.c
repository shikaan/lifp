#include "../lifp/evaluate.h"
#include "../lib/arena.h"
#include "../lib/list.h"
#include "../lifp/error.h"
#include "../lifp/node.h"
#include "../lifp/virtual_machine.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;
static environment_t *environment;

void expectEqlValueType(value_type_t actual, value_type_t expected,
                        const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected node type %d to equal %d", (int)actual,
           (int)expected);
  expect(actual == expected, name, msg);
}

void atoms() {
  value_t *result = nullptr;

  case("integer");
  node_t integer_node = nInt(42);
  tryAssertAssign(evaluate(test_arena, &integer_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_INTEGER,
                     "has correct type");
  expectEqlInt(result->value.integer, 42, "has correct value");

  case("boolean");
  node_t bool_node = nBool(true);
  tryAssertAssign(evaluate(test_arena, &bool_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_BOOLEAN,
                     "has correct type");
  expectTrue(result->value.boolean, "has correct value");

  case("nil");
  node_t nil_node = nNil();
  tryAssertAssign(evaluate(test_arena, &nil_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_NIL,
                     "has correct type");

  case("symbol");
  value_t *symbol = nullptr;
  tryAssertAssign(valueCreate(test_arena, VALUE_TYPE_INTEGER), symbol);
  mapSet(environment->values, "value", symbol);

  node_t symbol_node = nSym("value");
  tryAssertAssign(evaluate(test_arena, &symbol_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_INTEGER,
                     "has correct type");
}

void listOfElements() {
  node_list_t *expected = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 4), expected);

  node_t first = nInt(42);
  node_t second = nInt(123);
  tryAssert(listAppend(node_t, expected, &first));
  tryAssert(listAppend(node_t, expected, &second));

  node_t list_node = nList(2, expected->data);

  value_t *result = nullptr;
  tryAssertAssign(evaluate(test_arena, &list_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_LIST,
                     "has correct type");
  value_list_t reduced_list = result->value.list;
  expectEqlSize(reduced_list.count, 2, "has correct count");
  for (size_t i = 0; i < reduced_list.count; i++) {
    value_t node = listGet(value_t, &reduced_list, i);
    node_t expected_node = listGet(node_t, expected, i);
    expectEqlValueType(node.type, VALUE_TYPE_INTEGER, "has correct type");
    expectEqlInt(node.value.integer, expected_node.value.integer,
                 "has correct value");
  }
}

void functionCall() {
  node_list_t *list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 4), list);

  node_t symbol = nSym("+");
  node_t num1 = nInt(1);
  node_t num2 = nInt(2);
  node_t num3 = nInt(3);
  tryAssert(listAppend(node_t, list, &symbol));
  tryAssert(listAppend(node_t, list, &num1));
  tryAssert(listAppend(node_t, list, &num2));
  tryAssert(listAppend(node_t, list, &num3));

  node_t form_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;

  value_t *result = nullptr;
  tryAssertAssign(evaluate(test_arena, &form_node, environment), result);
  expectNotNull(result, "reduced result is not null");
  expectEqlInt(result->value.integer, 6, "has correct result");
  
  value_t val = { .type = VALUE_TYPE_INTEGER, .value.integer = 1};
  mapSet(environment->values, "lol", &val);
  node_t lol_symbol = nSym("lol");
  
  tryAssertAssign(listCreate(node_t, test_arena, 4), list);
  tryAssert(listAppend(node_t, list, &lol_symbol))
  tryAssert(listAppend(node_t, list, &num1))
  node_t list_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;
  tryAssertAssign(evaluate(test_arena, &list_node, environment), result);
  expectEqlUint(result->type, VALUE_TYPE_LIST, "does't invoke if symbol is not lambda");
}

void nested() {
  node_list_t *inner_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 4), inner_list);

  node_t inner1 = nInt(1);
  node_t inner2 = nInt(2);
  tryAssert(listAppend(node_t, inner_list, &inner1));
  tryAssert(listAppend(node_t, inner_list, &inner2));

  node_t inner_list_node = nList(2, inner_list->data);
  inner_list_node.value.list.capacity = inner_list->capacity;

  // Create outer list: (3 (1 2))
  node_list_t *outer_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 4), outer_list);

  node_t outer1 = nInt(3);
  tryAssert(listAppend(node_t, outer_list, &outer1));
  tryAssert(listAppend(node_t, outer_list, &inner_list_node));

  node_t outer_list_node = nList(2, outer_list->data);
  outer_list_node.value.list.capacity = outer_list->capacity;

  value_t *result = nullptr;
  tryAssertAssign(evaluate(test_arena, &outer_list_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_LIST,
                     "has correct type");
  expectEqlSize(result->value.list.count, 2, "has correct count");
  value_t first = listGet(value_t, &result->value.list, 0);
  value_t second = listGet(value_t, &result->value.list, 1);
  expectEqlValueType(first.type, VALUE_TYPE_INTEGER, "has correct type");
  expectEqlValueType(second.type, VALUE_TYPE_LIST, "has correct type");
}

void emptyList() {
  node_list_t *empty_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 4), empty_list); // capacity > 0

  node_t empty_list_node = nList(0, empty_list->data);
  empty_list_node.value.list.capacity = empty_list->capacity;

  value_t *result = nullptr;
  tryAssertAssign(evaluate(test_arena, &empty_list_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_LIST, "has correct type");
  expectEqlSize(result->value.list.count, 0, "has correct count");
}

void allocations() {
  arena_t *small_arena = nullptr;
  tryAssertAssign(arenaCreate(32), small_arena);

  arenaAllocate(small_arena, 31); // Use up most space
  node_t large_node = nInt(123);
  result_value_ref_t reduction = evaluate(small_arena, &large_node, environment);
  expectEqlInt(reduction.code, ERROR_CODE_ALLOCATION,
                "returns allocation error");

  arenaDestroy(&small_arena);
}

void errors() {
  node_list_t *list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 1), list);

  case("non-existing symbol");
  node_t sym = nSym("not-existent");
  tryAssert(listAppend(node_t, list, &sym));

  result_value_ref_t reduction = evaluate(test_arena, &sym, environment);
  expectEqlInt(reduction.code, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND, "with correct symbol");
}

void defSpecialForm() {
  node_list_t *list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 1), list);

  node_t special = nSym("def!");
  node_t var = nSym("foo");
  node_t value = nInt(1);

  tryAssert(listAppend(node_t, list, &special));
  tryAssert(listAppend(node_t, list, &var));
  tryAssert(listAppend(node_t, list, &value));

  node_t list_node = nList(3, list->data);
  list_node.value.list.arena = test_arena;

  tryAssert(evaluate(test_arena, &list_node, environment));
  value_t* val = mapGet(value_t, environment->values, "foo");

  expectNotNull(val, "environment is updated");
  expectEqlInt(val->value.integer, 1, "with correct value");
}

void fnSpecialForm() {
  // Test creating a function: (fn (x y) (+ x y))
  node_list_t *fn_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 3), fn_list);

  node_t fn_special = nSym("fn");
  
  // Create argument list (x y)
  node_list_t *args_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 2), args_list);
  node_t arg_x = nSym("x");
  node_t arg_y = nSym("y");
  tryAssert(listAppend(node_t, args_list, &arg_x));
  tryAssert(listAppend(node_t, args_list, &arg_y));
  node_t args_node = nList(2, args_list->data);
  args_node.value.list.arena = test_arena;

  // Create body (+ x y)
  node_list_t *body_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 3), body_list);
  node_t plus_sym = nSym("+");
  node_t body_x = nSym("x");
  node_t body_y = nSym("y");
  tryAssert(listAppend(node_t, body_list, &plus_sym));
  tryAssert(listAppend(node_t, body_list, &body_x));
  tryAssert(listAppend(node_t, body_list, &body_y));
  node_t body_node = nList(3, body_list->data);
  body_node.value.list.arena = test_arena;

  // Assemble the fn form
  tryAssert(listAppend(node_t, fn_list, &fn_special));
  tryAssert(listAppend(node_t, fn_list, &args_node));
  tryAssert(listAppend(node_t, fn_list, &body_node));

  node_t fn_node = nList(3, fn_list->data);
  fn_node.value.list.arena = test_arena;

  value_t *closure;
  tryAssertAssign(evaluate(test_arena, &fn_node, environment), closure);
  expectEqlUint(closure->type, VALUE_TYPE_CLOSURE, "creates closure");
  expectEqlSize(closure->value.closure.arguments.count, 2, "with correct argument count");
  expectEqlUint(closure->value.closure.form.type, NODE_TYPE_LIST, "with correct form type");
 
  case("not using symbols for arguments");
  node_list_t *fn_no_symbol = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 3), fn_no_symbol);
 
  // Create argument list (x 1)
  tryAssertAssign(listCreate(node_t, test_arena, 2), args_list);
  node_t arg_one = nInt(1);
  tryAssert(listAppend(node_t, args_list, &arg_x));
  tryAssert(listAppend(node_t, args_list, &arg_one));
  args_node.value.list.data = args_list->data;

  // Assemble the fn form
  tryAssert(listAppend(node_t, fn_no_symbol, &fn_special));
  tryAssert(listAppend(node_t, fn_no_symbol, &args_node));
  tryAssert(listAppend(node_t, fn_no_symbol, &body_x));

  fn_node.value.list.data = fn_no_symbol->data;
  result_value_ref_t evaluation = evaluate(test_arena, &fn_node, environment);
  expectEqlInt(evaluation.code, ERROR_CODE_RUNTIME_ERROR, "throws a syntax error"); 
  
  case("not using lists for arguments");
  node_list_t *fn_bad_bindings = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 3), fn_bad_bindings);
 
  // Assemble the fn form
  tryAssert(listAppend(node_t, fn_bad_bindings, &fn_special));
  tryAssert(listAppend(node_t, fn_bad_bindings, &arg_one));
  tryAssert(listAppend(node_t, fn_bad_bindings, &body_x));

  fn_node.value.list.data = fn_bad_bindings->data;
  evaluation = evaluate(test_arena, &fn_node, environment);
  expectEqlInt(evaluation.code, ERROR_CODE_RUNTIME_ERROR, "throws a syntax error"); 
}

void letSpecialForm() {
  // Test let binding: (let ((a 5) (b 10)) (+ a b))
  node_list_t *let_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 3), let_list);

  node_t let_special = nSym("let");

  // Create binding pairs ((a 5) (b 10))
  node_list_t *bindings_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 2), bindings_list);

  // First binding (a 5)
  node_list_t *pair1_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 2), pair1_list);
  node_t let_a = nSym("a");
  node_t let_val1 = nInt(5);
  tryAssert(listAppend(node_t, pair1_list, &let_a));
  tryAssert(listAppend(node_t, pair1_list, &let_val1));
  node_t pair1_node = nList(2, pair1_list->data);
  pair1_node.value.list.arena = test_arena;

  // Second binding (b 10)
  node_list_t *pair2_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 2), pair2_list);
  node_t let_b = nSym("b");
  node_t let_val2 = nInt(10);
  tryAssert(listAppend(node_t, pair2_list, &let_b));
  tryAssert(listAppend(node_t, pair2_list, &let_val2));
  node_t pair2_node = nList(2, pair2_list->data);
  pair2_node.value.list.arena = test_arena;

  // Add pairs to bindings list
  tryAssert(listAppend(node_t, bindings_list, &pair1_node));
  tryAssert(listAppend(node_t, bindings_list, &pair2_node));
  node_t bindings_node = nList(2, bindings_list->data);
  bindings_node.value.list.arena = test_arena;

  // Create let body (+ a b)
  node_list_t *let_body_list = nullptr;
  tryAssertAssign(listCreate(node_t, test_arena, 3), let_body_list);
  node_t let_plus = nSym("+");
  node_t let_a_ref = nSym("a");
  node_t let_b_ref = nSym("b");
  tryAssert(listAppend(node_t, let_body_list, &let_plus));
  tryAssert(listAppend(node_t, let_body_list, &let_a_ref));
  tryAssert(listAppend(node_t, let_body_list, &let_b_ref));
  node_t let_body_node = nList(3, let_body_list->data);
  let_body_node.value.list.arena = test_arena;

  // Assemble the let form
  tryAssert(listAppend(node_t, let_list, &let_special));
  tryAssert(listAppend(node_t, let_list, &bindings_node));
  tryAssert(listAppend(node_t, let_list, &let_body_node));

  node_t let_node = nList(3, let_list->data);
  let_node.value.list.arena = test_arena;

  value_t* let;
  tryAssertAssign(evaluate(test_arena, &let_node, environment), let);
  expectEqlUint(let->type, VALUE_TYPE_INTEGER, "evaluates to integer");
  expectEqlInt(let->value.integer, 15, "with correct result (5 + 10)");

  // Verify that let bindings don't leak to outer environment
  value_t* leaked_a = mapGet(value_t, environment->values, "a");
  value_t* leaked_b = mapGet(value_t, environment->values, "b");
  expectNull(leaked_a, "let binding 'a' doesn't leak to outer scope");
  expectNull(leaked_b, "let binding 'b' doesn't leak to outer scope");
}

void condSpecialForm() {
  node_t *cond = nullptr;
  tryAssertAssign(nodeCreate(test_arena, NODE_TYPE_LIST), cond);

  node_t cond_special = nSym("cond");
  node_t true_condition = nBool(true);
  node_t false_condition = nBool(false);
  node_t true_value = nInt(42);
  node_t false_value = nInt(22);
  node_t fallback_value = nInt(99);

  node_t *true_clause = nullptr;
  tryAssertAssign(nodeCreate(test_arena, NODE_TYPE_LIST), true_clause);
  tryAssert(listAppend(node_t, &true_clause->value.list, &true_condition));
  tryAssert(listAppend(node_t, &true_clause->value.list, &true_value));

  tryAssert(listAppend(node_t, &cond->value.list, &cond_special));
  tryAssert(listAppend(node_t, &cond->value.list, true_clause));
  tryAssert(listAppend(node_t, &cond->value.list, &fallback_value));

  value_t *result = nullptr;
  tryAssertAssign(evaluate(test_arena, cond, environment), result);
  expectEqlUint(result->type, VALUE_TYPE_INTEGER, "returns integer");
  expectEqlInt(result->value.integer, true_value.value.integer, "evaluates true clause");

  tryAssertAssign(nodeCreate(test_arena, NODE_TYPE_LIST), cond);
  
  node_t *false_clause = nullptr;
  tryAssertAssign(nodeCreate(test_arena, NODE_TYPE_LIST), false_clause);
  tryAssert(listAppend(node_t, &false_clause->value.list, &false_condition));
  tryAssert(listAppend(node_t, &false_clause->value.list, &false_value));

  tryAssert(listAppend(node_t, &cond->value.list, &cond_special));
  tryAssert(listAppend(node_t, &cond->value.list, false_clause));
  tryAssert(listAppend(node_t, &cond->value.list, &fallback_value));

  tryAssertAssign(evaluate(test_arena, cond, environment), result);
  expectEqlUint(result->type, VALUE_TYPE_INTEGER, "returns integer");
  expectEqlInt(result->value.integer, fallback_value.value.integer, "evaluates fallback clause");

  // (cond (false 42) (true 42) 99)
  tryAssertAssign(nodeCreate(test_arena, NODE_TYPE_LIST), cond);
   
  tryAssert(listAppend(node_t, &cond->value.list, &cond_special));
  tryAssert(listAppend(node_t, &cond->value.list, false_clause));
  tryAssert(listAppend(node_t, &cond->value.list, true_clause));
  tryAssert(listAppend(node_t, &cond->value.list, &fallback_value));

  tryAssertAssign(evaluate(test_arena, cond, environment), result);
  expectEqlUint(result->type, VALUE_TYPE_INTEGER, "returns integer");
  expectEqlInt(result->value.integer, 42, "evaluates the first true clause");
}

int main(void) {
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), test_arena);
  tryAssertAssign(vmInit(), environment);

  suite(atoms);
  suite(listOfElements);
  suite(functionCall);
  suite(nested);
  suite(emptyList);
  suite(allocations);
  suite(errors);
  suite(defSpecialForm);
  suite(fnSpecialForm);
  suite(letSpecialForm);
  suite(condSpecialForm);

  environmentDestroy(&environment);
  arenaDestroy(&test_arena);
  return report();
}
