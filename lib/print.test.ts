import { expect, test } from "bun:test";
import { print } from "./print.js";
import { type Value } from "./types.js";

test("atoms", () => {
  const tests: [Value, string][] = [
    [null, "nil"],
    [1, "1"],
    ["asd", '"asd"'],
    [true, "true"],
    [() => null, "#<function>"],
  ];

  for (const [input, expected] of tests) {
    expect(print(input), expected).toEqual(expected);
  }
});

test("lists", () => {
  const tests: [Value, string][] = [
    [[null], "(nil)"],
    [[null, 1, true, "lol"], '(nil 1 true "lol")'],
    [[null, [1]], "(nil (1))"],
  ];

  for (const [input, expected] of tests) {
    expect(print(input), expected).toEqual(expected);
  }
});
