import {
  type ASTNode,
  type ASTNodeList,
  ASTNodeType,
  type Token,
  type TokenType,
} from "../lib/types.js";

// Creates an ASTNode. Short notation for tests
export const n = (type: ASTNodeType, value: unknown): ASTNode =>
  ({ type, value }) as ASTNode;

// Creates an ASTNode List. Short notation for tests
export const l = (elements: unknown[]): ASTNodeList =>
  n(ASTNodeType.LIST, elements) as ASTNodeList;

// Creates a Lexer Token. Short notation for tests
export const t = (type: TokenType, literal?: string | number): Token =>
  ({ type, literal }) as Token;
