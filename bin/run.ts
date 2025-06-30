import * as fs from "node:fs";
import { LPAREN, RPAREN } from "../lib/constants.js";
import { defaultEnvironment, Environment } from "../lib/environment.js";
import { evaluate, read } from "../lib/index.js";

export const execute = async (buffer: string) => {
  const environment = new Environment(defaultEnvironment);

  let depth = -1;
  let lineStart = 0;
  let skip = false;
  for (let i = 0; i < buffer.length; i++) {
    const char = buffer[i];

    if (char === ";") {
      skip = true;
    }

    if (char === "\n") {
      skip = false;
    }

    if (skip) continue;

    if (char === LPAREN) {
      lineStart = depth === -1 ? i : lineStart;
      depth = depth === -1 ? 1 : depth + 1;
    } else if (char === RPAREN) depth--;

    if (depth === 0) {
      const line = buffer.slice(lineStart, i + 1);
      await evaluate(read(line), environment);
      depth = -1;
    }
  }
};

export async function run(args: string[]): Promise<number> {
  if (args.length < 1) {
    throw new Error(`'run' requires one file.`);
  }
  const file = args.at(-1);

  // TODO: this could be streamed since we are not looking up the environment first...
  const buffer = fs.readFileSync(file, "utf-8");
  await execute(buffer);
  return 0;
}
