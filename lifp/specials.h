#pragma once

#include "value.h"

constexpr char DEFINE[] = "def!";
result_void_position_t define(value_t *result, arena_t *temp_arena,
                              environment_t *env, const node_list_t *nodes);

constexpr char FUNCTION[] = "fn";
result_void_position_t function(value_t *result, arena_t *temp_arena,
                                environment_t *env, const node_list_t *nodes);
constexpr char LET[] = "let";
result_void_position_t let(value_t *result, arena_t *temp_arena,
                           environment_t *env, const node_list_t *nodes);
constexpr char COND[] = "cond";
result_void_position_t cond(value_t *result, arena_t *temp_arena,
                            environment_t *env, const node_list_t *nodes);