import { DEF, FN, LET } from "./constants.js";
import { Environment } from "./environment.js";
import { InvalidArgumentException } from "./errors.js";
import {
  type AbstractSyntaxTree,
  type ASTNodeList,
  type ASTNodeSymbol,
  ASTNodeType,
  type Expression,
  isListNode,
  isSymbol,
} from "./types.js";

const handleDef = (list: ASTNodeList, environment: Environment) => {
  if (list.value.length !== 3 || list.value[1].type !== ASTNodeType.KEYWORD) {
    throw new InvalidArgumentException(
      `'def!' requires a keyword and a form only. Example: (def! :a 123)`,
    );
  }

  const [, symbol, form] = list.value;
  environment.set(symbol.value.slice(1), evaluate(form, environment));
  return { type: ASTNodeType.NIL, value: null };
};

const handleLet = (list: ASTNodeList, environment: Environment) => {
  if (list.value.length !== 3 || !isListNode(list.value[1])) {
    throw new InvalidArgumentException(
      `'let*' requires a list of assignments. Example: (let* ((:a 12) (:b 34)) (other-form))`,
    );
  }

  const [, assignments, form] = list.value;
  const innerEnvironment = new Environment(environment);

  for (const pair of assignments.value) {
    if (!isListNode(pair) || pair.value[0].type !== ASTNodeType.KEYWORD) {
      throw new InvalidArgumentException(
        `'let*' requires a list of assignments. Example: (let* ((:a 12) (:b 34)) (other-form))`,
      );
    }
    const [sym, form] = pair.value;
    innerEnvironment.set(sym.value.slice(1), evaluate(form, innerEnvironment));
  }

  return evaluate(form, innerEnvironment);
};

const handleFn = (tree: ASTNodeList, environment: Environment): Expression => {
  if (!isListNode(tree) || tree.value.length !== 3) {
    throw new InvalidArgumentException(
      `'fn*' requires a binding list and a form. Example: (fn* (a b) (+ a b))`,
    );
  }

  const [, bindings, form] = tree.value;

  if (!isListNode(bindings) || bindings.value.some((n) => !isSymbol(n))) {
    throw new InvalidArgumentException(
      `'fn*' requires a binding list and a form. Example: (fn* (a b) (+ a b))`,
    );
  }

  // Array.some does not enforce types
  const values = bindings.value as ASTNodeSymbol[];
  return {
    type: ASTNodeType.FUNCTION,
    value: (nodes) => {
      const innerEnvironment = new Environment(environment);
      values.forEach((sym, i) => innerEnvironment.set(sym.value, nodes[i]));
      return evaluate(form, innerEnvironment);
    },
  };
};

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
    switch (firstNode.value) {
      case DEF:
        return handleDef(tree, environment);
      case LET:
        return handleLet(tree, environment);
      case FN:
        return handleFn(tree, environment);
      default:
        return handleEnvironmentSymbol(tree, environment, firstNode.value);
    }
  }

  return {
    type: ASTNodeType.LIST,
    value: tree.value.map((subtree) => evaluate(subtree, environment)),
  };
};
