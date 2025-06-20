export const TokenTag = {
  NUMBER: "number",
  STRING: "string",
  BOOLEAN: "boolean",
  OPERATOR: "operator",
} as const;

export const Operator = {
  NOT: "not",
  PLUS: "+",
} as const;

export type TokenTagType = (typeof TokenTag)[keyof typeof TokenTag];
export type OperatorType = (typeof Operator)[keyof typeof Operator];
export type TokenType = OperatorType | boolean | string | number;
export type Token<T = TokenType> = [TokenTagType, T];
export type AST = (Token | AST)[];

export const OPERATORS = Object.values(Operator);
