import { expect, test } from "bun:test";
import { l, n } from "../../tests/utils.js";
import { strings } from "./strings.js";
import { ASTNodeType } from "../types.js";

test("string.length", () => {
  expect(strings["string.length"]([n(ASTNodeType.STRING, "abc")])).toEqual(
    n(ASTNodeType.NUMBER, 3),
  );
  expect(() => strings["string.length"]([n(ASTNodeType.NUMBER, 1)])).toThrow();
});

test("string.join", () => {
  const join = strings["string.join"];
  expect(
    join([
      n(ASTNodeType.STRING, " "),
      l([n(ASTNodeType.STRING, "a"), n(ASTNodeType.STRING, "b")]),
    ]),
  ).toEqual(n(ASTNodeType.STRING, "a b"));
  expect(() => join([n(ASTNodeType.STRING, "a")])).toThrow();
  expect(() =>
    join([l([n(ASTNodeType.STRING, "a"), n(ASTNodeType.NUMBER, 1)])]),
  ).toThrow();
});

test("string.slice", () => {
  const slice = strings["string.slice"];
  expect(
    slice([
      n(ASTNodeType.STRING, "abcdef"),
      n(ASTNodeType.NUMBER, 1),
      n(ASTNodeType.NUMBER, 4),
    ]),
  ).toEqual(n(ASTNodeType.STRING, "bcd"));
  expect(() =>
    slice([n(ASTNodeType.STRING, "a"), n(ASTNodeType.NUMBER, 1)]),
  ).toThrow();
});

test("string.includes", () => {
  expect(
    strings["string.includes"]([
      n(ASTNodeType.STRING, "abc"),
      n(ASTNodeType.STRING, "b"),
    ]),
  ).toEqual(n(ASTNodeType.BOOLEAN, true));
  expect(
    strings["string.includes"]([
      n(ASTNodeType.STRING, "abc"),
      n(ASTNodeType.STRING, "z"),
    ]),
  ).toEqual(n(ASTNodeType.BOOLEAN, false));
  expect(() =>
    strings["string.includes"]([
      n(ASTNodeType.STRING, "a"),
      n(ASTNodeType.NUMBER, 1),
    ]),
  ).toThrow();
});

test("string.trim", () => {
  expect(strings["string.trim"]([n(ASTNodeType.STRING, "  abc  ")])).toEqual(
    n(ASTNodeType.STRING, "abc"),
  );
  expect(() => strings["string.trim"]([n(ASTNodeType.NUMBER, 1)])).toThrow();
});
