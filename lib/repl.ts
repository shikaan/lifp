//@ts-check
import {tokenize} from "./tokenize.ts";
import type {AST} from "./types.ts";

export function read(line: string): AST {
  return tokenize(line);
}

export function evaluate(tree: AST): string {
  return '';
}

export function print(line: string) {
  console.log(line)
}



