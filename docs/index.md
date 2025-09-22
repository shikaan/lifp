lifp - v0.2.1 (63e3102)
---
### Table of Contents
  * [core](#core)
  * [flow](#flow)
  * [io](#io)
  * [list](#list)
  * [math](#math)
  * [str](#str)

# core

Core lifp operators. For all intents and purposes, these should be thought as
language keywords.

```lisp
(and true false) ; returns false
```
### %

Performs a modulo division.

```lisp
(% 6 3) ; returns 0
```


### *

Multiplies a sequence of numbers.

```lisp
(* 1 2 3) ; returns 6
```


### +

Sums a sequence of numbers.

```lisp
(+ 1 2 3) ; returns 6
```


### -

Subtracts a sequence of numbers from the first.

```lisp
(- 6 3 2) ; returns 1
```


### /

Divides the first number by the rest.

```lisp
(/ 6 3 2) ; returns 1
```


### <

Checks if the first value is less than the second.

```lisp
(< 1 6) ; returns true
```


### <=

Checks if the first value is less than or equal to the second.

```lisp
(<= 1 6) ; returns true
```


### <>

Checks if two values are not equal.

```lisp
(<> 6 6) ; returns false
```


### =

Checks if two values are equal.

```lisp
(= 6 6) ; returns true
```


### >

Checks if the first value is greater than the second.

```lisp
(> 1 6) ; returns false
```


### >=

Checks if the first value is greater than or equal to the second.

```lisp
(>= 6 1) ; returns true
```


### and

Logical AND operation on two boolean values.

```lisp
(and true false) ; returns false
```


### or

Logical OR operation on two boolean values.

```lisp
(or true false) ; returns true
```


# flow

Flow control utilities for lifp. These functions provide basic control over
program execution, such as pausing execution for a specified duration.

```lisp
(flow:sleep! 1000) ; pauses execution for ~1 second
```
### flow:sleep!

Suspends execution for a given number of milliseconds.

```lisp
(flow:sleep! 1000) ; pauses for ~1 second
```


# io

Input/output utilities for lifp. These functions provide basic console IO.

```lisp
(io:stdout! "hello") ; prints to stdout
(io:stderr! "error") ; prints to stderr
(io:printf! "Hello, {}!" ["world"]) ; prints formatted string
(io:readline! "Enter your name: ") ; reads a line from stdin
(io:clear!) ; clears the terminal
```
### io:clear!

Clears the terminal screen.

```lisp
(io:clear!)
```


### io:printf!

Prints a formatted string to standard output, replacing each '{}' in the format string with the corresponding value from the list.

```lisp
(io:printf! "Hello, {}!" ("world")) ; prints "Hello, world!"
```


### io:readline!

Prints a prompt and returns the answer as a string.

```lisp
(io:readline! "What's your name?") ; returns user input
```


### io:stderr!

Prints a value to standard error.

```lisp
(io:stderr! "error")
```


### io:stdout!

Prints a value to standard output.

```lisp
(io:stdout! "hello")
```


# list

List manipulation operators for lifp. These functions provide core list
operations such as counting, creating, accessing, mapping, filtering, and
iterating over lists.

```lisp
(list:count (list:from 1 2 3)) ; returns 3
(list:nth 1 (list:from 10 20 30)) ; returns 20
(list:map (fn (x i) (* x 2)) (list:from 1 2 3)) ; returns (2 4 6)
```
### list:count

Counts the number of elements in a list.

```lisp
(list:count (1 2 3)) ; returns 3
```


### list:each

Applies a function to each element of a list for side effects. The function receives each element and its index.

```lisp
(list:each (fn (x i) (print x)) (1 2 3))
```


### list:filter

Filters a list using a predicate function. The function receives each element and its index, and should return a boolean. true.

```lisp
(list:filter (fn (x i) (> x 1)) (1 2 3)) ; returns (2 3)
```


### list:from

Creates a list from the given arguments.

```lisp
(list:from 1 2 3) ; returns (1 2 3)
```


### list:map

Maps a function over a list, returning a new list of results. The function receives each element and its index.

```lisp
(list:map (fn (x i) (* x 2)) (1 2 3)) ; returns (2 4 6)
```


### list:nth

Returns the nth element of a list, or nil if out of bounds.

```lisp
(list:nth 1 (10 20 30)) ; returns 20
```


### list:times

Calls a function a given number of times, collecting the results in a list. The function receives the current index.

```lisp
(list:times (fn (i) (* i 2)) 3) ; returns (0 2 4)
```


# math

Math operators for lifp. Provides mathematical utilities such as min, max,
random, ceil, and floor for working with numbers and lists of numbers.

```lisp
(math:max (list:from 1 2 3)) ; returns 3
(math:min (list:from 1 2 3)) ; returns 1
(math:random!) ; returns a random number between 0 and 1
(math:ceil 2.3) ; returns 3
(math:floor 2.7) ; returns 2
```
### math:ceil

Returns the smallest integer greater than or equal to the given number.

```lisp
(math:ceil 2.3) ; returns 3
```


### math:floor

Returns the largest integer less than or equal to the given number.

```lisp
(math:floor 2.7) ; returns 2
```


### math:max

Returns the maximum value in a sequence of numbers.

```lisp
(math:max 1 2 3) ; returns 3
```


### math:min

Returns the minimum value in a sequence of numbers.

```lisp
(math:min 1 2 3) ; returns 1
```


### math:random!

Returns a random number between 0 and 1.

```lisp
(math:random!) ; returns a random number between 0 and 1
```


# str

String manipulation functions for lifp.

```lisp
(str:length "hello") ; returns 5
(str:join "," ("a" "b" "c")) ; returns "a,b,c"
(str:slice "abcdef" 1 4) ; returns "bcde"
(str:include "hello world" "world") ; returns true
(str:trimLeft "   foo") ; returns "foo"
(str:trimRight "foo   ") ; returns "foo"
```
### str:include

Checks if a string contains a substring.

```lisp
(str:include "hello world" "world") ; returns true
```


### str:join

Joins a list of strings using a separator.

```lisp
(str:join "," ("a" "b" "c")) ; returns "a,b,c"
```


### str:length

Returns the length of a string.

```lisp
(str:length "hello") ; returns 5
```


### str:slice

Returns a substring from start to end (end not inclusive). Negative indices count from the end of the string.

```lisp
(str:slice "abcdef" 1 4) ; returns "bcde"
(str:slice "abcdef" 2) ; returns "cdef"
```


### str:trimLeft

Removes whitespace from the start of a string.

```lisp
(str:trimLeft "   foo") ; returns "foo"
```


### str:trimRight

Removes whitespace from the end of a string.

```lisp
(str:trimRight "foo   ") ; returns "foo"
```


