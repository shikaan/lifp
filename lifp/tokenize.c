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

result_void_position_t bufferToToken(token_t *token, string_buffer_t *buffer,
                                     position_t position, bool is_string) {
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

  if (is_string) {
    char *string = nullptr;
    tryWithMeta(result_void_position_t,
                arenaAllocate(buffer->arena, buffer->count - 2), position,
                string);
    stringCopy(string, buffer->data + 1, buffer->count - 2);
    token->position = position;
    token->type = TOKEN_TYPE_STRING;
    token->value.string = string;
  } else {
    char *literal = nullptr;
    tryWithMeta(result_void_position_t,
                arenaAllocate(buffer->arena, buffer->count), position, literal);
    stringCopy(literal, buffer->data, buffer->count);
    token->position = position;
    token->type = TOKEN_TYPE_LITERAL;
    token->value.literal = literal;
  }
  return ok(result_void_position_t);
}

result_token_list_ref_t tokenize(arena_t *arena, const char *source) {
  position_t current_char_pos = {1, 0};
  position_t curr_token_pos = {0, 0};

  token_list_t *tokens = nullptr;
  tryWithMeta(result_token_list_ref_t, listCreate(token_t, arena, 32),
              current_char_pos, tokens);
  string_buffer_t *buffer = nullptr;
  tryWithMeta(result_token_list_ref_t, listCreate(char, arena, 64),
              current_char_pos, buffer);

  bool is_tokenizing_string = false;

  token_t token;
  for (int i = 0; source[i] != '\0'; i++) {
    current_char_pos.column++;
    const char current_char = source[i];

    const bool should_collect =
        ((int)is_tokenizing_string ||
         (isprint(current_char) && current_char != ' ')) != 0;

    if (current_char == LPAREN) {
      token.type = TOKEN_TYPE_LPAREN;
      token.value.lparen = nullptr;
      token.position = current_char_pos;
      tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                  curr_token_pos);
    } else if (current_char == RPAREN) {
      if (buffer->count > 0) {
        tryFinally(
            result_token_list_ref_t,
            bufferToToken(&token, buffer, curr_token_pos, is_tokenizing_string),
            { is_tokenizing_string = false; });
        tryWithMeta(result_token_list_ref_t,
                    listAppend(token_t, tokens, &token), curr_token_pos);
        listClear(char, buffer);
      }

      token.type = TOKEN_TYPE_RPAREN;
      token.value.rparen = nullptr;
      token.position = current_char_pos;
      tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                  curr_token_pos);
    } else if (should_collect) {
      if (buffer->count == 0) {
        curr_token_pos.line = current_char_pos.line;
        curr_token_pos.column = current_char_pos.column;
        is_tokenizing_string = current_char == STRING_DELIMITER;
      }
      tryWithMeta(result_token_list_ref_t,
                  listAppend(char, buffer, &current_char), curr_token_pos);
      continue;
    } else if (isspace(current_char)) {
      if (current_char == '\n') {
        current_char_pos.line++;
        current_char_pos.column = 0;
      }

      if (buffer->count == 0)
        continue;

      tryFinally(
          result_token_list_ref_t,
          bufferToToken(&token, buffer, curr_token_pos, is_tokenizing_string),
          { is_tokenizing_string = false; });
      tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                  curr_token_pos);
      listClear(char, buffer);
    } else if (isprint(current_char)) {
      if (buffer->count == 0) {
        curr_token_pos.line = current_char_pos.line;
        curr_token_pos.column = current_char_pos.column;
        is_tokenizing_string = current_char == STRING_DELIMITER;
      }
      tryWithMeta(result_token_list_ref_t,
                  listAppend(char, buffer, &current_char), curr_token_pos);
      continue;
    } else {
      throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
            current_char_pos, "Unexpected token '%c'", current_char);
    }
  }

  if (buffer->count > 0) {
    try(result_token_list_ref_t,
        bufferToToken(&token, buffer, curr_token_pos, is_tokenizing_string));
    tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                curr_token_pos);
  }

  return ok(result_token_list_ref_t, tokens);
}
