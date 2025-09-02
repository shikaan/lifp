#include "../lifp/error.h"
#include "../lifp/parse.h"

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
  case NODE_TYPE_NUMBER:
    return self->value.number == other->value.number;
  case NODE_TYPE_SYMBOL:
    return strcmp(self->value.symbol, other->value.symbol) == 0;
  case NODE_TYPE_STRING:
    return strcmp(self->value.string, other->value.string) == 0;
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
  } cases[] = {{tNum(1), "number", nInt(1)},
               {tLit(test_arena, "test"), "symbol", nSym(test_arena, "test")},
               {tStr(test_arena, "test"), "string", nStr(test_arena, "test")},
               {tLit(test_arena, "true"), "true", nBool(true)},
               {tLit(test_arena, "false"), "false", nBool(false)},
               {tLit(test_arena, "nil"), "nil", nNil()}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list = makeTokenList(test_arena, &cases[i].input, 1);
    size_t depth = 0;
    size_t offset = 0;
    node_t *node = nullptr;
    tryAssert(parse(test_arena, list, &offset, &depth), node);
    expect(eqlNode(node, &cases[i].expected), cases[i].name,
           "Expected equal nodes");
  }
}

void unary(void) {
  node_t number = nInt(1);
  node_t boolean_true = nBool(true);
  node_t boolean_false = nBool(false);
  node_t nil = nNil();
  node_t symbol = nSym(test_arena, "sym");
  node_t string = nStr(test_arena, "str");

  token_t lparen = tParen('(');
  token_t rparen = tParen(')');

  token_t int_token = tNum(1);
  token_t int_tokens[3] = {lparen, int_token, rparen};

  token_t true_token = tLit(test_arena, "true");
  token_t true_tokens[3] = {lparen, true_token, rparen};

  token_t false_token = tLit(test_arena, "false");
  token_t false_tokens[3] = {lparen, false_token, rparen};

  token_t nil_token = tLit(test_arena, "nil");
  token_t nil_tokens[3] = {lparen, nil_token, rparen};

  token_t sym_token = tLit(test_arena, "sym");
  token_t sym_tokens[3] = {lparen, sym_token, rparen};

  token_t str_token = tStr(test_arena, "str");
  token_t str_tokens[3] = {lparen, str_token, rparen};

  struct {
    const char *name;
    size_t length;
    token_t *input;
    node_t expected;
  } cases[] = {{"number", 3, int_tokens, nList(1, (node_t *){&number})},
               {"symbol", 3, sym_tokens, nList(1, (node_t *){&symbol})},
               {"string", 3, str_tokens, nList(1, (node_t *){&string})},
               {"true", 3, true_tokens, nList(1, (node_t *){&boolean_true})},
               {"false", 3, false_tokens, nList(1, (node_t *){&boolean_false})},
               {"nil", 3, nil_tokens, nList(1, (node_t *){&nil})}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list =
        makeTokenList(test_arena, cases[i].input, cases[i].length);
    size_t depth = 0;
    size_t offset = 0;
    node_t *node = nullptr;
    tryAssert(parse(test_arena, list, &offset, &depth), node);
    expect(eqlNode(node, &cases[i].expected), cases[i].name,
           "Expected equal nodes");
  }
}

void complex(void) {
  token_t lparen = tParen('(');
  token_t rparen = tParen(')');
  token_t add_token = tLit(test_arena, "add");
  token_t int_token = tNum(1);
  token_t bool_token = tLit(test_arena, "true");

  token_t empty[2] = {lparen, rparen};
  token_t mixed[5] = {lparen, add_token, bool_token, int_token, rparen};
  token_t nested[9] = {lparen,     add_token, int_token, lparen, add_token,
                       bool_token, int_token, rparen,    rparen};

  node_t number = nInt(1);
  node_t boolean = nBool(true);
  node_t add = nSym(test_arena, "add");
  node_t mixed_nodes[3] = {add, boolean, number};
  node_t nested_nodes[3] = {add, number, nList(3, mixed_nodes)};

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
    node_t *node = nullptr;
    tryAssert(parse(test_arena, list, &offset, &depth), node);
    expect(eqlNode(node, &cases[i].expected), cases[i].name,
           "Expected equal nodes");
  }
}

void errors() {
  token_t lparen = tParen('(');
  token_t rparen = tParen(')');
  token_t number = tNum(1);
  token_t symbol = tLit(test_arena, "this_is_a_very_very_long_symbol");

  token_t unbalanced_right[4] = {lparen, lparen, number, rparen};
  token_t unbalanced_left[4] = {lparen, number, rparen, rparen};
  token_t dangling[4] = {lparen, number, rparen, number};
  token_t symbol_too_long[4] = {lparen, symbol, rparen};

  struct {
    const char *name;
    size_t length;
    token_t *input;
    int expected;
  } cases[] = {
      {"unbalanced parentheses right", 4, unbalanced_right,
       ERROR_CODE_SYNTAX_UNBALANCED_PARENTHESES},
      {"unbalanced parentheses left", 4, unbalanced_left,
       ERROR_CODE_SYNTAX_UNBALANCED_PARENTHESES},
      {"dangling symbols", 4, dangling, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN},
      {"symbol too long", 4, symbol_too_long,
       ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN},
  };

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *list =
        makeTokenList(test_arena, cases[i].input, cases[i].length);
    size_t depth = 0;
    size_t offset = 0;
    auto result = parse(test_arena, list, &offset, &depth);
    expectEqlInt(result.code, cases[i].expected, cases[i].name);
  }
}

int main(void) {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);

  suite(atoms);
  suite(unary);
  suite(complex);
  suite(errors);
  arenaDestroy(&test_arena);
  return report();
}
