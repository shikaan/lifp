#pragma once

#include "value.h"

constexpr char DEFINE[] = "def!";
result_void_position_t define(value_t *, const node_list_t *, arena_t *,
                              environment_t *, trampoline_t *);

constexpr char FUNCTION[] = "fn";
result_void_position_t function(value_t *, const node_list_t *, arena_t *,
                                environment_t *, trampoline_t *);

constexpr char LET[] = "let";
result_void_position_t let(value_t *, const node_list_t *, arena_t *,
                           environment_t *, trampoline_t *);

constexpr char COND[] = "cond";
result_void_position_t cond(value_t *, const node_list_t *, arena_t *,
                            environment_t *, trampoline_t *);
