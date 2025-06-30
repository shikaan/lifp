import { expect, mock, test } from "bun:test";
import { l, n } from "../tests/utils.js";
import { FN } from "./constants.ts";
import { Environment } from "./environment.js";
import { evaluate } from "./evaluate.js";
import type { specials } from "./specials.ts";
import { type Lambda, NodeType } from "./types.js";

test("number literal", () => {
  const env = new Environment();
  expect(evaluate(n(NodeType.NUMBER, 42), env)).resolves.toEqual(42);
});

test("string literal", () => {
  const env = new Environment();
  expect(evaluate(n(NodeType.STRING, "hi"), env)).resolves.toEqual("hi");
});

test("boolean literal", () => {
  const env = new Environment();
  expect(evaluate(n(NodeType.BOOLEAN, true), env)).resolves.toEqual(true);
});

test("nil literal", () => {
  const env = new Environment();
  expect(evaluate(n(NodeType.NIL, null), env)).resolves.toBeNull();
});

test("resolved symbols", () => {
  const env = new Environment();
  env.set("foo", 99);
  expect(evaluate(n(NodeType.SYMBOL, "foo"), env)).resolves.toEqual(99);
});

test("throws when symbol is not found", () => {
  const env = new Environment();
  expect(() => evaluate(n(NodeType.SYMBOL, "bar"), env)).toThrow();
});

test("resolved lambdas", () => {
  const fn: Lambda = async ([value]) => (value as number) + 1;
  const env = new Environment();
  env.set("inc", fn);
  const tree = l([n(NodeType.SYMBOL, "inc"), n(NodeType.NUMBER, 2)]);
  expect(evaluate(tree, env)).resolves.toEqual(3);
});

test("call special forms", () => {
  const specs = { [FN]: mock() } as unknown as typeof specials;
  const env = new Environment();
  const tree = l([n(NodeType.SYMBOL, FN), n(NodeType.NUMBER, 1)]);
  evaluate(tree, env, specs);
  expect(specs[FN]).toBeCalled();
});

test("empty list returns itself", () => {
  const env = new Environment();
  const tree = l([]);
  expect(evaluate(tree, env)).resolves.toEqual([]);
});

test("fallback list evaluation", () => {
  const env = new Environment();
  const tree = l([n(NodeType.NUMBER, 1), n(NodeType.NUMBER, 2)]);
  expect(evaluate(tree, env)).resolves.toEqual([1, 2]);
});
