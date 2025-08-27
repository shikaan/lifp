#ifndef TEST_H
#define TEST_H

#include <math.h> // fabs
#include <stddef.h>
#include <stdio.h>  // printf, snprintf
#include <string.h> // strncmp

// Test
// ---
//
// ## Getting Started
//
// Compile the test executable returning `report()` to get the number of
// failed tests as the status code.
//
// ```c
// // example.test.c
// #include "example.h"
// #include "../test/test.h"
//
// void testExample() {
//   expectTrue(example(), "returns true");
// }
//
// int main(void) {
//   suite(testExample);
//   return report();
// }
// ```
//
// ## Custom assertions
//
// ```c
// // my_struct.test.c
// #include "my_struct.h" // myStruct_eql, myStruct_toString
// #include "../test/test.h"
//
// void expectEqlMyStruct(const MyStruct* a, const MyStruct* b, const char*
// name) {
//   char msg[256];
//   snprintf(msg, 256, "Expected %s to equal %s", myStruct_toString(a),
//   myStruct_toString(b)); expect(myStruct_eql(a, b), name, msg);
// }
//
// // define main as above
// ```

#define FLOAT_THRESHOLD 1e-6f
#define DOUBLE_THRESHOLD 1e-6f

static int total = 0;
static int failed = 0;
void expect(bool condition, const char *test_name,
            const char *failure_message) {
  total++;
  if (!condition) {
    failed++;
    printf("    fail - %s: %s\n", test_name, failure_message);
    return;
  }

#ifndef FAILED_ONLY
  printf("     ok  - %s\n", test_name);
#endif
}

void expectTrue(bool condition, const char *name) {
  expect(condition, name, "Expected value to be true");
}

void expectFalse(bool condition, const char *name) {
  expect(!condition, name, "Expected value to be false");
}

void expectNotNull(const void *a, const char *name) {
  expect(a != NULL, name, "Expected value not to be null");
}

void expectNull(const void *a, const char *name) {
  expect(a == NULL, name, "Expected value to be null");
}

void expectEqlInt(const int a, const int b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %d to equal %d", a, b);
  expect(a == b, name, msg);
}

void expectNeqInt(const int a, const int b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %d not to equal %d", a, b);
  expect(a != b, name, msg);
}

void expectEqlUint(const unsigned int a, const unsigned int b,
                   const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %d to equal %d", a, b);
  expect(a == b, name, msg);
}

void expectEqlSize(const size_t a, const size_t b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %lu to equal %lu", a, b);
  expect(a == b, name, msg);
}

void expectEqlFloat(const float a, const float b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %f to equal %f", a, b);
  expect(fabsf(a - b) < FLOAT_THRESHOLD, name, msg);
}

void expectNeqFloat(const float a, const float b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %f not to equal %f", a, b);
  expect(fabsf(a - b) >= FLOAT_THRESHOLD, name, msg);
}

void expectEqlDouble(const double a, const double b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %f to equal %f", a, b);
  expect(fabs(a - b) < DOUBLE_THRESHOLD, name, msg);
}

void expectNeqDouble(const double a, const double b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %f not to equal %f", a, b);
  expect(fabs(a - b) >= DOUBLE_THRESHOLD, name, msg);
}

void expectEqlString(const char *a, const char *b, size_t max_size,
                     const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected '%s' to equal '%s'", a, b);
  expect(strncmp(a, b, max_size) == 0, name, msg);
}

void expectNeqString(const char *a, const char *b, size_t max_size,
                     const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected '%s' not to equal '%s'", a, b);
  expect(strncmp(a, b, max_size) != 0, name, msg);
}

void expectIncludeString(const char *a, const char *b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected '%s' to include '%s'", a, b);
  expect(strstr(a, b) != NULL, name, msg);
}

int report(void) {
#ifndef FAILED_ONLY
  printf("\n%d assertions, %d failed\n", total, failed);
#else
  printf("%d assertions, %d failed\n", total, failed);
#endif
  return failed;
}

#ifndef FAILED_ONLY
#define case(name) printf("  %s:\n", name)
#else
#define case(name)
#endif

#ifndef FAILED_ONLY
#define suite(name)                                                            \
  {                                                                            \
    printf("\n> %s\n", #name);                                                 \
    name();                                                                    \
  }
#else
#define suite(name)                                                            \
  {                                                                            \
    name();                                                                    \
  }
#endif

#endif // TEST_H
