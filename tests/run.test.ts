import {
  afterEach,
  beforeEach,
  expect,
  type Mock,
  spyOn,
  test,
} from "bun:test";
import { execute } from "../bin/run.js";

let write: Mock<(s: string) => void>;
beforeEach(() => {
  write = spyOn(process.stdout, "write");
  write.mockImplementation(() => null);
});

afterEach(() => {
  write.mockRestore();
});

test("variables across forms", () => {
  const script = `
(def! a 10)
(def! b (+ a 5))

(io.printf "%d" ((+ a b)))
`.trim();
  execute(script);
  expect(write).toBeCalledWith("25");
});

test("lambdas", () => {
  const script = `
(def! a 10)
(def! +5 (fn* (a) (+ a 5)))

(io.printf "%d" ((+5 a)))
`.trim();
  execute(script);
  expect(write).toBeCalledWith("15");
});

test("lambdas and variables", () => {
  const script = `
(def! planet "world")
(def! greet
  (fn* (name)
    (io.printf "Hello, %s!" (name))))

(greet "user")
(greet planet)
`.trim();
  execute(script);
  expect(write).toBeCalledWith("Hello, user!");
  expect(write).toBeCalledWith("Hello, world!");
});

test("special forms", () => {
  const script = `
(def! fibonacci
  (fn* (n) 
    (if (< n 2) 
      1 
      (+ (fibonacci (- n 1)) (fibonacci (- n 2))))))

(io.printf "%d" ((fibonacci 5)))
`.trim();
  execute(script);
  expect(write).toBeCalledWith("8");
});
