import { test, expect, mock } from "bun:test";
import { evaluate } from "./evaluate.js";
import { ASTNodeType, type Expression } from "./types.js";
import { n, l } from "../tests/utils.js";
import { Environment } from "./environment.js";
import { FN } from "./constants.ts";
import { specials } from "./specials.ts";

test("number literal", () => {
  const env = new Environment();
  expect(evaluate(n(ASTNodeType.NUMBER, 42), env)).toEqual(
    n(ASTNodeType.NUMBER, 42),
  );
});

test("string literal", () => {
  const env = new Environment();
  expect(evaluate(n(ASTNodeType.STRING, "hi"), env)).toEqual(
    n(ASTNodeType.STRING, "hi"),
  );
});

test("boolean literal", () => {
  const env = new Environment();
  expect(evaluate(n(ASTNodeType.BOOLEAN, true), env)).toEqual(
    n(ASTNodeType.BOOLEAN, true),
  );
});

test("nil literal", () => {
  const env = new Environment();
  expect(evaluate(n(ASTNodeType.NIL, null), env)).toEqual(
    n(ASTNodeType.NIL, null),
  );
});

test("resolved symbols", () => {
  const env = new Environment();
  env.set("foo", n(ASTNodeType.NUMBER, 99));
  expect(evaluate(n(ASTNodeType.SYMBOL, "foo"), env)).toEqual(
    n(ASTNodeType.NUMBER, 99),
  );
});

test("throws when symbol is not found", () => {
  const env = new Environment();
  expect(() => evaluate(n(ASTNodeType.SYMBOL, "bar"), env)).toThrow();
});

test("resolved lambdas", () => {
  // @ts-expect-error sloppy types for test purposes
  const fn = (nodes: Expression[]) => n(ASTNodeType.NUMBER, nodes[0].value + 1);
  const env = new Environment();
  env.set("inc", { type: ASTNodeType.FUNCTION, value: fn });
  const tree = l([n(ASTNodeType.SYMBOL, "inc"), n(ASTNodeType.NUMBER, 2)]);

  expect(evaluate(tree, env)).toEqual(n(ASTNodeType.NUMBER, 3));
});

test("call special forms", () => {
  const specs = { [FN]: mock() } as unknown as typeof specials;
  const env = new Environment();
  const tree = l([n(ASTNodeType.SYMBOL, FN), n(ASTNodeType.NUMBER, 1)]);
  evaluate(tree, env, specs);
  expect(specs[FN]).toBeCalled();
});

test("empty list returns itself", () => {
  const env = new Environment();
  const tree = l([]);
  expect(evaluate(tree, env)).toEqual(tree);
});

test("fallback list evaluation", () => {
  const env = new Environment();
  const tree = l([n(ASTNodeType.NUMBER, 1), n(ASTNodeType.NUMBER, 2)]);
  expect(evaluate(tree, env)).toEqual(
    l([n(ASTNodeType.NUMBER, 1), n(ASTNodeType.NUMBER, 2)]),
  );
});
