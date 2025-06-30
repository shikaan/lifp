import { LPAREN, RPAREN, STRING_DELIMITER } from "./constants.js";
import { SyntaxException } from "./errors.js";
import { type FilePointer, type Token, TokenType } from "./types.js";

const isStringLiteral = (s: string) =>
  s.startsWith(STRING_DELIMITER) && s.endsWith(STRING_DELIMITER);

export const tokenize = (line: string, ptr: FilePointer): Token[] => {
  line = line.trim();
  const rex =
    /((?<paren>[()])|(?<token>"(?:[^\\"]|\\.)*"|:?[a-zA-Z0-9$%^&*+<>=./\-_!?]+)|(?<comment>;\s*.*))\s*/g;

  const result: Token[] = [];

  let execArray: RegExpExecArray = rex.exec(line);
  if (!execArray) {
    throw new SyntaxException("Provided line is not a valid S-Expression");
  }

  let lastIndex = 0;
  while (execArray !== null) {
    // Check if we skipped any characters
    if (execArray.index > lastIndex) {
      const skipped = line.substring(lastIndex, execArray.index);
      throw new SyntaxException(`Unexpected character(s): "${skipped}"`);
    }

    const { groups } = execArray;
    const { token, paren, comment } = groups;
    if (comment) {
      lastIndex = rex.lastIndex;
      execArray = rex.exec(line);
      continue;
    }

    if (paren === LPAREN) {
      result.push({ type: TokenType.LPAREN, literal: LPAREN, ptr });
    } else if (paren === RPAREN) {
      result.push({ type: TokenType.RPAREN, literal: RPAREN, ptr });
    } else if (isStringLiteral(token)) {
      const unescaped = JSON.parse(`${token}`);
      result.push({
        type: TokenType.STRING,
        literal: unescaped,
        ptr,
      });
    } else {
      const isNumber = /^-?\d+(\.\d+)?$/.test(token);

      if (isNumber) {
        const maybeNumber = Number.parseFloat(token);
        if (Number.isNaN(maybeNumber))
          throw new SyntaxException(`"${token}" is not a valid number`);
        result.push({ type: TokenType.NUMBER, literal: maybeNumber, ptr });
      } else {
        result.push({ type: TokenType.SYMBOL, literal: token, ptr });
      }
    }

    lastIndex = rex.lastIndex;
    execArray = rex.exec(line);
  }

  if (lastIndex < line.length) {
    const remainingContent = line.substring(lastIndex);
    throw new SyntaxException(`Unexpected character(s): "${remainingContent}"`);
  }

  return result;
};
