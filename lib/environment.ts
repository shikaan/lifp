import { std } from "./std.js";
import type { ASTNode, Reduction } from "./types.js";

export class Environment {
  constructor(private parent: Environment = null) {}

  private readonly std = std;

  private functions: Map<string, Reduction> = new Map();
  private variables: Map<string, ASTNode> = new Map();

  setVariable(name: string, node: ASTNode) {
    this.variables.set(name, node);
  }

  getVariable(name: string): ASTNode | undefined {
    return this.variables.get(name) ?? this.parent?.getVariable(name);
  }

  getFunction(name: string): Reduction | undefined {
    return (
      this.std[name] ??
      this.functions.get(name) ??
      this.parent?.getFunction(name)
    );
  }
}

export const defaultEnvironment = new Environment();
