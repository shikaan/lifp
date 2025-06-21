const PROMPT = "> ";
const OUTPUT = "~> ";
const OUTPUT_ERROR = "!! ";

export const format = {
  error: (e: Error): string => `${OUTPUT_ERROR}${e.message}\n`,
  output: (s: string): string => `${OUTPUT}${s}\n`,
  prompt: (): string => PROMPT,
};
