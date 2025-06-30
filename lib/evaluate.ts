import type { Environment } from "./environment.js";
import { specials } from "./specials.js";
import {
  isListNode,
  type Node,
  NodeType,
  type SpecialFormHandler,
  type Value,
} from "./types.js";

export const evaluate = async (
  tree: Node,
  environment: Environment,
  specialForms: typeof specials = specials,
): Promise<Value> => {
  if (!isListNode(tree)) {
    return tree.type === NodeType.SYMBOL
      ? environment.get(tree.value)
      : tree.value;
  }

  if (tree.value.length === 0) return [];

  const firstNode = tree.value[0];
  if (firstNode.type === NodeType.SYMBOL) {
    const specialForm: SpecialFormHandler | undefined =
      specialForms[firstNode.value];

    if (specialForm) {
      return specialForm(tree.value, environment);
    }

    const value = environment.get(firstNode.value);

    if (typeof value === "function") {
      const lambda = value;
      const args = tree.value.slice(1);
      const resolvedArgs = await Promise.all(
        args.map((subtree) => evaluate(subtree, environment)),
      );

      return lambda(resolvedArgs);
    }
  }

  return Promise.all(
    tree.value.map((subtree) => evaluate(subtree, environment)),
  );
};
