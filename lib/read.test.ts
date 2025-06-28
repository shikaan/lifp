import { expect, test } from "bun:test";
import { l, n } from "../tests/utils.js";
import { read } from "./read.js";
import { type ASTNode, ASTNodeType } from "./types.js";

test("reads atoms", () => {
  const tests: [string, ASTNode][] = [
    ["1", n(ASTNodeType.NUMBER, 1)],
    ["true", n(ASTNodeType.BOOLEAN, true)],
    ["false", n(ASTNodeType.BOOLEAN, false)],
    ['"lol"', n(ASTNodeType.STRING, "lol")],
    ["add-one", n(ASTNodeType.SYMBOL, "add-one")],
    ["nil", n(ASTNodeType.NIL, null)],
  ];

  for (const [input, expected] of tests) {
    expect(read(input), input).toEqual(expected);
  }
});

test("reads unary lists", () => {
  const tests: [string, ASTNode][] = [
    ["(1)", l([n(ASTNodeType.NUMBER, 1)])],
    ["(true)", l([n(ASTNodeType.BOOLEAN, true)])],
    ['("true")', l([n(ASTNodeType.STRING, "true")])],
    ["(nil)", l([n(ASTNodeType.NIL, null)])],
  ];

  for (const [input, expected] of tests) {
    expect(read(input), input).toEqual(expected);
  }
});

test("reads empty list", () => {
  expect(read("()")).toEqual(n(ASTNodeType.LIST, []));
});

test("reads complex lists", () => {
  const tests: [string, ASTNode][] = [
    [
      "(add 1 2)",
      l([
        n(ASTNodeType.SYMBOL, "add"),
        n(ASTNodeType.NUMBER, 1),
        n(ASTNodeType.NUMBER, 2),
      ]),
    ],
    [
      "(def! a 2)",
      l([
        n(ASTNodeType.SYMBOL, "def!"),
        n(ASTNodeType.SYMBOL, "a"),
        n(ASTNodeType.NUMBER, 2),
      ]),
    ],
    [
      '(concat "hel" ("l" "o"))',
      l([
        n(ASTNodeType.SYMBOL, "concat"),
        n(ASTNodeType.STRING, "hel"),
        l([n(ASTNodeType.STRING, "l"), n(ASTNodeType.STRING, "o")]),
      ]),
    ],
    [
      '(key 1 (nil "l"))',
      l([
        n(ASTNodeType.SYMBOL, "key"),
        n(ASTNodeType.NUMBER, 1),
        l([n(ASTNodeType.NIL, null), n(ASTNodeType.STRING, "l")]),
      ]),
    ],
    [
      "(add 1 (1 2) (3 (4)))",
      l([
        n(ASTNodeType.SYMBOL, "add"),
        n(ASTNodeType.NUMBER, 1),
        l([n(ASTNodeType.NUMBER, 1), n(ASTNodeType.NUMBER, 2)]),
        l([n(ASTNodeType.NUMBER, 3), l([n(ASTNodeType.NUMBER, 4)])]),
      ]),
    ],
  ];

  for (const [input, expected] of tests) {
    expect(read(input), input).toEqual(expected);
  }
});
