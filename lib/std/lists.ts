import { InvalidArgumentException } from "../errors.js";
import { type ASTNodeList, ASTNodeType, type Lambda } from "../types.js";

export const lists: Record<string, Lambda> = {
  /**
   * Counts elements in a list.
   * @name list.count
   * @example
   *   (list.count (1 2)) ; 2
   */
  "list.count": (nodes) => {
    if (nodes.length !== 1 || nodes[0].type !== ASTNodeType.LIST) {
      throw new InvalidArgumentException(
        "'list.count' takes a list as argument. Example: (list.count my-list).",
      );
    }
    return {
      type: ASTNodeType.NUMBER,
      value: nodes[0].value.length,
    };
  },
  /**
   * Maps a lambda over a list.
   * @name list.map
   * @example
   *   (list.map (fn* (item idx) (+ item idx)) (1 2 3)) ; (1 3 5)
   */
  "list.map": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[0].type !== ASTNodeType.FUNCTION
    ) {
      throw new InvalidArgumentException(
        "'list.map' takes a lambda and a list as argument. Example: (list.map (fn* (item idx) (myfn item idx)) my-list).",
      );
    }
    const [lambda, list] = nodes;
    return {
      type: ASTNodeType.LIST,
      value: list.value.map((node: ASTNodeList, value: number) =>
        lambda.value([node, { type: ASTNodeType.NUMBER, value }]),
      ),
    };
  },
  /**
   * Applies a lambda to each element in a list (for side effects).
   * @name list.each
   * @example
   *   (list.each (fn* (item idx) (print item)) (1 2 3)) ; nil
   */
  "list.each": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[0].type !== ASTNodeType.FUNCTION
    ) {
      throw new InvalidArgumentException(
        "'list.each' takes a lambda and a list as argument. Example: (list.each (fn* (item idx) (myfn item idx)) my-list).",
      );
    }
    const [lambda, list] = nodes;
    list.value.forEach((node: ASTNodeList, value) =>
      lambda.value([node, { type: ASTNodeType.NUMBER, value }]),
    );
    return {
      type: ASTNodeType.NIL,
      value: null,
    };
  },
  /**
   * Creates a list from the given arguments.
   * @name list.from
   * @example
   *   (list.from 1 2 3) ; (1 2 3)
   */
  "list.from": (nodes) => ({
    type: ASTNodeType.LIST,
    value: nodes,
  }),
  /**
   * Returns the nth element of a list, or nil if out of bounds.
   * @name list.nth
   * @example
   *   (list.nth 1 (10 20 30)) ; 20
   */
  "list.nth": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[0].type !== ASTNodeType.NUMBER ||
      nodes[1].type !== ASTNodeType.LIST
    ) {
      throw new InvalidArgumentException(
        "'list.nth' takes a number and a list as argument. Example: (list.nth 0 my-list).",
      );
    }

    const [index, list] = nodes;
    const item = list.value[index.value];

    return (
      item ?? {
        type: ASTNodeType.NIL,
        value: null,
      }
    );
  },
  /**
   * Filters a list using a lambda predicate.
   * @name list.filter
   * @example
   *   (list.filter (fn* (item idx) (> item 0)) (-1 0 1 2)) ; (1 2)
   */
  "list.filter": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[0].type !== ASTNodeType.FUNCTION
    ) {
      throw new InvalidArgumentException(
        "'list.filter' takes a lambda and a list as argument. Example: (list.filter (fn* (item idx) (= item 0)) my-list).",
      );
    }
    const [lambda, list] = nodes;
    return {
      type: ASTNodeType.LIST,
      value: list.value.filter(
        (node: ASTNodeList, value: number) =>
          lambda.value([node, { type: ASTNodeType.NUMBER, value }]).value,
      ),
    };
  },
};
