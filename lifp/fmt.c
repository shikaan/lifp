#include "types.h"
#define _POSIX_C_SOURCE 200809L

#include "../lib/alloc.h"
#include "../lib/list.h"
#include "fmt.h"
#include "node.h"
#include "position.h"
#include "value.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define ensurePositive(n) (size_t)(n < 0 ? 0 : n)

#define append(Size, Buffer, Offset, ...)                                      \
  if (Size > *Offset) {                                                        \
    *Offset +=                                                                 \
        snprintf(Buffer + *Offset, (size_t)(Size - *Offset), __VA_ARGS__);     \
  }

static void formatCurrentLine(position_t caret, const char *input_buffer,
                              int size, char output_buffer[static size],
                              int *offset) {
  const char *separator = "\n";
  char *ctx = nullptr;
  char *line = nullptr;
  char *copy = strdup(input_buffer);
  size_t current_line = 0;

  for (line = strtok_r(copy, separator, &ctx); line;
       line = strtok_r(nullptr, separator, &ctx)) {
    current_line++;
    if (caret.line == current_line)
      break;
  }

  append(size, output_buffer, offset, "\n\n");
  int identation = *offset;
  append(size, output_buffer, offset, "%lu | ", caret.line);
  identation = *offset - identation;
  append(size, output_buffer, offset, "%s", line);
  append(size, output_buffer, offset, "\n%*c^\n",
         (int)caret.column - 1 + identation, ' ');
  deallocSafe(&copy);
}

static void formatNode(const node_t *node, int size, char buffer[static size],
                       int *offset) {
  switch (node->type) {
  case NODE_TYPE_BOOLEAN: {
    append(size, buffer, offset, "%s", node->value.boolean ? "true" : "false");
    return;
  }
  case NODE_TYPE_NIL: {
    append(size, buffer, offset, "nil");
    return;
  }
  case NODE_TYPE_NUMBER: {
    append(size, buffer, offset, "%g", node->value.number);
    return;
  }
  case NODE_TYPE_SYMBOL: {
    append(size, buffer, offset, "%s", node->value.symbol);
    return;
  }
  case NODE_TYPE_STRING: {
    append(size, buffer, offset, "\"%s\"", node->value.string);
    return;
  }
  case NODE_TYPE_LIST: {
    append(size, buffer, offset, "(");
    node_list_t list = node->value.list;

    if (list.count > 0) {
      for (size_t i = 0; i < list.count - 1; i++) {
        node_t sub_node = listGet(node_t, &list, i);
        formatNode(&sub_node, size, buffer, offset);
        append(size, buffer, offset, " ");
      }

      node_t sub_node = listGet(node_t, &list, list.count - 1);
      formatNode(&sub_node, size, buffer, offset);
    }
    append(size, buffer, offset, ")");
    return;
  }
  default:
    return;
  }
}

void formatErrorMessage(message_t message, position_t position,
                        const char *file_name, const char *input_buffer,
                        int size, char output_buffer[static size],
                        int *offset) {
  append(size, output_buffer, offset, "Error: %s", message);

  formatCurrentLine(position, input_buffer, size, output_buffer, offset);
  append(size, output_buffer, offset, "  at %s:%lu:%lu", file_name,
         position.line, position.column);
}

void formatValue(const value_t *value, int size,
                 char output_buffer[static size], int *offset) {
  switch (value->type) {
  case VALUE_TYPE_BOOLEAN: {
    append(size, output_buffer, offset, "%s",
           value->as.boolean ? "true" : "false");
    return;
  }
  case VALUE_TYPE_NIL: {
    append(size, output_buffer, offset, "nil");
    return;
  }
  case VALUE_TYPE_NUMBER: {
    append(size, output_buffer, offset, "%g", value->as.number);
    return;
  }
  case VALUE_TYPE_BUILTIN: {
    append(size, output_buffer, offset, "#<builtin>");
    return;
  }
  case VALUE_TYPE_SPECIAL: {
    append(size, output_buffer, offset, "#<special>");
    return;
  }
  case VALUE_TYPE_STRING: {
    append(size, output_buffer, offset, "\"%s\"", value->as.string);
    return;
  }
  case VALUE_TYPE_LIST: {
    append(size, output_buffer, offset, "(");
    value_array_t *list = value->as.list;

    if (list->count > 0) {
      for (size_t i = 0; i < list->count - 1; i++) {
        value_t sub_value = listGet(value_t, list, i);
        formatValue(&sub_value, size, output_buffer, offset);
        append(size, output_buffer, offset, " ");
      }

      value_t sub_value = listGet(value_t, list, list->count - 1);
      formatValue(&sub_value, size, output_buffer, offset);
    }
    append(size, output_buffer, offset, ")");
    return;
  }
  case VALUE_TYPE_CLOSURE:
    append(size, output_buffer, offset, "(fn (");
    arguments_t *arguments = value->as.closure.arguments;

    if (arguments->count > 0) {
      for (size_t i = 0; i < arguments->count - 1; i++) {
        string_t sub_node = arguments->data[i];
        append(size, output_buffer, offset, "%s ", sub_node);
      }

      string_t sub_node = arguments->data[arguments->count - 1];
      append(size, output_buffer, offset, "%s", sub_node);
    }
    append(size, output_buffer, offset, ") ");

    formatNode(value->as.closure.form, size, output_buffer, offset);
    append(size, output_buffer, offset, ")");
  default:
  }
}

const char *formatValueType(value_type_t type) {
  switch (type) {
  case VALUE_TYPE_BOOLEAN:
    return "boolean";
  case VALUE_TYPE_NUMBER:
    return "number";
  case VALUE_TYPE_BUILTIN:
    return "builtin";
  case VALUE_TYPE_SPECIAL:
    return "special";
  case VALUE_TYPE_CLOSURE:
    return "function";
  case VALUE_TYPE_NIL:
    return "nil";
  case VALUE_TYPE_LIST:
    return "list";
  case VALUE_TYPE_STRING:
    return "string";
  default:
    unreachable();
  }
}

#undef append
