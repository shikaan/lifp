import { math } from "./math.js";
import { io } from "./io.js";
import { lists } from "./lists.js";
import { Lambda } from "../types.js";
import { strings } from "./strings.ts";

export const std: Record<string, Lambda> = {
  ...math,
  ...io,
  ...lists,
  ...strings,
};
