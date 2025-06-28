import { expect, test } from "bun:test";
import { defaultEnvironment, Environment } from "../lib/environment.ts";
import { evaluate } from "../lib/index.ts";
import { print } from "../lib/print.ts";
import { read } from "../lib/read.ts";

test("prints exactly what comes in", () => {
  const input = '(1 key "lol" true)';
  expect(print(read(input))).toEqual(input);
});

test("defines a variable and uses is", () => {
  const env = new Environment(defaultEnvironment);
  evaluate(read("(def! a 1)"), env);
  const expr = evaluate(read("(+ a 1)"), env);

  expect(print(expr)).toEqual("2");
});

test("defines a variable, a function and uses them", () => {
  const env = new Environment(defaultEnvironment);
  evaluate(read("(def! a 1)"), env);
  evaluate(read("(def! add (fn* (a b) (+ a b)))"), env);
  const expr = evaluate(read("(add 4 1)"), env);

  expect(print(expr)).toEqual("5");
});

test("implements cond", () => {
  const env = new Environment(defaultEnvironment);
  evaluate(read("(def! a 1)"), env);
  evaluate(
    read("(def! cond2 (fn* (c1 b1 c2 b2 else) (if c1 b1 (if c2 b2 else))))"),
    env,
  );

  let expr = evaluate(read("(cond2 true 1 false 2 3)"), env);
  expect(print(expr)).toEqual("1");

  expr = evaluate(read("(cond2 false 1 true 2 3)"), env);
  expect(print(expr)).toEqual("2");

  expr = evaluate(read("(cond2 false 1 false 2 3)"), env);
  expect(print(expr)).toEqual("3");
});
