#pragma once

#include "value.h"

constexpr char DEFINE[] = "def!";
result_value_ref_t define(const node_array_t *, environment_t *,
                          trampoline_t *);

constexpr char FUNCTION[] = "fn";
result_value_ref_t function(const node_array_t *, environment_t *,
                            trampoline_t *);

constexpr char LET[] = "let";
result_value_ref_t let(const node_array_t *, environment_t *, trampoline_t *);

constexpr char COND[] = "cond";
result_value_ref_t cond(const node_array_t *, environment_t *, trampoline_t *);
