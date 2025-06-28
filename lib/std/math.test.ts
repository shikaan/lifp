import { expect, test } from "bun:test";
import { math } from "./math.js";

test("+", () => {
  const tests: [string, any[], any][] = [
    ["binary", [1, 2], 3],
    ["n-ary", [1, 2, 3], 6],
  ];
  for (const [name, input, output] of tests) {
    expect(math["+"](input), name).toBe(output);
  }
});

test("+ - errors", () => {
  const tests: [string, any[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["+"](input), name).toThrow();
  }
});

test("*", () => {
  const tests: [string, any[], any][] = [
    ["binary", [1, 2], 2],
    ["n-ary", [1, 2, 3], 6],
  ];
  for (const [name, input, output] of tests) {
    expect(math["*"](input), name).toBe(output);
  }
});

test("* - errors", () => {
  const tests: [string, any[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["*"](input), name).toThrow();
  }
});

test("-", () => {
  const tests: [string, any[], any][] = [
    ["binary", [1, 2], -1],
    ["n-ary", [1, 2, 3], -4],
    ["positive", [3, 2], 1],
  ];
  for (const [name, input, output] of tests) {
    expect(math["-"](input), name).toBe(output);
  }
});

test("- - errors", () => {
  const tests: [string, any[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["-"](input), name).toThrow();
  }
});

test("/", () => {
  const tests: [string, any[], any][] = [
    ["binary", [1, 2], 0.5],
    ["n-ary", [6, 3, 2], 1],
    ["zero", [6, 0], Infinity],
  ];
  for (const [name, input, output] of tests) {
    expect(math["/"](input), name).toBe(output);
  }
});

test("/ - errors", () => {
  const tests: [string, any[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(() => math["/"](input), name).toThrow();
  }
});

test("=", () => {
  const tests: [string, any[], any][] = [
    ["success", [1, 1], true],
    ["failure", [1, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math["="](input), name).toBe(output);
  }
});

test("<", () => {
  const tests: [string, any[], any][] = [
    ["success", [1, 2], true],
    ["failure", [2, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math["<"](input), name).toBe(output);
  }
});

test(">", () => {
  const tests: [string, any[], any][] = [
    ["success", [2, 1], true],
    ["failure", [1, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math[">"](input), name).toBe(output);
  }
});

test("!=", () => {
  const tests: [string, any[], any][] = [
    ["success", [1, 2], true],
    ["failure", [2, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math["!="](input), name).toBe(output);
  }
});

test("<=", () => {
  const tests: [string, any[], any][] = [
    ["success", [1, 2], true],
    ["success", [2, 2], true],
    ["failure", [2, 1], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math["<="](input), name).toBe(output);
  }
});

test(">=", () => {
  const tests: [string, any[], any][] = [
    ["failure", [1, 2], false],
    ["success", [2, 2], true],
    ["success", [2, 1], true],
  ];
  for (const [name, input, output] of tests) {
    expect(math[">="](input), name).toBe(output);
  }
});

test("and", () => {
  const tests: [string, any[], any][] = [
    ["success", [true, true], true],
    ["failure", [true, false], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math.and(input), name).toBe(output);
  }
});

test("or", () => {
  const tests: [string, any[], any][] = [
    ["success", [true, false], true],
    ["failure", [false, false], false],
  ];
  for (const [name, input, output] of tests) {
    expect(math.or(input), name).toBe(output);
  }
});

for (const fn of ["=", "<", ">", "!=", "<=", ">=", "and", "or"]) {
  test(`${fn} - errors`, () => {
    const tests: [string, any[]][] = [
      ["unary", [1]],
      ["n-ary", [1, 2, 3]],
      ["different types", [1, false]],
    ];
    for (const [name, input] of tests) {
      expect(() => math[fn](input), name).toThrow();
    }
  });
}
