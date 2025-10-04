#include "specials.h"
#include "../lib/list.h"
#include "../lib/result.h"
#include "error.h"
#include "evaluate.h"
#include "node.h"
#include "position.h"
#include "token.h"
#include "value.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

/**
 * Binds a value to a symbol in the current environment.
 * @name def!
 * @param {symbol} name - The identifier to bind.
 * @param {any} form - The expression whose value will be stored.
 * @returns {nil} Always returns nil after the binding is created.
 * @example
 *   (def! answer 42)
 *   (def! sum (fn (a b) (+ a b)))
 */
result_value_ref_t define(const node_array_t *nodes, environment_t *environment,
                          trampoline_t *trampoline) {
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a symbol and a form.", DEFINE);
  }

  node_t key = listGet(node_t, nodes, 1);
  if (key.type != NODE_TYPE_SYMBOL) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a symbol and a form.", DEFINE);
  }

  if (strchr(key.value.symbol, NAMESPACE_DELIMITER) != nullptr) {
    throw(result_value_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
          first.position,
          "Unexpected namespace delimiter '%c' in custom symbol '%s'.",
          NAMESPACE_DELIMITER, key.value.symbol);
  }

  node_t value = listGet(node_t, nodes, 2);

  value_t *reduced = nullptr;
  try(result_value_ref_t, evaluate(&value, environment), reduced);

  tryFinallyWithMeta(
      result_value_ref_t,
      environmentRegisterSymbol(environment, key.value.symbol, reduced),
      valueDestroy(&reduced), key.position);

  trampoline->more = false;
  return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, first.position);
}

result_value_ref_t function(const node_array_t *nodes,
                            environment_t *environment,
                            trampoline_t *trampoline) {
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a binding list and a form.", FUNCTION);
  }

  node_t arguments = listGet(node_t, nodes, 1);
  if (arguments.type != NODE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, arguments.position,
          "%s requires a binding list and a form.", FUNCTION);
  }

  node_t form = listGet(node_t, nodes, 2);

  arguments_t *closure_arguments = nullptr;
  tryWithMeta(result_value_ref_t, argumentsCreate(arguments.value.list.count),
              arguments.position, closure_arguments);

  for (size_t i = 0; i < arguments.value.list.count; i++) {
    node_t argument = listGet(node_t, &arguments.value.list, i);
    if (argument.type != NODE_TYPE_SYMBOL) {
      argumentsDestroy(&closure_arguments);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, argument.position,
            "%s requires a binding list of symbols.", FUNCTION);
    }

    if (environmentResolveSymbol(environment, argument.value.symbol)) {
      argumentsDestroy(&closure_arguments);
      throw(result_value_ref_t, ERROR_CODE_REFERENCE_SYMBOL_SHADOWED,
            argument.position, "Identifier '%s' shadows a value",
            argument.value.symbol);
    }

    closure_arguments->data[i] = strdup(argument.value.symbol);
  }

  node_t *closure_form = nullptr;
  tryCatchWithMeta(result_value_ref_t, nodeCopy(&form),
                   argumentsDestroy(&closure_arguments), first.position,
                   closure_form);

  environment_t *closure_environment = nullptr;
  tryCatchWithMeta(
      result_value_ref_t, environmentCreate(environment),
      {
        nodeDestroy(&closure_form);
        argumentsDestroy(&closure_arguments);
      },
      first.position, closure_environment);

  value_as_t value_as = {.closure = {.arguments = closure_arguments,
                                     .form = closure_form,
                                     .environment = closure_environment}};

  value_t *result = nullptr;
  tryCatchWithMeta(
      result_value_ref_t,
      valueCreate(VALUE_TYPE_CLOSURE, value_as, first.position),
      {
        nodeDestroy(&closure_form);
        argumentsDestroy(&closure_arguments);
        environmentDestroy(&closure_environment);
      },
      first.position, result);

  trampoline->more = false;
  return ok(result_value_ref_t, result);
}

