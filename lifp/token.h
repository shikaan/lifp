#pragma once

#include "../lib/list.h"
#include "position.h"
#include "types.h"
#include <stdint.h>

constexpr char LPAREN = '(';
constexpr char RPAREN = ')';
constexpr char STRING_DELIMITER = '"';
constexpr char COMMENT_DELIMITER = ';';
constexpr char BOOLEAN_DELIMITER = '?';
constexpr char EFFECTFUL_DEIMITER = '!';

typedef enum {
  TOKEN_TYPE_LPAREN,
  TOKEN_TYPE_RPAREN,
  TOKEN_TYPE_LITERAL,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_NUMBER,
} token_type_t;

typedef union {
  string_t literal;
  string_t string;
  number_t number;
  nullptr_t lparen;
  nullptr_t rparen;
} token_value_t;

typedef struct {
  position_t position;
  token_type_t type;
  token_value_t value;
} token_t;

typedef List(token_t) token_list_t;
