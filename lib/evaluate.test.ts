import { expect, test } from "bun:test";
import { AST, Symbol, TokenTag } from "./types.ts";
import { evaluate } from "./evaluate.ts";

test("plus - happy path", () => {
  const tests: [AST, AST, string][] = [
    [
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
      ],
      [[TokenTag.NUMBER, 5]],
      "two elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
        [TokenTag.NUMBER, 5],
      ],
      [[TokenTag.NUMBER, 10]],
      "many elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
        [
          [TokenTag.SYMBOL, Symbol.PLUS],
          [TokenTag.NUMBER, 1],
          [TokenTag.NUMBER, 3],
        ],
      ],
      [[TokenTag.NUMBER, 9]],
      "nested",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});

test("plus - errors", () => {
  const tests: [AST, string][] = [
    [[[TokenTag.SYMBOL, Symbol.PLUS]], "no arguments"],
    [
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.STRING, "string"],
      ],
      "wrong arguments",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.NUMBER, 1],
      ],
      "one argument",
    ],
  ];
  for (const [ast, name] of tests) {
    expect(() => evaluate(ast), name).toThrow();
  }
});

test("minus - happy path", () => {
  const tests: [AST, AST, string][] = [
    [
      [
        [TokenTag.SYMBOL, Symbol.MINUS],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
      ],
      [[TokenTag.NUMBER, -1]],
      "two elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.MINUS],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
        [TokenTag.NUMBER, 5],
      ],
      [[TokenTag.NUMBER, -6]],
      "many elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.MINUS],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
        [
          [TokenTag.SYMBOL, Symbol.MINUS],
          [TokenTag.NUMBER, 1],
          [TokenTag.NUMBER, 3],
        ],
      ],
      [[TokenTag.NUMBER, 1]],
      "nested",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});

test("minus - errors", () => {
  const tests: [AST, string][] = [
    [[[TokenTag.SYMBOL, Symbol.MINUS]], "no arguments"],
    [
      [
        [TokenTag.SYMBOL, Symbol.MINUS],
        [TokenTag.STRING, "string"],
      ],
      "wrong arguments",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.MINUS],
        [TokenTag.NUMBER, 1],
      ],
      "one argument",
    ],
  ];
  for (const [ast, name] of tests) {
    expect(() => evaluate(ast), name).toThrow();
  }
});

test("wildcard - happy path", () => {
  const tests: [AST, AST, string][] = [
    [
      [
        [TokenTag.SYMBOL, Symbol.WILDCARD],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
      ],
      [[TokenTag.NUMBER, 6]],
      "two elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.WILDCARD],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
        [TokenTag.NUMBER, 5],
      ],
      [[TokenTag.NUMBER, 30]],
      "many elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.WILDCARD],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
        [
          [TokenTag.SYMBOL, Symbol.WILDCARD],
          [TokenTag.NUMBER, 1],
          [TokenTag.NUMBER, 3],
        ],
      ],
      [[TokenTag.NUMBER, 18]],
      "nested",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});

test("wildcard - errors", () => {
  const tests: [AST, string][] = [
    [[[TokenTag.SYMBOL, Symbol.WILDCARD]], "no arguments"],
    [
      [
        [TokenTag.SYMBOL, Symbol.WILDCARD],
        [TokenTag.STRING, "string"],
      ],
      "wrong arguments",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.WILDCARD],
        [TokenTag.NUMBER, 1],
      ],
      "one argument",
    ],
  ];
  for (const [ast, name] of tests) {
    expect(() => evaluate(ast), name).toThrow();
  }
});

