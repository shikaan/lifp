//@ts-check
import * as readline from 'node:readline/promises'
import {stdin, stdout} from 'node:process'

import {read, evaluate, print} from './repl.ts'

const PROMPT = "user> ";
const rl = readline.createInterface({input: stdin, output: stdout});

process.on('beforeExit', () => {
  rl.close();
});

process.on('SIGKILL', () => process.exit(1))
process.on('SIGABRT', () => process.exit(1))
process.on('SIGTERM', () => process.exit(1))

while (true) {
  const line = await rl.question(PROMPT);
  try {
    const ast = read(line);
    evaluate(ast)
    print(line);
  } catch (e) {
    stdout.write(`>> ${e}\n`)
  }
}

