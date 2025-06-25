import { expect, test } from "bun:test";
import { n } from "../tests/utils.js";
import { Environment } from "./environment.js";
import { std } from "./std.js";
import { ASTNodeType } from "./types.js";

test("allows overrides", () => {
  const num = n(ASTNodeType.NUMBER, 12);
  const env = new Environment();
  env.setVariable("a", n(ASTNodeType.NUMBER, 23));
  env.setVariable("a", num);

  expect(env.getVariable("a")).toEqual(num);
});

test("looks for variables in current scope", () => {
  const num = n(ASTNodeType.NUMBER, 12);
  const env = new Environment();
  env.setVariable("a", num);

  expect(env.getVariable("a")).toEqual(num);
});

test("looks for variables in upper scopes", () => {
  const num = n(ASTNodeType.NUMBER, 12);
  const parent = new Environment();
  const child = new Environment(parent);
  parent.setVariable("a", num);

  expect(child.getVariable("a")).toEqual(num);
});

test("lower scopes shadow upper scopes", () => {
  const num = n(ASTNodeType.NUMBER, 12);
  const parent = new Environment();
  parent.setVariable("a", n(ASTNodeType.NUMBER, 34));
  const child = new Environment(parent);
  child.setVariable("a", num);

  expect(child.getVariable("a")).toEqual(num);
});

test("returns functions from standard library", () => {
  const environment = new Environment();

  for (const fn of Object.keys(std)) {
    expect(environment.getFunction(fn)).not.toBeNil();
  }
});
