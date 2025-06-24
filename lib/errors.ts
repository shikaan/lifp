export class SyntaxException extends Error {
  constructor(message?: string) {
    super(message);
    this.name = "SyntaxException";
  }
}

export class SymbolNotFoundException extends Error {
  constructor(message?: string) {
    super(message);
    this.name = "SymbolNotFoundException";
  }
}

export class InvalidArgumentException extends Error {
  constructor(message?: string) {
    super(message);
    this.name = "InvalidArgumentException";
  }
}
