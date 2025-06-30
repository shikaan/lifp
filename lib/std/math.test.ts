import { expect, test } from "bun:test";
import { math } from "./math.js";

test("math.floor", async () => {
  expect(math["math.floor"]([1.9])).resolves.toBe(1);
  expect(math["math.floor"]([-1.1])).resolves.toBe(-2);
  expect(math["math.floor"]([2])).resolves.toBe(2);
  expect(math["math.floor"]([1, 2])).rejects.toThrow();
  expect(math["math.floor"](["foo"])).rejects.toThrow();
});

test("math.ceil", async () => {
  expect(math["math.ceil"]([1.1])).resolves.toBe(2);
  expect(math["math.ceil"]([-1.9])).resolves.toBe(-1);
  expect(math["math.ceil"]([2])).resolves.toBe(2);
  expect(math["math.ceil"]([1, 2])).rejects.toThrow();
  expect(math["math.ceil"](["foo"])).rejects.toThrow();
});

test("math.max", async () => {
  expect(math["math.max"]([[1, 2, 3]])).resolves.toBe(3);
  expect(math["math.max"]([[-1, -2, -3]])).resolves.toBe(-1);
  expect(math["math.max"]([[42]])).resolves.toBe(42);
  expect(math["math.max"]([[]])).rejects.toThrow();
  expect(math["math.max"]([[1, "foo", 3]])).rejects.toThrow();
  expect(math["math.max"]([1, 2, 3])).rejects.toThrow();
});

test("math.min", async () => {
  expect(math["math.min"]([[1, 2, 3]])).resolves.toBe(1);
  expect(math["math.min"]([[-1, -2, -3]])).resolves.toBe(-3);
  expect(math["math.min"]([[42]])).resolves.toBe(42);
  expect(math["math.min"]([[]])).rejects.toThrow();
  expect(math["math.min"]([[1, "foo", 3]])).rejects.toThrow();
  expect(math["math.min"]([1, 2, 3])).rejects.toThrow();
});

test("math.random", async () => {
  const value = await math["math.random"]([]);
  expect(typeof value).toBe("number");
  expect(value).toBeGreaterThanOrEqual(0);
  expect(value).toBeLessThan(1);
  expect(math["math.random"]([1])).rejects.toThrow();
});
