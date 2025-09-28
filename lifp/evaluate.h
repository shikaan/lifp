#pragma once

#include "node.h"
#include "value.h"

// Evaluate a node in the context of a given environment.
// Returns pointer to a value that the caller needs to free.
result_value_ref_t evaluate(node_t *, environment_t *);

// Invokes a closure with the specified arguments and environment.
// Returns pointer to a value that the caller needs to free.
result_value_ref_t invokeClosure(value_t *, value_array_t *);
