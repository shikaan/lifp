#pragma once

#include "position.h"
#include "value.h"

void formatErrorMessage(message_t, position_t, const char *, const char *,
                        int size, char output_buffer[static size], int *);

void formatValue(const value_t *, int size, char output_buffer[static size],
                 int *);

const char *formatValueType(value_type_t);
