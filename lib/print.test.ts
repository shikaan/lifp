import { test, expect } from "bun:test";
import { AST, Symbol, TokenTag } from "./types.ts";
import { format } from "./print.ts";

type TestCase = [string, AST, string];

test("unary list", () => {
  const tests: TestCase[] = [
    ["(1)", [[TokenTag.NUMBER, 1]], "number"],
    ['("lol")', [[TokenTag.STRING, "lol"]], "string"],
    ["(true)", [[TokenTag.BOOLEAN, true]], "boolean"],
    ["(!)", [[TokenTag.SYMBOL, Symbol.NOT]], "operator"],
    ['(("lol"))', [[[TokenTag.STRING, "lol"]]], "nested"],
  ];

  for (const [expected, input, name] of tests) {
    expect(format(input), name).toEqual(expected);
  }
});

test("multi-item list", () => {
  const tests: TestCase[] = [
    [
      "(+ 1 2)",
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 2],
      ],
      "binary operator",
    ],
    [
      "(+ (1) 2)",
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [[TokenTag.NUMBER, 1]],
        [TokenTag.NUMBER, 2],
      ],
      "binary operator nested list",
    ],
    [
      "(+ (1 (2)) 3)",
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [[TokenTag.NUMBER, 1], [[TokenTag.NUMBER, 2]]],
        [TokenTag.NUMBER, 3],
      ],
      "binary operator nested list",
    ],
  ];

  for (const [expected, input, name] of tests) {
    expect(format(input), name).toEqual(expected);
  }
});
