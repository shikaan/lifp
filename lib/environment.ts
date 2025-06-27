import { SymbolNotFoundException } from "./errors.js";
import { std } from "./std/index.js";
import { ASTNodeType, type Expression } from "./types.js";

export class Environment {
  constructor(private parent: Environment = null) {}

  private readonly std = std;
  private locals: Map<string, Expression> = new Map();

  private getFromStandardLibrary(name: string): Expression | undefined {
    return this.std[name] != null
      ? {
          type: ASTNodeType.FUNCTION,
          value: this.std[name],
        }
      : null;
  }

  set(name: string, fn: Expression) {
    this.locals.set(name, fn);
  }

  get(name: string): Expression {
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
