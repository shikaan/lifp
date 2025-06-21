import {AST} from "./types.ts";

export const isSubtree = (node: any): node is AST => Array.isArray(node[0]);
