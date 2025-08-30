#include "../lifp/tokenize.h"
#include "../lifp/error.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static arena_t *test_arena;

static bool tokenEql(const token_t *self, const token_t *other) {
  if (self->type != other->type) {
    return false;
  }
  switch (self->type) {
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
    return true;
  case TOKEN_TYPE_NUMBER:
    return self->value.number == other->value.number;
  case TOKEN_TYPE_SYMBOL:
    return strcmp(self->value.symbol, other->value.symbol) == 0;
  default:
    return false;
  }
}

static bool tokenListEql(const token_list_t *self, const token_list_t *other) {
  if (self->count != other->count) {
    return false;
  }
  for (size_t i = 0; i < self->count; i++) {
    token_t self_token = listGet(token_t, self, i);
    token_t other_token = listGet(token_t, other, i);
    if (!tokenEql(&self_token, &other_token)) {
      return false;
    }
  }
  return true;
}

void atoms() {
  token_t lparen_token = tParen('(');
  token_t rparen_token = tParen(')');
  token_t int_token = tNum(12);
  token_t float_token = tNum(1.2);
  token_t symbol_token = tSym(test_arena, "lol");

  struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } cases[] = {
      {.input = "(",
       .expected = makeTokenList(test_arena, &lparen_token, 1),
       .name = "lparen"},
      {.input = ")",
       .expected = makeTokenList(test_arena, &rparen_token, 1),
       .name = "rparen"},
      {.input = "12",
       .expected = makeTokenList(test_arena, &int_token, 1),
       .name = "integer"},
      {.input = "1.2",
       .expected = makeTokenList(test_arena, &float_token, 1),
       .name = "float"},
      {.input = "lol",
       .expected = makeTokenList(test_arena, &symbol_token, 1),
       .name = "symbol"},
  };

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *tokens = nullptr;
    tryAssert(tokenize(test_arena, cases[i].input), tokens);
    expect(tokenListEql(cases[i].expected, tokens), cases[i].name,
           "Expected token lists to be equal.");
  }
}

void whitespaces() {
  token_t token = tSym(test_arena, "a");
  token_t other_token = tSym(test_arena, "b");

  struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } cases[] = {
      {.input = " a",
       .expected = makeTokenList(test_arena, &token, 1),
       .name = "blank space"},
      {.input = "\ta",
       .expected = makeTokenList(test_arena, &token, 1),
       .name = "tab space"},
      {.input = "\ra",
       .expected = makeTokenList(test_arena, &token, 1),
       .name = "carriage return"},
      {.input = "\nb\r",
       .expected = makeTokenList(test_arena, &other_token, 1),
       .name = "new line"},
  };

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *tokens = nullptr;
    tryAssert(tokenize(test_arena, cases[i].input), tokens);
    expect(tokenListEql(cases[i].expected, tokens), cases[i].name,
           "Expected token lists to be equal.");
  }
}

void errors() {
  struct {
    const char *input;
    size_t column;
    error_code_t code;
    const char *name;
  } cases[] = {
      {"\a", 1, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN, "unexpected character"},
      {"a\b", 2, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
       "unexpected character with symbol"},
      {"1\b", 2, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
       "unexpected character with number"}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    auto result = tokenize(test_arena, cases[i].input);
    case(cases[i].name);
    expectEqlInt(result.code, (int)cases[i].code, "has correct error code");
    expectEqlSize(result.meta.column, cases[i].column, "has correct position");
  }
}

void complex() {
  token_t lparen_token = tParen('(');
  token_t rparen_token = tParen(')');
  token_t twelve_token = tNum(12);
  token_t two_token = tNum(2);
  token_t def_token = tSym(test_arena, "def!");
  token_t x_token = tSym(test_arena, "x");

  token_t flat_list[4] = {lparen_token, twelve_token, two_token, rparen_token};
  token_t nested_list[6] = {lparen_token, two_token,    lparen_token,
                            twelve_token, rparen_token, rparen_token};
  token_t whitespaces[5] = {lparen_token, def_token, x_token, two_token,
                            rparen_token};

  struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } cases[] = {
      {"(12 2)", makeTokenList(test_arena, flat_list, 4), "flat list"},
      {"(2 (12))", makeTokenList(test_arena, nested_list, 6), "nested list"},
      {"(def!\nx 2\n)", makeTokenList(test_arena, whitespaces, 5),
       "with random whitespaces"}};
  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *tokens = nullptr;
    tryAssert(tokenize(test_arena, cases[i].input), tokens);
    expect(tokenListEql(cases[i].expected, tokens), cases[i].name,
           "Expected token lists to be equal.");
  }
}

int main(void) {
  tryAssert(arenaCreate((size_t)(1024 * 1024)), test_arena);

  suite(atoms);
  suite(complex);
  suite(whitespaces);
  suite(errors);

  arenaDestroy(&test_arena);
  return report();
}
