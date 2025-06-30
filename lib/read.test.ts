import { expect, test } from "bun:test";
import { l, mockPtr, n } from "../tests/utils.js";
import { read } from "./read.js";
import { type Node, NodeType } from "./types.js";

test("reads atoms", () => {
  const tests: [string, Node][] = [
    ["1", n(NodeType.NUMBER, 1)],
    ["true", n(NodeType.BOOLEAN, true)],
    ["false", n(NodeType.BOOLEAN, false)],
    ['"lol"', n(NodeType.STRING, "lol")],
    ["add-one", n(NodeType.SYMBOL, "add-one")],
    ["nil", n(NodeType.NIL, null)],
  ];

  for (const [input, expected] of tests) {
    expect(read(input, mockPtr), input).toEqual(expected);
  }
});

test("reads unary lists", () => {
  const tests: [string, Node][] = [
    ["(1)", l([n(NodeType.NUMBER, 1)])],
    ["(true)", l([n(NodeType.BOOLEAN, true)])],
    ['("true")', l([n(NodeType.STRING, "true")])],
    ["(nil)", l([n(NodeType.NIL, null)])],
  ];

  for (const [input, expected] of tests) {
    expect(read(input, mockPtr), input).toEqual(expected);
  }
});

test("reads empty list", () => {
  expect(read("()", mockPtr)).toEqual(n(NodeType.LIST, []));
});

test("reads complex lists", () => {
  const tests: [string, Node][] = [
    [
      "(add 1 2)",
      l([
        n(NodeType.SYMBOL, "add"),
        n(NodeType.NUMBER, 1),
        n(NodeType.NUMBER, 2),
      ]),
    ],
    [
      "(def! a 2)",
      l([
        n(NodeType.SYMBOL, "def!"),
        n(NodeType.SYMBOL, "a"),
        n(NodeType.NUMBER, 2),
      ]),
    ],
    [
      '(concat "hel" ("l" "o"))',
      l([
        n(NodeType.SYMBOL, "concat"),
        n(NodeType.STRING, "hel"),
        l([n(NodeType.STRING, "l"), n(NodeType.STRING, "o")]),
      ]),
    ],
    [
      '(key 1 (nil "l"))',
      l([
        n(NodeType.SYMBOL, "key"),
        n(NodeType.NUMBER, 1),
        l([n(NodeType.NIL, null), n(NodeType.STRING, "l")]),
      ]),
    ],
    [
      "(add 1 (1 2) (3 (4)))",
      l([
        n(NodeType.SYMBOL, "add"),
        n(NodeType.NUMBER, 1),
        l([n(NodeType.NUMBER, 1), n(NodeType.NUMBER, 2)]),
        l([n(NodeType.NUMBER, 3), l([n(NodeType.NUMBER, 4)])]),
      ]),
    ],
  ];

  for (const [input, expected] of tests) {
    expect(read(input, ["", 0]), input).toEqual(expected);
  }
});
