import { math } from "./math.js";
import { io } from "./io.js";
import { lists } from "./lists.js";
import { Lambda } from "../types.js";

export const std: Record<string, Lambda> = {
  ...math,
  ...io,
  ...lists,
};
