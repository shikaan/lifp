#include "specials.h"
#include "../lib/list.h"
#include "../lib/map.h"
#include "../lib/result.h"
#include "error.h"
#include "evaluate.h"
#include "node.h"
#include "position.h"
#include "value.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

static result_void_position_t addToEnvironment(const char *key, value_t *value,
                                               environment_t *environment,
                                               position_t position) {
  if (environmentResolveSymbol(environment, key)) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, position,
          "identifier '%s' has already been declared", key);
  }

  // If reduction is successful, we can move the closure to VM memory
  value_t copied;
  tryWithMeta(result_void_position_t,
              valueCopy(value, &copied, environment->arena), position);
  tryWithMeta(result_void_position_t,
              mapSet(value_t, environment->values, key, &copied), position);
  return ok(result_void_position_t);
}

const char *DEFINE_EXAMPLE = "(def! x (+ 1 2))";
result_void_position_t define(value_t *result, arena_t *scratch_arena,
                              environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // def! is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a symbol and a form. %s", DEFINE, DEFINE_EXAMPLE);
  }

  node_t key = listGet(node_t, nodes, 1);
  if (key.type != NODE_TYPE_SYMBOL) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a symbol and a form. %s", DEFINE, DEFINE_EXAMPLE);
  }

  frame_handle_t frame = arenaAllocationFrameStart(scratch_arena);

  // Perform reduction in the temporary memory
  value_t reduced;
  node_t value = listGet(node_t, nodes, 2);
  tryCatch(result_void_position_t,
           evaluate(&reduced, scratch_arena, scratch_arena, &value, env),
           arenaAllocationFrameEnd(scratch_arena, frame));

  // If reduction is successful, we can move the closure to VM memory
  tryFinally(result_void_position_t,
             addToEnvironment(key.value.symbol, &reduced, env, value.position),
             arenaAllocationFrameEnd(scratch_arena, frame));

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;
  result->position = first.position;
  return ok(result_void_position_t);
}

const char *FUNCTION_EXAMPLE = "(fn (a b) (+ a b))";
result_void_position_t function(value_t *result, arena_t *scratch_arena,
                                environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // fn is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a binding list and a form. %s", FUNCTION,
          FUNCTION_EXAMPLE);
  }

  node_t arguments = listGet(node_t, nodes, 1);
  if (arguments.type != NODE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, arguments.position,
          "%s requires a binding list and a form. %s", FUNCTION,
          FUNCTION_EXAMPLE);
  }

  node_t form = listGet(node_t, nodes, 2);

  frame_handle_t frame = arenaAllocationFrameStart(scratch_arena);
  tryWithMeta(result_void_position_t,
              valueInit(result, scratch_arena, form.type), result->position);

  for (size_t i = 0; i < arguments.value.list.count; i++) {
    node_t argument = listGet(node_t, &arguments.value.list, i);
    if (argument.type != NODE_TYPE_SYMBOL) {
      arenaAllocationFrameEnd(scratch_arena, frame);
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, argument.position,
            "%s requires a binding list of symbols. %s", FUNCTION,
            FUNCTION_EXAMPLE);
    }

    if (environmentResolveSymbol(env, argument.value.symbol)) {
      arenaAllocationFrameEnd(scratch_arena, frame);
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, argument.position,
            "identifier '%s' shadows a value", argument.value.symbol);
    }

    tryCatchWithMeta(
        result_void_position_t,
        listAppend(node_t, &result->value.closure.arguments, &argument),
        arenaAllocationFrameEnd(scratch_arena, frame), argument.position);
  }

  tryWithMeta(result_void_position_t,
              nodeCopy(&form, &result->value.closure.form, scratch_arena),
              form.position);

  return ok(result_void_position_t);
}

