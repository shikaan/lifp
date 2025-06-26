import {
  type ASTNodeSymbol,
  ASTNodeType,
  isListNode,
  isSymbol,
  SpecialFormHandler,
  SpecialFormType,
} from "./types.js";
import { Environment } from "./environment.js";
import { InvalidArgumentException } from "./errors.js";
import { evaluate } from "./evaluate.js";
import { DEF, FN, LET } from "./constants.js";

export const specials: Record<SpecialFormType, SpecialFormHandler> = {
  [DEF]: (nodes, environment) => {
    if (nodes.length !== 3 || nodes[1].type !== ASTNodeType.KEYWORD) {
      throw new InvalidArgumentException(
        `'def!' requires a keyword and a form only. Example: (def! :a 123)`,
      );
    }

    const [, symbol, form] = nodes;
    environment.set(symbol.value.slice(1), evaluate(form, environment));
    return { type: ASTNodeType.NIL, value: null };
  },
  [FN]: (nodes, environment) => {
    if (nodes.length !== 3) {
      throw new InvalidArgumentException(
        `'fn*' requires a binding list and a form. Example: (fn* (a b) (+ a b))`,
      );
    }

    const [, bindings, form] = nodes;
    if (!isListNode(bindings) || bindings.value.some((n) => !isSymbol(n))) {
      throw new InvalidArgumentException(
        `'fn*' requires a binding list and a form. Example: (fn* (a b) (+ a b))`,
      );
    }

    // Array.some does not enforce types
    const values = bindings.value as ASTNodeSymbol[];
    return {
      type: ASTNodeType.FUNCTION,
      value: (nodes) => {
        const innerEnvironment = new Environment(environment);
        values.forEach((sym, i) => innerEnvironment.set(sym.value, nodes[i]));
        return evaluate(form, innerEnvironment);
      },
    };
  },
  [LET]: (list, environment) => {
    if (list.length !== 3 || !isListNode(list[1])) {
      throw new InvalidArgumentException(
        `'let*' requires a list of assignments. Example: (let* ((:a 12) (:b 34)) (other-form))`,
      );
    }

    const [, assignments, form] = list;
    const innerEnvironment = new Environment(environment);

    for (const pair of assignments.value) {
      if (!isListNode(pair) || pair.value[0].type !== ASTNodeType.KEYWORD) {
        throw new InvalidArgumentException(
          `'let*' requires a list of assignments. Example: (let* ((:a 12) (:b 34)) (other-form))`,
        );
      }
      const [sym, form] = pair.value;
      innerEnvironment.set(
        sym.value.slice(1),
        evaluate(form, innerEnvironment),
      );
    }

    return evaluate(form, innerEnvironment);
  },
};
