//@ts-check
import * as readline from 'node:readline/promises'
import {stdin, stdout} from 'node:process'

import {read, evaluate, print} from './repl.ts'

const PROMPT = "user> ";
const rl = await readline.createInterface({input: stdin, output: stdout});

process.on('beforeExit', () => {
  console.log('beforeExit')
  rl.close();
});

process.on('exit', () => {
  console.log('exit')
  rl.close();
})

while (true) {
  const line = await rl.question(PROMPT);
  const ast = read(line);
  evaluate(ast)
  print(line);
}

