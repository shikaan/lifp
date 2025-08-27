#pragma once

#include "../lifp/value.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define arraySize(array) (sizeof(array) / sizeof((array)[0]))

#define _concat_detail(x, y) x##y
#define _concat(x, y) _concat_detail(x, y)

#define tryAssertAssign(Action, Destination)                                   \
  auto _concat(result, __LINE__) = (Action);                                   \
  assert(_concat(result, __LINE__).code == RESULT_OK);                         \
  (Destination) = (_concat(result, __LINE__).value);

// Prevent unused value warnings when built in RELEASE mode
#ifdef NODEBUG
#define tryAssert(Action)                                                      \
  auto _concat(result, __LINE__) = (Action);                                   \
  assert(_concat(result, __LINE__).code == RESULT_OK);
#else
#define tryAssert(Action) (void)(Action);
#endif

#ifndef NODEBUG
#define tryFail(Action, ...)                                                   \
  auto _concat(result, __LINE__) = (Action);                                   \
  assert(_concat(result, __LINE__).code != RESULT_OK);                         \
  __VA_OPT__((__VA_ARGS__) = _concat(result, __LINE__))
#else
#define tryFail(Action) (void)(Action);
#endif

static inline token_list_t *
makeTokenList(arena_t *arena, const token_t *elements, size_t capacity) {
  token_list_t *list = nullptr;
  tryAssertAssign(listCreate(token_t, arena, capacity), list);

  for (size_t i = 0; i < capacity; i++) {
    tryAssert(listAppend(token_t, list, &elements[i]));
  }
  return list;
}

static inline token_t tInt(int number) {
  return (token_t){
      .position = {.column = 1, .line = 1},
      .type = TOKEN_TYPE_NUMBER,
      .value = {.number = number},
  };
}

static inline token_t tSym(const char symbol[SYMBOL_SIZE]) {
  const size_t len = strlen(symbol);
  token_value_t value = {};
  memcpy(value.symbol, symbol, len);

  return (token_t){.position = {.column = 1, .line = 1},
                   .type = TOKEN_TYPE_SYMBOL,
                   .value = value};
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

static inline node_t nSym(const char symbol[]) {
  size_t len = strlen(symbol);
  return (node_t){.type = NODE_TYPE_SYMBOL,
                  .position.column = 1,
                  .position.line = 1,
                  .value.symbol[0] = (char)(len > 0 ? symbol[0] : '\0'),
                  .value.symbol[1] = (char)(len > 1 ? symbol[1] : '\0'),
                  .value.symbol[2] = (char)(len > 2 ? symbol[2] : '\0'),
                  .value.symbol[3] = (char)(len > 3 ? symbol[3] : '\0'),
                  .value.symbol[4] = (char)(len > 4 ? symbol[4] : '\0'),
                  .value.symbol[5] = (char)(len > 5 ? symbol[5] : '\0'),
                  .value.symbol[6] = (char)(len > 6 ? symbol[6] : '\0'),
                  .value.symbol[7] = (char)(len > 7 ? symbol[7] : '\0'),
                  .value.symbol[8] = (char)(len > 8 ? symbol[8] : '\0'),
                  .value.symbol[9] = (char)(len > 9 ? symbol[9] : '\0'),
                  .value.symbol[10] = (char)(len > 10 ? symbol[10] : '\0'),
                  .value.symbol[11] = (char)(len > 11 ? symbol[11] : '\0'),
                  .value.symbol[12] = (char)(len > 12 ? symbol[12] : '\0'),
                  .value.symbol[13] = (char)(len > 13 ? symbol[13] : '\0'),
                  .value.symbol[14] = (char)(len > 14 ? symbol[14] : '\0'),
                  .value.symbol[15] = (char)(len > 15 ? symbol[15] : '\0')};
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
