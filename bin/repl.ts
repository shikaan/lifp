import { stderr, stdin, stdout } from "node:process";
import { createInterface } from "node:readline/promises";
import { evaluate, format, print, read } from "../lib/index.js";
import packageJson from "../package.json" with { type: "json" };
import { defaultEnvironment } from "../lib/environment.js";

const rl = createInterface({ input: stdin, output: stdout });

console.log(`Lisp.js - ${packageJson.version}`);
console.log("Press Ctrl+C twice to exit\n");
while (true) {
  try {
    const line = await rl.question(format.prompt());
    const output = print(evaluate(read(line), defaultEnvironment));
    stdout.write(format.output(output));
  } catch (e) {
    stderr.write(format.error(e));
  }
}
