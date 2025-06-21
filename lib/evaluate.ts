import {
  AST,
  Symbol,
  SymbolType,
  Token,
  TokenTag,
  TokenType,
} from "./types.ts";
import { isSubtree } from "./utils.ts";

class InvalidArgumentException extends Error {}

class UnrecognizedSymbolException extends Error {}

class InvalidASTException extends Error {}

const areNumbers = (a: AST): a is Token<number>[] =>
  a.every((n) => typeof n[1] === "number");

export type Environment = { [K in SymbolType]: (ast: AST) => AST };

const ENVIRONMENT: Environment = {
  [Symbol.PLUS]: (args: AST) => {
    if (!areNumbers(args) || args.length < 2)
      throw new InvalidArgumentException();
    return [[TokenTag.NUMBER, args.reduce((sum, [, value]) => sum + value, 0)]];
  },
  [Symbol.MINUS]: (args: AST) => {
    if (!areNumbers(args) || args.length < 2)
      throw new InvalidArgumentException();
    const [first, ...rest] = args;
    return [
      [
        TokenTag.NUMBER,
        rest.reduce((diff, [, value]) => diff - value, first[1]),
      ],
    ];
  },
  [Symbol.WILDCARD]: (args: AST) => {
    if (!areNumbers(args) || args.length < 2)
      throw new InvalidArgumentException();
    return [
      [TokenTag.NUMBER, args.reduce((prod, [, value]) => prod * value, 1)],
    ];
  },
  [Symbol.SLASH]: (args: AST) => {
    if (!areNumbers(args) || args.length < 2)
      throw new InvalidArgumentException();
    const [first, ...rest] = args;
    return [
      [
        TokenTag.NUMBER,
        rest.reduce((prod, [, value]) => prod / value, first[1]),
      ],
    ];
  },
  [Symbol.NOT]: (args: AST) => {
    if (args.length !== 1 || typeof args[0][1] !== "boolean")
      throw new InvalidArgumentException();
    return [[TokenTag.BOOLEAN, !args[0]]];
  },
};

const isRoutine = (a: unknown): a is Token<SymbolType> =>
  a[0] === TokenTag.SYMBOL;

export const evaluate = (ast: AST): AST => {
  let argsTobeReduced = ast,
    routine: (ast: AST) => AST = null;
  if (isRoutine(ast[0])) {
    const [, rootValue] = ast[0];
    argsTobeReduced = ast.slice(1);
    routine = ENVIRONMENT[rootValue as string];

    if (!routine) {
      throw new InvalidArgumentException();
    }
  }

  const result = [];
  for (const node of argsTobeReduced) {
    if (isSubtree(node)) {
      result.push(...evaluate(node));
      continue;
    }

    result.push(node);
  }

  return routine?.(result) ?? result;
};
