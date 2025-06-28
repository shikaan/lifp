import { InvalidArgumentException } from "../errors.js";
import { type ASTNode, ASTNodeType, type Lambda } from "../types.js";

export const strings: Record<string, Lambda> = {
  /**
   * Returns the length of a string.
   * @name string.length
   * @example
   *   (string.length "hello") ; 5
   */
  "string.length": (nodes) => {
    if (nodes.length !== 1 || nodes[0].type !== ASTNodeType.STRING) {
      throw new InvalidArgumentException(
        "'string.length' takes a string as argument. Example: (string.length \"hello\")",
      );
    }
    return { type: ASTNodeType.NUMBER, value: nodes[0].value.length };
  },
  /**
   * Joins a list of strings with a separator.
   * @name string.join
   * @example
   *   (string.join "," ("foo" "bar")) ; "foo,bar"
   */
  "string.join": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[0].type !== ASTNodeType.STRING ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[1].value.some((n) => n.type !== ASTNodeType.STRING)
    ) {
      throw new InvalidArgumentException(
        '\'string.join\' takes a separator and a list of strings as arguments. Example: (string.join "," ("foo" "bar"))',
      );
    }

    const [separator, list] = nodes;

    return {
      type: ASTNodeType.STRING,
      value: list.value.map((n: ASTNode) => n.value).join(separator.value),
    };
  },
  /**
   * Returns a substring from start to end indices.
   * @name string.slice
   * @example
   *   (string.slice "hello" 1 4) ; "ell"
   */
  "string.slice": (nodes) => {
    if (
      nodes.length !== 3 ||
      nodes[0].type !== ASTNodeType.STRING ||
      nodes[1].type !== ASTNodeType.NUMBER ||
      nodes[2].type !== ASTNodeType.NUMBER
    ) {
      throw new InvalidArgumentException(
        "'string.slice' takes a string and two numbers as arguments.",
      );
    }
    return {
      type: ASTNodeType.STRING,
      value: nodes[0].value.slice(nodes[1].value, nodes[2].value),
    };
  },
  /**
   * Checks if a string contains a substring.
   * @name string.includes
   * @example
   *   (string.includes "hello" "ell") ; true
   */
  "string.includes": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[0].type !== ASTNodeType.STRING ||
      nodes[1].type !== ASTNodeType.STRING
    ) {
      throw new InvalidArgumentException(
        "'string.includes' takes two strings as arguments.",
      );
    }
    return {
      type: ASTNodeType.BOOLEAN,
      value: nodes[0].value.includes(nodes[1].value),
    };
  },
  /**
   * Trims whitespace from both ends of a string.
   * @name string.trim
   * @example
   *   (string.trim "  hello  ") ; "hello"
   */
  "string.trim": (nodes) => {
    if (nodes.length !== 1 || nodes[0].type !== ASTNodeType.STRING) {
      throw new InvalidArgumentException(
        "'string.trim' takes a string as argument.",
      );
    }
    return { type: ASTNodeType.STRING, value: nodes[0].value.trim() };
  },
};
