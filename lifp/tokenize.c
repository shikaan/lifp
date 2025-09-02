#include "tokenize.h"
#include "../lib/string.h"
#include "error.h"
#include "position.h"
#include "token.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef List(char) string_buffer_t;
typedef ResultVoid(position_t) result_void_position_t;

typedef enum {
  LEXER_STATE_DEFAULT,
  LEXER_STATE_COLLECTING_STRING
} lexer_state_t;

typedef struct {
  lexer_state_t state;
  position_t token_position;
  string_buffer_t *buffer;
  token_list_t *tokens;
} lexer_t;

static result_void_t lexerInit(lexer_t *self, arena_t *arena) {
  self->state = LEXER_STATE_DEFAULT;
  try(result_void_t, listCreate(char, arena, 64), self->buffer);
  try(result_void_t, listCreate(token_t, arena, 32), self->tokens);
  return ok(result_void_t);
}

static result_void_position_t lexerDecode(lexer_t *self, token_type_t type) {
  token_t token;
  token.position = self->token_position;

#define resetAndAppendResult()                                                 \
  self->state = LEXER_STATE_DEFAULT;                                           \
  listClear(char, self->buffer);                                               \
  tryWithMeta(result_void_position_t,                                          \
              listAppend(token_t, self->tokens, &token),                       \
              self->token_position);

  if (type == TOKEN_TYPE_LPAREN || type == TOKEN_TYPE_RPAREN) {
    token.type = type;
    token.value.lparen = nullptr;
    token.value.rparen = nullptr;

    resetAndAppendResult();
    return ok(result_void_position_t);
  }

  string_buffer_t *buffer = self->buffer;
  char null = 0;
  tryWithMeta(result_void_position_t, listAppend(char, buffer, &null),
              self->token_position);

  char *remainder;
  number_t number = (number_t)strtod(buffer->data, &remainder);

  // This condition is met when all the chars of the token represent an number
  // This includes also leading +/-
  const bool is_number = ((!remainder) || (strlen(remainder) == 0)) != 0;
  if (is_number) {
    token.type = TOKEN_TYPE_NUMBER;
    token.value.number = number;
    resetAndAppendResult();
    return ok(result_void_position_t);
  }

  if (self->state == LEXER_STATE_COLLECTING_STRING) {
    char *string = nullptr;
    tryWithMeta(result_void_position_t,
                arenaAllocate(buffer->arena, buffer->count - 2),
                self->token_position, string);
    stringCopy(string, buffer->data + 1, buffer->count - 2);
    token.type = TOKEN_TYPE_STRING;
    token.value.string = string;
  } else {
    char *literal = nullptr;
    tryWithMeta(result_void_position_t,
                arenaAllocate(buffer->arena, buffer->count),
                self->token_position, literal);
    stringCopy(literal, buffer->data, buffer->count);
    token.type = TOKEN_TYPE_LITERAL;
    token.value.literal = literal;
  }

  resetAndAppendResult();
  return ok(result_void_position_t);
#undef resetAndAppendResult
}

static bool lexerIsEmpty(lexer_t *self) { return self->buffer->count == 0; }

static result_void_position_t lexerAppend(lexer_t *self, char character) {
  tryWithMeta(result_void_position_t,
              listAppend(char, self->buffer, &character), self->token_position);
  return ok(result_void_position_t);
}

result_token_list_ref_t tokenize(arena_t *arena, const char *source) {
  position_t cursor = {1, 0};

  lexer_t lexer;
  tryWithMeta(result_token_list_ref_t, lexerInit(&lexer, arena), cursor);

  for (int i = 0; source[i] != '\0'; i++) {
    cursor.column++;
    const char current_char = source[i];

    if (current_char == LPAREN) {
      try(result_token_list_ref_t, lexerDecode(&lexer, TOKEN_TYPE_LPAREN));
      continue;
    }

    if (current_char == RPAREN) {
      if (lexer.buffer->count > 0) {
        try(result_token_list_ref_t, lexerDecode(&lexer, TOKEN_TYPE_STRING));
      }

      try(result_token_list_ref_t, lexerDecode(&lexer, TOKEN_TYPE_RPAREN));
      continue;
    }

    const bool should_collect =
        ((int)lexer.state == LEXER_STATE_COLLECTING_STRING ||
         (isprint(current_char) && current_char != ' ')) != 0;

    if (should_collect) {
      if (lexerIsEmpty(&lexer)) {
        lexer.token_position = cursor;
        lexer.state = current_char == STRING_DELIMITER
                          ? LEXER_STATE_COLLECTING_STRING
                          : LEXER_STATE_DEFAULT;
      }

      try(result_token_list_ref_t, lexerAppend(&lexer, current_char));
      continue;
    }

    if (isspace(current_char)) {
      if (current_char == '\n') {
        cursor.line++;
        cursor.column = 0;
      }

      if (lexerIsEmpty(&lexer))
        continue;

      try(result_token_list_ref_t, lexerDecode(&lexer, TOKEN_TYPE_STRING));
      continue;
    }

    throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN, cursor,
          "Unexpected token '%c'", current_char);
  }

  if (!lexerIsEmpty(&lexer)) {
    try(result_token_list_ref_t, lexerDecode(&lexer, TOKEN_TYPE_STRING));
  }

  return ok(result_token_list_ref_t, lexer.tokens);
}
