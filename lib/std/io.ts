import { stdin, stdout } from "node:process";
import { createInterface } from "node:readline/promises";
import * as util from "node:util";
import { InvalidArgumentException } from "../errors.js";
import { print } from "../print.js";
import { isList, isString, type Lambda, type Value } from "../types.js";

const ioWriteFunction = (
  nodes: Value[],
  name: string,
  stream: NodeJS.WriteStream,
) => {
  if (nodes.length !== 1) {
    throw new InvalidArgumentException(
      `'${name}' requires one argument. Got ${nodes.length}`,
    );
  }
  stream.write(print(nodes[0]));
  return null;
};

export const io: Record<string, Lambda> = {
  /**
   * Stringifies an atom and writes it to stdout. Use `io.print` to format output.
   * @name io.stdout
   * @example
   *   (io.stdout "hello")
   */
  "io.stdout": (nodes) => ioWriteFunction(nodes, "io.stdout", process.stdout),
  /**
   * Stringifies an atom and writes it to stderr.
   * @name io.stderr
   * @example
   *   (io.stderr "error")
   */
  "io.stderr": (nodes) => ioWriteFunction(nodes, "io.stderr", process.stderr),
  /**
   * Writes a formatted string to stdout. Specifiers:
   *  - `%s` for strings
   *  - `%d` for numbers
   *  - `%i` casts a string to an integer
   *  - `%f` casts a string to a float
   * @name io.printf
   * @example
   *   (io.printf "hello %s %d" ("world" 42))
   */
  "io.printf": (nodes) => {
    if (nodes.length !== 2 || !isString(nodes[0]) || !isList(nodes[1])) {
      throw new InvalidArgumentException(
        `'io.printf' requires a format string, and a list of arguments.`,
      );
    }
    const [format, values] = nodes;
    process.stdout.write(util.format(format, ...values));
    return null;
  },
  /**
   * Writes the question on stdout and waits for user input.
   *
   * @name io.readline
   * @example
   *   (io.readline "What is your favorite food? ") ; "USER_TYPED_CONTENT"
   */
  "io.readline": async (nodes) => {
    if (nodes.length !== 1 || !isString(nodes[0])) {
      throw new InvalidArgumentException(`'io.readline' requires a question.`);
    }

    const rl = createInterface({ input: stdin, output: stdout });
    const result = await rl.question(nodes[0]);
    rl.close();
    return result;
  },
  /**
   * Clear the console output.
   *
   * @name io.clear
   * @example
   *   (io.clear)
   */
  "io.clear": async (nodes) => {
    if (nodes.length !== 0) {
      throw new InvalidArgumentException(`'io.clear' requires no arguments.`);
    }
    console.clear();
    return null;
  },
};
