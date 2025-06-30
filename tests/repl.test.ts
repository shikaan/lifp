import { expect, test } from "bun:test";
import { defaultEnvironment, Environment } from "../lib/environment.ts";
import { evaluate } from "../lib/index.ts";
import { print } from "../lib/print.ts";
import { read } from "../lib/read.ts";
import { mockPtr } from "./utils.js";

test("prints exactly what comes in", async () => {
  const input = '(1 "lol" true)';
  expect(
    print(await evaluate(read(input, mockPtr), defaultEnvironment)),
  ).toEqual(input);
});

test("defines a variable and uses is", async () => {
  const env = new Environment(defaultEnvironment);
  await evaluate(read("(def! a 1)", mockPtr), env);
  const expr = await evaluate(read("(+ a 1)", mockPtr), env);

  expect(print(expr)).toEqual("2");
});

test("defines a variable, a function and uses them", async () => {
  const env = new Environment(defaultEnvironment);
  await evaluate(read("(def! a 1)", mockPtr), env);
  await evaluate(read("(def! add (fn* (a b) (+ a b)))", mockPtr), env);
  const expr = await evaluate(read("(add 4 1)", mockPtr), env);

  expect(print(expr)).toEqual("5");
});

test("implements cond", async () => {
  const env = new Environment(defaultEnvironment);
  await evaluate(read("(def! a 1)", mockPtr), env);
  await evaluate(
    read(
      "(def! cond2 (fn* (c1 b1 c2 b2 else) (if c1 b1 (if c2 b2 else))))",
      mockPtr,
    ),
    env,
  );

  let expr = await evaluate(read("(cond2 true 1 false 2 3)", mockPtr), env);
  expect(print(expr)).toEqual("1");

  expr = await evaluate(read("(cond2 false 1 true 2 3)", mockPtr), env);
  expect(print(expr)).toEqual("2");

  expr = await evaluate(read("(cond2 false 1 false 2 3)", mockPtr), env);
  expect(print(expr)).toEqual("3");
});
