import { FALSE, NIL, STRING_DELIMITER, TRUE } from "./constants.js";
import { isList, isNull, type Value } from "./types.js";

export const print = (value: Value): string => {
  switch (typeof value) {
    case "string":
      return `${STRING_DELIMITER}${value}${STRING_DELIMITER}`;
    case "number":
      return `${value}`;
    case "boolean":
      return value ? TRUE : FALSE;
    case "function":
      return `#<function>`;
    case "object": {
      if (isNull(value)) {
        return NIL;
      } else if (isList(value)) {
        return `(${value.map((i) => print(i)).join(" ")})`;
      }
    }
  }
};
