import { InvalidArgumentException } from "../errors.js";
import { ASTNodeType, type Expression, type Lambda } from "../types.js";

const addOrMultiply = (
  nodes: Expression[],
  name: "+" | "*",
  cb: (a: number, b: number) => number,
  initialValue = 0,
): Expression => {
  if (nodes.length < 2) {
    throw new InvalidArgumentException(
      `'${name}' requires at least 2 arguments. Got ${nodes.length}.`,
    );
  }

  let value = initialValue;
  for (const node of nodes) {
    if (node.type !== ASTNodeType.NUMBER) {
      throw new InvalidArgumentException(
        `'${name}' takes only numbers as argument. Got '${node.value}'.`,
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
      `'${name}' requires at least 2 arguments. Got ${nodes.length}.`,
    );
  }

  if (nodes[0].type !== ASTNodeType.NUMBER) {
    throw new InvalidArgumentException(
      `'${name}' takes only numbers as argument. Got '${nodes[0].value}'.`,
    );
  }

  let value = nodes[0].value;
  for (let i = 1; i < nodes.length; i++) {
    const node = nodes[i];
    if (node.type !== ASTNodeType.NUMBER) {
      throw new InvalidArgumentException(
        `'${name}' takes only numbers as argument. Got '${node.value}'.`,
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
      `'${name}' requires 2 arguments. Got ${nodes.length}`,
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

export const math: Record<string, Lambda> = {
  /**
   * Adds numbers together.
   * @name +
   * @example
   *   (+ 1 2 3) ; 6
   */
  "+": (nodes) => addOrMultiply(nodes, "+", (a, b) => a + b),
  /**
   * Subtracts numbers from the first argument.
   * @name -
   * @example
   *   (- 5 2 1) ; 2
   */
  "-": (nodes) => subtractOrDivide(nodes, "-", (a, b) => a - b),
  /**
   * Multiplies numbers together.
   * @name *
   * @example
   *   (* 2 3 4) ; 24
   */
  "*": (nodes) => addOrMultiply(nodes, "*", (a, b) => a * b, 1),
  /**
   * Divides the first argument by the rest.
   * @name /
   * @example
   *   (/ 8 2 2) ; 2
   */
  "/": (nodes) => subtractOrDivide(nodes, "/", (a, b) => a / b),
  /**
   * Checks if two values are equal.
   * @name =
   * @example
   *   (= 1 1) ; true
   */
  "=": (nodes) => compareFunction(nodes, "=", (a, b) => a === b),
  /**
   * Checks if the first value is less than the second.
   * @name <
   * @example
   *   (< 1 2) ; true
   */
  "<": (nodes) => compareFunction(nodes, "<", (a, b) => a < b),
  /**
   * Checks if the first value is greater than the second.
   * @name >
   * @example
   *   (> 2 1) ; true
   */
  ">": (nodes) => compareFunction(nodes, ">", (a, b) => a > b),
  /**
   * Checks if two values are not equal.
   * @name !=
   * @example
   *   (!= 1 2) ; true
   */
  "!=": (nodes) => compareFunction(nodes, "!=", (a, b) => a !== b),
  /**
   * Checks if the first value is less than or equal to the second.
   * @name <=
   * @example
   *   (<= 1 2) ; true
   */
  "<=": (nodes) => compareFunction(nodes, "<=", (a, b) => a <= b),
  /**
   * Checks if the first value is greater than or equal to the second.
   * @name >=
   * @example
   *   (>= 2 1) ; true
   */
  ">=": (nodes) => compareFunction(nodes, ">=", (a, b) => a >= b),
  /**
   * Logical AND for two boolean values.
   * @name and
   * @example
   *   (and true false) ; false
   */
  and: (nodes) => compareFunction(nodes, "and", (a, b) => !!(a && b)),
  /**
   * Logical OR for two boolean values.
   * @name or
   * @example
   *   (or true false) ; true
   */
  or: (nodes) => compareFunction(nodes, "or", (a, b) => !!(a || b)),
};
