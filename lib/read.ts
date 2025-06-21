import type { AST, SymbolType, Token } from "./types.ts";
import { TokenTag, SYMBOLS } from "./types.ts";

class UnexpectedTokenException extends Error {}

const isOperator = (s: any): s is SymbolType => SYMBOLS.includes(s);
const isWhitespace = (s: string) => /\s+/.test(s);

class Reader {
  public static EOF = Symbol("EOF");
  public nesting = 1;

  private position = 0;
  private readonly stream: string;

  constructor(string: string) {
    this.stream = string;
  }

  next(stride = 1): string | symbol {
    if (this.position >= this.stream.length) return Reader.EOF;
    const char = this.stream[this.position];
    this.position += stride;
    return char;
  }

  peek(): string | symbol {
    if (this.position >= this.stream.length) return Reader.EOF;
    return this.stream[this.position];
  }
}

function parseRawToken(rawToken: string): Token {
  if (isOperator(rawToken)) {
    return [TokenTag.SYMBOL, rawToken] as Token<SymbolType>;
  }
  if (["true", "false"].includes(rawToken)) {
    return [TokenTag.BOOLEAN, rawToken === "true"] as Token<boolean>;
  }
  if (rawToken.startsWith('"') && rawToken.endsWith('"')) {
    return [
      TokenTag.STRING,
      rawToken.slice(1, rawToken.length - 1),
    ] as Token<string>;
  }
  const parsed = Number.parseFloat(rawToken);
  if (!Number.isNaN(parsed)) {
    return [TokenTag.NUMBER, parsed] as Token<number>;
  }
  throw new UnexpectedTokenException(`Unable to read atom '${rawToken}'`);
}

function tokenizeList(string: string, reader: Reader) {
  const tokens = [];
  let currentToken = "";

  while (reader.peek() !== Reader.EOF) {
    const char = reader.next() as string; // if it wasn't a string, we'd be out
    if (char === "(") {
      reader.nesting++;
      const token = tokenizeList(string, reader);
      tokens.push(token);
      continue;
    }

    if (char === ")") {
      reader.nesting--;
      break;
    }

    if (isWhitespace(char)) {
      if (currentToken !== "") {
        const token = parseRawToken(currentToken);
        tokens.push(token);
        currentToken = "";
      }
      continue;
    }

    currentToken += char;
  }

  if (currentToken !== "") {
    const token = parseRawToken(currentToken);
    tokens.push(token);
  }

  return tokens;
}

/**
 * Tokenizes a string returning a nested list of tokens
 */
export function read(string: string): AST {
  string = string.trim();
  const reader = new Reader(string);

  const first = reader.next();
  if (first !== "(") throw new UnexpectedTokenException("Input must be a list");

  const tokens = tokenizeList(string, reader);
  if (reader.nesting !== 0 || reader.peek() !== Reader.EOF) {
    throw new UnexpectedTokenException("Invalid input");
  }

  return tokens;
}
