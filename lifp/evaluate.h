#pragma once

#include "node.h"
#include "value.h"

// Evaluate a node in the context of a given environment.
//
// The result arena is where all the values required for the result will be
// stored. This is where values that need to survive across `evaluate` calls
// need to be stored.
//
// The scratch arena is used for intermediate allocations and it's expected to
// be wiped at the end of each call. No values to be persisted should be stored
// there.
result_value_ref_t evaluate(node_t *, environment_t *);

// Invokes a closure with the specified arguments and environment.
//
// This function executes the given closure, passing in the provided arguments,
// using the specified scratch arena for temporary allocations, and within the
// given environment. The result of the closure execution is stored in the
// location pointed to by `result`.
result_value_ref_t invokeClosure(value_t *, value_array_t *);
