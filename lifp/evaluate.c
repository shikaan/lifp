#include "evaluate.h"
#include "../lib/profile.h"
#include "../lib/string.h"
#include "error.h"
#include "node.h"
#include "position.h"
#include "value.h"
#include "virtual_machine.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static result_void_position_t
invokeSpecial(value_t *result, value_t special_value, arena_t *scratch_arena,
              environment_t *parent_environment, const node_list_t *list) {
  assert(special_value.type == VALUE_TYPE_SPECIAL);
  special_form_t special = special_value.value.special;
  try(result_void_position_t,
      special(result, scratch_arena, parent_environment, list));
  return ok(result_void_position_t);
}

static result_void_position_t invokeBuiltin(value_t *result,
                                            arena_t *scratch_arena,
                                            value_list_t *evaluated,
                                            value_t builtin_value) {
  assert(builtin_value.type == VALUE_TYPE_BUILTIN);
  builtin_t builtin = builtin_value.value.builtin;
  listUnshift(value_t, evaluated);
  try(result_void_position_t, builtin(scratch_arena, result, evaluated));
  return ok(result_void_position_t);
}

static result_void_position_t
invokeClosure(value_t *result, value_list_t *evaluated, value_t closure_value,
              arena_t *scratch_arena, environment_t *parent_environment) {
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
    tryCatchWithMeta(result_void_position_t,
                     mapSet(value_t, local_environment->values,
                            argument.value.symbol, &value),
                     environmentDestroy(&local_environment), value.position);
  }

  // TODO: even better here would be to bubble up where in the form the error
  // occurs as opposed to point to the call site
  tryFinallyWithMeta(result_void_position_t,
                     evaluate(result, scratch_arena, scratch_arena,
                              &closure.form, local_environment),
                     environmentDestroy(&local_environment),
                     closure_value.position);

  return ok(result_void_position_t);
}

typedef Result(value_list_t *, position_t) result_value_list_t;
static result_value_list_t evaluateNodes(arena_t *scratch_arena, node_t *ast,
                                         environment_t *env,
                                         value_t *first_value) {
  const auto list = ast->value.list;
  value_list_t *evaluated;
  tryWithMeta(result_value_list_t,
              listCreate(value_t, scratch_arena, list.count), ast->position,
              evaluated);

  // Just for the sake of not re-evaluating the first value
  tryWithMeta(result_value_list_t, listAppend(value_t, evaluated, first_value),
              first_value->position);

  for (size_t i = 1; i < list.count; i++) {
    auto node = listGet(node_t, &list, i);
    value_t reduced;
    try(result_value_list_t,
        evaluate(&reduced, scratch_arena, scratch_arena, &node, env));
    tryWithMeta(result_value_list_t, listAppend(value_t, evaluated, &reduced),
                node.position);
  }
  return ok(result_value_list_t, evaluated);
}

result_void_position_t evaluate(value_t *result, arena_t *result_arena,
                                arena_t *scratch_arena, node_t *ast,
                                environment_t *env) {
  profileSafeAlloc();
  profileArena(scratch_arena);
  profileArena(result_arena);

  result->position.column = ast->position.column;
  result->position.line = ast->position.line;

  switch (ast->type) {
  case NODE_TYPE_BOOLEAN: {
    tryWithMeta(result_void_position_t,
                valueInit(result, result_arena, ast->value.boolean),
                result->position);
    return ok(result_void_position_t);
  }
  case NODE_TYPE_NIL: {
    tryWithMeta(result_void_position_t,
                valueInit(result, result_arena, ast->value.nil),
                result->position);
    return ok(result_void_position_t);
  }
  case NODE_TYPE_NUMBER: {
    tryWithMeta(result_void_position_t,
                valueInit(result, result_arena, ast->value.number),
                result->position);
    return ok(result_void_position_t);
  }
  case NODE_TYPE_STRING: {
    result->type = VALUE_TYPE_STRING;
    size_t len = strlen(ast->value.string);
    string_t string;
    tryWithMeta(result_void_position_t, arenaAllocate(result_arena, len + 1),
                ast->position, string);
    stringCopy(string, ast->value.string, len + 1);
    result->value.string = string;
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
      result->value.list.arena = result_arena;
      return ok(result_void_position_t);
    }

    frame_handle_t scratch_frame = arenaAllocationFrameStart(scratch_arena);
    frame_handle_t result_frame = arenaAllocationFrameStart(result_arena);

    node_t first_node = listGet(node_t, &list, 0);

    value_t first_value;
    try(result_void_position_t,
        evaluate(&first_value, result_arena, scratch_arena, &first_node, env));

    value_list_t *evaluated = nullptr;
    switch (first_value.type) {
    case VALUE_TYPE_SPECIAL: {
      value_t temp;
      try(result_void_position_t,
          invokeSpecial(&temp, first_value, scratch_arena, env, &list));
      tryWithMeta(result_void_position_t,
                  valueCopy(&temp, result, result_arena), ast->position);
      goto done;
    }
    case VALUE_TYPE_BUILTIN: {
      try(result_void_position_t,
          evaluateNodes(scratch_arena, ast, env, &first_value), evaluated);
      value_t temp;
      try(result_void_position_t,
          invokeBuiltin(&temp, scratch_arena, evaluated, first_value));
      tryWithMeta(result_void_position_t,
                  valueCopy(&temp, result, result_arena), ast->position);
      goto done;
    }
    case VALUE_TYPE_CLOSURE: {
      try(result_void_position_t,
          evaluateNodes(scratch_arena, ast, env, &first_value), evaluated);
      value_t temp;
      try(result_void_position_t,
          invokeClosure(&temp, evaluated, first_value, scratch_arena, env));
      arenaAllocationFrameEnd(result_arena, result_frame);
      tryWithMeta(result_void_position_t,
                  valueCopy(&temp, result, result_arena), ast->position);
      goto done;
    }
    case VALUE_TYPE_NUMBER:
    case VALUE_TYPE_LIST:
    case VALUE_TYPE_NIL:
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_BOOLEAN:
    default:
      try(result_void_position_t,
          evaluateNodes(scratch_arena, ast, env, &first_value), evaluated);

      tryWithMeta(result_void_position_t,
                  valueInit(result, result_arena, evaluated->count),
                  ast->position);

      for (size_t i = 0; i < evaluated->count; i++) {
        value_t value = listGet(value_t, evaluated, i);
        value_t duplicated;
        tryWithMeta(result_void_position_t,
                    valueCopy(&value, &duplicated, result_arena),
                    ast->position);
        tryWithMeta(result_void_position_t,
                    listAppend(value_t, &result->value.list, &duplicated),
                    ast->position);
      }
    done:
      arenaAllocationFrameEnd(scratch_arena, scratch_frame);
      return ok(result_void_position_t);
    }
  }
  default:
    unreachable();
  }
}
