#include "evaluate.h"
#include "../lib/profile.h"
#include "error.h"
#include "node.h"
#include "value.h"
#include "virtual_machine.h"

// NOLINTNEXTLINE
#include "specials.c"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static bool isSpecialFormNode(const node_t FIRST_NODE) {
  if (FIRST_NODE.type != NODE_TYPE_SYMBOL)
    return false;

  size_t len = strlen(FIRST_NODE.value.symbol);
  return ((strncmp(FIRST_NODE.value.symbol, DEFINE, 4) == 0 && len == 4) ||
          (strncmp(FIRST_NODE.value.symbol, FUNCTION, 2) == 0 && len == 2) ||
          (strncmp(FIRST_NODE.value.symbol, LET, 3) == 0 && len == 3) != 0 ||
          (strncmp(FIRST_NODE.value.symbol, COND, 4) == 0 && len == 4)) != 0;
}

static result_void_position_t
invokeBuiltin(value_t *result, value_list_t *evaluated, value_t builtin_value) {
  assert(builtin_value.type == VALUE_TYPE_BUILTIN);
  builtin_t builtin = builtin_value.value.builtin;
  listUnshift(value_t, evaluated);
  try(result_void_position_t, builtin(result, evaluated));
  return ok(result_void_position_t);
}

static result_void_position_t
invokeClosure(value_t *result, value_list_t *evaluated, value_t closure_value,
              arena_t *arena, environment_t *parent_environment) {
  assert(closure_value.type == VALUE_TYPE_CLOSURE);
  closure_t closure = closure_value.value.closure;

  if (evaluated->count - 1 != closure.arguments.count) {
    throw(result_void_position_t, ERROR_CODE_TYPE_UNEXPECTED_ARITY,
          closure_value.position,
          "Unexpected arity. Expected %lu arguments, got %lu.",
          closure.arguments.count, evaluated->count - 1);
  }

  environment_t *local_environment = nullptr;
  tryWithMeta(result_void_position_t, environmentCreate(parent_environment),
              closure_value.position, local_environment);

  // Populate the closure with the values, skipping the closure symbol
  for (size_t i = 1; i < evaluated->count; i++) {
    auto argument = listGet(node_t, &closure.arguments, i - 1);
    auto value = listGet(value_t, evaluated, i);
    tryCatchWithMeta(
        result_void_position_t,
        mapSet(local_environment->values, argument.value.symbol, &value),
        environmentDestroy(&local_environment), value.position);
  }

  // TODO: even better here would be to bubble up where in the form the error
  // occurs as opposed to point to the call site
  tryFinallyWithMeta(result_void_position_t,
                     evaluate(result, arena, &closure.form, local_environment),
                     environmentDestroy(&local_environment),
                     closure_value.position);

  return ok(result_void_position_t);
}

static result_void_position_t invokeSpecialForm(arena_t *arena, value_t *result,
                                                node_t form_node,
                                                const node_list_t *nodes,
                                                environment_t *environment) {
  if (form_node.value.symbol[0] == 'd') {
    try(result_void_position_t, define(result, arena, environment, nodes));
  } else if (form_node.value.symbol[0] == 'f') {
    try(result_void_position_t, function(result, arena, environment, nodes));
  } else if (form_node.value.symbol[0] == 'l') {
    try(result_void_position_t, let(result, arena, environment, nodes));
  } else {
    try(result_void_position_t, cond(result, arena, environment, nodes));
  }

  return ok(result_void_position_t);
}

result_void_position_t evaluate(value_t *result, arena_t *temp_arena,
                                node_t *ast, environment_t *env) {
  profileSafeAlloc();
  profileArena(temp_arena);

  result->position.column = ast->position.column;
  result->position.line = ast->position.line;

  switch (ast->type) {
  case NODE_TYPE_BOOLEAN: {
    result->type = VALUE_TYPE_BOOLEAN;
    result->value.boolean = ast->value.boolean;
    return ok(result_void_position_t);
  }
  case NODE_TYPE_NIL: {
    result->type = VALUE_TYPE_NIL;
    result->value.nil = ast->value.nil;
    return ok(result_void_position_t);
  }
  case NODE_TYPE_NUMBER: {
    result->type = VALUE_TYPE_NUMBER;
    result->value.number = ast->value.number;
    return ok(result_void_position_t);
  }
  case NODE_TYPE_SYMBOL: {
    const value_t *resolved_value =
        environmentResolveSymbol(env, ast->value.symbol);

    if (!resolved_value) {
      throw(result_void_position_t, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND,
            result->position,
            "Symbol '%s' cannot be found in the current environment",
            ast->value.symbol);
    }

    result->type = resolved_value->type;
    result->value = resolved_value->value;
    return ok(result_void_position_t);
  }
  case NODE_TYPE_LIST: {
    const auto list = ast->value.list;

    if (list.count == 0) {
      result->type = VALUE_TYPE_LIST;
      result->value.list.count = 0;
      result->value.list.capacity = 0;
      result->value.list.data = nullptr;
      result->value.list.arena = temp_arena;
      return ok(result_void_position_t);
    }

    auto first_node = listGet(node_t, &list, 0);
    if (isSpecialFormNode(first_node)) {
      return invokeSpecialForm(temp_arena, result, first_node, &list, env);
    }

    value_list_t *evaluated;
    frame_handle_t frame = arenaAllocationFrameStart(temp_arena);
    tryWithMeta(result_void_position_t,
                listCreate(value_t, temp_arena, list.count),
                first_node.position, evaluated);

    for (size_t i = 0; i < list.count; i++) {
      auto node = listGet(node_t, &list, i);
      value_t reduced;
      tryCatch(result_void_position_t,
               evaluate(&reduced, temp_arena, &node, env),
               arenaAllocationFrameEnd(temp_arena, frame));
      tryWithMeta(result_void_position_t,
                  listAppend(value_t, evaluated, &reduced), node.position);
    }

    value_t first_value = listGet(value_t, evaluated, 0);

    switch (first_value.type) {
    case VALUE_TYPE_BUILTIN:
      tryFinally(result_void_position_t,
                 invokeBuiltin(result, evaluated, first_value),
                 arenaAllocationFrameEnd(temp_arena, frame));
      return ok(result_void_position_t);
    case VALUE_TYPE_CLOSURE:
      tryFinally(result_void_position_t,
                 invokeClosure(result, evaluated, first_value, temp_arena, env),
                 arenaAllocationFrameEnd(temp_arena, frame));
      return ok(result_void_position_t);
    case VALUE_TYPE_NUMBER:
    case VALUE_TYPE_LIST:
    case VALUE_TYPE_NIL:
    case VALUE_TYPE_BOOLEAN:
    default:
      result->type = VALUE_TYPE_LIST;
      memcpy(&result->value.list, evaluated, sizeof(result->value.list));
      arenaAllocationFrameEnd(temp_arena, frame);
      return ok(result_void_position_t);
    }
  }
  default:
    unreachable();
  }
}
