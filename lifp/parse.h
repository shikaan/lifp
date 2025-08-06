#pragma once

#include "../lib/arena.h"
#include "../lib/result.h"
#include "node.h"
#include "position.h"
#include "token.h"
#include <stddef.h>
#include <stdint.h>

constexpr char TRUE[] = "true";
constexpr char FALSE[] = "false";
constexpr char NIL[] = "nil";

typedef Result(node_t *, position_t) result_node_ref_t;
result_node_ref_t parse(arena_t *arena, const token_list_t *tokens,
                        size_t *offset, size_t *depth);
