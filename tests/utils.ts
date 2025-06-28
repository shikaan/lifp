import {
  type Node,
  type NodeList,
  NodeType,
  type Token,
  type TokenType,
} from "../lib/types.js";

// Creates a Node. Short notation for tests
export const n = (type: NodeType, value: unknown): Node =>
  ({ type, value }) as Node;

// Creates a NodeList. Short notation for tests
export const l = (elements: unknown[]): NodeList =>
  n(NodeType.LIST, elements) as NodeList;

// Creates a Lexer Token. Short notation for tests
export const t = (type: TokenType, literal?: string | number): Token =>
  ({ type, literal }) as Token;
