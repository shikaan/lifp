import type { DEF, FN, IF, LET, LPAREN, RPAREN } from "./constants.ts";
import type { Environment } from "./environment.js";

export type Expression =
  | ASTNode
  | { type: ASTNodeType.LIST; value: Expression[] }
  | { type: ASTNodeType.FUNCTION; value: Lambda };

export enum ASTNodeType {
  LIST,
  NUMBER,
  BOOLEAN,
  NIL,
  STRING,
  SYMBOL,
  FUNCTION,
}

export type ASTNodeList = { type: ASTNodeType.LIST; value: ASTNode[] };
export const isListNode = (n: ASTNode): n is ASTNodeList =>
  n.type === ASTNodeType.LIST;

export type ASTNodeSymbol = { type: ASTNodeType.SYMBOL; value: string };
export const isSymbol = (n: ASTNode): n is ASTNodeSymbol =>
  n.type === ASTNodeType.SYMBOL;

export type ASTNode =
  | ASTNodeList
  | ASTNodeSymbol
  | { type: ASTNodeType.NUMBER; value: number }
  | { type: ASTNodeType.BOOLEAN; value: boolean }
  | { type: ASTNodeType.NIL; value: null }
  | { type: ASTNodeType.STRING; value: string };

export type AbstractSyntaxTree = ASTNode;

export enum TokenType {
  LPAREN,
  RPAREN,
  NUMBER,
  SYMBOL,
  STRING,
}

export type AtomToken =
  | { type: TokenType.NUMBER; literal: number }
  | { type: TokenType.SYMBOL; literal: string }
  | { type: TokenType.STRING; literal: string };

export const isAtomToken = (token: Token): token is AtomToken =>
  [TokenType.NUMBER, TokenType.STRING, TokenType.SYMBOL].includes(token.type);

export type Token =
  | AtomToken
  | { type: TokenType.LPAREN; literal: typeof LPAREN }
  | { type: TokenType.RPAREN; literal: typeof RPAREN };

export type Lambda = (nodes: Expression[]) => Expression;

export type SpecialFormType = typeof DEF | typeof FN | typeof LET | typeof IF;
export type SpecialFormHandler = (
  nodes: ASTNode[],
  environment: Environment,
) => Expression;
