import { FALSE, NIL, STRING_DELIMITER, TRUE } from "./constants.js";
import { type Expression, ASTNodeType } from "./types.js";

export const print = (expression: Expression): string => {
  switch (expression.type) {
    case ASTNodeType.STRING:
      return `${STRING_DELIMITER}${expression.value}${STRING_DELIMITER}`;
    case ASTNodeType.SYMBOL:
    case ASTNodeType.NUMBER:
      return `${expression.value}`;
    case ASTNodeType.NIL:
      return NIL;
    case ASTNodeType.BOOLEAN:
      return expression.value ? TRUE : FALSE;
    case ASTNodeType.FUNCTION:
      return `#<function>`;
    case ASTNodeType.LIST:
      return `(${expression.value.map((i) => print(i)).join(" ")})`;
  }
};
