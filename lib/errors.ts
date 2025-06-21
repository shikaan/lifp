export class UnexpectedTokenException extends Error {
  constructor(token: string, message?: string) {
    super("UnexpectedTokenException");
    this.message = `"${token}" was not expected in this context. ${message ?? 'Check syntax reference for more details.'}`;
  }
}

export class InvalidArgumentException extends Error {
  constructor(routine: string) {
    super("InvalidArgumentException");
    this.message = `Routine "${routine}" received arguments. See documentation for more details.`;
  }
}

export class RoutineNotFound extends Error {
  constructor(routine: string) {
    super("RoutineNotFound");
    this.message = `Routine "${routine}" is not defined in this environment. Make sure it's one of the built-ins.`;
  }
}