export type Environment = string;
export type Expression = string;

export enum ASTNodeType {
  LIST,
  NUMBER,
  BOOLEAN,
  NIL,
  STRING,
  SYMBOL,
  KEYWORD,
}

export type ASTNodeList = { type: ASTNodeType.LIST; value: ASTNode[] };

export type ASTNode =
  | ASTNodeList
  | { type: ASTNodeType.NUMBER; value: number }
  | { type: ASTNodeType.BOOLEAN; value: boolean }
  | { type: ASTNodeType.NIL; value: null }
  | { type: ASTNodeType.STRING; value: string }
  | { type: ASTNodeType.SYMBOL; value: string }
  | { type: ASTNodeType.KEYWORD; value: string };

export type AbstractSyntaxTree = ASTNode;

export enum TokenType {
  LPAREN = "LPAREN",
  RPAREN = "RPAREN",
  NUMBER = "NUMBER",
  SYMBOL = "SYMBOL",
  STRING = "STRING",
  EOF = "EOF",
}

export type AtomToken =
  | { type: TokenType.NUMBER; literal: number }
  | { type: TokenType.SYMBOL; literal: string }
  | { type: TokenType.STRING; literal: string };

export const isAtomToken = (token: Token): token is AtomToken =>
  [TokenType.NUMBER, TokenType.STRING, TokenType.SYMBOL].includes(token.type);

export type Token =
  | AtomToken
  | { type: TokenType.LPAREN; literal: "(" }
  | { type: TokenType.RPAREN; literal: ")" }
  | { type: TokenType.EOF };