result_value_ref_t let(const node_array_t *nodes, environment_t *environment,
                       trampoline_t *trampoline) {
  assert(nodes->count > 0); // let is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, first.position,
          "%s requires a list of symbol-form assignments.", LET);
  }

  node_t couples = listGet(node_t, nodes, 1);
  if (couples.type != NODE_TYPE_LIST) {
    throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, couples.position,
          "%s requires a list of symbol-form assignments.", LET);
  }

  environment_t *local_env = nullptr;
  tryWithMeta(result_value_ref_t, environmentCreate(environment),
              couples.position, local_env);

  for (size_t i = 0; i < couples.value.list.count; i++) {
    node_t couple = listGet(node_t, &couples.value.list, i);

    if (couple.type != NODE_TYPE_LIST || couple.value.list.count != 2) {
      environmentForceDestroy(&local_env);

      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, couple.position,
            "%s requires a list of symbol-form assignments.", LET);
    }

    node_t symbol = listGet(node_t, &couple.value.list, 0);
    if (symbol.type != NODE_TYPE_SYMBOL) {
      environmentForceDestroy(&local_env);

      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, symbol.position,
            "%s requires a list of symbol-form assignments.", LET);
    }

    if (strchr(symbol.value.symbol, NAMESPACE_DELIMITER) != nullptr) {
      environmentForceDestroy(&local_env);

      throw(result_value_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
            first.position,
            "Unexpected namespace delimiter '%c' in custom symbol '%s'.",
            NAMESPACE_DELIMITER, symbol.value.symbol);
    }

    node_t body = listGet(node_t, &couple.value.list, 1);
    value_t *intermediate = nullptr;
    tryCatch(result_value_ref_t, evaluate(&body, local_env),
             environmentForceDestroy(&local_env), intermediate);
    position_t position = intermediate->position;

    tryCatchWithMeta(
        result_value_ref_t,
        environmentRegisterSymbol(local_env, symbol.value.symbol, intermediate),
        {
          environmentForceDestroy(&local_env);
          valueDestroy(&intermediate);
        },
        position);
    valueDestroy(&intermediate);
  }

  // TODO: TCO
  // trampoline->more = true;
  // trampoline->environment = local_env;
  // trampoline->node = &nodes->data[2];

  value_t *evaluated = nullptr;
  tryCatch(result_value_ref_t, evaluate(&nodes->data[2], local_env),
           environmentForceDestroy(&local_env), evaluated);

  if (evaluated->type == VALUE_TYPE_CLOSURE) {
    environment_t *env = evaluated->as.closure.environment;

    while (env) {
      if (env == local_env) {
        position_t position = evaluated->position;
        valueDestroy(&evaluated);
        environmentForceDestroy(&local_env);
        throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, position,
              "Cannot return pointer to ephemeral environment.")
      }
      env = env->parent;
    }
  }

  environmentForceDestroy(&local_env);

  trampoline->more = false;
  return ok(result_value_ref_t, evaluated);
}

result_value_ref_t cond(const node_array_t *nodes, environment_t *environment,
                        trampoline_t *trampoline) {
  for (size_t i = 1; i < nodes->count - 1; i++) {
    node_t node = listGet(node_t, nodes, i);
    if (node.type != NODE_TYPE_LIST || node.value.list.count != 2) {
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "%s requires a list of condition-form assignments.", COND);
    }

    node_t condition = listGet(node_t, &node.value.list, 0);

    value_t *condition_value = nullptr;
    try(result_value_ref_t, evaluate(&condition, environment), condition_value);

    if (condition_value->type != VALUE_TYPE_BOOLEAN) {
      value_type_t type = condition_value->type;
      valueDestroy(&condition_value);
      throw(result_value_ref_t, ERROR_CODE_RUNTIME_ERROR, node.position,
            "Conditions should resolve to a boolean, got %d.", type);
    }

    bool is_true = condition_value->as.boolean;
    valueDestroy(&condition_value);

    if (is_true) {
      trampoline->more = true;
      trampoline->environment = environment;
      trampoline->node = &node.value.list.data[1];
      return ok(result_value_ref_t, nullptr);
    }
  }

  trampoline->more = true;
  trampoline->environment = environment;
  trampoline->node = &nodes->data[nodes->count - 1];
  return ok(result_value_ref_t, nullptr);
}
