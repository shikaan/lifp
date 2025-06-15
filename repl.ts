//@ts-check
import {tokenize} from "./tokenize.ts";

/**
 * @param {string} line
 * @returns {AST}
 */
export function read(line) {
  return tokenize(line);
}

/**
 * @param {AST} tree
 * @returns {string}
 */
export function evaluate(tree) {
  return tree;
}

/**
 * @param {string} line
 * @returns {string}
 */
export function print(line) {
  return line;
}



