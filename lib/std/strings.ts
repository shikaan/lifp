import { InvalidArgumentException } from "../errors.js";
import { type Lambda, isString, isList, isNumber } from "../types.js";

export const strings: Record<string, Lambda> = {
  /**
   * Returns the length of a string.
   * @name string.length
   * @example
   *   (string.length "hello") ; 5
   */
  "string.length": async (nodes) => {
    if (nodes.length !== 1 || !isString(nodes[0])) {
      throw new InvalidArgumentException(
        "'string.length' takes a string as argument. Example: (string.length \"hello\")",
      );
    }
    return nodes[0].length;
  },
  /**
   * Joins a list of strings with a separator.
   * @name string.join
   * @example
   *   (string.join "," ("foo" "bar")) ; "foo,bar"
   */
  "string.join": async (nodes) => {
    if (
      nodes.length !== 2 ||
      !isString(nodes[0]) ||
      !isList(nodes[1]) ||
      !nodes[1].every(isString)
    ) {
      throw new InvalidArgumentException(
        '\'string.join\' takes a separator and a list of strings as arguments. Example: (string.join "," ("foo" "bar"))',
      );
    }

    const [separator, list] = nodes;
    return list.join(separator);
  },
  /**
   * Returns a substring from start to end indices.
   * @name string.slice
   * @example
   *   (string.slice "hello" 1 4) ; "ell"
   */
  "string.slice": async (nodes) => {
    if (
      nodes.length !== 3 ||
      !isString(nodes[0]) ||
      !isNumber(nodes[1]) ||
      !isNumber(nodes[2])
    ) {
      throw new InvalidArgumentException(
        "'string.slice' takes a string and two numbers as arguments. Example: (string.slice \"hello\" 1 4)",
      );
    }
    return nodes[0].slice(nodes[1], nodes[2]);
  },
  /**
   * Checks if a string contains a substring.
   * @name string.includes
   * @example
   *   (string.includes "hello" "ell") ; true
   */
  "string.includes": async (nodes) => {
    if (nodes.length !== 2 || !nodes.every(isString)) {
      throw new InvalidArgumentException(
        "'string.includes' takes two strings as arguments.",
      );
    }
    return nodes[0].includes(nodes[1]);
  },
  /**
   * Trims whitespace from both ends of a string.
   * @name string.trim
   * @example
   *   (string.trim "  hello  ") ; "hello"
   */
  "string.trim": async (nodes) => {
    if (nodes.length !== 1 || !isString(nodes[0])) {
      throw new InvalidArgumentException(
        "'string.trim' takes a string as argument.",
      );
    }
    return nodes[0].trim();
  },
};
