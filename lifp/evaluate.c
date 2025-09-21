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

result_void_position_t invokeClosure(value_t *result, closure_t closure,
                                     value_list_t *arguments,
                                     arena_t *scratch_arena) {
  profileArena(scratch_arena);
  if (arguments->count < closure.arguments->count) {
    throw(result_void_position_t, ERROR_CODE_TYPE_UNEXPECTED_ARITY,
          closure.form->position,
          "Unexpected arity. Expected %lu arguments, got %lu.",
          closure.arguments->count, arguments->count);
  }

  environment_t *local_environment = nullptr;
  tryWithMeta(result_void_position_t,
              environmentCreate(scratch_arena, closure.captured_environment),
              result->position, local_environment);

  // Populate the closure with the values, skipping the closure
  for (size_t i = 0; i < closure.arguments->count; i++) {
    auto argument = listGet(node_t, closure.arguments, i);
    auto value = listGet(value_t, arguments, i);
    tryWithMeta(result_void_position_t,
                mapSet(value_t, local_environment->values,
                       argument.value.symbol, &value),
                value.position);
  }

  try(result_void_position_t, evaluate(result, scratch_arena, scratch_arena,
                                       closure.form, local_environment));

  return ok(result_void_position_t);
}

static result_void_position_t
evaluateNodeList(value_list_t **result, size_t initial_index, node_list_t *list,
                 arena_t *scratch_arena, environment_t *environment,
                 position_t position) {
  assert(initial_index <= list->count);
  tryWithMeta(result_void_position_t,
              listCreate(value_t, scratch_arena, list->count), position,
              *result);

  for (size_t i = initial_index; i < list->count; i++) {
    auto node = listGet(node_t, list, i);
    value_t reduced;
    try(result_void_position_t,
        evaluate(&reduced, scratch_arena, scratch_arena, &node, environment));
    tryWithMeta(result_void_position_t, listAppend(value_t, *result, &reduced),
                node.position);
  }
  return ok(result_void_position_t);
}

result_void_position_t evaluate(value_t *result, arena_t *result_arena,
                                arena_t *scratch_arena, node_t *node,
                                environment_t *environment) {
  profileSafeAlloc();
  profileArena(scratch_arena);

  result->position.column = node->position.column;
  result->position.line = node->position.line;

  switch (node->type) {
  case NODE_TYPE_BOOLEAN: {
    tryWithMeta(result_void_position_t,
                valueInit(result, result_arena, node->value.boolean),
                result->position);
    return ok(result_void_position_t);
  }
  case NODE_TYPE_NIL: {
    tryWithMeta(result_void_position_t,
                valueInit(result, result_arena, node->value.nil),
                result->position);
    return ok(result_void_position_t);
  }
  case NODE_TYPE_NUMBER: {
    tryWithMeta(result_void_position_t,
                valueInit(result, result_arena, node->value.number),
                result->position);
    return ok(result_void_position_t);
  }
  case NODE_TYPE_STRING: {
    result->type = VALUE_TYPE_STRING;
    size_t len = strlen(node->value.string);
    string_t string = nullptr;
    tryWithMeta(result_void_position_t, arenaAllocate(result_arena, len + 1),
                node->position, string);
    stringCopy(string, node->value.string, len + 1);
    result->value.string = string;
    return ok(result_void_position_t);
  }
  case NODE_TYPE_SYMBOL: {
    const value_t *resolved_value =
        environmentResolveSymbol(environment, node->value.symbol);

    if (!resolved_value) {
      throw(result_void_position_t, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND,
            result->position,
            "Symbol '%s' cannot be found in the current environment",
            node->value.symbol);
    }

    result->type = resolved_value->type;
    result->value = resolved_value->value;
    return ok(result_void_position_t);
  }
  case NODE_TYPE_LIST: {
    const auto list = node->value.list;

    if (list.count == 0) {
      result->type = VALUE_TYPE_LIST;
      result->value.list.count = 0;
      result->value.list.capacity = 0;
      result->value.list.data = nullptr;
      result->value.list.arena = result_arena;
      return ok(result_void_position_t);
    }

    node_t first_node = listGet(node_t, &list, 0);
    value_t first_value;
    try(result_void_position_t,
        evaluate(&first_value, scratch_arena, scratch_arena, &first_node,
                 environment));

    value_t scratch_result = {
        .position.column = node->position.column,
        .position.line = node->position.line,
    };

    switch (first_value.type) {
    case VALUE_TYPE_SPECIAL: {
      special_form_t special = first_value.value.special;
      try(result_void_position_t,
          special(&scratch_result, &list, scratch_arena, environment));
      tryWithMeta(result_void_position_t,
                  valueCopy(&scratch_result, result, result_arena),
                  node->position);
      return ok(result_void_position_t);
    }
    case VALUE_TYPE_BUILTIN: {
      // Start from 1 to drop builtin node
      value_list_t *arguments = nullptr;
      try(result_void_position_t,
          evaluateNodeList(&arguments, 1, &node->value.list, scratch_arena,
                           environment, node->position));

      builtin_t builtin = first_value.value.builtin;
      try(result_void_position_t,
          builtin(&scratch_result, arguments, scratch_arena));
      tryWithMeta(result_void_position_t,
                  valueCopy(&scratch_result, result, result_arena),
                  node->position);
      return ok(result_void_position_t);
    }
    case VALUE_TYPE_CLOSURE: {
      // Start from 1 to drop closure node
      value_list_t *arguments = nullptr;
      try(result_void_position_t,
          evaluateNodeList(&arguments, 1, &node->value.list, scratch_arena,
                           environment, node->position));

      closure_t closure = first_value.value.closure;
      try(result_void_position_t,
          invokeClosure(&scratch_result, closure, arguments, scratch_arena));
      tryWithMeta(result_void_position_t,
                  valueCopy(&scratch_result, result, result_arena),
                  node->position);
      return ok(result_void_position_t);
    }
    case VALUE_TYPE_NUMBER:
    case VALUE_TYPE_LIST:
    case VALUE_TYPE_NIL:
    case VALUE_TYPE_STRING:
    case VALUE_TYPE_BOOLEAN:
    default: {
      value_list_t *list_values = nullptr;
      try(result_void_position_t,
          evaluateNodeList(&list_values, 0, &node->value.list, scratch_arena,
                           environment, node->position));

      tryWithMeta(result_void_position_t,
                  valueInit(result, result_arena, list_values->count),
                  node->position);

      for (size_t i = 0; i < list_values->count; i++) {
        value_t value = listGet(value_t, list_values, i);
        value_t duplicated;
        tryWithMeta(result_void_position_t,
                    valueCopy(&value, &duplicated, result_arena),
                    value.position);
        tryWithMeta(result_void_position_t,
                    listAppend(value_t, &result->value.list, &duplicated),
                    value.position);
      }
      return ok(result_void_position_t);
    }
    }
  }
  default:
    unreachable();
  }
}
