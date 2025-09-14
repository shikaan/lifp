#include "specials.h"
#include "../lib/list.h"
#include "../lib/result.h"
#include "error.h"
#include "evaluate.h"
#include "node.h"
#include "token.h"
#include "value.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

static result_void_t captureEnvironment(const node_t *node,
                                        const environment_t *source,
                                        environment_t *destination) {
  if (node->type == NODE_TYPE_SYMBOL) {
    const value_t *value = environmentResolveSymbol(source, node->value.symbol);
    if (value && (value->type != VALUE_TYPE_SPECIAL &&
                  value->type != VALUE_TYPE_BUILTIN)) {
      try(result_void_t, environmentUnsafeRegisterSymbol(
                             destination, node->value.symbol, value));
    }
  } else if (node->type == NODE_TYPE_LIST) {
    for (size_t i = 0; i < node->value.list.count; i++) {
      const node_t sub_node = listGet(node_t, &node->value.list, i);
      try(result_void_t, captureEnvironment(&sub_node, source, destination));
    }
  }

  return ok(result_void_t);
}

const char *DEFINE_EXAMPLE = "(def! x (+ 1 2))";
result_void_position_t define(value_t *result, const node_list_t *nodes,
                              arena_t *scratch_arena,
                              environment_t *environment) {
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

  if (strchr(key.value.symbol, NAMESPACE_DELIMITER) != NULL) {
    throw(result_void_position_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
          first.position,
          "Unexpected namespace delimiter '%c' in custom symbol '%s'.",
          NAMESPACE_DELIMITER, key.value.symbol);
  }

  // Perform reduction in the temporary memory
  value_t reduced;
  node_t value = listGet(node_t, nodes, 2);
  try(result_void_position_t,
      evaluate(&reduced, scratch_arena, scratch_arena, &value, environment));

  // If reduction is successful, we can move the closure to VM memory
  tryWithMeta(
      result_void_position_t,
      environmentRegisterSymbol(environment, key.value.symbol, &reduced),
      value.position);

  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;
  result->position = first.position;
  return ok(result_void_position_t);
}

const char *FUNCTION_EXAMPLE = "(fn (a b) (+ a b))";
result_void_position_t function(value_t *result, const node_list_t *nodes,
                                arena_t *scratch_arena,
                                environment_t *environment) {
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

  tryWithMeta(result_void_position_t,
              valueInit(result, scratch_arena, form.type), result->position);

  for (size_t i = 0; i < arguments.value.list.count; i++) {
    node_t argument = listGet(node_t, &arguments.value.list, i);
    if (argument.type != NODE_TYPE_SYMBOL) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, argument.position,
            "%s requires a binding list of symbols. %s", FUNCTION,
            FUNCTION_EXAMPLE);
    }

    if (environmentResolveSymbol(environment, argument.value.symbol)) {
      throw(result_void_position_t, ERROR_CODE_REFERENCE_SYMBOL_SHADOWED,
            argument.position, "Identifier '%s' shadows a value",
            argument.value.symbol);
    }

    tryWithMeta(result_void_position_t,
                listAppend(node_t, &result->value.closure.arguments, &argument),
                argument.position);
  }

  tryWithMeta(result_void_position_t,
              nodeCopy(&form, &result->value.closure.form, scratch_arena),
              form.position);

  environment_t *captured = nullptr;
  tryWithMeta(result_void_position_t,
              environmentCreate(scratch_arena, environment), form.position,
              captured);

  tryWithMeta(result_void_position_t,
              captureEnvironment(&form, environment, captured), form.position);
  result->value.closure.captured_environment = captured;

  return ok(result_void_position_t);
}

const char *LET_EXAMPLE = "(let ((a 1) (b 2)) (+ a b))";
result_void_position_t let(value_t *result, const node_list_t *nodes,
                           arena_t *scratch_arena, environment_t *environment) {
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
  tryWithMeta(result_void_position_t,
              environmentCreate(scratch_arena, environment), couples.position,
              local_env);

  for (size_t i = 0; i < couples.value.list.count; i++) {
    node_t couple = listGet(node_t, &couples.value.list, i);

    if (couple.type != NODE_TYPE_LIST || couple.value.list.count != 2) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, couple.position,
            "%s requires a list of symbol-form assignments. %s", LET,
            LET_EXAMPLE);
    }

    node_t symbol = listGet(node_t, &couple.value.list, 0);
    if (symbol.type != NODE_TYPE_SYMBOL) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, symbol.position,
            "%s requires a list of symbol-form assignments. %s", LET,
            LET_EXAMPLE);
    }

    if (strchr(symbol.value.symbol, NAMESPACE_DELIMITER) != NULL) {
      throw(result_void_position_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
            first.position,
            "Unexpected namespace delimiter '%c' in custom symbol '%s'.",
            NAMESPACE_DELIMITER, symbol.value.symbol);
    }

    node_t body = listGet(node_t, &couple.value.list, 1);
    value_t evaluated;
    try(result_void_position_t,
        evaluate(&evaluated, scratch_arena, scratch_arena, &body, local_env));
    tryWithMeta(
        result_void_position_t,
        environmentRegisterSymbol(local_env, symbol.value.symbol, &evaluated),
        evaluated.position);
  }

  node_t form = listGet(node_t, nodes, 2);
  value_t temp_result;
  try(result_void_position_t,
      evaluate(&temp_result, scratch_arena, scratch_arena, &form, local_env));

  // The result from the evaluation might be allocated in the local_env's arena,
  // which will be destroyed. We need to copy it to the temp arena to allow
  // expressions like `(let ((l (1 2))) l)` where values can escape the env
  tryWithMeta(result_void_position_t,
              valueCopy(&temp_result, result, scratch_arena),
              temp_result.position);

  return ok(result_void_position_t);
}

const char *COND_EXAMPLE = "\n  (cond\n    ((!= x 0) (/ 10 x))\n    (+ x 10))";
result_void_position_t cond(value_t *result, const node_list_t *nodes,
                            arena_t *scratch_arena,
                            environment_t *environment) {
  assert(nodes->count > 0); // cond is always there

  for (size_t i = 1; i < nodes->count - 1; i++) {
    node_t node = listGet(node_t, nodes, i);
    if (node.type != NODE_TYPE_LIST || node.value.list.count != 2) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "%s requires a list of condition-form assignments. %s", COND,
            COND_EXAMPLE);
    }

    node_t condition = listGet(node_t, &node.value.list, 0);
    value_t condition_value;
    condition_value.position = condition.position;
    try(result_void_position_t,
        evaluate(&condition_value, scratch_arena, scratch_arena, &condition,
                 environment));

    if (condition_value.type != VALUE_TYPE_BOOLEAN) {
      throw(result_void_position_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "Conditions should resolve to a boolean, got %d. %s",
            condition_value.type, COND_EXAMPLE);
    }

    if (condition_value.value.boolean) {
      node_t form = listGet(node_t, &node.value.list, 1);
      try(result_void_position_t,
          evaluate(result, scratch_arena, scratch_arena, &form, environment));
      return ok(result_void_position_t);
    }
  }

  node_t fallback = listGet(node_t, nodes, nodes->count - 1);
  try(result_void_position_t,
      evaluate(result, scratch_arena, scratch_arena, &fallback, environment));
  return ok(result_void_position_t);
}
