export const TokenTag = {
  NUMBER: "number",
  STRING: "string",
  BOOLEAN: "boolean",
  SYMBOL: "symbol",
} as const;

export const Symbol = {
  NOT: "!",
  PLUS: "+",
  MINUS: "-",
  WILDCARD: "*",
  SLASH: "/",
} as const;

export type TokenTagType = (typeof TokenTag)[keyof typeof TokenTag];
export type SymbolType = (typeof Symbol)[keyof typeof Symbol];
export type TokenType = SymbolType | boolean | string | number;
export type Token<T = TokenType> = [TokenTagType, T];
export type AST = (Token | AST)[];

export const SYMBOLS = Object.values(Symbol);
