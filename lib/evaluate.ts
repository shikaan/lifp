import { SymbolNotFoundException } from "./errors.js";
import {
  type AbstractSyntaxTree,
  ASTNodeType,
  type Environment,
  type Expression,
  isListNode,
} from "./types.js";

export const evaluate = (
  tree: AbstractSyntaxTree,
  environment: Environment,
): Expression => {
  if (!isListNode(tree) || tree.value.length === 0) {
    return tree;
  }

  const firstNode = tree.value[0];
  if (firstNode.type === ASTNodeType.SYMBOL) {
    const fn = environment.functions[firstNode.value];

    if (!fn) {
      throw new SymbolNotFoundException(
        `Symbol '${firstNode.value}' cannot be found in the current environment.`,
      );
    }

    const args = tree.value
      .slice(1)
      .map((subtree) => evaluate(subtree, environment));

    return fn(args);
  }

  return {
    type: ASTNodeType.LIST,
    value: tree.value.map((subtree) => evaluate(subtree, environment)),
  };
};
