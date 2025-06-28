import packageJSON from "../package.json" with { type: "json" };
import { repl } from "./repl.js";
import { run } from "./run.js";

const HELP = `
  ${packageJSON.name} is a Lisp dialect and a toolchain to build general purpose applications.

Usage:
  ${packageJSON.name} <command> [...flags] [...args]
  
Commands:
  repl                          Start a REPL session
  run     ./my-file.lisp        Execute a file with ${packageJSON.name}
  help                          Show this help
  
Flags:
  --help                        Show this help
  
Learn more about ${packageJSON.name}           https://github.com/shikaan/lifp`;

const printHelp = () => {
  console.log(HELP);
  process.exit(1);
};

(async (): Promise<number> => {
  const [, , ...argv] = process.argv;

  const flags = argv.filter((i) => /^--.+/.test(i));
  if (flags.includes("--help")) {
    return printHelp();
  }

  const [command = "", ...args] = argv;

  try {
    switch (command) {
      case "run":
        return run(args);
      case "repl":
      case "":
        return await repl();
      case "help":
        return printHelp();
      default:
        throw new Error(`'${command}' command not found.`);
    }
  } catch (e) {
    console.error(`${e}`);
    printHelp();
  }
})()
  .then((status = 0) => process.exit(status))
  .catch(() => process.exit(1));
