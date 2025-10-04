#pragma once

// Profile (v0.0.1)
//
// Span-based memory profilers to catch leaks and track allocations.
//
// !!! All profilers must be able to be compiled away with a flag !!!
//
// In this case, the flag is `MEMORY_PROFILE`.
//
// `arena` tracks arena utilization. Arenas by definition cannot leak,
// so this profiler is just there to see where memory gets allocated.
// Tracking is done by checking the arena offset across the span
//
// ```c
// void myFunction(arena_t *arena) {
//   // Profile the whole function
//   profileArena(arena);
//
//   // Profile a specific section
//   span_t span = arenaSpanStart(arena, "label");
//     // profiled code
//   arenaSpanEnd(&span);
// }
// ```
//

#ifdef MEMORY_PROFILE

// Update these values for finer reports
#define MEMORY_PROFILE_ARENA_ALLOCATIONS 0
#define MEMORY_PROFILE_ARENA_ALLOCATIONS_SUMMARY 0
#define MEMORY_PROFILE_ARENA_SATURATION 0

#include "arena.h"
#include "result.h"

constexpr size_t MAX_SUBSPAN = 16;
constexpr size_t SPAN_LABEL_LEN = 16;
constexpr size_t MAX_SUMMARY_SPANS = 16;

typedef struct span_t {
  char label[SPAN_LABEL_LEN];
  unsigned long hits;
  unsigned long last;
  unsigned long total;
  unsigned long subspans_count;
  unsigned long span_id;
  struct span_t *parent;
  struct span_t *subspans[MAX_SUBSPAN];
  void *payload;
} span_t;

void profileInit(void);
void profileReport(void);
void profileEnd(void);

#define profileArena(Arena)                                                    \
  span_t *_concat(metrics_, __LINE__) __attribute__((cleanup(arenaSpanEnd))) = \
      arenaSpanStart(Arena, __FUNCTION__);

#else

typedef void span_t;

#define profileReport()
#define profileInit()
#define profileEnd()

#define arenaSpanStart(Arena, Label)                                           \
  {                                                                            \
  }
#define arenaSpanEnd(Span) (void)Span

#define profileArena(Arena)

#endif
