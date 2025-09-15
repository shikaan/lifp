Profilers
---

This project hosts some profilers to ensure we can have a good handle of what
happens at any point in time and optimize/debug issues accordingly.

## Guidelines

Profilers must be designed to be compiled away when not needed. They have to be
guarded with `#ifdef` clauses such that, when the build does not require so, no
symbols nor memory is used by compilers.

References:
  - [`profile.h`](../lib/profile.h) for a reference profiler;
  - [`Makefile`](../Makefile) to allow turning on your profiler with PROFILE=1;

## Span-based Profiling

Profiling is done using a span-based approach. A span is a code section enclosed
by calls to `...SpanStart` and `...SpanEnd`. There are different start and end
functions depending on the type of profiling you need.

Typically, a span is a function. Macros making use of the [`cleanup`
attribute](https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html)
ease instrumentation of a function with just one intruction at the beginning of
the function body.

For example,
```c
#include "lib/profile.h"

void myFunction() {
    profileSafeAlloc();  // Enable safeAlloc profiling for this function

    // ...rest of the code
}
```

## Available profilers

Available profilers in this project are:

* `safeAlloc`: track usage of the `safeAlloc` function to detect leaks;
* `arena`: track arena allocation and report stats on arena usage;

You can read more about them in [`profile.h`](../lib/profile.h).

## Off-the-shelf profilers

For more coarse profiling and bug detection, we make use of Address Sanitizer.

The debug flags can be consulted in [`flags.mk`](../flags.mk).

; vim: tw=80 cc=+1