test("slash - happy path", () => {
  const tests: [AST, AST, string][] = [
    [
      [
        [TokenTag.SYMBOL, Symbol.SLASH],
        [TokenTag.NUMBER, 6],
        [TokenTag.NUMBER, 2],
      ],
      [[TokenTag.NUMBER, 3]],
      "two elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.SLASH],
        [TokenTag.NUMBER, 18],
        [TokenTag.NUMBER, 3],
        [TokenTag.NUMBER, 2],
      ],
      [[TokenTag.NUMBER, 3]],
      "many elements",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.SLASH],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 1],
        [
          [TokenTag.SYMBOL, Symbol.SLASH],
          [TokenTag.NUMBER, 1],
          [TokenTag.NUMBER, 1],
        ],
      ],
      [[TokenTag.NUMBER, 2]],
      "nested",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.SLASH],
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 0],
      ],
      [[TokenTag.NUMBER, Infinity]],
      "divide by zero",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});

test("slash - errors", () => {
  const tests: [AST, string][] = [
    [[[TokenTag.SYMBOL, Symbol.SLASH]], "no arguments"],
    [
      [
        [TokenTag.SYMBOL, Symbol.SLASH],
        [TokenTag.STRING, "string"],
      ],
      "wrong arguments",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.SLASH],
        [TokenTag.NUMBER, 1],
      ],
      "one argument",
    ],
  ];
  for (const [ast, name] of tests) {
    expect(() => evaluate(ast), name).toThrow();
  }
});

test("not - happy path", () => {
  const tests: [AST, AST, string][] = [
    [
      [
        [TokenTag.SYMBOL, Symbol.NOT],
        [TokenTag.BOOLEAN, true],
      ],
      [[TokenTag.BOOLEAN, false]],
      "boolean",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});

test("not - errors", () => {
  const tests: [AST, string][] = [
    [[[TokenTag.SYMBOL, Symbol.NOT]], "no arguments"],
    [
      [
        [TokenTag.SYMBOL, Symbol.NOT],
        [TokenTag.STRING, "string"],
      ],
      "wrong arguments",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.NOT],
        [TokenTag.BOOLEAN, true],
        [TokenTag.BOOLEAN, false],
      ],
      "two argument",
    ],
  ];
  for (const [ast, name] of tests) {
    expect(() => evaluate(ast), name).toThrow();
  }
});

test("reduction - errors", () => {
  const tests: [AST, string][] = [
    [
      [
        [TokenTag.SYMBOL, "unknown_symbol"],
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 2],
      ],
      "unknown symbol",
    ],
  ];
  for (const [ast, name] of tests) {
    expect(() => evaluate(ast), name).toThrow();
  }
});

test("reduction - no routines", () => {
  const tests: [AST, AST, string][] = [
    [[[TokenTag.NUMBER, 42]], [[TokenTag.NUMBER, 42]], "single number"],
    [
      [[TokenTag.STRING, "hello"]],
      [[TokenTag.STRING, "hello"]],
      "single string",
    ],
    [[[TokenTag.BOOLEAN, true]], [[TokenTag.BOOLEAN, true]], "single boolean"],
    [
      [
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
      ],
      [
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 2],
        [TokenTag.NUMBER, 3],
      ],
      "multiple tokens",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});

test("reduction - non-linear cases", () => {
  const tests: [AST, AST, string][] = [
    [
      [
        [TokenTag.NUMBER, 1],
        [
          [TokenTag.SYMBOL, Symbol.PLUS],
          [TokenTag.NUMBER, 2],
          [TokenTag.NUMBER, 3],
        ],
      ],
      [
        [TokenTag.NUMBER, 1],
        [TokenTag.NUMBER, 5],
      ],
      "nested evaluation without routine at root",
    ],
    [
      [
        [TokenTag.SYMBOL, Symbol.PLUS],
        [TokenTag.NUMBER, 1],
        [
          [TokenTag.SYMBOL, Symbol.MINUS],
          [TokenTag.NUMBER, 10],
          [TokenTag.NUMBER, 5],
        ],
        [
          [TokenTag.SYMBOL, Symbol.WILDCARD],
          [TokenTag.NUMBER, 2],
          [TokenTag.NUMBER, 3],
        ],
      ],
      [[TokenTag.NUMBER, 12]],
      "complex nested evaluation",
    ],
  ];
  for (const [ast, expected, name] of tests) {
    expect(evaluate(ast), name).toEqual(expected);
  }
});
