import { InvalidArgumentException } from "./errors.js";
import { type ASTNode, ASTNodeType, type Environment } from "./types.js";

const mathFunction = (
  nodes: ASTNode[],
  name: string,
  cb: (a: number, b: number) => number,
  initialValue = 0,
): ASTNode => {
  if (nodes.length < 2) {
    throw new InvalidArgumentException(
      `Function '${name}' requires at least 2 arguments. Got ${nodes.length}.`,
    );
  }

  let value = initialValue;
  for (const node of nodes) {
    if (node.type !== ASTNodeType.NUMBER) {
      throw new InvalidArgumentException(
        `Function '${name}' takes only numbers as argument. Got '${node.value}'.`,
      );
    }

    value = cb(node.value, value);
  }

  return {
    type: ASTNodeType.NUMBER,
    value,
  };
};

const compareFunction = (
  nodes: ASTNode[],
  name: string,
  callback: <T>(a: T, b: T) => boolean,
): ASTNode => {
  if (nodes.length !== 2) {
    throw new InvalidArgumentException(
      `Function '${name}' requires 2 arguments. Got ${nodes.length}`,
    );
  }

  const [first, second] = nodes;

  if (first.type !== second.type) {
    throw new InvalidArgumentException(
      `Cannot compare arguments of different type`,
    );
  }

  return {
    type: ASTNodeType.BOOLEAN,
    value: callback(first.value, second.value),
  };
};

export const defaultEnvironment: Environment = {
  functions: {
    "+": (nodes) => mathFunction(nodes, "+", (a, b) => a + b),
    "-": (nodes) => mathFunction(nodes, "-", (a, b) => a - b),
    "*": (nodes) => mathFunction(nodes, "*", (a, b) => a * b, 1),
    "/": (nodes) => {
      if (nodes.length < 2) {
        throw new InvalidArgumentException(
          `Function '/' requires at least 2 arguments. Got ${nodes.length}.`,
        );
      }

      if (nodes[0].type !== ASTNodeType.NUMBER) {
        throw new InvalidArgumentException(
          `Function '/' takes only numbers as argument. Got '${nodes[0].value}'.`,
        );
      }

      let value = nodes[0].value;
      for (let i = 1; i < nodes.length; i++) {
        const node = nodes[i];
        if (node.type !== ASTNodeType.NUMBER) {
          throw new InvalidArgumentException(
            `Function '/' takes only numbers as argument. Got '${nodes[0].value}'.`,
          );
        }
        if (node.value === 0) {
          throw new InvalidArgumentException(`Cannot divide by zero.`);
        }
        value /= node.value;
      }

      return {
        type: ASTNodeType.NUMBER,
        value,
      };
    },
    "=": (nodes) => compareFunction(nodes, "=", (a, b) => a === b),
    "<": (nodes) => compareFunction(nodes, "<", (a, b) => a < b),
    ">": (nodes) => compareFunction(nodes, ">", (a, b) => a > b),
    "!=": (nodes) => compareFunction(nodes, "!=", (a, b) => a !== b),
    "<=": (nodes) => compareFunction(nodes, "<=", (a, b) => a <= b),
    ">=": (nodes) => compareFunction(nodes, ">=", (a, b) => a >= b),
  },
  variables: {},
};
