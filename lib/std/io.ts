import { InvalidArgumentException } from "../errors.js";
import { print } from "../print.js";
import { ASTNodeType, type Expression, type Lambda } from "../types.js";
import * as util from "node:util";

const ioWriteFunction = (
  nodes: Expression[],
  name: string,
  stream: NodeJS.WriteStream,
) => {
  if (nodes.length !== 1) {
    throw new InvalidArgumentException(
      `'${name}' requires one argument. Got ${nodes.length}`,
    );
  }
  stream.write(print(nodes[0]));
  return { type: ASTNodeType.NIL, value: null };
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
    if (
      nodes.length !== 2 ||
      nodes[0].type !== ASTNodeType.STRING ||
      nodes[1].type !== ASTNodeType.LIST
    ) {
      throw new InvalidArgumentException(
        `'io.printf' requires a format string, and a list of arguments. Example: (printf "hello %s" ("world"))`,
      );
    }
    const [format, values] = nodes;
    process.stdout.write(
      util.format(format.value, ...values.value.map((i: any) => i.value)),
    );
    return { type: ASTNodeType.NIL, value: null };
  },
};
