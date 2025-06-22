import type { AbstractSyntaxTree, Environment, Expression } from "./types.js";

export const evaluate = (
  tree: AbstractSyntaxTree,
  _environment: Environment,
): Expression => tree;
