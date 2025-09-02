#pragma once

#include "../lib/string.h"
#include "../lifp/token.h"
#include "../lifp/value.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define arraySize(array) (sizeof(array) / sizeof((array)[0]))

#define _concat_detail(x, y) x##y
#define _concat(x, y) _concat_detail(x, y)

// Prevent unused value warnings when built in RELEASE mode
#ifdef NODEBUG
#define tryAssert(Action, ...)                                                 \
  auto _concat(result, __LINE__) = (Action);                                   \
  assert(_concat(result, __LINE__).code == RESULT_OK);                         \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__).value);
#else
#define tryAssert(Action, ...)                                                 \
  __VA_OPT__(__VA_ARGS__ =)(Action) __VA_OPT__(.value);
#endif

#ifdef NODEBUG
#define tryFail(Action, ...)                                                   \
  auto _concat(result, __LINE__) = (Action);                                   \
  assert(_concat(result, __LINE__).code != RESULT_OK);                         \
  __VA_OPT__(__VA_ARGS__ = _concat(result, __LINE__))
#else
#define tryFail(Action, ...) __VA_OPT__(__VA_ARGS__ =)(Action);
#endif

static inline token_list_t *
makeTokenList(arena_t *arena, const token_t *elements, size_t capacity) {
  token_list_t *list = nullptr;
  tryAssert(listCreate(token_t, arena, capacity), list);

  for (size_t i = 0; i < capacity; i++) {
    tryAssert(listAppend(token_t, list, &elements[i]));
  }
  return list;
}

static inline token_t tNum(double number) {
  return (token_t){
      .position = {.column = 1, .line = 1},
      .type = TOKEN_TYPE_NUMBER,
      .value = {.number = number},
  };
}

static inline token_t tLit(arena_t *arena, const char *string) {
  size_t len = strlen(string);
  string_t value;
  tryAssert(arenaAllocate(arena, len + 1), value);
  stringCopy(value, string, len + 1);
  return (token_t){
      .position = {1, 1}, .type = TOKEN_TYPE_LITERAL, .value.literal = value};
}

static inline token_t tParen(char paren) {
  auto value = paren == '(' ? (token_value_t){.lparen = nullptr}
                            : (token_value_t){.rparen = nullptr};
  token_type_t type = paren == '(' ? TOKEN_TYPE_LPAREN : TOKEN_TYPE_RPAREN;
  return (token_t){
      .position = {.column = 1, .line = 1},
      .type = type,
      .value = value,
  };
}

static inline value_t pInt(int number) {
  return (value_t){.type = VALUE_TYPE_NUMBER,
                   .position.column = 1,
                   .position.line = 1,
                   .value.number = number};
}

static inline node_t nInt(int number) {
  return (node_t){.type = NODE_TYPE_NUMBER,
                  .position.column = 1,
                  .position.line = 1,
                  .value.number = number};
}

static inline node_t nBool(bool boolean) {
  return (node_t){.type = NODE_TYPE_BOOLEAN,
                  .position.column = 1,
                  .position.line = 1,
                  .value.boolean = boolean};
}

static inline node_t nNil() {
  return (node_t){.type = NODE_TYPE_NIL,
                  .position.column = 1,
                  .position.line = 1,
                  .value.nil = nullptr};
}

static inline node_t nSym(arena_t *arena, const char *symbol) {
  size_t len = strlen(symbol);
  string_t value;
  tryAssert(arenaAllocate(arena, len + 1), value);
  stringCopy(value, symbol, len + 1);
  return (node_t){.position = {.column = 1, .line = 1},
                  .type = NODE_TYPE_SYMBOL,
                  .value.symbol = value};
}

static inline node_t nStr(arena_t *arena, const char *string) {
  size_t len = strlen(string);
  string_t value;
  tryAssert(arenaAllocate(arena, len + 1), value);
  stringCopy(value, string, len + 1);
  return (node_t){.position = {.column = 1, .line = 1},
                  .type = NODE_TYPE_STRING,
                  .value.string = value};
}

#define nList(Count, Data)                                                     \
  {                                                                            \
      .type = NODE_TYPE_LIST,                                                  \
      .position.column = 1,                                                    \
      .position.line = 1,                                                      \
      .value.list.item_size = sizeof(node_t),                                  \
      .value.list.count = (Count),                                             \
      .value.list.capacity = (Count),                                          \
      .value.list.data = (Data),                                               \
  }
