import { FALSE, NIL, TRUE } from "./constants.js";
import { SyntaxException } from "./errors.js";
import { tokenize } from "./lexer.js";
import {
  type AtomToken,
  type FilePointer,
  isAtomToken,
  type Node,
  type NodeList,
  NodeType,
  type Token,
  TokenType,
} from "./types.js";

const parseAtom = (token: AtomToken, ptr: FilePointer): Node => {
  switch (token.type) {
    case TokenType.NUMBER:
      return { type: NodeType.NUMBER, value: token.literal, ptr };
    case TokenType.STRING:
      return { type: NodeType.STRING, value: token.literal, ptr };
    case TokenType.SYMBOL: {
      if ([TRUE, FALSE].includes(token.literal)) {
        return { type: NodeType.BOOLEAN, value: token.literal === TRUE, ptr };
      }

      if (token.literal === NIL) {
        return { type: NodeType.NIL, value: null, ptr };
      }

      return { type: NodeType.SYMBOL, value: token.literal, ptr };
    }
  }
};

const parseList = (
  tokens: Token[],
  reader: { depth: number; index: number },
  ptr: FilePointer,
): NodeList => {
  if (tokens.length <= 1 || tokens[reader.index].type !== TokenType.LPAREN) {
    throw new SyntaxException("Invalid list");
  }

  const result: NodeList = { type: NodeType.LIST, value: [], ptr };

  reader.index++;
  reader.depth++;
  for (; reader.index < tokens.length; reader.index++) {
    const token = tokens[reader.index];
    if (token.type === TokenType.LPAREN) {
      result.value.push(parseList(tokens, reader, ptr));
      continue;
    }

    if (token.type === TokenType.RPAREN) {
      reader.depth--;
      break;
    }

    result.value.push(parseAtom(token, ptr));
  }

  return result;
};

export const read = (expression: string, ptr: FilePointer): Node => {
  const tokens = tokenize(expression, ptr);

  if (tokens.length === 0) return { type: NodeType.NIL, value: null, ptr };

  const reader = { depth: 0, index: 0 };
  const first = tokens[0];

  if (first.type === TokenType.LPAREN) {
    const result = parseList(tokens, reader, ptr);
    const hasReadEverything = reader.index === tokens.length - 1;
    if (reader.depth !== 0 || !hasReadEverything) {
      throw new SyntaxException("Unbalanced parentheses.");
    }
    return result;
  }

  // This can only be a single atom plus an end of file, else it's a mistake
  if (tokens.length !== 1 || !isAtomToken(first))
    throw new SyntaxException(`Invalid tokens ${expression}.`);

  return parseAtom(first, ptr);
};
