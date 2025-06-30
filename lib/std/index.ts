import type { Lambda } from "../types.js";
import { core } from "./core.js";
import { io } from "./io.js";
import { lists } from "./lists.js";
import { math } from "./math.ts";
import { strings } from "./strings.ts";

export const std: Record<string, Lambda> = {
  ...core,
  ...io,
  ...lists,
  ...strings,
  ...math,
};
