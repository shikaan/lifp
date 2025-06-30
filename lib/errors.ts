import { getFile, getLine } from "./evaluation-context.js";

class Exception extends Error {
  constructor(name: string, message: string) {
    super(message);
    this.name = name;
    this.message = `\n${this.name}: ${this.message}\n    at ${getFile()}:${getLine()}\n`;
  }
}

export class SyntaxException extends Exception {
  constructor(message: string) {
    super("SyntaxException", message);
  }
}

export class SymbolNotFoundException extends Exception {
  constructor(message: string) {
    super("SymbolNotFoundException", message);
  }
}

export class InvalidArgumentException extends Exception {
  constructor(message?: string) {
    super("InvalidArgumentException", message);
  }
}
