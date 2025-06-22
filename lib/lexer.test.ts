import { expect, test } from "bun:test";
import { tokenize } from "./lexer.js";
import { type Token, TokenType } from "./types.js";

const t = (type: TokenType, literal?: string | number): Token =>
  ({ type, literal }) as Token;
const LPAREN = t(TokenType.LPAREN, "(");
const RPAREN = t(TokenType.RPAREN, ")");
const EOF = t(TokenType.EOF);

test("whitespace", () => {
  const tests = ["  (1)", "\n(1)", "\t(1)", "\r(1)", "(\n1\n)"];

  for (const test of tests) {
    expect(tokenize(test), JSON.stringify(test)).toEqual([
      LPAREN,
      t(TokenType.NUMBER, 1),
      RPAREN,
      EOF,
    ]);
  }
});

test("numbers", () => {
  const tests: [string, Token[]][] = [
    ["(12)", [LPAREN, t(TokenType.NUMBER, 12), RPAREN, EOF]],
    ["(1)", [LPAREN, t(TokenType.NUMBER, 1), RPAREN, EOF]],
    ["(1.56)", [LPAREN, t(TokenType.NUMBER, 1.56), RPAREN, EOF]],
    ["(-1)", [LPAREN, t(TokenType.NUMBER, -1), RPAREN, EOF]],
    ["(-1.0)", [LPAREN, t(TokenType.NUMBER, -1), RPAREN, EOF]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input)).toEqual(expected);
  }
});

test("symbols", () => {
  const tests: [string, Token[]][] = [
    ["(true)", [LPAREN, t(TokenType.SYMBOL, "true"), RPAREN, EOF]],
    ["(add)", [LPAREN, t(TokenType.SYMBOL, "add"), RPAREN, EOF]],
    ["(def!)", [LPAREN, t(TokenType.SYMBOL, "def!"), RPAREN, EOF]],
    ["(add-one)", [LPAREN, t(TokenType.SYMBOL, "add-one"), RPAREN, EOF]],
    ["(add-1)", [LPAREN, t(TokenType.SYMBOL, "add-1"), RPAREN, EOF]],
    ["(1-dos)", [LPAREN, t(TokenType.SYMBOL, "1-dos"), RPAREN, EOF]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input), JSON.stringify(input)).toEqual(expected);
  }
});

test("strings", () => {
  const tests: [string, Token[]][] = [
    ['("true")', [LPAREN, t(TokenType.STRING, "true"), RPAREN, EOF]],
    ['("t\"rue")', [LPAREN, t(TokenType.STRING, 't"rue'), RPAREN, EOF]],
    ["('t\'rue')", [LPAREN, t(TokenType.STRING, "t'rue"), RPAREN, EOF]],
    ["('1')", [LPAREN, t(TokenType.STRING, "1"), RPAREN, EOF]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input)).toEqual(expected);
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
        EOF,
      ],
    ],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input)).toEqual(expected);
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
        EOF,
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
        EOF,
      ],
    ],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input)).toEqual(expected);
  }
});

test("atoms", () => {
  const tests: [string, Token[]][] = [
    ["1", [t(TokenType.NUMBER, 1), EOF]],
    ['"str"', [t(TokenType.STRING, "str"), EOF]],
    ["str", [t(TokenType.SYMBOL, "str"), EOF]],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input)).toEqual(expected);
  }
});

test("comments", () => {
  const tests: [string, Token[]][] = [
    ["1 ; one", [t(TokenType.NUMBER, 1), EOF]],
    ["; one\n1", [t(TokenType.NUMBER, 1), EOF]],
    [
      "(1\n; lol\n2)",
      [LPAREN, t(TokenType.NUMBER, 1), t(TokenType.NUMBER, 2), RPAREN, EOF],
    ],
  ];

  for (const [input, expected] of tests) {
    expect(tokenize(input)).toEqual(expected);
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
    expect(() => tokenize(input), JSON.stringify(input)).toThrow(expected);
  }
});
