import { expect, test } from "bun:test";
import { lists } from "./lists.js";
import { isList } from "../types.ts";

test("list.count", () => {
  expect(lists["list.count"]([[1, 2]])).toBe(2);
  expect(() => lists["list.count"]([1])).toThrow();
});

test("list.map", () => {
  const fn = ([val, idx]: number[]) => val + idx;
  const result = lists["list.map"]([fn, [1, 2]]);
  expect(Array.isArray(result)).toBe(true);
  expect(result[0]).toBe(1);
  expect(result[1]).toBe(3);
  expect(() => lists["list.map"]([fn, 1])).toThrow();
});

test("list.each", () => {
  let acc = 0;
  const fn = ([val, idx]: number[]) => {
    acc += val + idx;
    return null;
  };
  const result = lists["list.each"]([fn, [1, 2]]);
  expect(result).toBeNull();
  expect(acc).toBe(1 + 0 + 2 + 1);
  expect(() => lists["list.each"]([fn, 1])).toThrow();
});

test("list.nth", () => {
  const arr = [1, 2, 3];
  expect(lists["list.nth"]([0, arr])).toBe(1);
  expect(lists["list.nth"]([2, arr])).toBe(3);
  expect(lists["list.nth"]([10, arr])).toBeNull();
  expect(() => lists["list.nth"]([0, 1])).toThrow();
});

test("list.filter", () => {
  const fn = ([val, _]: number[]) => val % 2 === 1;
  const arr = [1, 2, 3];
  const result = lists["list.filter"]([fn, arr]);
  expect(isList(result)).toBe(true);
  expect(result[0]).toBe(1);
  expect(result[1]).toBe(3);
  expect(() => lists["list.filter"]([fn, 1])).toThrow();
});
