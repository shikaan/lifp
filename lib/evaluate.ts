import type { AbstractSyntaxTree, Environment, Expression } from "./types.ts";

export const evaluate = (
  tree: AbstractSyntaxTree,
  _environment: Environment,
): Expression => tree;
