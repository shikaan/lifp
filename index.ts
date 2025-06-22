import { stderr, stdin, stdout } from "node:process";
import { createInterface } from "node:readline/promises";
import { evaluate, format, print, read } from "./lib/index.js";

const rl = createInterface({ input: stdin, output: stdout });

while (true) {
  try {
    const line = await rl.question(format.prompt());
    const output = print(evaluate(read(line), ""));
    stdout.write(format.output(output));
  } catch (e) {
    stderr.write(format.error(e));
  }
}
