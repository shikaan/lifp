import { test, expect, spyOn, beforeEach, afterEach } from "bun:test";
import { execute } from "../bin/run.js";

let write;
beforeEach(() => {
  write = spyOn(process.stdout, "write");
  write.mockImplementation(() => null);
});

afterEach(() => {
  write.mockRestore();
});

test("variables across forms", () => {
  const script = `
(def! :a 10)
(def! :b (+ a 5))

(printf "%d" ((+ a b)))
`.trim();
  execute(script);
  expect(write).toBeCalledWith("25");
});

test("lambdas", () => {
  const script = `
(def! :a 10)
(def! :+5 (fn* (a) (+ a 5)))

(printf "%d" ((+5 a)))
`.trim();
  execute(script);
  expect(write).toBeCalledWith("15");
});

test("lambdas and variables", () => {
  const script = `
(def! :planet "world")
(def! :greet
  (fn* (name)
    (printf "Hello, %s!" (name))))

(greet "user")
(greet planet)
`.trim();
  execute(script);
  expect(write).toBeCalledWith("Hello, user!");
  expect(write).toBeCalledWith("Hello, world!");
});

test("special forms", () => {
  const script = `
(def! :fibonacci
  (fn* (n) 
    (if (< n 2) 1 (fibonacci (- n 1))))))
`.trim();
  execute(script);
  expect(write).toBeCalledWith("Hello, user!");
  expect(write).toBeCalledWith("Hello, world!");
});
