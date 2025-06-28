import { expect, test } from "bun:test";
import { n } from "../../tests/utils.js";
import { type ASTNode, ASTNodeType } from "../types.js";
import { math } from "./math.js";

const num = (x: number) => n(ASTNodeType.NUMBER, x);
const bool = (x: boolean) => n(ASTNodeType.BOOLEAN, x);

test("+", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(3)],
    ["n-ary", [num(1), num(2), num(3)], num(6)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["+"](input), name).toEqual(output);
  }
});

test("+ - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["+"](input), name).toThrow();
  }
});

test("*", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(2)],
    ["n-ary", [num(1), num(2), num(3)], num(6)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["*"](input), name).toEqual(output);
  }
});

test("* - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["*"](input), name).toThrow();
  }
});

test("-", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(-1)],
    ["n-ary", [num(1), num(2), num(3)], num(-4)],
    ["positive", [num(3), num(2)], num(1)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["-"](input), name).toEqual(output);
  }
});

test("- - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["-"](input), name).toThrow();
  }
});

test("/", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["binary", [num(1), num(2)], num(0.5)],
    ["n-ary", [num(6), num(3), num(2)], num(1)],
    ["zero", [num(6), num(0)], num(Infinity)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["/"](input), name).toEqual(output);
  }
});

test("/ - errors", () => {
  const tests: [string, ASTNode[]][] = [
    ["unary", [num(1)]],
    ["not numbers", [num(1), bool(false)]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["/"](input), name).toThrow();
  }
});

test("=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(1)], bool(true)],
    ["failure", [num(1), num(2)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["="](input), name).toEqual(output);
  }
});

test("<", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(2)], bool(true)],
    ["failure", [num(2), num(2)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["<"](input), name).toEqual(output);
  }
});

test(">", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(2), num(1)], bool(true)],
    ["failure", [num(1), num(2)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math[">"](input), name).toEqual(output);
  }
});

test("!=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(2)], bool(true)],
    ["failure", [num(2), num(2)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["!="](input), name).toEqual(output);
  }
});

test("<=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [num(1), num(2)], bool(true)],
    ["success", [num(2), num(2)], bool(true)],
    ["failure", [num(2), num(1)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math["<="](input), name).toEqual(output);
  }
});

test(">=", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["failure", [num(1), num(2)], bool(false)],
    ["success", [num(2), num(2)], bool(true)],
    ["success", [num(2), num(1)], bool(true)],
  ];
  for (const [name, input, output] of tests) {
    expect(math[">="](input), name).toEqual(output);
  }
});

test("and", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [bool(true), bool(true)], bool(true)],
    ["failure", [bool(true), bool(false)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math.and(input), name).toEqual(output);
  }
});

test("or", () => {
  const tests: [string, ASTNode[], ASTNode][] = [
    ["success", [bool(true), bool(false)], bool(true)],
    ["failure", [bool(false), bool(false)], bool(false)],
  ];
  for (const [name, input, output] of tests) {
    expect(math.or(input), name).toEqual(output);
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
      expect(() => math[fn](input), name).toThrow();
    }
  });
}
