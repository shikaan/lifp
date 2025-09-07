#pragma once

#include "../lib/arena.h"
#include "node.h"
#include "value.h"
#include "virtual_machine.h"

// Evaluate a node in the context of a given environment.
//
// The result arena is where all the values required for the result will be
// stored. This is where values that need to survive across `evaluate` calls
// need to be stored.
//
// The scratch arena is used for intermediate allocations and it's expected to
// be wiped at the end of each call. No values to be persisted should be stored
// there.
result_void_position_t evaluate(value_t *result, arena_t *result_arena,
                                arena_t *scratch_arena, node_t *ast,
                                environment_t *env);
