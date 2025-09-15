#pragma once

#include <stdio.h>
#include <string.h>

constexpr char NAME[] = "lifp";
constexpr char DEFAULT_VERSION[] = "v0.0.0";
constexpr char DEFAULT_SHA[] = "dev";

// Defined as a macro to allow CI to inject version in the executable
#ifndef VERSION
#define VERSION DEFAULT_VERSION
#endif

#ifndef SHA
#define SHA DEFAULT_SHA
#endif

#define error(Fmt, ...)                                                        \
  {                                                                            \
    fprintf(stderr, "%s: ", NAME);                                             \
    fprintf(stderr, Fmt __VA_OPT__(, ) __VA_ARGS__);                           \
    fprintf(stderr, "\n");                                                     \
  }

#define tryCLI(Action, Destination, ErrorMessage)                              \
  auto _concat(result, __LINE__) = Action;                                     \
  if (_concat(result, __LINE__).code != RESULT_OK) {                           \
    error(ErrorMessage);                                                       \
    return 1;                                                                  \
  }                                                                            \
  (Destination) = _concat(result, __LINE__).value;

void formatVersion(size_t size, char buffer[static size]) {
  // NOLINTBEGIN - if macro is undefined the two values are the same, by design
  const char *version = strlen(VERSION) != 0 ? VERSION : DEFAULT_VERSION;
  const char *sha = strlen(SHA) != 0 ? SHA : DEFAULT_SHA;
  // NOLINTEND
  snprintf(buffer, size, "%s - %s (%s)", NAME, version, sha);
}
