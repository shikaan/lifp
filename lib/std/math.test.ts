import { expect, test } from "bun:test";
import { math } from "./math.js";

test("math.floor", async () => {
  await expect(math["math.floor"]([1.9])).resolves.toBe(1);
  await expect(math["math.floor"]([-1.1])).resolves.toBe(-2);
  await expect(math["math.floor"]([2])).resolves.toBe(2);
  await expect(math["math.floor"]([1, 2])).rejects.toThrow();
  await expect(math["math.floor"](["foo"])).rejects.toThrow();
});

test("math.ceil", async () => {
  await expect(math["math.ceil"]([1.1])).resolves.toBe(2);
  await expect(math["math.ceil"]([-1.9])).resolves.toBe(-1);
  await expect(math["math.ceil"]([2])).resolves.toBe(2);
  await expect(math["math.ceil"]([1, 2])).rejects.toThrow();
  await expect(math["math.ceil"](["foo"])).rejects.toThrow();
});

test("math.max", async () => {
  await expect(math["math.max"]([[1, 2, 3]])).resolves.toBe(3);
  await expect(math["math.max"]([[-1, -2, -3]])).resolves.toBe(-1);
  await expect(math["math.max"]([[42]])).resolves.toBe(42);
  await expect(math["math.max"]([[]])).rejects.toThrow();
  await expect(math["math.max"]([[1, "foo", 3]])).rejects.toThrow();
  await expect(math["math.max"]([1, 2, 3])).rejects.toThrow();
});

test("math.min", async () => {
  await expect(math["math.min"]([[1, 2, 3]])).resolves.toBe(1);
  await expect(math["math.min"]([[-1, -2, -3]])).resolves.toBe(-3);
  await expect(math["math.min"]([[42]])).resolves.toBe(42);
  await expect(math["math.min"]([[]])).rejects.toThrow();
  await expect(math["math.min"]([[1, "foo", 3]])).rejects.toThrow();
  await expect(math["math.min"]([1, 2, 3])).rejects.toThrow();
});
