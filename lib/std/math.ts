import { InvalidArgumentException } from "../errors.js";
import { isList, isNumber, type Lambda } from "../types.js";

export const math: Record<string, Lambda> = {
  // ...existing code...
  /**
   * Returns the floor of a number.
   * @name math.floor
   * @example
   *   (math.floor 1.9) ; 1
   */
  "math.floor": async (nodes) => {
    if (nodes.length !== 1 || !isNumber(nodes[0])) {
      throw new InvalidArgumentException(
        "'math.floor' takes a single number argument.",
      );
    }
    return Math.floor(nodes[0]);
  },
  /**
   * Returns the ceiling of a number.
   * @name math.ceil
   * @example
   *   (math.ceil 1.1) ; 2
   */
  "math.ceil": async (nodes) => {
    if (nodes.length !== 1 || !isNumber(nodes[0])) {
      throw new InvalidArgumentException(
        "'math.ceil' takes a single number argument.",
      );
    }
    return Math.ceil(nodes[0]);
  },
  /**
   * Returns the maximum value in a list of numbers.
   * @name math.max
   * @example
   *   (math.max (1 2 3)) ; 3
   */
  "math.max": async (nodes) => {
    if (
      nodes.length !== 1 ||
      !isList(nodes[0]) ||
      nodes[0].length === 0 ||
      !nodes[0].every(isNumber)
    ) {
      throw new InvalidArgumentException(
        "'math.max' takes a single list of numbers as argument.",
      );
    }
    return Math.max(...nodes[0]);
  },
  /**
   * Returns the minimum value in a list of numbers.
   * @name math.min
   * @example
   *   (math.min (1 2 3)) ; 1
   */
  "math.min": async (nodes) => {
    if (
      nodes.length !== 1 ||
      !isList(nodes[0]) ||
      nodes[0].length === 0 ||
      !nodes[0].every(isNumber)
    ) {
      throw new InvalidArgumentException(
        "'math.min' takes a single list of numbers as argument.",
      );
    }
    return Math.min(...nodes[0]);
  },
};
