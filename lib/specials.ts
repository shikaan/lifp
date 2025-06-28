import { DEF, FN, IF, LET } from "./constants.js";
import { Environment } from "./environment.js";
import { InvalidArgumentException } from "./errors.js";
import { evaluate } from "./evaluate.js";
import {
  type ASTNodeSymbol,
  ASTNodeType,
  isListNode,
  isSymbol,
  type SpecialFormHandler,
  type SpecialFormType,
} from "./types.js";

export const specials: Record<SpecialFormType, SpecialFormHandler> = {
  [DEF]: (nodes, environment) => {
    if (nodes.length !== 3 || nodes[1].type !== ASTNodeType.SYMBOL) {
      throw new InvalidArgumentException(
        `'${DEF}' requires a keyword and a form only. Example: (${DEF} a 123)`,
      );
    }

    const [, symbol, form] = nodes;
    environment.set(symbol.value, evaluate(form, environment));
    return { type: ASTNodeType.NIL, value: null };
  },
  [FN]: (nodes, environment) => {
    if (nodes.length !== 3) {
      throw new InvalidArgumentException(
        `'${FN}' requires a binding list and a form. Example: (${FN} (a b) (+ a b))`,
      );
    }

    const [, bindings, form] = nodes;
    if (!isListNode(bindings) || bindings.value.some((n) => !isSymbol(n))) {
      throw new InvalidArgumentException(
        `'${FN}' requires a binding list and a form. Example: (${FN} (a b) (+ a b))`,
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
        `'${LET}' requires a list of assignments. Example: (${LET} ((:a 12) (:b 34)) (other-form))`,
      );
    }

    const [, assignments, form] = list;
    const innerEnvironment = new Environment(environment);

    for (const pair of assignments.value) {
      if (!isListNode(pair) || pair.value[0].type !== ASTNodeType.SYMBOL) {
        throw new InvalidArgumentException(
          `'${LET}' requires a list of assignments. Example: (${LET} ((a 12) (b 34)) (other-form))`,
        );
      }
      const [sym, form] = pair.value;
      innerEnvironment.set(sym.value, evaluate(form, innerEnvironment));
    }

    return evaluate(form, innerEnvironment);
  },
  [IF]: (list, environment) => {
    if (list.length < 3 || list.length > 4) {
      throw new InvalidArgumentException(
        `'${IF}' requires a condition, a then branch, and on optional else branch.\nExamples: (${IF} (= 1 2) (1) (+ 1 2)) (${IF} (= 1 2) (1))`,
      );
    }

    const [, condition, thenBranch, elseBranch] = list;

    const resolved = evaluate(condition, environment);

    if (resolved.type !== ASTNodeType.BOOLEAN) {
      throw new InvalidArgumentException(
        `Condition must resolve to a boolean.`,
      );
    }

    if (resolved.value) {
      return evaluate(thenBranch, environment);
    }

    if (elseBranch) {
      return evaluate(elseBranch, environment);
    }

    return {
      type: ASTNodeType.NIL,
      value: null,
    };
  },
};
