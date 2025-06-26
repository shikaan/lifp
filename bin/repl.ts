import { stderr, stdin, stdout } from "node:process";
import { createInterface } from "node:readline/promises";
import { defaultEnvironment } from "../lib/environment.js";
import { evaluate, format, print, read } from "../lib/index.js";

const rl = createInterface({ input: stdin, output: stdout });

export async function repl(): Promise<number> {
  // @ts-expect-error this comes from the --define flag
  console.log(`lifp - ${__VERSION__}`);
  console.log("Press Ctrl+C exit\n");
  while (true) {
    try {
      const line = await rl.question(format.prompt());
      const output = print(evaluate(read(line), defaultEnvironment));
      stdout.write(format.output(output));
    } catch (e) {
      stderr.write(format.error(e));
    }
  }
}
