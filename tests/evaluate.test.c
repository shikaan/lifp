#include "../lifp/evaluate.h"
#include "../lib/list.h"
#include "../lifp/error.h"
#include "../lifp/node.h"
#include "../lifp/virtual_machine.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

static environment_t *global;
static arena_t *test_arena;

void expectEqlValueType(value_type_t actual, value_type_t expected,
                        const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected node type %d to equal %d", (int)actual,
           (int)expected);
  expect(actual == expected, name, msg);
}

void atoms() {
  value_t *result = nullptr;

  case("number");
  node_t number_node = nInt(42);
  tryAssert(evaluate(&number_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_NUMBER,
                     "has correct type");
  expectEqlDouble(result->as.number, 42, "has correct value");
  valueDestroy(&result);

  case("boolean");
  node_t bool_node = nBool(true);
  tryAssert(evaluate(&bool_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_BOOLEAN,
                     "has correct type");
  expectTrue(result->as.boolean, "has correct value");
  valueDestroy(&result);

  case("nil");
  node_t nil_node = nNil();
  tryAssert(evaluate(&nil_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_NIL,
                     "has correct type");
  valueDestroy(&result);

  case("string");
  node_t string_node = nStr(test_arena, "str");
  tryAssert(evaluate(&string_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_STRING,
                     "has correct type");
  expectEqlString(result->as.string, "str", 4, "has correct value");
  valueDestroy(&result);

  case("symbol");
  value_t symbol;
  symbol = (value_t){.type = VALUE_TYPE_NUMBER, .as.number = 0};
  tryAssert(environmentRegisterSymbol(global, "value", &symbol));

  node_t symbol_node = nSym(test_arena, "value");
  tryAssert(evaluate(&symbol_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_NUMBER,
                     "has correct type");
  expectEqlDouble(result->as.number, 0, "has correct value");
  valueDestroy(&result);
}

void listOfElements() {
  node_list_t *expected = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), expected);

  node_t first = nInt(42);
  node_t second = nInt(123);
  tryAssert(listAppend(node_t, expected, &first));
  tryAssert(listAppend(node_t, expected, &second));

  node_t list_node = nList(2, expected->data);

  value_t *result;
  tryAssert(evaluate(&list_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_LIST,
                     "has correct type");
  value_array_t *reduced_list = result->as.list;
  expectEqlSize(reduced_list->count, 2, "has correct count");

  for (size_t i = 0; i < reduced_list->count; i++) {
    value_t node = listGet(value_t, reduced_list, i);
    node_t expected_node = listGet(node_t, expected, i);
    expectEqlValueType(node.type, VALUE_TYPE_NUMBER, "has correct type");
    expectEqlDouble(node.as.number, expected_node.value.number,
                 "has correct value");
  }
  valueDestroy(&result);
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

  value_t *result;
  tryAssert(evaluate(&form_node, global), result);
  expectEqlDouble(result->as.number, 6, "has correct result");
  valueDestroy(&result);
  
  value_t val = { .type = VALUE_TYPE_NUMBER, .as.number = 1};
  tryAssert(environmentRegisterSymbol(global, "lol", &val));
  node_t lol_symbol = nSym(test_arena, "lol");
  
  tryAssert(listCreate(node_t, test_arena, 4), list);
  tryAssert(listAppend(node_t, list, &lol_symbol))
  tryAssert(listAppend(node_t, list, &num1))
  node_t list_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;
  tryAssert(evaluate(&list_node, global), result);
  expectEqlUint(result->type, VALUE_TYPE_LIST, "does't invoke if symbol is not lambda");
  valueDestroy(&result);
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

  value_t *result;
  tryAssert(evaluate(&outer_list_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_LIST,
                     "has correct type");
  expectEqlSize(result->as.list->count, 2, "has correct count");
  value_t first = listGet(value_t, result->as.list, 0);
  value_t second = listGet(value_t, result->as.list, 1);
  expectEqlValueType(first.type, VALUE_TYPE_NUMBER, "has correct type");
  expectEqlValueType(second.type, VALUE_TYPE_LIST, "has correct type");
  valueDestroy(&result);
}

void emptyList() {
  node_list_t *empty_list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 4), empty_list);

  node_t empty_list_node = nList(0, empty_list->data);
  empty_list_node.value.list.capacity = empty_list->capacity;

  value_t* result;
  tryAssert(evaluate(&empty_list_node, global), result);
  expectEqlValueType(result->type, VALUE_TYPE_LIST, "has correct type");
  expectEqlSize(result->as.list->count, 0, "has correct count");
  valueDestroy(&result);
}

void allocations() {
  // Make a nested list (1 (2 "a"))
  value_array_t *inner_list_values = nullptr;
  tryAssert(valueArrayCreate(2), inner_list_values);
  string_t string = strdup("a");
  string_t inner_string = strdup(string);
  inner_list_values->data[0] = (value_t){.type = VALUE_TYPE_NUMBER,
                                         .as.number = 2,
                                         .position = {1, 1}};
  inner_list_values->data[1] = (value_t){.type = VALUE_TYPE_STRING,
                                         .as.string = inner_string,
                                         .position = {1, 1}};

  value_t inner_list_value = {.type = VALUE_TYPE_LIST,
                              .as.list = inner_list_values,
                              .position = {1, 1}};

  value_array_t *outer_list_values = nullptr;
  tryAssert(valueArrayCreate(2), outer_list_values);
  outer_list_values->data[0] = (value_t){.type = VALUE_TYPE_NUMBER,
                                         .as.number = 1,
                                         .position = {1, 1}};
  outer_list_values->data[1] = inner_list_value;

  value_t outer_list_value = {.type = VALUE_TYPE_LIST,
                              .as.list = outer_list_values,
                              .position = {1, 1}};

  tryAssert(environmentRegisterSymbol(global, "nested", &outer_list_value));
  valueArrayDestroy(&outer_list_values); 

  // Evaluate the symbol and verify nested retrieval
  node_t sym_node = nSym(test_arena, "nested");
  value_t *result = nullptr;
  tryAssert(evaluate(&sym_node, global), result);

  case("nested list retrieval");
  expectEqlValueType(result->type, VALUE_TYPE_LIST, "symbol returns a list");
  expectEqlSize(result->as.list->count, 2, "top-level has correct size");

  value_t first = result->as.list->data[0];
  expectEqlValueType(first.type, VALUE_TYPE_NUMBER, "first has correct type");
  expectEqlDouble(first.as.number, 1, "first has correct value");

  value_t second = result->as.list->data[1];
  expectEqlValueType(second.type, VALUE_TYPE_LIST, "nested list has correct type");
  expectEqlSize(second.as.list->count, 2, "nested list has correct count");

  value_t inner_first = second.as.list->data[0];
  expectEqlValueType(inner_first.type, VALUE_TYPE_NUMBER, "inner first type");
  expectEqlDouble(inner_first.as.number, 2, "inner first value");

  value_t inner_second = second.as.list->data[1];
  expectEqlValueType(inner_second.type, VALUE_TYPE_STRING, "inner second type");
  expectEqlString(inner_second.as.string, "a", 2, "inner second value");

  valueDestroy(&result);
  
  // TODO: this should be done for each value type
  case("deep copy");

  const value_t* retrieved = environmentResolveSymbol(global, "nested");

  value_array_t *retrieved_outer = retrieved->as.list;
  value_array_t* retrieved_inner = retrieved->as.list->data[1].as.list;
   expectTrue(
    (retrieved->type == VALUE_TYPE_LIST && 
      retrieved->position.line == 1 && 
      retrieved->position.column == 1 && 
      retrieved_outer->count == 2 && 
      retrieved_outer->data[0].type == VALUE_TYPE_NUMBER &&
      retrieved_outer->data[0].as.number == 1 &&  
      retrieved_inner->data[0].type == VALUE_TYPE_NUMBER &&
      retrieved_inner->data[0].as.number == 2 &&
      retrieved_inner->data[1].type == VALUE_TYPE_STRING &&
      strncmp(retrieved_inner->data[1].as.string, string, 2) == 0
    ) != 0, "value exists in environment after destroy");

   deallocSafe(&string);
}

void errors() { 
  node_list_t *list = nullptr;
  tryAssert(listCreate(node_t, test_arena, 1), list);

  case("non-existing symbol");
  node_t sym = nSym(test_arena, "not-existent");
  tryAssert(listAppend(node_t, list, &sym));

  auto reduction = evaluate(&sym, global);
  expectEqlInt(reduction.code, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND, "with correct symbol");
}

int main(void) {
  vm_t *machine;
  tryAssert(vmCreate(VM_TEST_OPTIONS), machine);
  global = machine->global;

  tryAssert(arenaCreate((size_t)1024*64), test_arena);

  suite(atoms);
  suite(listOfElements);
  suite(functionCall);
  suite(nested);
  suite(emptyList);
  suite(allocations);
  suite(errors);

  vmDestroy(&machine);
  return report();
}
