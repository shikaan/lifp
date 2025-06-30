import { InvalidArgumentException } from "../errors.js";
import { isNumber, type Lambda } from "../types.js";
import { setTimeout } from "node:timers/promises";

export const flow: Record<string, Lambda> = {
  /**
   * Sleeps for the given number of milliseconds.
   * @name flow.sleep
   * @example
   *   (flow.sleep 1000) ; sleeps for ~1 second
   */
  "flow.sleep": async (nodes) => {
    if (nodes.length !== 1 || !isNumber(nodes[0])) {
      throw new InvalidArgumentException(
        "'flow.sleep' takes a single number (ms) argument.",
      );
    }
    const ms = nodes[0];
    if (ms < 0)
      throw new InvalidArgumentException(
        "'flow.sleep' requires a non-negative number.",
      );
    return setTimeout(ms, null);
  },
};
