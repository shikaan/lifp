#include "../lifp/parser.h"

#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static arena_t *test_arena;

static bool eqlNode(node_t *self, node_t *other) {
  // Purposefully leaving out the position check
  if (self->type != other->type) {
    return false;
  }

  switch (self->type) {
  case NODE_TYPE_NIL:
    return true;
  case NODE_TYPE_BOOLEAN:
    return self->value.boolean == other->value.boolean;
  case NODE_TYPE_INTEGER:
    return self->value.integer == other->value.integer;
  case NODE_TYPE_SYMBOL:
    return strncmp(self->value.symbol, other->value.symbol, SYMBOL_SIZE) == 0;
  case NODE_TYPE_LIST: {
    if (self->value.list.count != other->value.list.count) {
      return false;
    }

    for (size_t i = 0; i < self->value.list.count; i++) {
      node_t self_node = listGet(node_t, &self->value.list, i);
      node_t other_node = listGet(node_t, &other->value.list, i);
      if (!eqlNode(&self_node, &other_node)) {
        return false;
      }
    }
    return true;
  }
  default:
    return false;
  }
}

void atoms(void) {
  struct {
    token_t input;
    const char *name;
    node_t expected;
  } cases[] = {{
                   tInt(1),
                   "integer",
                   (node_t){.type = NODE_TYPE_INTEGER, .value.integer = 1},
               },
               {
                   tSym("test"),
                   "symbol",
                   (node_t){.type = NODE_TYPE_SYMBOL, .value.symbol = "test"},
               },
               {
                   tSym("true"),
                   "true",
                   (node_t){.type = NODE_TYPE_BOOLEAN, .value.boolean = true},
               },
               {tSym("false"), "false",
                (node_t){.type = NODE_TYPE_BOOLEAN, .value.boolean = false}},
               {
                   tSym("nil"),
                   "nil",
                   (node_t){.type = NODE_TYPE_NIL, .value.nil = nullptr},
               }};

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list = makeTokenList(test_arena, &cases[i].input, 1);
    size_t depth = 0;
    size_t offset = 0;
    auto result = parse(test_arena, list, &offset, &depth);
    assert(result.ok);
    expect(eqlNode(result.value, &cases[i].expected), cases[i].name,
           "Expected equal nodes");
  }
}

void unary(void) {
  node_t integer = nInt(1);
  node_t boolean_true = nBool(true);
  node_t boolean_false = nBool(false);
  node_t nil = nNil();
  node_t symbol = nSym("sym");

  token_t lparen = tParen('(');
  token_t rparen = tParen(')');

  token_t int_token = tInt(1);
  token_t int_tokens[3] = {lparen, int_token, rparen};

  token_t true_token = tSym("true");
  token_t true_tokens[3] = {lparen, true_token, rparen};

  token_t false_token = tSym("false");
  token_t false_tokens[3] = {lparen, false_token, rparen};

  token_t nil_token = tSym("nil");
  token_t nil_tokens[3] = {lparen, nil_token, rparen};

  token_t sym_token = tSym("sym");
  token_t sym_tokens[3] = {lparen, sym_token, rparen};

  struct {
    const char *name;
    size_t length;
    token_t *input;
    node_t expected;
  } cases[] = {{"integer", 3, int_tokens, nList(1, (node_t *){&integer})},
               {"symbol", 3, sym_tokens, nList(1, (node_t *){&symbol})},
               {"true", 3, true_tokens, nList(1, (node_t *){&boolean_true})},
               {"false", 3, false_tokens, nList(1, (node_t *){&boolean_false})},
               {"nil", 3, nil_tokens, nList(1, (node_t *){&nil})}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list =
        makeTokenList(test_arena, cases[i].input, cases[i].length);
    size_t depth = 0;
    size_t offset = 0;
    auto result = parse(test_arena, list, &offset, &depth);
    assert(result.ok);
    expect(eqlNode(result.value, &cases[i].expected), cases[i].name,
           "Expected equal nodes");
  }
}

void complex(void) {
  token_t lparen = tParen('(');
  token_t rparen = tParen(')');
  token_t add_token = tSym("add");
  token_t int_token = tInt(1);
  token_t bool_token = tSym("true");

  token_t empty[2] = {lparen, rparen};
  token_t mixed[5] = {lparen, add_token, bool_token, int_token, rparen};
  token_t nested[9] = {lparen,     add_token, int_token, lparen, add_token,
                       bool_token, int_token, rparen,    rparen};

  node_t integer = nInt(1);
  node_t boolean = nBool(true);
  node_t add = nSym("add");
  node_t mixed_nodes[3] = {add, boolean, integer};
  node_t nested_nodes[3] = {add, integer, nList(3, mixed_nodes)};

  struct {
    const char *name;
    size_t length;
    token_t *input;
    node_t expected;
  } cases[] = {{"empty", 2, empty, nList(0, nullptr)},
               {"mixed", 5, mixed, nList(3, mixed_nodes)},
               {"nested", 9, nested, nList(3, nested_nodes)}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list =
        makeTokenList(test_arena, cases[i].input, cases[i].length);
    size_t depth = 0;
    size_t offset = 0;
    auto result = parse(test_arena, list, &offset, &depth);
    assert(result.ok);
    expect(eqlNode(result.value, &cases[i].expected), cases[i].name,
           "Expected equal nodes");
  }
}

void errors() {
  token_t lparen = tParen('(');
  token_t rparen = tParen(')');
  token_t integer = tInt(1);

  token_t unbalanced_right[4] = {lparen, lparen, integer, rparen};
  token_t unbalanced_left[4] = {lparen, integer, rparen, rparen};

  struct {
    const char *name;
    size_t length;
    token_t *input;
    error_kind_t expected;
  } cases[] = {
      {"no tokens", 0, {}, ERROR_KIND_INVALID_EXPRESSION},
      {"unbalanced parentheses right", 4, unbalanced_right,
       ERROR_KIND_UNBALANCED_PARENTHESES},
      {"unbalanced parentheses left", 4, unbalanced_left,
       ERROR_KIND_INVALID_EXPRESSION},
  };

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list =
        makeTokenList(test_arena, cases[i].input, cases[i].length);
    size_t depth = 0;
    size_t offset = 0;
    auto result = parse(test_arena, list, &offset, &depth);
    assert(!result.ok);
    expectEqlUint(result.error.kind, cases[i].expected,
                  "returns correct exception");
  }
}

int main(void) {
  result_ref_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.ok);
  test_arena = allocation.value;

  suite(atoms);
  suite(unary);
  suite(complex);
  suite(errors);
  arenaDestroy(test_arena);
  return report();
}
