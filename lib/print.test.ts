import { expect, test } from "bun:test";
import { l, n } from "../tests/utils.js";
import { print } from "./print.js";
import { ASTNodeType, type Expression } from "./types.js";

test("atoms", () => {
	const tests: [Expression, string][] = [
		[n(ASTNodeType.NIL, null), "nil"],
		[n(ASTNodeType.NUMBER, 1), "1"],
		[n(ASTNodeType.SYMBOL, "asd"), "asd"],
		[n(ASTNodeType.STRING, "asd"), '"asd"'],
		[n(ASTNodeType.KEYWORD, ":asd"), ":asd"],
		[n(ASTNodeType.BOOLEAN, true), "true"],
	];

	for (const [input, expected] of tests) {
		expect(print(input), expected).toEqual(expected);
	}
});

test("lists", () => {
	const tests: [Expression, string][] = [
		[l([n(ASTNodeType.NIL, null)]), "(nil)"],
		[
			l([
				n(ASTNodeType.NIL, null),
				n(ASTNodeType.NUMBER, 1),
				n(ASTNodeType.BOOLEAN, true),
				n(ASTNodeType.KEYWORD, ":lol"),
			]),
			"(nil 1 true :lol)",
		],
		[l([n(ASTNodeType.NIL, null), l([n(ASTNodeType.NUMBER, 1)])]), "(nil (1))"],
	];

	for (const [input, expected] of tests) {
		expect(print(input), expected).toEqual(expected);
	}
});
