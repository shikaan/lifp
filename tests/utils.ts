import {
  type FilePointer,
  type Node,
  type NodeList,
  NodeType,
  type Token,
  type TokenType,
} from "../lib/types.js";

export const mockPtr: FilePointer = ["", 0];

// Creates a Node. Short notation for tests
export const n = (type: NodeType, value: unknown): Node =>
  ({ type, value, ptr: mockPtr }) as Node;

// Creates a NodeList. Short notation for tests
export const l = (elements: unknown[]): NodeList =>
  n(NodeType.LIST, elements) as NodeList;

// Creates a Lexer Token. Short notation for tests
export const t = (type: TokenType, literal?: string | number): Token =>
  ({ type, literal, ptr: mockPtr }) as Token;
