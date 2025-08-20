#pragma once

#include "../lib/arena.h"
#include "node.h"
#include "value.h"
#include "virtual_machine.h"

result_void_position_t evaluate(value_t *result, arena_t *temp_arena,
                                node_t *ast, environment_t *env);
