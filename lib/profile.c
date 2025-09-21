#include "profile.h"

#ifdef MEMORY_PROFILE

#include "alloc.h"
#include "arena.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

span_t *CURRENT_SAFE_ALLOC_SPAN = nullptr;
span_t *ROOT_SAFE_ALLOC_SPAN = nullptr;

span_t *CURRENT_ARENA_SPAN = nullptr;
span_t *ROOT_ARENA_SPAN = nullptr;

static unsigned long getAllocatedBytes(void) { return allocGetMetrics().bytes; }

static unsigned long hash(const char *key) {
  static constexpr unsigned long PRIME = 1099511628211U;
  unsigned long hash = 14695981039346656037U;
  const size_t len = strlen(key);

  for (size_t i = 0; i < len; i++) {
    hash ^= (unsigned long)(unsigned char)key[i];
    hash *= PRIME;
  }

  return hash;
}

static span_t *spanCreate(const char *label) {
  span_t *span = malloc(sizeof(span_t));
  span->hits = 0;
  span->total = 0;
  span->last = 0;
  span->label[0] = 0;
  span->subspans_count = 0;
  span->span_id = hash(label);
  span->payload = nullptr;

  auto label_len = strlen(label);
  label_len = label_len >= SPAN_LABEL_LEN ? SPAN_LABEL_LEN - 1 : label_len;
  strncpy(span->label, label, label_len);
  span->label[label_len] = 0;
  return span;
}

static void spanFree(span_t **span) {
  if (*span == nullptr)
    return;

  for (unsigned long i = 0; i < (*span)->subspans_count; i++) {
    spanFree(&(*span)->subspans[i]);
  }
  deallocSafe(span);
}

void profileInit(void) {
  CURRENT_SAFE_ALLOC_SPAN = spanCreate("root");
  ROOT_SAFE_ALLOC_SPAN = CURRENT_SAFE_ALLOC_SPAN;

  CURRENT_ARENA_SPAN = spanCreate("root");
  ROOT_ARENA_SPAN = CURRENT_ARENA_SPAN;

  ROOT_SAFE_ALLOC_SPAN->hits = 1; // prevent division by zero
  ROOT_ARENA_SPAN->hits = 1;      // prevent division by zero
}

span_t *safeAllocSpanStart(const char *label) {
  assert(CURRENT_SAFE_ALLOC_SPAN);
  const unsigned long span_id = hash(label);

  span_t *span = nullptr;
  for (unsigned long i = 0; i < CURRENT_SAFE_ALLOC_SPAN->subspans_count; i++) {
    span_t *subspan = CURRENT_SAFE_ALLOC_SPAN->subspans[i];
    if (subspan->span_id == span_id && strcmp(label, subspan->label) == 0) {
      span = subspan;
    }
  }

  if (!span) {
    span = spanCreate(label);
    if (CURRENT_SAFE_ALLOC_SPAN->subspans_count < MAX_SUBSPAN) {
      CURRENT_SAFE_ALLOC_SPAN
          ->subspans[CURRENT_SAFE_ALLOC_SPAN->subspans_count] = span;
      CURRENT_SAFE_ALLOC_SPAN->subspans_count++;
    } else {
      printf("Profiling error: Maximum subspans (%lu) reached for span '%s'\n",
             MAX_SUBSPAN, CURRENT_SAFE_ALLOC_SPAN->label);
      return nullptr;
    }
  }

  span->hits++;
  span->last = getAllocatedBytes();
  span->parent = CURRENT_SAFE_ALLOC_SPAN;
  CURRENT_SAFE_ALLOC_SPAN = span;
  return span;
}

span_t *arenaSpanStart(arena_t *arena, const char *label) {
  assert(CURRENT_ARENA_SPAN);
  const unsigned long span_id = hash(label);

  span_t *span = nullptr;
  for (unsigned long i = 0; i < CURRENT_ARENA_SPAN->subspans_count; i++) {
    span_t *subspan = CURRENT_ARENA_SPAN->subspans[i];
    if (subspan->span_id == span_id && strcmp(label, subspan->label) == 0) {
      span = subspan;
    }
  }

  if (!span) {
    span = spanCreate(label);
    if (CURRENT_ARENA_SPAN->subspans_count < MAX_SUBSPAN) {
      CURRENT_ARENA_SPAN->subspans[CURRENT_ARENA_SPAN->subspans_count] = span;
      CURRENT_ARENA_SPAN->subspans_count++;
    } else {
      printf("Profiling error: Maximum subspans (%lu) reached for span '%s'\n",
             MAX_SUBSPAN, CURRENT_ARENA_SPAN->label);
      return nullptr;
    }
  }

  span->hits++;
  span->last = arena->offset;
  span->parent = CURRENT_ARENA_SPAN;
  span->payload = arena;
  CURRENT_ARENA_SPAN = span;
  return span;
}

