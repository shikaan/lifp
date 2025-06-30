import { InvalidArgumentException } from "../errors.js";
import type { Value } from "../types.js";
import { isLambda, isList, isNumber, type Lambda } from "../types.js";

export const lists: Record<string, Lambda> = {
  /**
   * Counts elements in a list.
   * @name list.count
   * @example
   *   (list.count (1 2)) ; 2
   */
  "list.count": async (nodes) => {
    if (nodes.length !== 1 || !isList(nodes[0])) {
      throw new InvalidArgumentException(
        "'list.count' takes a list as argument.",
      );
    }
    return nodes[0].length;
  },
  /**
   * Maps a lambda over a list.
   * @name list.map
   * @example
   *   (list.map (fn* (item idx) (+ item idx)) (1 2 3)) ; (1 3 5)
   */
  "list.map": async (nodes) => {
    if (nodes.length !== 2 || !isLambda(nodes[0]) || !isList(nodes[1])) {
      throw new InvalidArgumentException(
        "'list.map' takes a lambda and a list as argument.",
      );
    }
    const [lambda, list] = nodes;
    return Promise.all(list.map((value, idx) => lambda([value, idx])));
  },
  /**
   * Applies a lambda to each element in a list (for side effects).
   * @name list.each
   * @example
   *   (list.each (fn* (item idx) (print item)) (1 2 3)) ; nil
   */
  "list.each": async (nodes) => {
    if (nodes.length !== 2 || !isLambda(nodes[0]) || !isList(nodes[1])) {
      throw new InvalidArgumentException(
        "'list.each' takes a lambda and a list as argument.",
      );
    }
    const [lambda, list] = nodes;
    for (let i = 0; i < list.length; i++) {
      const value = list[i];
      await lambda([value, i]);
    }
    return null;
  },
  /**
   * Creates a list from the given arguments.
   * @name list.from
   * @example
   *   (list.from 1 2 3) ; (1 2 3)
   */
  "list.from": async (nodes: Value[]) => nodes,
  /**
   * Creates a list by repeatedly calling a lambda.
   *
   * @name list.times
   * @example
   *   (list.times (fn* (idx) idx) 3) ; (0 1 2)
   */
  "list.times": async (nodes: Value[]) => {
    if (nodes.length !== 2 || !isNumber(nodes[1]) || !isLambda(nodes[0])) {
      throw new InvalidArgumentException(
        "'list.times' takes a number and a list as arguments.",
      );
    }

    const [lambda, count] = nodes;

    const result = [];
    for (let i = 0; i < count; i++) {
      result.push(await lambda([i]));
    }

    return result;
  },
  /**
   * Returns the nth element of a list, or nil if out of bounds.
   * @name list.nth
   * @example
   *   (list.nth 1 (10 20 30)) ; 20
   */
  "list.nth": async (nodes) => {
    if (nodes.length !== 2 || !isNumber(nodes[0]) || !isList(nodes[1])) {
      throw new InvalidArgumentException(
        "'list.nth' takes a number and a list as arguments.",
      );
    }

    const [index, list] = nodes;
    return list[index] ?? null;
  },
  /**
   * Filters a list using a lambda predicate.
   * @name list.filter
   * @example
   *   (list.filter (fn* (item idx) (> item 0)) (-1 0 1 2)) ; (1 2)
   */
  "list.filter": async (nodes) => {
    if (nodes.length !== 2 || !isLambda(nodes[0]) || !isList(nodes[1])) {
      throw new InvalidArgumentException(
        "'list.filter' takes a lambda and a list as argument.",
      );
    }
    const [lambda, list] = nodes;

    const result = [];
    for (let i = 0; i < list.length; i++) {
      const value = list[i];
      if (await lambda([value, i])) {
        result.push(value);
      }
    }

    return result;
  },
};
