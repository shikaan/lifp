#pragma once

#include "../lib/arena.h"
#include "../lib/result.h"
#include "token.h"

typedef Result(token_list_t *, position_t) result_token_list_ref_t;
result_token_list_ref_t tokenize(arena_t *arena, const char *source);
