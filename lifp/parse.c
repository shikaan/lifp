#include "parse.h"
#include "error.h"
#include "node.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

result_node_ref_t parseAtom(arena_t *arena, token_t token) {
  assert(token.type == TOKEN_TYPE_NUMBER || token.type == TOKEN_TYPE_SYMBOL);

  node_t *node = nullptr;
  tryWithMeta(result_node_ref_t, nodeCreate(arena, NODE_TYPE_NUMBER),
              token.position, node);

  node->position = token.position;

  switch (token.type) {
  case TOKEN_TYPE_NUMBER:
    node->type = NODE_TYPE_NUMBER;
    node->value.number = token.value.number;
    return ok(result_node_ref_t, node);
  case TOKEN_TYPE_SYMBOL:
    if (strncmp(token.value.symbol, TRUE, 4) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = true;
      return ok(result_node_ref_t, node);
    }

    if (strncmp(token.value.symbol, FALSE, 5) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = false;
      return ok(result_node_ref_t, node);
    }

    if (strncmp(token.value.symbol, NIL, 3) == 0) {
      node->type = NODE_TYPE_NIL;
      node->value.nil = nullptr;
      return ok(result_node_ref_t, node);
    }

    size_t len = strlen(token.value.symbol);
    char *string = nullptr;
    tryWithMeta(result_node_ref_t, arenaAllocate(arena, len + 1),
                token.position, string);
    strlcpy(string, token.value.symbol, len + 1);
    node->type = NODE_TYPE_SYMBOL;
    node->position = token.position;
    node->value.symbol = string;
    memcpy(node->value.symbol, token.value.symbol, 16);
    return ok(result_node_ref_t, node);
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
