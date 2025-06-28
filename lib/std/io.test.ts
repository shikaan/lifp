import { expect, spyOn, test } from "bun:test";
import { io } from "./io.js";

test("io.stdout", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const value = "hello";
  const result = io["io.stdout"]([value]);
  expect(spy).toBeCalledWith('"hello"');
  expect(result).toBeNull();
  spy.mockRestore();
});

test("io.stderr", () => {
  const spy = spyOn(process.stderr, "write");
  spy.mockImplementation(() => null);
  const value = "hello";
  const result = io["io.stderr"]([value]);
  expect(spy).toBeCalledWith('"hello"');
  expect(result).toBeNull();
  spy.mockRestore();
});

test("io stream - errors", () => {
  const value = "hello";
  expect(() => io["io.stdout"]([])).toThrow();
  expect(() => io["io.stderr"]([value, value])).toThrow();
});

test("io.printf", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const format = "hello %s %d";
  const values = ["world", 42];
  const result = io["io.printf"]([format, values]);
  expect(spy).toBeCalledWith("hello world 42");
  expect(result).toBeNull();
  spy.mockRestore();
});

test("io.printf - line breaks", () => {
  const spy = spyOn(process.stdout, "write");
  spy.mockImplementation(() => null);
  const format = "hello %s\n";
  const values = ["world"];
  const result = io["io.printf"]([format, values]);
  expect(spy, JSON.stringify(spy.mock.calls)).toBeCalledWith("hello world\n");
  expect(result).toBeNull();
  spy.mockRestore();
});

test("printf - errors", () => {
  const format = "hello %s";
  const values = ["world"];
  expect(() => io["io.printf"]([])).toThrow();
  expect(() => io["io.printf"]([format])).toThrow();
  expect(() => io["io.printf"]([values, values])).toThrow();
  expect(() => io["io.printf"]([format, 1])).toThrow();
});
