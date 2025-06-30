import { expect, test } from "bun:test";
import { strings } from "./strings.js";

test("string.length", () => {
  expect(strings["string.length"](["abc"])).resolves.toBe(3);
  expect(() => strings["string.length"]([1])).toThrow();
});

test("string.join", () => {
  expect(strings["string.join"]([" ", ["a", "b"]])).resolves.toBe("a b");
  expect(() => strings["string.join"](["a"])).toThrow();
  expect(() => strings["string.join"]([["a", 1]])).toThrow();
});

test("string.slice", () => {
  expect(strings["string.slice"](["abcdef", 1, 4])).resolves.toBe("bcd");
  expect(() => strings["string.slice"](["a", 1])).toThrow();
});

test("string.includes", () => {
  expect(strings["string.includes"](["abc", "b"])).resolves.toBe(true);
  expect(strings["string.includes"](["abc", "z"])).resolves.toBe(false);
  expect(() => strings["string.includes"](["a", 1])).toThrow();
});

test("string.trim", () => {
  expect(strings["string.trim"](["  abc  "])).resolves.toBe("abc");
  expect(() => strings["string.trim"]([1])).toThrow();
});
