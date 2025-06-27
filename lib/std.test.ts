import { expect, spyOn, test } from "bun:test";
import { n } from "../tests/utils.js";
import { std } from "./std.js";
import { type ASTNode, ASTNodeType } from "./types.js";

const num = (x: number) => n(ASTNodeType.NUMBER, x);
const bool = (x: boolean) => n(ASTNodeType.BOOLEAN, x);

test("+", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(3)],
    ["n-ary", [num(1), num(2), num(3)], num(6)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["+"](input), name).toEqual(output);
  }
});

test("+ - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];

  for (const [name, input] of tests) {
    expect(() => std["+"](input), name).toThrow();
  }
});

test("*", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(2)],
    ["n-ary", [num(1), num(2), num(3)], num(6)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["*"](input), name).toEqual(output);
  }
});

test("* - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];

  for (const [name, input] of tests) {
    expect(() => std["*"](input), name).toThrow();
  }
});

test("-", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(-1)],
    ["n-ary", [num(1), num(2), num(3)], num(-4)],
    ["positive", [num(3), num(2)], num(1)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["-"](input), name).toEqual(output);
  }
});

test("- - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];

  for (const [name, input] of tests) {
    expect(() => std["-"](input), name).toThrow();
  }
});

test("/", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(0.5)],
    ["n-ary", [num(6), num(3), num(2)], num(1)],
    ["zero", [num(6), num(0)], num(Infinity)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["/"](input), name).toEqual(output);
  }
});

test("/ - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];

  for (const [name, input] of tests) {
    expect(() => std["/"](input), name).toThrow();
  }
});

test("=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(1)], bool(true)],
    ["failure", [num(1), num(2)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["="](input), name).toEqual(output);
  }
});

test("<", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(2)], bool(true)],
    ["failure", [num(2), num(2)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["<"](input), name).toEqual(output);
  }
});

test(">", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(2), num(1)], bool(true)],
    ["failure", [num(1), num(2)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std[">"](input), name).toEqual(output);
  }
});

test("!=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(2)], bool(true)],
    ["failure", [num(2), num(2)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["!="](input), name).toEqual(output);
  }
});

test("<=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(2)], bool(true)],
    ["success", [num(2), num(2)], bool(true)],
    ["failure", [num(2), num(1)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std["<="](input), name).toEqual(output);
  }
});

test(">=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["failure", [num(1), num(2)], bool(false)],
    ["success", [num(2), num(2)], bool(true)],
    ["success", [num(2), num(1)], bool(true)],
  ];

  for (const [name, input, output] of tests) {
    expect(std[">="](input), name).toEqual(output);
  }
});

test("and", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [bool(true), bool(true)], bool(true)],
    ["failure", [bool(true), bool(false)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std.and(input), name).toEqual(output);
  }
});

test("or", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [bool(true), bool(false)], bool(true)],
    ["failure", [bool(false), bool(false)], bool(false)],
  ];

  for (const [name, input, output] of tests) {
    expect(std.or(input), name).toEqual(output);
  }
});

for (const fn of ["=", "<", ">", "!=", "<=", ">=", "and", "or"]) {
  test(`${fn} - errors`, () => {
    const tests: [string, ASTNode[]][] = [
      ["unary", [num(1)]],
      ["n-ary", [num(1), num(2), num(3)]],
      ["different types", [num(1), bool(false)]],
    ];

    for (const [name, input] of tests) {
      expect(() => std[fn](input), name).toThrow();
    }
  });
}

test("io.stdout", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const node = n(ASTNodeType.STRING, "hello");
  const result = std["io.stdout"]([node]);
  expect(spy).toBeCalledWith('"hello"');
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("io.stderr", () => {
  const spy = spyOn(process.stderr, "write");
  spy.mockImplementation(() => null);
  const node = n(ASTNodeType.STRING, "hello");
  const result = std["io.stderr"]([node]);
  expect(spy).toBeCalledWith('"hello"');
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("io functions - errors", () => {
  const node = n(ASTNodeType.STRING, "hello");
  expect(() => std["io.stdout"]([])).toThrow();
  expect(() => std["io.stderr"]([node, node])).toThrow();
});

test("printf", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const format = n(ASTNodeType.STRING, "hello %s %d");
  const values = n(ASTNodeType.LIST, [
    n(ASTNodeType.STRING, "world"),
    n(ASTNodeType.NUMBER, 42),
  ]);
  const result = std.printf([format, values]);
  expect(spy).toBeCalledWith("hello world 42");
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("printf - errors", () => {
  const format = n(ASTNodeType.STRING, "hello %s");
  const values = n(ASTNodeType.LIST, [n(ASTNodeType.STRING, "world")]);
  expect(() => std.printf([])).toThrow();
  expect(() => std.printf([format])).toThrow();
  expect(() => std.printf([values, values])).toThrow();
  expect(() => std.printf([format, n(ASTNodeType.NUMBER, 1)])).toThrow();
});
