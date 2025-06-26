import * as fs from "node:fs";
import { LPAREN, RPAREN } from "../lib/constants.js";
import { evaluate, print, read } from "../lib/index.js";
import { defaultEnvironment, Environment } from "../lib/environment.js";

export function run(args: string[]): number {
  if (args.length < 1) {
    throw new Error(`'run' requires one file.`);
  }
  const file = args[0];

  // TODO: this could actually be streamed...
  const buffer = fs.readFileSync(file, "utf-8");

  const environment = new Environment(defaultEnvironment);

  let depth = -1;
  let lineStart = 0;
  for (let i = 0; i < buffer.length; i++) {
    const char = buffer[i];
    if (char == LPAREN) {
      lineStart = depth === -1 ? i : lineStart;
      depth = depth === -1 ? 1 : depth + 1;
    } else if (char == RPAREN) depth--;

    if (depth == 0) {
      const line = buffer.slice(lineStart, i + 1);
      const output = evaluate(read(line), environment);
      console.log(line);
      console.log("~>", print(output));
      depth = -1;
    }
  }

  return 0;
}
