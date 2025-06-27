import { InvalidArgumentException } from "../errors.js";
import { type ASTNodeList, ASTNodeType, type Lambda } from "../types.js";

export const lists: Record<string, Lambda> = {
  "list.count": (nodes) => {
    if (nodes.length !== 1 || nodes[0].type !== ASTNodeType.LIST) {
      throw new InvalidArgumentException(
        "'list.count' takes a list as argument. Example: (list.count my-list).",
      );
    }
    return {
      type: ASTNodeType.NUMBER,
      value: nodes[0].value.length,
    };
  },
  "list.map": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[0].type !== ASTNodeType.FUNCTION
    ) {
      throw new InvalidArgumentException(
        "'list.map' takes a lambda and a list as argument. Example: (list.map (fn* (item idx) (myfn item idx)) my-list).",
      );
    }
    const [lambda, list] = nodes;
    return {
      type: ASTNodeType.LIST,
      value: list.value.map((node: ASTNodeList, value: number) =>
        lambda.value([node, { type: ASTNodeType.NUMBER, value }]),
      ),
    };
  },
  "list.each": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[0].type !== ASTNodeType.FUNCTION
    ) {
      throw new InvalidArgumentException(
        "'list.each' takes a lambda and a list as argument. Example: (list.each (fn* (item idx) (myfn item idx)) my-list).",
      );
    }
    const [lambda, list] = nodes;
    list.value.forEach((node: ASTNodeList, value) =>
      lambda.value([node, { type: ASTNodeType.NUMBER, value }]),
    );
    return {
      type: ASTNodeType.NIL,
      value: null,
    };
  },
  "list.from": (nodes) => ({
    type: ASTNodeType.LIST,
    value: nodes,
  }),
  "list.nth": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[0].type !== ASTNodeType.NUMBER ||
      nodes[1].type !== ASTNodeType.LIST
    ) {
      throw new InvalidArgumentException(
        "'list.nth' takes a number and a list as argument. Example: (list.nth 0 my-list).",
      );
    }

    const [index, list] = nodes;
    const item = list.value[index.value];

    return (
      item ?? {
        type: ASTNodeType.NIL,
        value: null,
      }
    );
  },
  "list.filter": (nodes) => {
    if (
      nodes.length !== 2 ||
      nodes[1].type !== ASTNodeType.LIST ||
      nodes[0].type !== ASTNodeType.FUNCTION
    ) {
      throw new InvalidArgumentException(
        "'list.filter' takes a lambda and a list as argument. Example: (list.filter (fn* (item idx) (= item 0)) my-list).",
      );
    }
    const [lambda, list] = nodes;
    return {
      type: ASTNodeType.LIST,
      value: list.value.filter(
        (node: ASTNodeList, value: number) =>
          lambda.value([node, { type: ASTNodeType.NUMBER, value }]).value,
      ),
    };
  },
};
