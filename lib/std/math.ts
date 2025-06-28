import { InvalidArgumentException } from "../errors.js";
import { isList, isNull, isNumber, type Lambda, type Value } from "../types.js";

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

const isSameType = (a: unknown, b: unknown) => {
  if (isNull(a) || isNull(b)) return isNull(a) && isNull(b);
  if (isList(a) || isList(b)) return isList(a) && isList(b);
  return typeof a === typeof b;
};

const compareFunction = (
  nodes: Value[],
  name: string,
  callback: <T>(a: T, b: T) => boolean,
): Value => {
  if (nodes.length !== 2) {
    throw new InvalidArgumentException(
      `'${name}' requires 2 arguments. Got ${nodes.length}`,
    );
  }

  const [first, second] = nodes;

  if (!isSameType(first, second)) {
    throw new InvalidArgumentException(
      `Cannot compare arguments of different type`,
    );
  }

  return callback(first, second);
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
