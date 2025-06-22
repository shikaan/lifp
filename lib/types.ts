export type AbstractSyntaxTree = string;
export type Environment = string;
export type Expression = string;

export enum TokenType {
  LPAREN = "LPAREN",
  RPAREN = "RPAREN",
  NUMBER = "NUMBER",
  SYMBOL = "SYMBOL",
  STRING = "STRING",
  EOF = "EOF",
}

export type Token =
  | { type: TokenType.LPAREN; literal: "(" }
  | { type: TokenType.RPAREN; literal: ")" }
  | { type: TokenType.NUMBER; literal: number }
  | { type: TokenType.SYMBOL; literal: string }
  | { type: TokenType.STRING; literal: string }
  | { type: TokenType.EOF };
