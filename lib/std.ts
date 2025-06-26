import { InvalidArgumentException } from "./errors.js";
import { ASTNodeType, type Expression, type Reduction } from "./types.js";

const addOrMultiply = (
  nodes: Expression[],
  name: "+" | "*",
  cb: (a: number, b: number) => number,
  initialValue = 0,
): Expression => {
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

const subtractOrDivide = (
  nodes: Expression[],
  name: "-" | "/",
  cb: (a: number, b: number) => number,
): Expression => {
  if (nodes.length < 2) {
    throw new InvalidArgumentException(
      `Function '${name}' requires at least 2 arguments. Got ${nodes.length}.`,
    );
  }

  if (nodes[0].type !== ASTNodeType.NUMBER) {
    throw new InvalidArgumentException(
      `Function '${name}' takes only numbers as argument. Got '${nodes[0].value}'.`,
    );
  }

  let value = nodes[0].value;
  for (let i = 1; i < nodes.length; i++) {
    const node = nodes[i];
    if (node.type !== ASTNodeType.NUMBER) {
      throw new InvalidArgumentException(
        `Function '${name}' takes only numbers as argument. Got '${node.value}'.`,
      );
    }
    value = cb(value, node.value);
  }

  return {
    type: ASTNodeType.NUMBER,
    value,
  };
};

const compareFunction = (
  nodes: Expression[],
  name: string,
  callback: <T>(a: T, b: T) => boolean,
): Expression => {
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

export const std: Record<string, Reduction> = {
  "+": (nodes) => addOrMultiply(nodes, "+", (a, b) => a + b),
  "-": (nodes) => subtractOrDivide(nodes, "-", (a, b) => a - b),
  "*": (nodes) => addOrMultiply(nodes, "*", (a, b) => a * b, 1),
  "/": (nodes) => subtractOrDivide(nodes, "/", (a, b) => a / b),
  "=": (nodes) => compareFunction(nodes, "=", (a, b) => a === b),
  "<": (nodes) => compareFunction(nodes, "<", (a, b) => a < b),
  ">": (nodes) => compareFunction(nodes, ">", (a, b) => a > b),
  "!=": (nodes) => compareFunction(nodes, "!=", (a, b) => a !== b),
  "<=": (nodes) => compareFunction(nodes, "<=", (a, b) => a <= b),
  ">=": (nodes) => compareFunction(nodes, ">=", (a, b) => a >= b),
  and: (nodes) => compareFunction(nodes, "and", (a, b) => !!(a && b)),
  or: (nodes) => compareFunction(nodes, "or", (a, b) => !!(a || b)),
};
