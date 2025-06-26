import { Environment } from "./environment.js";
import {
  type AbstractSyntaxTree,
  type ASTNodeList,
  ASTNodeType,
  type Expression,
  isListNode,
} from "./types.js";
import { specials } from "./specials.js";

const handleEnvironmentSymbol = (
  tree: ASTNodeList,
  environment: Environment,
  symbol: string,
) => {
  const expression = environment.get(symbol);
  if (expression.type === ASTNodeType.FUNCTION) {
    const args = tree.value
      .slice(1)
      .map((subtree) => evaluate(subtree, environment));

    return expression.value(args);
  }

  return expression;
};

export const evaluate = (
  tree: AbstractSyntaxTree,
  environment: Environment,
): Expression => {
  if (!isListNode(tree)) {
    return tree.type === ASTNodeType.SYMBOL
      ? environment.get(tree.value)
      : tree;
  }

  if (tree.value.length === 0) return tree;

  const firstNode = tree.value[0];
  if (firstNode.type === ASTNodeType.SYMBOL) {
    const specialForm = specials[firstNode.value];

    if (specialForm) {
      return specialForm(tree.value, environment);
    }

    return handleEnvironmentSymbol(tree, environment, firstNode.value);
  }

  return {
    type: ASTNodeType.LIST,
    value: tree.value.map((subtree) => evaluate(subtree, environment)),
  };
};
