import { expect, test } from "bun:test";
import { mockPtr, t } from "../tests/utils.js";
import { tokenize } from "./lexer.js";
import { type Token, TokenType } from "./types.js";

const LPAREN = t(TokenType.LPAREN, "(");
const RPAREN = t(TokenType.RPAREN, ")");

test("whitespace", () => {
  const tests = ["  (1)", "\n(1)", "\t(1)", "\r(1)", "(\n1\n)"];

  for (const test of tests) {
    expect(tokenize(test, mockPtr), JSON.stringify(test)).toEqual([
      LPAREN,
      t(TokenType.NUMBER, 1),
      RPAREN,
    ]);
  }
});

test("numbers", () => {
  const tests: [string, Token[]][] = [
    ["(12)", [LPAREN, t(TokenType.NUMBER, 12), RPAREN]],
    ["(1)", [LPAREN, t(TokenType.NUMBER, 1), RPAREN]],
    ["(1.56)", [LPAREN, t(TokenType.NUMBER, 1.56), RPAREN]],
    ["(-1)", [LPAREN, t(TokenType.NUMBER, -1), RPAREN]],
    ["(-1.0)", [LPAREN, t(TokenType.NUMBER, -1), RPAREN]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr)).toEqual(expected);
  }
});

test("symbols", () => {
  const tests: [string, Token[]][] = [
    ["(true)", [LPAREN, t(TokenType.SYMBOL, "true"), RPAREN]],
    ["(add)", [LPAREN, t(TokenType.SYMBOL, "add"), RPAREN]],
    ["(def!)", [LPAREN, t(TokenType.SYMBOL, "def!"), RPAREN]],
    ["(add-one)", [LPAREN, t(TokenType.SYMBOL, "add-one"), RPAREN]],
    ["(add-1)", [LPAREN, t(TokenType.SYMBOL, "add-1"), RPAREN]],
    ["(1-dos)", [LPAREN, t(TokenType.SYMBOL, "1-dos"), RPAREN]],
    ["(:keyword)", [LPAREN, t(TokenType.SYMBOL, ":keyword"), RPAREN]],
    ["(/)", [LPAREN, t(TokenType.SYMBOL, "/"), RPAREN]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr), JSON.stringify(input)).toEqual(expected);
  }
});

test("strings", () => {
  const tests: [string, Token[]][] = [
    ['("true")', [LPAREN, t(TokenType.STRING, "true"), RPAREN]],
    [`("t\\"rue")`, [LPAREN, t(TokenType.STRING, 't"rue'), RPAREN]],
    [`("t\\true")`, [LPAREN, t(TokenType.STRING, "t\true"), RPAREN]],
    [
      `("t\\"r" "u e")`,
      [LPAREN, t(TokenType.STRING, 't"r'), t(TokenType.STRING, "u e"), RPAREN],
    ],
    ['("1")', [LPAREN, t(TokenType.STRING, "1"), RPAREN]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr), input).toEqual(expected);
  }
});

test("sub expressions", () => {
  const tests: [string, Token[]][] = [
    [
      "(1 (2))",
      [
        LPAREN,
        t(TokenType.NUMBER, 1),
        LPAREN,
        t(TokenType.NUMBER, 2),
        RPAREN,
        RPAREN,
      ],
    ],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr)).toEqual(expected);
  }
});

test("complex expressions", () => {
  const tests: [string, Token[]][] = [
    [
      "(add 1 2)",
      [
        LPAREN,
        t(TokenType.SYMBOL, "add"),
        t(TokenType.NUMBER, 1),
        t(TokenType.NUMBER, 2),
        RPAREN,
      ],
    ],
    [
      "(add 1 (mul 2 3))",
      [
        LPAREN,
        t(TokenType.SYMBOL, "add"),
        t(TokenType.NUMBER, 1),
        LPAREN,
        t(TokenType.SYMBOL, "mul"),
        t(TokenType.NUMBER, 2),
        t(TokenType.NUMBER, 3),
        RPAREN,
        RPAREN,
      ],
    ],
    [
      '(concat "hel" ("l" "o"))',
      [
        LPAREN,
        t(TokenType.SYMBOL, "concat"),
        t(TokenType.STRING, "hel"),
        LPAREN,
        t(TokenType.STRING, "l"),
        t(TokenType.STRING, "o"),
        RPAREN,
        RPAREN,
      ],
    ],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr)).toEqual(expected);
  }
});

test("atoms", () => {
  const tests: [string, Token[]][] = [
    ["1", [t(TokenType.NUMBER, 1)]],
    ['"str"', [t(TokenType.STRING, "str")]],
    ["str", [t(TokenType.SYMBOL, "str")]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr)).toEqual(expected);
  }
});

test("comments", () => {
  const tests: [string, Token[]][] = [
    ["1 ; one", [t(TokenType.NUMBER, 1)]],
    ["; one\n1", [t(TokenType.NUMBER, 1)]],
    [
      "(1\n; lol\n2)",
      [LPAREN, t(TokenType.NUMBER, 1), t(TokenType.NUMBER, 2), RPAREN],
    ],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input, mockPtr)).toEqual(expected);
  }
});

test("syntax exception", () => {
  const tests: [string, RegExp][] = [
    ["", /Provided/],
    ["(={}1)", /Unexpected/],
    ["(1={})", /Unexpected/],
    ["([])", /Unexpected/],
  ];

  for (const [input, expected] of tests) {
    expect(() => tokenize(input, mockPtr), JSON.stringify(input)).toThrow(
      expected,
    );
  }
});
