#include "../lib/list.h"
#include "../lib/map.h"
#include "../lib/result.h"
#include "error.h"
#include "evaluate.h"
#include "node.h"
#include "value.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef result_value_ref_t (*special_form_t)(arena_t *, environment_t *,
                                             const node_list_t *);

const char *DEFINE_EXAMPLE = "(def! x (+ 1 2))";
const char *DEFINE = "def!";
result_value_ref_t define(arena_t *arena, environment_t *env,
                          const node_list_t *nodes) {
  assert(nodes->count > 0); // def! is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a symbol and a form. %s", DEFINE, DEFINE_EXAMPLE);
  }

  node_t key = listGet(node_t, nodes, 1);
  if (key.type != NODE_TYPE_SYMBOL) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a symbol and a form. %s", DEFINE, DEFINE_EXAMPLE);
  }

  // Perform reduction in the temporary memory
  frame_handle_t frame = arenaAllocationFrameStart(arena);
  value_t *result = nullptr;
  node_t value = listGet(node_t, nodes, 2);
  tryFinally(result_value_ref_t, evaluate(arena, &value, env),
             arenaAllocationFrameEnd(arena, frame), result);

  // If reduction is successful, we can move the closure to VM memory
  value_t *copy = nullptr;
  tryWithMeta(result_value_ref_t, valueClone(env->arena, result),
              value.position, copy);
  tryWithMeta(result_value_ref_t, mapSet(env->values, key.value.symbol, copy),
              value.position);

  value_t nil = {
      .type = VALUE_TYPE_NIL,
      .value.nil = nullptr,
      .position = first.position,
  };
  return ok(result_value_ref_t, &nil);
}

const char *FUNCTION_EXAMPLE = "(fn (a b) (+ a b))";
const char *FUNCTION = "fn";
result_value_ref_t function(arena_t *arena, environment_t *env,
                            const node_list_t *nodes) {
  (void)env;
  assert(nodes->count > 0); // fn is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a binding list and a form. %s", FUNCTION,
          FUNCTION_EXAMPLE);
  }

  node_t arguments = listGet(node_t, nodes, 1);
  if (arguments.type != NODE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, arguments.position,
          "%s requires a binding list and a form. %s", FUNCTION,
          FUNCTION_EXAMPLE);
  }

  for (size_t i = 0; i < arguments.value.list.count; i++) {
    node_t argument = listGet(node_t, &arguments.value.list, i);
    if (argument.type != NODE_TYPE_SYMBOL) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, argument.position,
            "%s requires a binding list of symbols. %s", FUNCTION,
            FUNCTION_EXAMPLE);
    }
  }

  node_t form = listGet(node_t, nodes, 2);

  value_t *closure = nullptr;
  tryWithMeta(result_value_ref_t, valueCreate(arena, VALUE_TYPE_CLOSURE),
              form.position, closure);

  closure->position.column = first.position.column;
  closure->position.line = first.position.line;

  tryWithMeta(result_value_ref_t,
              listCopy(node_t, &arguments.value.list,
                       &closure->value.closure.arguments),
              form.position);
  tryWithMeta(result_value_ref_t, nodeCopy(&form, &closure->value.closure.form),
              form.position);

  return ok(result_value_ref_t, closure);
}

const char *LET_EXAMPLE = "(let ((a 1) (b 2)) (+ a b))";
const char *LET = "let";
result_value_ref_t let(arena_t *arena, environment_t *env,
                       const node_list_t *nodes) {
  assert(nodes->count > 0); // let is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list of symbol-form assignments. %s", LET,
          LET_EXAMPLE);
  }

  node_t couples = listGet(node_t, nodes, 1);
  if (couples.type != NODE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, couples.position,
          "%s requires a list of symbol-form assignments. %s", LET,
          LET_EXAMPLE);
  }

  environment_t *local_env = nullptr;
  tryWithMeta(result_value_ref_t, environmentCreate(env), couples.position,
              local_env);

  for (size_t i = 0; i < couples.value.list.count; i++) {
    node_t couple = listGet(node_t, &couples.value.list, i);

    if (couple.type != NODE_TYPE_LIST || couple.value.list.count != 2) {
      environmentDestroy(&local_env);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, couple.position,
            "%s requires a list of symbol-form assignments. %s", LET,
            LET_EXAMPLE);
    }

    node_t symbol = listGet(node_t, &couple.value.list, 0);
    if (symbol.type != NODE_TYPE_SYMBOL) {
      environmentDestroy(&local_env);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, symbol.position,
            "%s requires a list of symbol-form assignments. %s", LET,
            LET_EXAMPLE);
    }

    node_t body = listGet(node_t, &couple.value.list, 1);
    frame_handle_t bindings_frame = arenaAllocationFrameStart(arena);
    value_t *evaluated = nullptr;
    tryCatch(result_value_ref_t, evaluate(arena, &body, local_env),
             environmentDestroy(&local_env), evaluated);
    tryCatchWithMeta(result_value_ref_t,
                     mapSet(local_env->values, symbol.value.symbol, evaluated),
                     environmentDestroy(&local_env), evaluated->position);
    arenaAllocationFrameEnd(arena, bindings_frame);
  }

  node_t form = listGet(node_t, nodes, 2);

  value_t *result = nullptr;
  tryFinally(result_value_ref_t, evaluate(arena, &form, local_env),
             environmentDestroy(&local_env), result);

  return ok(result_value_ref_t, result);
}

const char *COND_EXAMPLE = "\n  (cond\n    ((!= x 0) (/ 10 x))\n    (+ x 10))";
const char *COND = "cond";
result_value_ref_t cond(arena_t *arena, environment_t *env,
                        const node_list_t *nodes) {
  assert(nodes->count > 0);
  value_t *result = nullptr;

  for (size_t i = 1; i < nodes->count - 1; i++) {
    frame_handle_t frame = arenaAllocationFrameStart(arena);
    node_t node = listGet(node_t, nodes, i);
    if (node.type != NODE_TYPE_LIST || node.value.list.count != 2) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "%s requires a list of condition-form assignments. %s", COND,
            COND_EXAMPLE);
    }

    node_t condition = listGet(node_t, &node.value.list, 0);
    try(result_value_ref_t, evaluate(arena, &condition, env), result);

    if (result->type != VALUE_TYPE_BOOLEAN) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "Conditions should resolve to a boolean. %s", COND_EXAMPLE);
    }

    if (result->value.boolean) {
      node_t form = listGet(node_t, &node.value.list, 1);
      try(result_value_ref_t, evaluate(arena, &form, env), result);
      return ok(result_value_ref_t, result);
    }
    arenaAllocationFrameEnd(arena, frame);
  }

  node_t fallback = listGet(node_t, nodes, nodes->count - 1);
  try(result_value_ref_t, evaluate(arena, &fallback, env), result);
  return ok(result_value_ref_t, result);
}
