#include "../lifp/lexer.h"
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
  case TOKEN_TYPE_INTEGER:
    return self->value.integer == other->value.integer;
  case TOKEN_TYPE_SYMBOL:
    return strncmp(self->value.symbol, other->value.symbol, SYMBOL_SIZE) == 0;
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
  token_t integer_token = tInt(12);
  token_t symbol_token = tSym("lol");

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
       .expected = makeTokenList(test_arena, &integer_token, 1),
       .name = "integer"},
      {.input = "lol",
       .expected = makeTokenList(test_arena, &symbol_token, 1),
       .name = "symbol"},
  };

  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *tokens = nullptr;
    tryAssertAssign(tokenize(test_arena, cases[i].input), tokens);
    expect(tokenListEql(cases[i].expected, tokens), cases[i].name,
           "Expected token lists to be equal.");
  }
}

void whitespaces() {
  token_t token = {.type = TOKEN_TYPE_SYMBOL,
                   .value = {.symbol = {'a'}},
                   .position = {.column = 2, .line = 1}};
  token_t other_token = {.type = TOKEN_TYPE_SYMBOL,
                         .value = {.symbol = {'b'}},
                         .position = {.column = 1, .line = 2}};

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
    tryAssertAssign(tokenize(test_arena, cases[i].input), tokens);
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
       "unexpected character with integer"},
      {"symbol_way_too_long", 1, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
       "symbol too long"}};

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
  token_t twelve_token = tInt(12);
  token_t two_token = tInt(2);
  token_t def_token = tSym("def!");
  token_t x_token = tSym("x");

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
    tryAssertAssign(tokenize(test_arena, cases[i].input), tokens);
    expect(tokenListEql(cases[i].expected, tokens), cases[i].name,
           "Expected token lists to be equal.");
  }
}

int main(void) {
  tryAssertAssign(arenaCreate((size_t)(1024 * 1024)), test_arena);

  suite(atoms);
  suite(complex);
  suite(whitespaces);
  suite(errors);

  arenaDestroy(test_arena);
  return report();
}