void safeAllocSpanEnd(span_t **span_double_ref) {
  span_t *span = *span_double_ref;
  unsigned long delta = getAllocatedBytes() - span->last;
  span->total += delta;

  if (span->parent) {
    span->parent->total += delta;
  }

  CURRENT_SAFE_ALLOC_SPAN = span->parent;
}

void arenaSpanEnd(span_t **span_double_ref) {
  span_t *span = *span_double_ref;
  arena_t *arena = span->payload;
  unsigned long delta = arena->offset - span->last;
  span->total += delta;

  if (span->parent) {
    span->parent->total += delta;
  }

  CURRENT_ARENA_SPAN = span->parent;
}

void printSpan(const span_t *span, int indentation) {
  char prefix[128];
  snprintf(prefix, 128, "%*c", indentation * 2, ' ');

  printf("%s %s[%lu]: %lu bytes (%0.2f bytes/hit)\n", prefix, span->label,
         span->hits, span->total, (double)span->total / (double)span->hits);

  for (unsigned long i = 0; i < span->subspans_count; i++) {
    printSpan(span->subspans[i], indentation + 1);
  }
}

void makeSpanSummary(const span_t *root, size_t *len, span_t *summary) {
  bool found = false;
  for (size_t i = 0; i < *len; i++) {
    span_t *span = &summary[i];
    if (strcmp(root->label, span->label) == 0) {
      span->hits += root->hits;
      span->total += root->total;
      found = true;
      break;
    }
  }

  if (!found) {
    if (*len == MAX_SUMMARY_SPANS) {
      printf(
          "Profiling error: Maximum number (%lu) of summary spans reached.\n",
          MAX_SUMMARY_SPANS);
      return;
    }

    span_t *new_item = &summary[*len];
    strcpy(new_item->label, root->label);
    new_item->hits = root->hits;
    new_item->total = root->total;
    (*len)++;
  }

  for (unsigned long i = 0; i < root->subspans_count; i++) {
    makeSpanSummary(root->subspans[i], len, summary);
  }
}

void printSpanSummary(const span_t *root) {
  span_t summary[MAX_SUMMARY_SPANS];
  size_t count = 0;
  makeSpanSummary(root, &count, summary);

  size_t total = 0;
  for (size_t i = 0; i < count; i++) {
    span_t span = summary[i];
    total += span.total;
    printf(" %16s: %12lu bytes %6lu hits\n", span.label, span.total, span.hits);
  }
  printf(" %16s\n", "---");
  printf(" %16s: %12lu bytes\n", "Total", total);
}

void printArenas(void) {
  size_t freed = 0;
  for (size_t i = 0; i < arena_metrics.arenas_count; i++) {
    arena_t *arena = arena_metrics.arenas[i];
    if (arena_metrics.freed[i]) {
      freed++;
    } else {
      printf("  arena[%lu]: %lu/%lu bytes (%0.2f%%)\n", i, arena->offset,
             arena->size,
             (double)(arena->offset * 100) / (double)(arena->size));
    }
  }

  printf("\n"
         "  Stats:\n"
         "    tracked:   %lu arenas\n"
         "    destroyed: %lu arenas\n",
         arena_metrics.arenas_count, freed);
}

void profileReport(void) {
  if (ROOT_SAFE_ALLOC_SPAN) {
#if MEMORY_PROFILE_SAFE_ALLOC == 1
    printf("\n === Memory Metrics: Leaked safeAlloc ===\n");
    printSpan(ROOT_SAFE_ALLOC_SPAN, 0);
#endif
  }

  if (ROOT_ARENA_SPAN) {
#if MEMORY_PROFILE_ARENA_ALLOCATIONS == 1
    printf("\n === Memory Metrics: Arena Allocations ===\n");
    printSpan(ROOT_ARENA_SPAN, 0);
#endif

#if MEMORY_PROFILE_ARENA_ALLOCATIONS_SUMMARY == 1
    printf("\n === Memory Metrics: Arena Allocation Summary ===\n");
    printSpanSummary(ROOT_ARENA_SPAN);
#endif

#if MEMORY_PROFILE_ARENA_SATURATION == 1
    printf("\n === Memory Metrics: Arena Saturation ===\n");
    printArenas();
#endif
  }
}

void profileEnd(void) {
  spanFree(&ROOT_SAFE_ALLOC_SPAN);
  CURRENT_SAFE_ALLOC_SPAN = nullptr;
  spanFree(&ROOT_ARENA_SPAN);
  CURRENT_ARENA_SPAN = nullptr;
}
#endif
