import { FALSE, KEYWORD_PREFIX, NIL, TRUE } from "./constants.js";
import { SyntaxException } from "./errors.js";
import { tokenize } from "./lexer.js";
import {
  type AbstractSyntaxTree,
  type ASTNode,
  type ASTNodeList,
  ASTNodeType,
  type AtomToken,
  isAtomToken,
  isKeyword,
  type Token,
  TokenType,
} from "./types.js";

const parseAtom = (token: AtomToken): ASTNode => {
  switch (token.type) {
    case TokenType.NUMBER:
      return { type: ASTNodeType.NUMBER, value: token.literal };
    case TokenType.STRING:
      return { type: ASTNodeType.STRING, value: token.literal };
    case TokenType.SYMBOL: {
      if (isKeyword(token.literal)) {
        return { type: ASTNodeType.KEYWORD, value: token.literal };
      }
      if ([TRUE, FALSE].includes(token.literal)) {
        return { type: ASTNodeType.BOOLEAN, value: token.literal === TRUE };
      }

      if (token.literal === NIL) {
        return { type: ASTNodeType.NIL, value: null };
      }

      return { type: ASTNodeType.SYMBOL, value: token.literal };
    }
  }
};

const parseList = (
  tokens: Token[],
  reader: { depth: number; index: number },
): ASTNodeList => {
  if (tokens.length <= 2 || tokens[reader.index].type !== TokenType.LPAREN) {
    throw new SyntaxException("Invalid list");
  }

  const result: ASTNodeList = { type: ASTNodeType.LIST, value: [] };

  reader.index++;
  reader.depth++;
  for (; reader.index < tokens.length; reader.index++) {
    const token = tokens[reader.index];
    if (token.type === TokenType.LPAREN) {
      result.value.push(parseList(tokens, reader));
      continue;
    }

    if (token.type === TokenType.RPAREN) {
      reader.depth--;
      break;
    }

    if (token.type === TokenType.EOF) {
      reader.depth--;
      break;
    }

    result.value.push(parseAtom(token));
  }

  return result;
};

export const read = (line: string): AbstractSyntaxTree => {
  const tokens = tokenize(line);

  if (tokens.length === 0) return { type: ASTNodeType.NIL, value: null };

  const reader = { depth: 0, index: 0 };
  const first = tokens[0];

  if (first.type === TokenType.LPAREN) {
    const result = parseList(tokens, reader);
    if (reader.depth !== 0)
      throw new SyntaxException("Unbalanced parentheses.");
    return result;
  }

  // This can only be a single atom plus an end of file, else it's a mistake
  if (tokens.length !== 2 || !isAtomToken(first))
    throw new SyntaxException(`Invalid tokens ${line}.`);

  return parseAtom(first);
};
