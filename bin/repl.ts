import { stderr, stdin, stdout } from "node:process";
import { createInterface } from "node:readline/promises";
import { defaultEnvironment, Environment } from "../lib/environment.js";
import { evaluate, format, print, read } from "../lib/index.js";
import packageJSON from "../package.json" with { type: "json" };
import docsJSON from "../docs/docs.json" with { type: "json" };

const rl = createInterface({ input: stdin, output: stdout });

const help = () => {
  console.log(
    `
${packageJSON.name} is a lisp. Here's your first program:

  (io.printf "Hello, %s! The answer is %d\\n" ("user" 42)) ; "Hello, user! The answer is 42"

To learn about the functions and and the variables available in this environment, use '?':

  ?             ; lists all functions in this environment
  ? io.printf   ; shows documentation and examples about io.printf

For more information, feedback, or bug reports https://github.com/shikaan/lifp
    `.trim(),
  );
};

const man = (arg: string) => {
  if (!arg) {
    console.log("FUNCTIONS");
    Object.keys(docsJSON.functions).forEach((methodName) => {
      console.log(`  - ${methodName}`);
    });
    return;
  }

  const doc = docsJSON.functions[arg];
  if (doc) {
    stdout.write(`
NAME
  ${doc.name}

DESCRIPTION
  ${doc.description.replace(/\n/g, "\n  ")}

EXAMPLE
  ${doc.example}

      `);
  } else {
    stderr.write(format.error(new Error(`Symbol '${arg}' not found.`)));
  }
};

export async function repl(): Promise<number> {
  const environment = new Environment(defaultEnvironment);
  // @ts-expect-error this comes from the --define flag
  console.log(`lifp - ${__VERSION__}`);
  console.log("Type 'help' for help. Press Ctrl+C exit.\n");
  while (true) {
    try {
      const line = await rl.question(format.prompt());
      const cleanLine = line.trim();

      if (cleanLine === "help") {
        help();
        continue;
      }

      if (cleanLine.startsWith("man") || cleanLine.startsWith("?")) {
        const [, arg] = cleanLine.split(/\s/);
        man(arg);
        continue;
      }

      const output = print(
        await evaluate(read(line, ["repl", 1]), environment),
      );
      stdout.write(format.output(output));
    } catch (e) {
      stderr.write(format.error(e));
    }
  }
}
