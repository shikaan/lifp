lifp - v0.5.1 (5646b17)
---

## math.floor

Returns the floor of a number.

```lisp
(math.floor 1.9) ; 1
```



## math.ceil

Returns the ceiling of a number.

```lisp
(math.ceil 1.1) ; 2
```



## math.max

Returns the maximum value in a list of numbers.

```lisp
(math.max (1 2 3)) ; 3
```



## math.min

Returns the minimum value in a list of numbers.

```lisp
(math.min (1 2 3)) ; 1
```



## math.random

Returns a random number between 0 (inclusive) and 1 (exclusive).

```lisp
(math.random) ; 0.123456
```



## string.length

Returns the length of a string.

```lisp
(string.length "hello") ; 5
```



## string.join

Joins a list of strings with a separator.

```lisp
(string.join "," ("foo" "bar")) ; "foo,bar"
```



## string.slice

Returns a substring from start to end indices.

```lisp
(string.slice "hello" 1 4) ; "ell"
```



## string.includes

Checks if a string contains a substring.

```lisp
(string.includes "hello" "ell") ; true
```



## string.trim

Trims whitespace from both ends of a string.

```lisp
(string.trim "  hello  ") ; "hello"
```



## flow.sleep

Sleeps for the given number of milliseconds.

```lisp
(flow.sleep 1000) ; sleeps for ~1 second
```



## list.count

Counts elements in a list.

```lisp
(list.count (1 2)) ; 2
```



## list.map

Maps a lambda over a list.

```lisp
(list.map (fn* (item idx) (+ item idx)) (1 2 3)) ; (1 3 5)
```



## list.each

Applies a lambda to each element in a list (for side effects).

```lisp
(list.each (fn* (item idx) (print item)) (1 2 3)) ; nil
```



## list.from

Creates a list from the given arguments.

```lisp
(list.from 1 2 3) ; (1 2 3)
```



## list.nth

Returns the nth element of a list, or nil if out of bounds.

```lisp
(list.nth 1 (10 20 30)) ; 20
```



## list.filter

Filters a list using a lambda predicate.

```lisp
(list.filter (fn* (item idx) (> item 0)) (-1 0 1 2)) ; (1 2)
```



## +

Adds numbers together.

```lisp
(+ 1 2 3) ; 6
```



## -

Subtracts numbers from the first argument.

```lisp
(- 5 2 1) ; 2
```



## *

Multiplies numbers together.

```lisp
(* 2 3 4) ; 24
```



## /

Divides the first argument by the rest.

```lisp
(/ 8 2 2) ; 2
```



## %

Performs division with modulo.

```lisp
(% 4 2) ; 0
```



## =

Checks if two values are equal.

```lisp
(= 1 1) ; true
```



## <

Checks if the first value is less than the second.

```lisp
(< 1 2) ; true
```



## >

Checks if the first value is greater than the second.

```lisp
(> 2 1) ; true
```



## !=

Checks if two values are not equal.

```lisp
(!= 1 2) ; true
```



## <=

Checks if the first value is less than or equal to the second.

```lisp
(<= 1 2) ; true
```



## >=

Checks if the first value is greater than or equal to the second.

```lisp
(>= 2 1) ; true
```



## and

Logical AND for two boolean values.

```lisp
(and true false) ; false
```



## or

Logical OR for two boolean values.

```lisp
(or true false) ; true
```



## io.stdout

Stringifies an atom and writes it to stdout. Use `io.print` to format output.

```lisp
(io.stdout "hello")
```



## io.stderr

Stringifies an atom and writes it to stderr.

```lisp
(io.stderr "error")
```



## io.printf

Writes a formatted string to stdout. Specifiers:
 - `%s` for strings
 - `%d` for numbers
 - `%i` casts a string to an integer
 - `%f` casts a string to a float

```lisp
(io.printf "hello %s %d" ("world" 42))
```



## io.readline

Writes the question on stdout and waits for user input.


```lisp
(io.readline "What is your favorite food? ") ; "USER_TYPED_CONTENT"
```



## io.clear

Clear the console output.


```lisp
(io.clear)
```



