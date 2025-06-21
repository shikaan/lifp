import { AST, TokenTag } from "./types.ts";
import { isSubtree } from "./utils.ts";

export function format(list: AST): string {
  let outputBuffer = "(";
  for (const node of list) {
    if (isSubtree(node)) {
      outputBuffer += `${format(node)} `;
      continue;
    }

    const [tokenType, value] = node;
    switch (tokenType) {
      case TokenTag.NUMBER:
        outputBuffer += `${value} `;
        break;
      case TokenTag.BOOLEAN:
        outputBuffer += value ? "true " : "false ";
        break;
      case TokenTag.STRING:
        outputBuffer += `"${value}" `;
        break;
      case TokenTag.SYMBOL:
        outputBuffer += `${value} `;
        break;
      default:
        throw new Error("Unsupported token type");
    }
  }
  outputBuffer = outputBuffer.trimEnd();
  outputBuffer += ")";
  return outputBuffer;
}

export function print(ast: AST) {
  console.log(format(ast));
}
