import { expect, test } from "bun:test";
import type { Value } from "../types.js";
import { core } from "./core.js";

test("+", () => {
  const tests: [string, Value[], Value][] = [
    ["binary", [1, 2], 3],
    ["n-ary", [1, 2, 3], 6],
  ];
  for (const [name, input, output] of tests) {
    expect(core["+"](input), name).resolves.toBe(output);
  }
});

test("+ - errors", () => {
  const tests: [string, unknown[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(core["+"](input as Value[]), name).rejects.toThrow();
  }
});

test("*", () => {
  const tests: [string, Value[], Value][] = [
    ["binary", [1, 2], 2],
    ["n-ary", [1, 2, 3], 6],
  ];
  for (const [name, input, output] of tests) {
    expect(core["*"](input), name).resolves.toBe(output);
  }
});

test("* - errors", () => {
  const tests: [string, Value[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(core["*"](input), name).rejects.toThrow();
  }
});

test("-", () => {
  const tests: [string, Value[], Value][] = [
    ["binary", [1, 2], -1],
    ["n-ary", [1, 2, 3], -4],
    ["positive", [3, 2], 1],
  ];
  for (const [name, input, output] of tests) {
    expect(core["-"](input), name).resolves.toBe(output);
  }
});

test("- - errors", () => {
  const tests: [string, Value[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(core["-"](input), name).rejects.toThrow();
  }
});

test("/", () => {
  const tests: [string, Value[], Value][] = [
    ["binary", [1, 2], 0.5],
    ["n-ary", [6, 3, 2], 1],
    ["zero", [6, 0], Infinity],
  ];
  for (const [name, input, output] of tests) {
    expect(core["/"](input), name).resolves.toBe(output);
  }
});

test("/ - errors", () => {
  const tests: [string, Value[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
  ];
  for (const [name, input] of tests) {
    expect(() => core["/"](input), name).toThrow();
  }
});

test("%", () => {
  const tests: [string, Value[], Value][] = [
    ["binary", [4, 2], 0],
    ["zero", [6, 0], NaN],
  ];
  for (const [name, input, output] of tests) {
    expect(core["%"](input), name).resolves.toBe(output);
  }
});

test("% - errors", () => {
  const tests: [string, Value[]][] = [
    ["unary", [1]],
    ["not numbers", [1, false]],
    ["too many", [1, 2, 3]],
  ];
  for (const [name, input] of tests) {
    expect(() => core["%"](input), name).toThrow();
  }
});

test("=", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [1, 1], true],
    ["failure", [1, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core["="](input), name).resolves.toBe(output);
  }
});

test("<", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [1, 2], true],
    ["failure", [2, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core["<"](input), name).resolves.toBe(output);
  }
});

test(">", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [2, 1], true],
    ["failure", [1, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core[">"](input), name).resolves.toBe(output);
  }
});

test("!=", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [1, 2], true],
    ["failure", [2, 2], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core["!="](input), name).resolves.toBe(output);
  }
});

test("<=", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [1, 2], true],
    ["success", [2, 2], true],
    ["failure", [2, 1], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core["<="](input), name).resolves.toBe(output);
  }
});

test(">=", () => {
  const tests: [string, Value[], Value][] = [
    ["failure", [1, 2], false],
    ["success", [2, 2], true],
    ["success", [2, 1], true],
  ];
  for (const [name, input, output] of tests) {
    expect(core[">="](input), name).resolves.toBe(output);
  }
});

for (const fn of ["=", "<", ">", "!=", "<=", ">="]) {
  test(`${fn} - errors`, () => {
    const tests: [string, Value[]][] = [
      ["unary", [1]],
      ["n-ary", [1, 2, 3]],
    ];
    for (const [name, input] of tests) {
      expect(core[fn](input), name).rejects.toThrow();
    }
  });
}

test("and", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [true, true], true],
    ["failure", [true, false], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core.and(input), name).resolves.toBe(output);
  }
});

test(`and - errors`, () => {
  const tests: [string, Value[]][] = [
    ["unary", [true]],
    ["not bool", [1, true, 3]],
  ];
  for (const [name, input] of tests) {
    expect(core.and(input), name).rejects.toThrow();
  }
});

test("or", () => {
  const tests: [string, Value[], Value][] = [
    ["success", [true, false], true],
    ["failure", [false, false], false],
  ];
  for (const [name, input, output] of tests) {
    expect(core.or(input), name).resolves.toBe(output);
  }
});

test(`or - errors`, () => {
  const tests: [string, Value[]][] = [
    ["unary", [true]],
    ["not bool", [1, true, 3]],
  ];
  for (const [name, input] of tests) {
    expect(core.or(input), name).rejects.toThrow();
  }
});
