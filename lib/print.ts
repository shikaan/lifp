import { FALSE, NIL, STRING_DELIMITER, TRUE } from "./constants.js";
import { ASTNodeType, type Expression } from "./types.js";

export const print = (expression: Expression): string => {
  switch (expression.type) {
    case ASTNodeType.STRING:
      return `${STRING_DELIMITER}${expression.value}${STRING_DELIMITER}`;
    case ASTNodeType.SYMBOL:
    case ASTNodeType.NUMBER:
    case ASTNodeType.KEYWORD:
      return `${expression.value}`;
    case ASTNodeType.NIL:
      return NIL;
    case ASTNodeType.BOOLEAN:
      return expression.value ? TRUE : FALSE;
    case ASTNodeType.LIST:
      return `(${expression.value.map((i) => print(i)).join(" ")})`;
  }
};
