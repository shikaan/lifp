#include "tokenize.h"
#include "error.h"
#include "position.h"
#include "token.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef List(char) string_buffer_t;
typedef ResultVoid(position_t) result_void_position_t;

// All printable symbols, except for special symbols, and whitespace are valid
static int isValidSeparator(char character) {
  return (character == 0 || isspace(character) || character == RPAREN ||
          character == COMMENT_DELIMITER) != 0;
}

// All printable symbols, except for special symbols, and whitespace are valid
static int isValidSymbolChar(char character) {
  return ((isprint(character) != 0) && character != ' ' && character != '\t' &&
          character != '\n' && character != '\r' &&
          character != COMMENT_DELIMITER && character != STRING_DELIMITER &&
          character != LPAREN && character != RPAREN) != 0;
}

static int isValidSymbolName(size_t len, const char *symbol) {
  // Empty names are not allowed
  if (len == 0)
    return 0;

  // ? and ! alone are not allowed
  if (len == 1 &&
      (symbol[0] == BOOLEAN_DELIMITER || symbol[0] == EFFECTFUL_DEIMITER))
    return 0;

  // ? and ! can only appear at the end
  for (size_t i = 0; i < len - 1; i++) {
    if (symbol[i] == BOOLEAN_DELIMITER || symbol[i] == EFFECTFUL_DEIMITER) {
      return 0;
    }
  }
  return 1;
}

// Helper function to process escape sequences in strings
static char processEscapeSequence(const char **source, position_t *pos) {
  char character = **source;
  (*source)++;
  pos->column++;

  switch (character) {
  case 'n':
    return '\n';
  case 't':
    return '\t';
  case 'r':
    return '\r';
  case 'b':
    return '\b';
  case 'f':
    return '\f';
  case 'v':
    return '\v';
  case 'a':
    return '\a';
  case '0':
    return '\0';
  case '\\':
    return '\\';
  case '"':
    return '"';
  default:
    // For unrecognized escapes, return the character as-is
    return character;
  }
}

static void skipWhitespaceAndComments(const char **source, position_t *pos) {
  while (**source) {
    if (isspace(**source)) {
      if (**source == '\n') {
        pos->line++;
        pos->column = 1;
      } else {
        pos->column++;
      }
      (*source)++;
    } else if (**source == COMMENT_DELIMITER) {
      // Skip comment until end of line
      while (**source && **source != '\n') {
        (*source)++;
        pos->column++;
      }
      // Don't increment here, let the next iteration handle the newline
    } else {
      break;
    }
  }
}

result_token_list_ref_t tokenize(arena_t *arena, const char *source) {
  position_t pos = {.line = 1, .column = 1};

  token_list_t *tokens = nullptr;
  tryWithMeta(result_token_list_ref_t, listCreate(token_t, arena, 16), pos,
              tokens);

  const char *current = source;

  const char null = '\0';

  while (*current) {
    skipWhitespaceAndComments(&current, &pos);
    if (!*current)
      break;

    position_t token_pos = pos;
    token_t token = {.position = token_pos};

    char current_char = *current;

    if (current_char == LPAREN) {
      token.type = TOKEN_TYPE_LPAREN;
      token.value.lparen = nullptr;
      current++;
      pos.column++;
      goto append_and_continue;
    }

    if (current_char == RPAREN) {
      token.type = TOKEN_TYPE_RPAREN;
      token.value.rparen = nullptr;
      current++;
      pos.column++;
      goto append_and_continue;
    }

    if (current_char == STRING_DELIMITER) {
      current++; // Skip opening quote
      pos.column++;

      string_buffer_t *str = nullptr;
      tryWithMeta(result_token_list_ref_t, listCreate(char, arena, 16), pos,
                  str);

      while (*current && *current != STRING_DELIMITER) {
        char string_char;
        if (*current == '\\') {
          current++; // Skip backslash
          pos.column++;

          if (!*current) {
            throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                  pos, "Unterminated string escape sequence");
          }

          string_char = processEscapeSequence(&current, &pos);
        } else {
          string_char = *current;
          current++;
          if (string_char == '\n') {
            pos.line++;
            pos.column = 1;
          } else {
            pos.column++;
          }
        }

        tryWithMeta(result_token_list_ref_t,
                    listAppend(char, str, &string_char), pos);
      }

      if (!*current) {
        throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
              token_pos, "Incomplete string literal");
      }

      // After closing string, check if next character is a valid separator
      // We don't want to allow '"' in the middle of a literal
      char next_char = *(current + 1);
      if (!isValidSeparator(next_char)) {
        pos.column++;
        throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN, pos,
              "Invalid string literal");
      }

      current++; // Skip closing quote
      pos.column++;

      // Null-terminate the string
      tryWithMeta(result_token_list_ref_t, listAppend(char, str, &null), pos);

      token.type = TOKEN_TYPE_STRING;
      token.value.string = str->data;
      goto append_and_continue;
    }

    if (isValidSymbolChar(current_char)) {
      string_buffer_t *str = nullptr;
      tryWithMeta(result_token_list_ref_t, listCreate(char, arena, 16), pos,
                  str);

      while (*current != '\0' && isValidSymbolChar(*current)) {
        tryWithMeta(result_token_list_ref_t, listAppend(char, str, current),
                    pos);
        current++;
        pos.column++;
      }

      // Null-terminate
      tryWithMeta(result_token_list_ref_t, listAppend(char, str, &null), pos);

      if (!isValidSymbolName(str->count - 1, str->data)) {
        throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
              token_pos, "Invalid symbol name");
      }

      char *remainder;
      number_t number = (number_t)strtod(str->data, &remainder);

      // This condition is met when all the chars of the token represent an
      // number. This includes also leading +/- and scientific notation
      const bool is_number = ((!remainder) || (strlen(remainder) == 0)) != 0;
      if (is_number) {
        token.type = TOKEN_TYPE_NUMBER;
        token.value.number = number;
        goto append_and_continue;
      }

      // Else, default to literal
      token.type = TOKEN_TYPE_LITERAL;
      token.value.literal = str->data;
      goto append_and_continue;
    }

    throw(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN, pos,
          "Unexpected character(s): \"%c\"", current_char);

  append_and_continue:
    tryWithMeta(result_token_list_ref_t, listAppend(token_t, tokens, &token),
                pos);
  }

  return ok(result_token_list_ref_t, tokens);
}
