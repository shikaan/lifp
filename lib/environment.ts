import { SymbolNotFoundException } from "./errors.js";
import { std } from "./std/index.js";
import type { Value } from "./types.js";

export class Environment {
  constructor(private parent: Environment = null) {}

  private readonly std = std;
  private locals: Map<string, Value> = new Map();

  private getFromStandardLibrary(name: string): Value | undefined {
    return this.std[name] ?? null;
  }

  set(name: string, fn: Value) {
    this.locals.set(name, fn);
  }

  get(name: string): Value {
    const expression =
      this.locals.get(name) ??
      this.parent?.get(name) ??
      this.getFromStandardLibrary(name);

    if (expression == null) {
      throw new SymbolNotFoundException(
        `Symbol '${name}' cannot be found in the current environment.`,
      );
    }

    return expression;
  }
}

export const defaultEnvironment = new Environment();
