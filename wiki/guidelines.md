Code Guidelines
---

A raw list of coding guidelines for things I didn't automate yet.

## Structure

* `bin`: code that results in the binaries we ship
* `cmd`: code for commands exposed by the binary
* `lib`: potentially reusable code, not project-dependent
* `lifp`: project-specific code
* `test`: test files, harness, and utils for both lib and lifp
* `vendor`: external dependencies as git submodules

## Naming

Broadly speaking, we have three types of modules in the project: controllers,
workers, and entities.

**Controllers** are very shallow: they typically import _many_ modules and pipe
them one to another to accomplish a specific task. Currently, they all live
in the `cmd` folder.

Modules are named after the command they map to (e.g., run, repl,...)

**Workers** are deep: they encapsulate a specific unit of work that makes sense
as a whole and abstract the details of that to their consumers. They don't
directly depend on each other; they typically produce _entities_ that can be
consumed by other worker modules.

Modules are named after the action they perform; for example, `tokenize.h` is
the module that performs tokenization whose output type is a `token_t`.

**Entities** are the basic structures that make up the domain. They map to a
type and have a few methods to manage them. Typically they feature a
`nameOfTheEntityCreate` method a `...copy` and a `...clone` method.

Modules are names after the type they expose; for exampe, `node.h` for `node_t`.

## Error Handling

Happens by means of the `result.h` module: each action that can fail is wrapped
in a `Result` type that the caller needs to handle. Error are surfaced by the
`throw` macro, and handled with the `try...` macros.

Errors thrown by the files in `lifp/*` all feature a `position_t` meta object,
for the error renderer to give a pointer on where the exception occurred.

Errors thrown from anywhere else will not have a position, and need to be
enriched with metadata in the call site. For example:

```c
// ... code

 tryWithMeta(result_type_t, listCreate(...)) // enriches the error with meta
```

`tryCatch` will execute the associated cleanup function only in case of errors.
`tryFinally` will execute the cleanup function regardless of the outcome.

; vim: tw=80 cc=+1
