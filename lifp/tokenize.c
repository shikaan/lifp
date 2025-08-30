#include "tokenize.h"
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

result_void_position_t bufferToToken(token_t *token, string_buffer_t *buffer,
                                     position_t position) {
  char null = 0;
  tryWithMeta(result_void_position_t, listAppend(char, buffer, &null),
              position);

  char *remainder;
  number_t number = (number_t)strtod(buffer->data, &remainder);

  // This condition is met when all the chars of the token represent an number
  // This includes also leading +/-
  const bool is_number = ((!remainder) || (strlen(remainder) == 0)) != 0;
  if (is_number) {
    token->type = TOKEN_TYPE_NUMBER;
    token->value.number = number;
    token->position = position;
    return ok(result_void_position_t);
  }

  // else, it's just a symbol
  char *string = nullptr;
  tryWithMeta(result_void_position_t,
              arenaAllocate(buffer->arena, buffer->count), position, string);
  strlcpy(string, buffer->data, buffer->count);
  token->type = TOKEN_TYPE_SYMBOL;
  token->position = position;
  token->value.symbol = string;
  return ok(result_void_position_t);
}

result_token_list_ref_t tokenize(arena_t *arena, const char *source) {
  position_t curr_char = {1, 0};
  position_t curr_token = {0, 0};

  token_list_t *tokens = nullptr;
  tryWithMeta(result_token_list_ref_t, listCreate(token_t, arena, 32),
              curr_char, tokens);
  string_buffer_t *buffer = nullptr;
  tryWithMeta(result_token_list_ref_t, listCreate(char, arena, 64), curr_char,
              buffer);

  token_t token;
  for (int i = 0; source[i] != '\0'; i++) {
    curr_char.column++;
    const char current_char = source[i];

    if (current_char == LPAREN) {
      token.type = TOKEN_TYPE_LPAREN;
      token.value.lparen = nullptr;
      token.position = curr_char;
      tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                  curr_token);
    } else if (current_char == RPAREN) {
      if (buffer->count > 0) {
        try(result_token_list_ref_t, bufferToToken(&token, buffer, curr_token));
        tryWithMeta(result_token_list_ref_t,
                    listAppend(token_t, tokens, &token), curr_token);
        listClear(char, buffer);
      }

      token.type = TOKEN_TYPE_RPAREN;
      token.value.rparen = nullptr;
      token.position = curr_char;
      tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                  curr_token);
    } else if (isspace(current_char)) {
      if (current_char == '\n') {
        curr_char.line++;
        curr_char.column = 0;
      }

      if (buffer->count == 0)
        continue;

      try(result_token_list_ref_t, bufferToToken(&token, buffer, curr_token));
      tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                  curr_token);
      listClear(char, buffer);
    } else if (isprint(current_char)) {
      if (buffer->count == 0) {
        curr_token.line = curr_char.line;
        curr_token.column = curr_char.column;
      }
      tryWithMeta(result_token_list_ref_t,
                  listAppend(char, buffer, &current_char), curr_token);
      continue;
    } else {
      throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
            curr_char, "Unexpected token '%c'", current_char);
    }
  }

  if (buffer->count > 0) {
    try(result_token_list_ref_t, bufferToToken(&token, buffer, curr_token));
    tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                curr_token);
  }

  return ok(result_token_list_ref_t, tokens);
}
