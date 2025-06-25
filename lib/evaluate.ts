import { DEFINE, LET } from "./constants.js";
import { Environment } from "./environment.js";
import { InvalidArgumentException, SymbolNotFoundException } from "./errors.js";
import {
  type AbstractSyntaxTree,
  type ASTNodeList,
  ASTNodeType,
  type Expression,
  isListNode,
} from "./types.js";

const handleDefine = (list: ASTNodeList, environment: Environment) => {
  if (list.value.length !== 3 || list.value[1].type !== ASTNodeType.KEYWORD) {
    throw new InvalidArgumentException(
      `'def!' requires a keyword and a form only. Example: (def! :a 123)`,
    );
  }

  const [_, symbol, form] = list.value;
  environment.setVariable(symbol.value.slice(1), evaluate(form, environment));
  return { type: ASTNodeType.NIL, value: null };
};

const handleLet = (list: ASTNodeList, environment: Environment) => {
  if (list.value.length !== 3 || !isListNode(list.value[1])) {
    throw new InvalidArgumentException(
      `'let*' requires a list of assignments. Example: (let* ((:a 12) (:b 34)) (other-form))`,
    );
  }

  const [_, assignments, form] = list.value;
  const innerEnvironment = new Environment(environment);

  for (const pair of assignments.value) {
    if (!isListNode(pair) || pair.value[0].type !== ASTNodeType.KEYWORD) {
      throw new InvalidArgumentException(
        `'let*' requires a list of assignments. Example: (let* ((:a 12) (:b 34)) (other-form))`,
      );
    }
    const [symbol, form] = pair.value;
    innerEnvironment.setVariable(
      symbol.value.slice(1),
      evaluate(form, innerEnvironment),
    );
  }

  return evaluate(form, innerEnvironment);
};

const handleFunctionOrVariable = (
  tree: ASTNodeList,
  environment: Environment,
  firstNode: {
    type: ASTNodeType.SYMBOL;
    value: string;
  },
) => {
  const fn = environment.getFunction(firstNode.value);
  if (fn) {
    const args = tree.value
      .slice(1)
      .map((subtree) => evaluate(subtree, environment));

    return fn(args);
  }

  const val = environment.getVariable(firstNode.value);
  if (val) return val;

  throw new SymbolNotFoundException(
    `Symbol '${firstNode.value}' cannot be found in the current environment.`,
  );
};

export const evaluate = (
  tree: AbstractSyntaxTree,
  environment: Environment,
): Expression => {
  if (!isListNode(tree)) {
    if (tree.type === ASTNodeType.SYMBOL) {
      const fn = environment.getFunction(tree.value);
      if (fn) return tree;

      const val = environment.getVariable(tree.value);
      if (val) return val;

      throw new SymbolNotFoundException(
        `Symbol '${tree.value}' cannot be found in the current environment.`,
      );
    }

    return tree;
  }

  if (tree.value.length === 0) {
    return tree;
  }

  const firstNode = tree.value[0];
  if (firstNode.type === ASTNodeType.SYMBOL) {
    if (firstNode.value === DEFINE) {
      return handleDefine(tree, environment);
    }

    if (firstNode.value === LET) {
      return handleLet(tree, environment);
    }

    return handleFunctionOrVariable(tree, environment, firstNode);
  }

  return {
    type: ASTNodeType.LIST,
    value: tree.value.map((subtree) => evaluate(subtree, environment)),
  };
};
