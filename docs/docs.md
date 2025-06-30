lifp - v0.4.0 (dfd2cbd)
---

## math.floor

Returns the floor of a number.

```common lisp
(math.floor 1.9) ; 1
```



## math.ceil

Returns the ceiling of a number.

```common lisp
(math.ceil 1.1) ; 2
```



## math.max

Returns the maximum value in a list of numbers.

```common lisp
(math.max (1 2 3)) ; 3
```



## math.min

Returns the minimum value in a list of numbers.

```common lisp
(math.min (1 2 3)) ; 1
```



## string.length

Returns the length of a string.

```common lisp
(string.length "hello") ; 5
```



## string.join

Joins a list of strings with a separator.

```common lisp
(string.join "," ("foo" "bar")) ; "foo,bar"
```



## string.slice

Returns a substring from start to end indices.

```common lisp
(string.slice "hello" 1 4) ; "ell"
```



## string.includes

Checks if a string contains a substring.

```common lisp
(string.includes "hello" "ell") ; true
```



## string.trim

Trims whitespace from both ends of a string.

```common lisp
(string.trim "  hello  ") ; "hello"
```



## list.count

Counts elements in a list.

```common lisp
(list.count (1 2)) ; 2
```



## list.map

Maps a lambda over a list.

```common lisp
(list.map (fn* (item idx) (+ item idx)) (1 2 3)) ; (1 3 5)
```



## list.each

Applies a lambda to each element in a list (for side effects).

```common lisp
(list.each (fn* (item idx) (print item)) (1 2 3)) ; nil
```



## list.from

Creates a list from the given arguments.

```common lisp
(list.from 1 2 3) ; (1 2 3)
```



## list.nth

Returns the nth element of a list, or nil if out of bounds.

```common lisp
(list.nth 1 (10 20 30)) ; 20
```



## list.filter

Filters a list using a lambda predicate.

```common lisp
(list.filter (fn* (item idx) (> item 0)) (-1 0 1 2)) ; (1 2)
```



## +

Adds numbers together.

```common lisp
(+ 1 2 3) ; 6
```



## -

Subtracts numbers from the first argument.

```common lisp
(- 5 2 1) ; 2
```



## *

Multiplies numbers together.

```common lisp
(* 2 3 4) ; 24
```



## /

Divides the first argument by the rest.

```common lisp
(/ 8 2 2) ; 2
```



## %

Performs division with modulo.

```common lisp
(% 4 2) ; 0
```



## =

Checks if two values are equal.

```common lisp
(= 1 1) ; true
```



## <

Checks if the first value is less than the second.

```common lisp
(< 1 2) ; true
```



## >

Checks if the first value is greater than the second.

```common lisp
(> 2 1) ; true
```



## !=

Checks if two values are not equal.

```common lisp
(!= 1 2) ; true
```



## <=

Checks if the first value is less than or equal to the second.

```common lisp
(<= 1 2) ; true
```



## >=

Checks if the first value is greater than or equal to the second.

```common lisp
(>= 2 1) ; true
```



## and

Logical AND for two boolean values.

```common lisp
(and true false) ; false
```



## or

Logical OR for two boolean values.

```common lisp
(or true false) ; true
```



## io.stdout

Stringifies an atom and writes it to stdout. Use `io.print` to format output.

```common lisp
(io.stdout "hello")
```



## io.stderr

Stringifies an atom and writes it to stderr.

```common lisp
(io.stderr "error")
```



## io.printf

Writes a formatted string to stdout. Specifiers:
 - `%s` for strings
 - `%d` for numbers
 - `%i` casts a string to an integer
 - `%f` casts a string to a float

```common lisp
(io.printf "hello %s %d" ("world" 42))
```



## io.readline

Writes the question on stdout and waits for user input.


```common lisp
(io.readline "What is your favorite food? ") ; "USER_TYPED_CONTENT"
```



## io.clear

Clear the console output.


```common lisp
(io.clear)
```