const char *LET_EXAMPLE = "(let ((a 1) (b 2)) (+ a b))";
result_void_position_t let(value_t *result, arena_t *scratch_arena,
                           environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // let is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list of symbol-form assignments. %s", LET,
          LET_EXAMPLE);
  }

  node_t couples = listGet(node_t, nodes, 1);
  if (couples.type != NODE_TYPE_LIST) {
    throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, couples.position,
          "%s requires a list of symbol-form assignments. %s", LET,
          LET_EXAMPLE);
  }

  environment_t *local_env = nullptr;
  tryWithMeta(result_void_position_t, environmentCreate(env), couples.position,
              local_env);

  for (size_t i = 0; i < couples.value.list.count; i++) {
    node_t couple = listGet(node_t, &couples.value.list, i);

    if (couple.type != NODE_TYPE_LIST || couple.value.list.count != 2) {
      environmentDestroy(&local_env);
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, couple.position,
            "%s requires a list of symbol-form assignments. %s", LET,
            LET_EXAMPLE);
    }

    node_t symbol = listGet(node_t, &couple.value.list, 0);
    if (symbol.type != NODE_TYPE_SYMBOL) {
      environmentDestroy(&local_env);
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, symbol.position,
            "%s requires a list of symbol-form assignments. %s", LET,
            LET_EXAMPLE);
    }

    node_t body = listGet(node_t, &couple.value.list, 1);
    frame_handle_t bindings_frame = arenaAllocationFrameStart(scratch_arena);
    value_t evaluated;
    tryCatch(
        result_void_position_t,
        evaluate(&evaluated, scratch_arena, scratch_arena, &body, local_env),
        environmentDestroy(&local_env));
    tryCatch(result_void_position_t,
             addToEnvironment(symbol.value.symbol, &evaluated, local_env,
                              evaluated.position),
             environmentDestroy(&local_env));
    arenaAllocationFrameEnd(scratch_arena, bindings_frame);
  }

  node_t form = listGet(node_t, nodes, 2);
  value_t temp_result;
  tryCatch(
      result_void_position_t,
      evaluate(&temp_result, scratch_arena, scratch_arena, &form, local_env),
      environmentDestroy(&local_env));

  // The result from the evaluation might be allocated in the local_env's arena,
  // which will be destroyed. We need to copy it to the temp arena to allow
  // expressions like `(let ((l (1 2))) l)` where values can escape the env
  tryWithMeta(result_void_position_t,
              valueCopy(&temp_result, result, scratch_arena),
              temp_result.position);

  environmentDestroy(&local_env);
  return ok(result_void_position_t);
}

const char *COND_EXAMPLE = "\n  (cond\n    ((!= x 0) (/ 10 x))\n    (+ x 10))";
result_void_position_t cond(value_t *result, arena_t *scratch_arena,
                            environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0);

  for (size_t i = 1; i < nodes->count - 1; i++) {
    frame_handle_t frame = arenaAllocationFrameStart(scratch_arena);
    node_t node = listGet(node_t, nodes, i);
    if (node.type != NODE_TYPE_LIST || node.value.list.count != 2) {
      arenaAllocationFrameEnd(scratch_arena, frame);
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "%s requires a list of condition-form assignments. %s", COND,
            COND_EXAMPLE);
    }

    node_t condition = listGet(node_t, &node.value.list, 0);
    try(result_void_position_t,
        evaluate(result, scratch_arena, scratch_arena, &condition, env));

    if (result->type != VALUE_TYPE_BOOLEAN) {
      arenaAllocationFrameEnd(scratch_arena, frame);
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "Conditions should resolve to a boolean. %s", COND_EXAMPLE);
    }

    if (result->value.boolean) {
      node_t form = listGet(node_t, &node.value.list, 1);
      try(result_void_position_t,
          evaluate(result, scratch_arena, scratch_arena, &form, env));
      return ok(result_void_position_t);
    }
    arenaAllocationFrameEnd(scratch_arena, frame);
  }

  node_t fallback = listGet(node_t, nodes, nodes->count - 1);
  try(result_void_position_t,
      evaluate(result, scratch_arena, scratch_arena, &fallback, env));
  return ok(result_void_position_t);
}
