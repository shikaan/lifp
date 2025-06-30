import { InvalidArgumentException } from "../errors.js";
import { isNumber, type Lambda, type Value } from "../types.js";

const addOrMultiply = (
  values: Value[],
  name: "+" | "*",
  cb: (a: number, b: number) => number,
  initialValue = 0,
): Value => {
  if (values.length < 2) {
    throw new InvalidArgumentException(
      `'${name}' requires at least 2 arguments. Got ${values.length}.`,
    );
  }

  let result = initialValue;
  for (const value of values) {
    if (!isNumber(value)) {
      throw new InvalidArgumentException(
        `'${name}' takes only numbers as argument. Got '${value}'.`,
      );
    }
    result = cb(value, result);
  }

  return result;
};

const subtractOrDivide = (
  values: Value[],
  name: "-" | "/",
  cb: (a: number, b: number) => number,
): Value => {
  if (values.length < 2) {
    throw new InvalidArgumentException(
      `'${name}' requires at least 2 arguments. Got ${values.length}.`,
    );
  }

  if (!isNumber(values[0])) {
    throw new InvalidArgumentException(
      `'${name}' takes only numbers as argument. Got '${values[0]}'.`,
    );
  }

  let result = values[0];
  for (let i = 1; i < values.length; i++) {
    const value = values[i];
    if (!isNumber(value)) {
      throw new InvalidArgumentException(
        `'${name}' takes only numbers as argument. Got '${value}'.`,
      );
    }
    result = cb(result, value);
  }

  return result;
};

const compare = (
  nodes: Value[],
  name: string,
  callback: (a: unknown, b: unknown) => boolean,
): Value => {
  if (nodes.length !== 2) {
    throw new InvalidArgumentException(
      `'${name}' requires 2 arguments. Got ${nodes.length}`,
    );
  }

  return callback(nodes[0], nodes[1]);
};

export const core: Record<string, Lambda> = {
  /**
   * Adds numbers together.
   * @name +
   * @example
   *   (+ 1 2 3) ; 6
   */
  "+": async (nodes) => addOrMultiply(nodes, "+", (a, b) => a + b),
  /**
   * Subtracts numbers from the first argument.
   * @name -
   * @example
   *   (- 5 2 1) ; 2
   */
  "-": async (nodes) => subtractOrDivide(nodes, "-", (a, b) => a - b),
  /**
   * Multiplies numbers together.
   * @name *
   * @example
   *   (* 2 3 4) ; 24
   */
  "*": async (nodes) => addOrMultiply(nodes, "*", (a, b) => a * b, 1),
  /**
   * Divides the first argument by the rest.
   * @name /
   * @example
   *   (/ 8 2 2) ; 2
   */
  "/": async (nodes) => subtractOrDivide(nodes, "/", (a, b) => a / b),
  /**
   * Performs division with modulo.
   * @name %
   * @example
   *   (% 4 2) ; 0
   */
  "%": async (nodes) => {
    if (nodes.length !== 2 || !nodes.every(isNumber)) {
      throw new InvalidArgumentException(
        `'%' requires 2 number arguments. Example: (% 4 2)`,
      );
    }
    return nodes[0] % nodes[1];
  },
  /**
   * Checks if two values are equal.
   * @name =
   * @example
   *   (= 1 1) ; true
   */
  "=": async (nodes) => compare(nodes, "=", (a, b) => a === b),
  /**
   * Checks if the first value is less than the second.
   * @name <
   * @example
   *   (< 1 2) ; true
   */
  "<": async (nodes) => compare(nodes, "<", (a, b) => a < b),
  /**
   * Checks if the first value is greater than the second.
   * @name >
   * @example
   *   (> 2 1) ; true
   */
  ">": async (nodes) => compare(nodes, ">", (a, b) => a > b),
  /**
   * Checks if two values are not equal.
   * @name !=
   * @example
   *   (!= 1 2) ; true
   */
  "!=": async (nodes) => compare(nodes, "!=", (a, b) => a !== b),
  /**
   * Checks if the first value is less than or equal to the second.
   * @name <=
   * @example
   *   (<= 1 2) ; true
   */
  "<=": async (nodes) => compare(nodes, "<=", (a, b) => a <= b),
  /**
   * Checks if the first value is greater than or equal to the second.
   * @name >=
   * @example
   *   (>= 2 1) ; true
   */
  ">=": async (nodes) => compare(nodes, ">=", (a, b) => a >= b),
  /**
   * Logical AND for two boolean values.
   * @name and
   * @example
   *   (and true false) ; false
   */
  and: async (nodes) => compare(nodes, "and", (a, b) => !!(a && b)),
  /**
   * Logical OR for two boolean values.
   * @name or
   * @example
   *   (or true false) ; true
   */
  or: async (nodes) => compare(nodes, "or", (a, b) => !!(a || b)),
};
