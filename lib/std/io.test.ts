import { expect, spyOn, test } from "bun:test";
import { n } from "../../tests/utils.js";
import { io } from "./io.js";
import { ASTNodeType } from "../types.js";

test("io.stdout", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const node = n(ASTNodeType.STRING, "hello");
  const result = io["io.stdout"]([node]);
  expect(spy).toBeCalledWith('"hello"');
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("io.stderr", () => {
  const spy = spyOn(process.stderr, "write");
  spy.mockImplementation(() => null);
  const node = n(ASTNodeType.STRING, "hello");
  const result = io["io.stderr"]([node]);
  expect(spy).toBeCalledWith('"hello"');
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("io stream - errors", () => {
  const node = n(ASTNodeType.STRING, "hello");
  expect(() => io["io.stdout"]([])).toThrow();
  expect(() => io["io.stderr"]([node, node])).toThrow();
});

test("io.printf", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const format = n(ASTNodeType.STRING, "hello %s %d");
  const values = n(ASTNodeType.LIST, [
    n(ASTNodeType.STRING, "world"),
    n(ASTNodeType.NUMBER, 42),
  ]);
  const result = io["io.printf"]([format, values]);
  expect(spy).toBeCalledWith("hello world 42");
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("io.printf - line breaks", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const format = n(ASTNodeType.STRING, "hello %s\n");
  const values = n(ASTNodeType.LIST, [n(ASTNodeType.STRING, "world")]);
  const result = io["io.printf"]([format, values]);
  expect(spy, JSON.stringify(spy.mock.calls)).toBeCalledWith("hello world\n");
  expect(result).toEqual({ type: ASTNodeType.NIL, value: null });
  spy.mockRestore();
});

test("printf - errors", () => {
  const format = n(ASTNodeType.STRING, "hello %s");
  const values = n(ASTNodeType.LIST, [n(ASTNodeType.STRING, "world")]);
  expect(() => io["io.printf"]([])).toThrow();
  expect(() => io["io.printf"]([format])).toThrow();
  expect(() => io["io.printf"]([values, values])).toThrow();
  expect(() => io["io.printf"]([format, n(ASTNodeType.NUMBER, 1)])).toThrow();
});
