import type { Environment } from "./environment.js";
import { specials } from "./specials.js";
import {
  type AbstractSyntaxTree,
  ASTNodeType,
  type Expression,
  isListNode,
  type SpecialFormHandler,
} from "./types.js";

export const evaluate = (
  tree: AbstractSyntaxTree,
  environment: Environment,
  specialForms: typeof specials = specials,
): Expression => {
  if (!isListNode(tree)) {
    return tree.type === ASTNodeType.SYMBOL
      ? environment.get(tree.value)
      : tree;
  }

  if (tree.value.length === 0) return tree;

  const firstNode = tree.value[0];
  if (firstNode.type === ASTNodeType.SYMBOL) {
    const specialForm: SpecialFormHandler | undefined =
      specialForms[firstNode.value];

    if (specialForm) {
      return specialForm(tree.value, environment);
    }

    const expression = environment.get(firstNode.value);

    if (expression.type === ASTNodeType.FUNCTION) {
      const lambda = expression.value;
      const args = tree.value.slice(1);
      const resolvedArgs = args.map((subtree) => {
        return evaluate(subtree, environment);
      });

      return lambda(resolvedArgs, environment);
    }
  }

  return {
    type: ASTNodeType.LIST,
    value: tree.value.map((subtree) => evaluate(subtree, environment)),
  };
};
