import type { Lambda } from "../types.js";
import { io } from "./io.js";
import { lists } from "./lists.js";
import { math } from "./math.js";
import { strings } from "./strings.ts";

export const std: Record<string, Lambda> = {
  ...math,
  ...io,
  ...lists,
  ...strings,
};
