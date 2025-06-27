import { expect, test } from "bun:test";
import { n, l } from "../../tests/utils.js";
import { lists } from "./lists.js";
import { ASTNodeType } from "../types.js";

test("list.count", () => {
  const listNode = l([n(ASTNodeType.NUMBER, 1), n(ASTNodeType.NUMBER, 2)]);
  expect(lists["list.count"]([listNode])).toEqual(n(ASTNodeType.NUMBER, 2));
  expect(() => lists["list.count"]([n(ASTNodeType.NUMBER, 1)])).toThrow();
});

test("list.map", () => {
  const fn = {
    type: ASTNodeType.FUNCTION,
    value: (([item, idx]: any[]) =>
      n(ASTNodeType.NUMBER, item.value + idx.value)) as any,
  };
  const listNode = l([n(ASTNodeType.NUMBER, 1), n(ASTNodeType.NUMBER, 2)]);
  const result = lists["list.map"]([fn, listNode]);
  expect(result.type).toBe(ASTNodeType.LIST);
  expect(result.value[0].value).toBe(1);
  expect(result.value[1].value).toBe(3);
  expect(() => lists["list.map"]([fn, n(ASTNodeType.NUMBER, 1)])).toThrow();
});

test("list.each", () => {
  let acc = 0;
  const fn = {
    type: ASTNodeType.FUNCTION,
    value: (([item, idx]: any[]) => {
      acc += item.value + idx.value;
      return n(ASTNodeType.NIL, null);
    }) as any,
  };
  const listNode = l([n(ASTNodeType.NUMBER, 1), n(ASTNodeType.NUMBER, 2)]);
  const result = lists["list.each"]([fn, listNode]);
  expect(result).toEqual(n(ASTNodeType.NIL, null));
  expect(acc).toBe(1 + 0 + 2 + 1);
  expect(() => lists["list.each"]([fn, n(ASTNodeType.NUMBER, 1)])).toThrow();
});
