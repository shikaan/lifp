#include "../lifp/tokenize.h"
#include "../lifp/error.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static arena_t *test_arena;

static void printToken(const token_t *token) {
  switch (token->type) {
  case TOKEN_TYPE_LPAREN:
    printf("LPAREN ");
    break;
  case TOKEN_TYPE_RPAREN:
    printf("RPAREN ");
    break;
  case TOKEN_TYPE_NUMBER:
    printf("NUMBER(%g) ", token->value.number);
    break;
  case TOKEN_TYPE_STRING:
    printf("STRING(\"%s\") ", token->value.string);
    break;
  case TOKEN_TYPE_SYMBOL:
    printf("SYMBOL(%s) ", token->value.symbol);
    break;
  default:
    printf("UNKNOWN ");
    break;
  }
}

static void printTokenList(const token_list_t *list) {
  printf("[ ");
  for (size_t i = 0; i < list->count; i++) {
    token_t token = listGet(token_t, list, i);
    printToken(&token);
  }
  printf("]\n");
}

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
  case TOKEN_TYPE_STRING:
    return strcmp(self->value.string, other->value.string) == 0;
  case TOKEN_TYPE_SYMBOL:
    return strcmp(self->value.symbol, other->value.symbol) == 0;
  default:
    return false;
  }
}

static bool tokenListEql(const token_list_t *self, const token_list_t *other) {
  if (self->count != other->count) {
    printTokenList(self);
    printTokenList(other);
    return false;
  }
  for (size_t i = 0; i < self->count; i++) {
    token_t self_token = listGet(token_t, self, i);
    token_t other_token = listGet(token_t, other, i);
    if (!tokenEql(&self_token, &other_token)) {
      printTokenList(self);
      printTokenList(other);
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
  token_t bool_symbol_token = tSym(test_arena, "lol?");
  token_t effectful_symbol_token = tSym(test_arena, "lol!");
  token_t string_token = tStr(test_arena, "str ing");

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
      {.input = "lol?",
       .expected = makeTokenList(test_arena, &bool_symbol_token, 1),
       .name = "boolean symbol"},
      {.input = "lol!",
       .expected = makeTokenList(test_arena, &effectful_symbol_token, 1),
       .name = "effectful symbol"},
      {.input = "\"str ing\"",
       .expected = makeTokenList(test_arena, &string_token, 1),
       .name = "string"},
      {.input = "; comment",
       .expected = makeTokenList(test_arena, nullptr, 0),
       .name = "comments"}};

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
  token_t line_breaks = tStr(test_arena, "a\nb");

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
      {.input = "\"a\nb\"",
       .expected = makeTokenList(test_arena, &line_breaks, 1),
       .name = "string with line breaks"},
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
    const char *error;
  } cases[] = {{"\a", 1, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "unexpected character", "Unexpected character"},
               {"a\b", 2, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "unexpected character with symbol", "Unexpected character"},
               {"1\b", 2, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "unexpected character with number", "Unexpected character"},
               {"\"a\"1", 4, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "no space after string", "Invalid string"},
               {"a\"", 2, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "incomplete string (end quotes)", "Incomplete string"},
               {"\"a", 1, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "incomplete string (start quotes)", "Incomplete string"},
               {"a!sd", 1, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "effectful mid-token", "Invalid symbol"},
               {"a?sd", 1, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                "boolean mid-token", "Invalid symbol"}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    auto result = tokenize(test_arena, cases[i].input);
    case(cases[i].name);
    expectEqlInt(result.code, (int)cases[i].code, "has correct error code");
    expectEqlSize(result.meta.column, cases[i].column, "has correct position");
    expectIncludeString(result.message, cases[i].error, "has correct error");
  }
}

void complex() {
  token_t lparen_token = tParen('(');
  token_t rparen_token = tParen(')');
  token_t twelve_token = tNum(12);
  token_t two_token = tNum(2);
  token_t def_token = tSym(test_arena, "def!");
  token_t x_token = tSym(test_arena, "x");
  token_t str_token = tStr(test_arena, "lol");

  token_t flat_list[4] = {lparen_token, twelve_token, two_token, rparen_token};
  token_t nested_list[6] = {lparen_token, two_token,    lparen_token,
                            twelve_token, rparen_token, rparen_token};
  token_t whitespaces[5] = {lparen_token, def_token, x_token, two_token,
                            rparen_token};
  token_t mixed[4] = {lparen_token, str_token, twelve_token,
                            rparen_token};

  struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } cases[] = {
      {"(12 2)", makeTokenList(test_arena, flat_list, 4), "flat list"},
      {"(2 (12))", makeTokenList(test_arena, nested_list, 6), "nested list"},
      {"; hello\n(12 2)", makeTokenList(test_arena, flat_list, 4), "form with comments"},
      {"(def!\nx 2\n)", makeTokenList(test_arena, whitespaces, 5),
       "with random whitespaces"},
       {"(\"lol\" 12)", makeTokenList(test_arena, mixed, 4),
       "mixed list"},
      };
  for (size_t i = 0; i < arraySize(cases); i++) {
    token_list_t *tokens = nullptr;
    tryAssert(tokenize(test_arena, cases[i].input), tokens);
    expect(tokenListEql(cases[i].expected, tokens), cases[i].name,
           "Expected token lists to be equal.");
  }
}

void escape() {
  token_t quotes = tStr(test_arena, "hello \"world\"");
  token_t new_line = tStr(test_arena, "hello \n world");
  token_t tab = tStr(test_arena, "hello \t world");
  token_t ret = tStr(test_arena, "hello \r world");
  token_t quote = tStr(test_arena, "hello ' world");
  token_t null = tStr(test_arena, "hello \0 world");

  struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } cases[] = {
      {.input = "\"hello \\\"world\\\"\"",
       .expected = makeTokenList(test_arena, &quotes, 1),
       .name = "escaped quotes"},
      {.input = "\"hello \\n world\"",
       .expected = makeTokenList(test_arena, &new_line, 1),
       .name = "escaped new line"},
      {.input = "\"hello \\t world\"",
       .expected = makeTokenList(test_arena, &tab, 1),
       .name = "escaped tab"},
      {.input = "\"hello \\r world\"",
       .expected = makeTokenList(test_arena, &ret, 1),
       .name = "escaped return"},
      {.input = "\"hello ' world\"",
       .expected = makeTokenList(test_arena, &quote, 1),
       .name = "unescaped single quotes"},
      {.input = "\"hello \\0 world\"",
       .expected = makeTokenList(test_arena, &null, 1),
       .name = "escaped null-byte"}

  };

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
  suite(whitespaces);
  suite(complex);
  suite(errors);
  suite(escape);

  arenaDestroy(&test_arena);
  return report();
}
