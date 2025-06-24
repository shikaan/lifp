import { expect, test } from "bun:test";
import { read } from "../lib/read.ts";
import { print } from "../lib/print.ts";

test("prints exactly what comes in", () => {
  const input = '(1 :key "lol" true)';
  expect(print(read(input))).toEqual(input);
});
