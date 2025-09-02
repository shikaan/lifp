#pragma once

#include <stddef.h>
#include <string.h>

// This is a portable re-implementation of strlcpy
static inline size_t stringCopy(char *dst, const char *src, size_t size) {
  size_t srclen = strlen(src);

  if (size > 0) {
    size_t copylen = (srclen >= size) ? size - 1 : srclen;
    memcpy(dst, src, copylen);
    dst[copylen] = '\0';
  }
  return srclen;
}