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
  value_t result;

  case("number");
  node_t number_node = nInt(42);
  tryAssert(evaluate(&result, test_arena, &number_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_NUMBER,
                     "has correct type");
  expectEqlDouble(result.value.number, 42, "has correct value");

  case("boolean");
  node_t bool_node = nBool(true);
  tryAssert(evaluate(&result, test_arena, &bool_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_BOOLEAN,
                     "has correct type");
  expectTrue(result.value.boolean, "has correct value");

  case("nil");
  node_t nil_node = nNil();
  tryAssert(evaluate(&result, test_arena, &nil_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_NIL,
                     "has correct type");

  case("string");
  node_t string_node = nStr(test_arena, "str");
  tryAssert(evaluate(&result, test_arena, &string_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_STRING,
                     "has correct type");

  case("symbol");
  value_t symbol;
  tryAssert(valueInit(&symbol, test_arena, (number_t)0));
  mapSet(value_t, environment->values, "value", &symbol);

  node_t symbol_node = nSym(test_arena, "value");
  tryAssert(evaluate(&result, test_arena, &symbol_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_NUMBER,
                     "has correct type");
}

void listOfElements() {
  node_list_t *expected = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), expected);

  node_t first = nInt(42);
  node_t second = nInt(123);
  tryAssert(listAppend(node_t, expected, &first));
  tryAssert(listAppend(node_t, expected, &second));

  node_t list_node = nList(2, expected->data);

  value_t result;
  tryAssert(evaluate(&result, test_arena, &list_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_LIST,
                     "has correct type");
  value_list_t reduced_list = result.value.list;
  expectEqlSize(reduced_list.count, 2, "has correct count");
  for (size_t i = 0; i < reduced_list.count; i++) {
    value_t node = listGet(value_t, &reduced_list, i);
    node_t expected_node = listGet(node_t, expected, i);
    expectEqlValueType(node.type, VALUE_TYPE_NUMBER, "has correct type");
    expectEqlDouble(node.value.number, expected_node.value.number,
                 "has correct value");
  }
}

void functionCall() {
  node_list_t *list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), list);

  node_t symbol = nSym(test_arena, "+");
  node_t num1 = nInt(1);
  node_t num2 = nInt(2);
  node_t num3 = nInt(3);
  tryAssert(listAppend(node_t, list, &symbol));
  tryAssert(listAppend(node_t, list, &num1));
  tryAssert(listAppend(node_t, list, &num2));
  tryAssert(listAppend(node_t, list, &num3));

  node_t form_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;

  value_t result;
  tryAssert(evaluate(&result, test_arena, &form_node, environment));
  expectEqlDouble(result.value.number, 6, "has correct result");
  
  value_t val = { .type = VALUE_TYPE_NUMBER, .value.number = 1};
  mapSet(value_t, environment->values, "lol", &val);
  node_t lol_symbol = nSym(test_arena, "lol");
  
  tryAssert(listCreate(node_t, test_arena, 4), list);
  tryAssert(listAppend(node_t, list, &lol_symbol))
  tryAssert(listAppend(node_t, list, &num1))
  node_t list_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;
  tryAssert(evaluate(&result, test_arena, &list_node, environment));
  expectEqlUint(result.type, VALUE_TYPE_LIST, "does't invoke if symbol is not lambda");
}

void nested() {
  node_list_t *inner_list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), inner_list);

  node_t inner1 = nInt(1);
  node_t inner2 = nInt(2);
  tryAssert(listAppend(node_t, inner_list, &inner1));
  tryAssert(listAppend(node_t, inner_list, &inner2));

  node_t inner_list_node = nList(2, inner_list->data);
  inner_list_node.value.list.capacity = inner_list->capacity;

  // Create outer list: (3 (1 2))
  node_list_t *outer_list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), outer_list);

  node_t outer1 = nInt(3);
  tryAssert(listAppend(node_t, outer_list, &outer1));
  tryAssert(listAppend(node_t, outer_list, &inner_list_node));

  node_t outer_list_node = nList(2, outer_list->data);
  outer_list_node.value.list.capacity = outer_list->capacity;

  value_t result;
  tryAssert(evaluate(&result, test_arena, &outer_list_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_LIST,
                     "has correct type");
  expectEqlSize(result.value.list.count, 2, "has correct count");
  value_t first = listGet(value_t, &result.value.list, 0);
  value_t second = listGet(value_t, &result.value.list, 1);
  expectEqlValueType(first.type, VALUE_TYPE_NUMBER, "has correct type");
  expectEqlValueType(second.type, VALUE_TYPE_LIST, "has correct type");
}

void emptyList() {
  node_list_t *empty_list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), empty_list); // capacity > 0

  node_t empty_list_node = nList(0, empty_list->data);
  empty_list_node.value.list.capacity = empty_list->capacity;

  value_t result;
  tryAssert(evaluate(&result, test_arena, &empty_list_node, environment));
  expectEqlValueType(result.type, VALUE_TYPE_LIST, "has correct type");
  expectEqlSize(result.value.list.count, 0, "has correct count");
}

void allocations() {
  arena_t *small_arena = nullptr;
  tryAssert(arenaCreate(32), small_arena);

  arenaAllocate(small_arena, 32); // Use up all space
  node_t large_node = nInt(123);
  value_t result;
  auto reduction = evaluate(&result, small_arena, &large_node, environment);
  expectEqlInt(reduction.code, RESULT_OK, "does not allocate memory on temp arena");

  arenaDestroy(&small_arena);
}

void errors() {
  node_list_t *list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 1), list);

  case("non-existing symbol");
  node_t sym = nSym(test_arena, "not-existent");
  tryAssert(listAppend(node_t, list, &sym));

  value_t result;
  auto reduction = evaluate(&result, test_arena, &sym, environment);
  expectEqlInt(reduction.code, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND, "with correct symbol");
}

int main(void) {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);
  tryAssert(vmInit(), environment);

  suite(atoms);
  suite(listOfElements);
  suite(functionCall);
  suite(nested);
  suite(emptyList);
  suite(allocations);
  suite(errors);

  environmentDestroy(&environment);
  arenaDestroy(&test_arena);
  return report();
}
