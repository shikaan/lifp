#include "parse.h"
#include "../lib/string.h"
#include "error.h"
#include "node.h"
#include "token.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

result_node_ref_t parseAtom(arena_t *arena, token_t token) {
  assert(token.type == TOKEN_TYPE_NUMBER || token.type == TOKEN_TYPE_LITERAL ||
         token.type == TOKEN_TYPE_STRING);

  node_t *node = nullptr;
  tryWithMeta(result_node_ref_t, nodeCreate(arena, NODE_TYPE_NUMBER),
              token.position, node);
  tryWithMeta(result_node_ref_t, nodeInit(node, arena), token.position);

  node->position = token.position;

  switch (token.type) {
  case TOKEN_TYPE_NUMBER: {
    node->type = NODE_TYPE_NUMBER;
    node->value.number = token.value.number;
    return ok(result_node_ref_t, node);
  }
  case TOKEN_TYPE_LITERAL: {
    if (strncmp(token.value.literal, TRUE, 4) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = true;
      return ok(result_node_ref_t, node);
    }

    if (strncmp(token.value.literal, FALSE, 5) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = false;
      return ok(result_node_ref_t, node);
    }

    if (strncmp(token.value.literal, NIL, 3) == 0) {
      node->type = NODE_TYPE_NIL;
      node->value.nil = nullptr;
      return ok(result_node_ref_t, node);
    }

    size_t len = strlen(token.value.literal);

    if (len >= MAX_SYMBOL_LENGTH) {
      throw(result_node_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
            token.position, "Token too long. Expected length <= %lu, got %lu",
            MAX_SYMBOL_LENGTH, len);
    }
    node->type = NODE_TYPE_SYMBOL;
    char *symbol = nullptr;
    tryWithMeta(result_node_ref_t, arenaAllocate(arena, len + 1),
                token.position, symbol);
    stringCopy(symbol, token.value.literal, len + 1);
    node->value.symbol = symbol;
    return ok(result_node_ref_t, node);
  }
  case TOKEN_TYPE_STRING: {
    size_t len = strlen(token.value.string);
    node->type = NODE_TYPE_STRING;
    char *string = nullptr;
    tryWithMeta(result_node_ref_t, arenaAllocate(arena, len + 1),
                token.position, string);
    stringCopy(string, token.value.string, len + 1);
    node->value.string = string;
    return ok(result_node_ref_t, node);
  }
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
  default:
    unreachable();
  }
}

result_node_ref_t parseList(arena_t *arena, const token_list_t *tokens,
                            size_t *offset, size_t *depth) {
  token_t first_token = listGet(token_t, tokens, *offset);

  node_t *node = nullptr;
  tryWithMeta(result_node_ref_t, nodeCreate(arena, NODE_TYPE_LIST),
              first_token.position, node);
  tryWithMeta(result_node_ref_t, nodeInit(node, arena), first_token.position);

  node->position = first_token.position;
  node->type = NODE_TYPE_LIST;
  (*depth)++;
  (*offset)++;

  for (; *offset < tokens->capacity; (*offset)++) {
    const token_t token = listGet(token_t, tokens, *offset);
    if (token.type == TOKEN_TYPE_RPAREN) {
      (*depth)--;
      break;
    }

    node_t *sub_node = nullptr;
    try(result_node_ref_t, parse(arena, tokens, offset, depth), sub_node);
    tryWithMeta(result_node_ref_t,
                listAppend(node_t, &node->value.list, sub_node),
                token.position);
  }

  return ok(result_node_ref_t, node);
}

result_node_ref_t parse(arena_t *arena, const token_list_t *tokens,
                        size_t *offset, size_t *depth) {
  if (tokens->count == 0) {
    return ok(result_node_ref_t, nullptr);
  }

  const token_t first_token = listGet(token_t, tokens, *offset);
  size_t initial_depth = *depth;

  if (first_token.type == TOKEN_TYPE_LPAREN) {
    result_node_ref_t parse_list_result =
        parseList(arena, tokens, offset, depth);

    // There are left parens that don't match right parens
    if (*depth != initial_depth) {
      throw(result_node_ref_t, ERROR_CODE_SYNTAX_UNBALANCED_PARENTHESES,
            first_token.position, "Unbalanced parentheses");
    }

    // There are dangling chars after top level list
    if (initial_depth == 0 && *offset != (tokens->count - 1)) {
      const token_t last_token = listGet(token_t, tokens, *offset + 1);

      if (last_token.type == TOKEN_TYPE_RPAREN) {
        throw(result_node_ref_t, ERROR_CODE_SYNTAX_UNBALANCED_PARENTHESES,
              first_token.position, "Unbalanced parentheses");
      }

      throw(result_node_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
            last_token.position, "Unexpected token at the end input");
    }

    return parse_list_result;
  }

  return parseAtom(arena, first_token);
}
