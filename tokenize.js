//@ts-check
class UnexpectedTokenException extends Error {
}

/**
 * @typedef {[TokenType, unknown]} Token
 */

export const TokenType = {
  NUMBER: 'number',
  STRING: 'string',
  BOOLEAN: 'boolean',
  OPERATOR: 'operator',
}

export const Operator = {
  NOT: 'not',
  PLUS: '+',
}

const OPERATORS = Object.values(Operator)

/**
 * @param {string} rawToken
 * @returns {Token}
 */
function parseRawToken(rawToken) {
  if (OPERATORS.includes(rawToken)) {
    return [TokenType.OPERATOR, rawToken]
  }
  if (["true", "false"].includes(rawToken)) {
    return [TokenType.BOOLEAN, rawToken === "true"]
  }
  if (rawToken.startsWith("\"") && rawToken.endsWith("\"")) {
    return [TokenType.STRING, rawToken.slice(1, rawToken.length - 1)]
  }
  const parsed = Number.parseFloat(rawToken);
  if (!Number.isNaN(parsed)) {
    return [TokenType.NUMBER, parsed]
  }
  throw new UnexpectedTokenException(`Unable to read atom '${rawToken}'`);
}

// Represents the end of the stream
const EOF = Symbol('EOF');

class Reader {
  #string;
  #position = 0;
  nesting = 1;

  constructor(string) {
    this.#string = string
  }

  next(stride = 1) {
    if (this.#position >= this.#string.length) return EOF;
    const char = this.#string[this.#position];
    this.#position += stride;
    return char;
  }

  peek() {
    if (this.#position >= this.#string.length) return EOF;
    return this.#string[this.#position]
  }
}

const isWhitespace = (s) => /\s+/.test(s)

/**
 * @param {string} string
 * @param {Reader} reader
 * @return {(Token | Token[])[]}
 */
function tokenizeList(string, reader) {
  const tokens = []
  let currentToken = ''

  while (reader.peek() !== EOF) {
    const char = reader.next()
    if (char === '(') {
      reader.nesting++
      const token = tokenizeList(string, reader)
      tokens.push(token);
      continue;
    }

    if (char === ')') {
      reader.nesting--
      break
    }

    if (isWhitespace(char)) {
      if (currentToken !== '') {
        const token = parseRawToken(currentToken);
        tokens.push(token)
        currentToken = ''
      }
      continue
    }

    currentToken += char;
  }

  if (currentToken !== '') {
    const token = parseRawToken(currentToken);
    tokens.push(token)
  }

  return tokens;
}

/**
 * Tokenizes a string returning a nested list of tokens
 *
 * @param {string} string
 * @return {(Token|Token[])[]}
 */
export function tokenize(string) {
  string = string.trim()
  const reader = new Reader(string);

  const first = reader.next()
  if (first !== '(') throw new UnexpectedTokenException('Input must be a list')

  const tokens = tokenizeList(string, reader);
  if (reader.nesting !== 0 || reader.peek() !== EOF) {
    throw new UnexpectedTokenException('Invalid input')
  }

  return tokens
}

