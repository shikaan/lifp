import { expect, test } from "bun:test";
import { Environment } from "./environment.js";
import { std } from "./std/index.ts";

test("allows same-scope overrides", () => {
  const num = 12;
  const env = new Environment();
  env.set("a", 23);
  env.set("a", num);

  expect(env.get("a")).toEqual(num);
});

test("shadows upper scopes", () => {
  const num = 12;
  const parent = new Environment();
  parent.set("a", 34);
  const child = new Environment(parent);
  child.set("a", num);

  expect(child.get("a")).toEqual(num);
});

test("shadows std", () => {
  const num = 12;
  const env = new Environment();
  env.set("+", num);

  expect(env.get("+")).toEqual(num);
});

test("looks for variables in current scope", () => {
  const num = 12;
  const env = new Environment();
  env.set("a", num);

  expect(env.get("a")).toEqual(num);
});

test("looks for variables in upper scopes", () => {
  const num = 12;
  const parent = new Environment();
  const child = new Environment(parent);
  parent.set("a", num);

  expect(child.get("a")).toEqual(num);
});

test("returns functions from standard library", () => {
  const environment = new Environment();

  for (const fn of Object.keys(std)) {
    expect(environment.get(fn)).not.toBeNil();
  }
});
