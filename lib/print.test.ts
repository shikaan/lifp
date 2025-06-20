import { test } from "node:test";
import { AST, Operator, TokenTag } from "./types.ts";
import { format, print } from "./print.ts";
import * as assert from "node:assert";

type TestCase = [string, AST, string];

test("unary list", () => {
  const tests: TestCase[] = [
    ["(1)", [[TokenTag.NUMBER, 1]], "number"],
    ['("lol")', [[TokenTag.STRING, "lol"]], "string"],
    ["(true)", [[TokenTag.BOOLEAN, true]], "boolean"],
    ["(not)", [[TokenTag.OPERATOR, Operator.NOT]], "operator"],
    ['(("lol"))', [[[TokenTag.STRING, "lol"]]], "nested"],
  ];

  for (const [expected, input, name] of tests) {
    assert.deepStrictEqual(format(input), expected, name);
  }
});

test("multi-item list", () => {
  const tests: TestCase[] = [
    [
      "(+ 1 2)",
      [
        [TokenTag.OPERATOR, Operator.PLUS],
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 2],
      ],
      "binary operator",
    ],
    [
      "(+ (1) 2)",
      [
        [TokenTag.OPERATOR, Operator.PLUS],
        [[TokenTag.NUMBER, 1]],
        [TokenTag.NUMBER, 2],
      ],
      "binary operator nested list",
    ],
    [
      "(+ (1 (2)) 3)",
      [
        [TokenTag.OPERATOR, Operator.PLUS],
        [[TokenTag.NUMBER, 1], [[TokenTag.NUMBER, 2]]],
        [TokenTag.NUMBER, 3],
      ],
      "binary operator nested list",
    ],
  ];

  for (const [expected, input, name] of tests) {
    assert.deepStrictEqual(expected, format(input), name);
  }
});
