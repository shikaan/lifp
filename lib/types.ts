import type { DEF, FN, IF, LET, LPAREN, RPAREN } from "./constants.ts";
import type { Environment } from "./environment.js";

// Expressions

export type Lambda = (nodes: Value[]) => Promise<Value>;
export type Value = Value[] | number | boolean | null | string | Lambda;

export const isNumber = (n: unknown): n is number => typeof n === "number";
export const isString = (n: unknown): n is string => typeof n === "string";
export const isBoolean = (n: unknown): n is boolean => typeof n === "boolean";
export const isNull = (n: unknown): n is null => n == null;
export const isLambda = (n: unknown): n is Lambda => typeof n === "function";
export const isList = (n: unknown): n is Value[] => Array.isArray(n);

// Abstract Syntax Tree

export enum NodeType {
  LIST,
  NUMBER,
  BOOLEAN,
  NIL,
  STRING,
  SYMBOL,
}

export type Node =
  | NodeList
  | NodeSymbol
  | { type: NodeType.NUMBER; value: number; ptr: FilePointer }
  | { type: NodeType.BOOLEAN; value: boolean; ptr: FilePointer }
  | { type: NodeType.NIL; value: null; ptr: FilePointer }
  | { type: NodeType.STRING; value: string; ptr: FilePointer };

export type NodeList = { type: NodeType.LIST; value: Node[]; ptr: FilePointer };
export const isListNode = (n: Node): n is NodeList => n.type === NodeType.LIST;

export type NodeSymbol = {
  type: NodeType.SYMBOL;
  value: string;
  ptr: FilePointer;
};
export const isSymbol = (n: Node): n is NodeSymbol =>
  n.type === NodeType.SYMBOL;

// Tokenization

export enum TokenType {
  LPAREN,
  RPAREN,
  NUMBER,
  SYMBOL,
  STRING,
}

export type FilePointer = [file: string, line: number];

export type AtomToken =
  | { type: TokenType.NUMBER; literal: number; ptr: FilePointer }
  | { type: TokenType.SYMBOL; literal: string; ptr: FilePointer }
  | { type: TokenType.STRING; literal: string; ptr: FilePointer };

export const isAtomToken = (token: Token): token is AtomToken =>
  [TokenType.NUMBER, TokenType.STRING, TokenType.SYMBOL].includes(token.type);

export type Token =
  | AtomToken
  | { type: TokenType.LPAREN; literal: typeof LPAREN; ptr: FilePointer }
  | { type: TokenType.RPAREN; literal: typeof RPAREN; ptr: FilePointer };

////

export type SpecialFormType = typeof DEF | typeof FN | typeof LET | typeof IF;
export type SpecialFormHandler = (
  nodes: Node[],
  environment: Environment,
) => Promise<Value>;
