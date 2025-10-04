#include "evaluate.h"
#include "error.h"
#include "node.h"
#include "position.h"
#include "value.h"
#include "virtual_machine.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

// TODO: trampoline and TCO
result_value_ref_t invokeClosure(value_t *closure_value,
                                 value_array_t *arguments) {
  assert(closure_value->type == VALUE_TYPE_CLOSURE);
  closure_t closure = closure_value->as.closure;

  if (arguments->count < closure.arguments->count) {
    throw(result_value_ref_t, ERROR_CODE_TYPE_UNEXPECTED_ARITY,
          closure.form->position,
          "Unexpected arity. Expected %lu arguments, got %lu.",
          closure.arguments->count, arguments->count);
  }

  environment_t *local_environment = nullptr;
  tryWithMeta(result_value_ref_t, environmentCreate(closure.environment),
              closure_value->position, local_environment);

  for (size_t i = 0; i < closure.arguments->count; i++) {
    const value_t value = arguments->data[i];
    const char *key = closure.arguments->data[i];
    tryWithMeta(result_value_ref_t,
                environmentRegisterSymbol(local_environment, key, &value),
                closure_value->position);
  }

  result_value_ref_t result = evaluate(closure.form, local_environment);
  environmentDestroy(&local_environment);
  return result;
}

result_value_ref_t evaluate(node_t *node, environment_t *environment) {
  trampoline_t trampoline;
  position_t position = node->position;

  while (true) {
    switch (node->type) {
    case NODE_TYPE_BOOLEAN: {
      return valueCreate(VALUE_TYPE_BOOLEAN,
                         (value_as_t){.boolean = node->value.boolean},
                         position);
    }

    case NODE_TYPE_NUMBER: {
      return valueCreate(VALUE_TYPE_NUMBER,
                         (value_as_t){.number = node->value.number}, position);
    }

    case NODE_TYPE_NIL: {
      return valueCreate(VALUE_TYPE_NIL, (value_as_t){}, position);
    }

    case NODE_TYPE_STRING: {
      return valueCreate(VALUE_TYPE_STRING,
                         (value_as_t){.string = strdup(node->value.string)},
                         position);
    }

    case NODE_TYPE_SYMBOL: {
      const value_t *value =
          environmentResolveSymbol(environment, node->value.symbol);

      if (!value) {
        throw(result_value_ref_t, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND,
              position,
              "Symbol '%s' cannot be found in the current environment",
              node->value.symbol);
      }

      value_t *copy;
      try(result_value_ref_t, valueDeepCopy(value), copy);
      copy->position = position;
      return ok(result_value_ref_t, copy);
    }

    case NODE_TYPE_LIST: {
      const node_list_t list = node->value.list;

      if (list.count == 0) {
        value_array_t *array = nullptr;
        tryWithMeta(result_value_ref_t, valueArrayCreate(0), position, array);
        return valueCreate(VALUE_TYPE_LIST, (value_as_t){.list = array},
                           position);
      }

      node_t first_node = listGet(node_t, &list, 0);
      value_t *scratch = nullptr;
      try(result_value_ref_t, evaluate(&first_node, environment), scratch);

      switch (scratch->type) {
      case VALUE_TYPE_BUILTIN: {
        builtin_t builtin = scratch->as.builtin;
        value_array_t *values;
        tryCatchWithMeta(result_value_ref_t, valueArrayCreate(list.count - 1),
                         valueDestroy(&scratch), first_node.position, values);

        for (size_t i = 1; i < list.count; i++) {
          value_t *item = nullptr;
          tryCatch(
              result_value_ref_t, evaluate(&list.data[i], environment),
              {
                valueArrayDestroy(&values);
                valueDestroy(&scratch);
              },
              item);
          values->data[i - 1] = *item;
          deallocSafe(&item);
        }

        value_t *value = nullptr;
        tryFinally(
            result_value_ref_t, builtin(values, position),
            {
              valueDestroy(&scratch);
              valueArrayDestroy(&values);
            },
            value);

        return ok(result_value_ref_t, value);
      }
      case VALUE_TYPE_SPECIAL: {
        special_form_t special = scratch->as.special;
        node_array_t nodes = {.data = list.data, .count = list.count};

        value_t *value = nullptr;
        tryCatch(result_value_ref_t, special(&nodes, environment, &trampoline),
                 valueDestroy(&scratch), value);

        valueDestroy(&scratch);
        if (trampoline.more) {
          environment = trampoline.environment;
          node = trampoline.node;
          valueDestroy(&value);
          continue;
        }

        return ok(result_value_ref_t, value);
      }
      case VALUE_TYPE_CLOSURE: {
        size_t arguments_count = list.count - 1;

        value_array_t *arguments;
        tryWithMeta(result_value_ref_t, valueArrayCreate(arguments_count),
                    first_node.position, arguments);

        // skip the closure symbol
        for (size_t i = 1; i < list.count; i++) {
          value_t *item = nullptr;
          try(result_value_ref_t, evaluate(&list.data[i], environment), item);
          arguments->data[i - 1] = *item;
          deallocSafe(&item);
        }

        result_value_ref_t result = invokeClosure(scratch, arguments);

        valueArrayDestroy(&arguments);
        valueDestroy(&scratch);
        return result;
      }
      case VALUE_TYPE_BOOLEAN:
      case VALUE_TYPE_NIL:
      case VALUE_TYPE_LIST:
      case VALUE_TYPE_STRING:
      case VALUE_TYPE_NUMBER: {
        value_array_t *array;
        tryWithMeta(result_value_ref_t, valueArrayCreate(list.count),
                    first_node.position, array);

        array->data[0] = *scratch;
        deallocSafe(&scratch);

        for (size_t i = 1; i < list.count; i++) {
          value_t *item = nullptr;
          try(result_value_ref_t, evaluate(&list.data[i], environment), item);
          array->data[i] = *item;
          deallocSafe(&item);
        }

        return valueCreate(VALUE_TYPE_LIST, (value_as_t){.list = array},
                           position);
      }
      default:
        unreachable();
      }
    }
    default:
      unreachable();
    }
  }
};
