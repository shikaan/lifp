#pragma once

#include "position.h"
#include "value.h"

void formatErrorMessage(message_t message, position_t position,
                        const char *file_name, const char *input_buffer,
                        int size, char output_buffer[static size], int *offset);

void formatValue(const value_t *value, int size,
                 char output_buffer[static size], int *offset);

const char *formatValueType(value_type_t type);