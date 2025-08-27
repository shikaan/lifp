#pragma once

#include "../lib/list.h"
#include "position.h"
#include <stdint.h>

constexpr char LPAREN = '(';
constexpr char RPAREN = ')';

constexpr size_t SYMBOL_SIZE = 16;

typedef double number_t;

typedef enum {
  TOKEN_TYPE_LPAREN,
  TOKEN_TYPE_RPAREN,
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_NUMBER,
} token_type_t;

typedef union {
  char symbol[SYMBOL_SIZE];
  number_t number;
  nullptr_t lparen;
  nullptr_t rparen;
} token_value_t;

typedef struct {
  token_value_t value;
  position_t position;
  token_type_t type;
} token_t;

typedef List(token_t) token_list_t;