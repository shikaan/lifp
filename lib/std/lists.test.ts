import { expect, test } from "bun:test";
import { isList } from "../types.js";
import { lists } from "./lists.js";

test("list.count", () => {
  expect(lists["list.count"]([[1, 2]])).resolves.toBe(2);
  expect(lists["list.count"]([1])).rejects.toThrow();
});

test("list.map", async () => {
  const fn = async ([val, idx]: number[]) => val + idx;
  const result = await lists["list.map"]([fn, [1, 2]]);
  expect(Array.isArray(result)).toBe(true);
  expect(result[0]).toBe(1);
  expect(result[1]).toBe(3);
  expect(lists["list.map"]([fn, 1])).rejects.toThrow();
});

test("list.each", () => {
  let acc = 0;
  const fn = ([val, idx]: number[]) => {
    acc += val + idx;
    return null;
  };
  const result = lists["list.each"]([fn, [1, 2]]);
  expect(result).resolves.toBeNull();
  expect(acc).toBe(1 + 2 + 1);
  expect(lists["list.each"]([fn, 1])).rejects.toThrow();
});

test("list.nth", () => {
  const arr = [1, 2, 3];
  expect(lists["list.nth"]([0, arr])).resolves.toBe(1);
  expect(lists["list.nth"]([2, arr])).resolves.toBe(3);
  expect(lists["list.nth"]([10, arr])).resolves.toBeNull();
  expect(lists["list.nth"]([0, 1])).rejects.toThrow();
});

test("list.filter", async () => {
  const fn = async ([val, _]: number[]) => val % 2 === 1;
  const arr = [1, 2, 3];
  const result = await lists["list.filter"]([fn, arr]);
  expect(isList(result)).toBe(true);
  expect(result[0]).toBe(1);
  expect(result[1]).toBe(3);
  expect(lists["list.filter"]([fn, 1])).rejects.toThrow();
});
