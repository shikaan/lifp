import {test} from 'node:test'
import * as assert from "node:assert";

import {read} from "./read.ts";
import type {AST} from './types.ts';
import {Operator, TokenTag} from "./types.ts";

type TestCase = [string, AST, string];

test('single item list', () => {
  const testCases: TestCase[] = [
    ['(1)', [[TokenTag.NUMBER, 1]], 'number'],
    ['(1.23)', [[TokenTag.NUMBER, 1.23]], 'float'],
    ['(true)', [[TokenTag.BOOLEAN, true]], 'boolean'],
    ['("string")', [[TokenTag.STRING, "string"]], 'string'],
    ['(not)', [[TokenTag.OPERATOR, Operator.NOT]], 'operator'],
  ]

  for (const [input, expected, name] of testCases) {
    const result = read(input)
    assert.deepStrictEqual(result, expected, name)
  }
})

test('multi-item lists', () => {
  const testCases: TestCase[] = [
    ['(not 1)', [[TokenTag.OPERATOR, Operator.NOT], [TokenTag.NUMBER, 1]], 'unary operator'],
    ['(+ 1 2)', [[TokenTag.OPERATOR, Operator.PLUS], [TokenTag.NUMBER, 1], [TokenTag.NUMBER, 2]], 'binary operator'],
    ['(1\n2\t3 4\r)', [[TokenTag.NUMBER, 1], [TokenTag.NUMBER, 2], [TokenTag.NUMBER, 3], [TokenTag.NUMBER, 4]], 'whitespaces'],
  ]

  for (const [input, expected, name] of testCases) {
    const result = read(input)
    assert.deepStrictEqual(result, expected, name)
  }
})

test('tokenizes strings', () => {
  const testCases: TestCase[] = [
    [
      '(+ "a" "b")',
      [
        [TokenTag.OPERATOR, Operator.PLUS],
        [TokenTag.STRING, "a"],
        [TokenTag.STRING, "b"]
      ],
      'single char with binary operator'
    ],
    [
      '("lol")',
      [
        [TokenTag.STRING, "lol"],
      ],
      'multi-char list'
    ],
    [
      '("lol"\n\t "asdf")',
      [
        [TokenTag.STRING, "lol"],
        [TokenTag.STRING, "asdf"],
      ],
      'multi-char list with whitespaces'
    ]
  ]

  for (const [input, expected, name] of testCases) {
    const result = read(input)
    assert.deepStrictEqual(result, expected, name)
  }
})

test('tokenizes sub lists', () => {
  const result = read('(+ (1 2) (1))')
  assert.deepStrictEqual(result, [
    [TokenTag.OPERATOR, Operator.PLUS],
    [[TokenTag.NUMBER, 1], [TokenTag.NUMBER, 2]],
    [[TokenTag.NUMBER, 1]],
  ])
})

test('rejects invalid string', () => {
  const invalid = [
    '(1 2))', // unmatched closed
    '((0)', // unmatched open
    'not', // no parenthesis
    '~test', // invalid chars,
    '(not-an-operator 1 2)', // invalid operator
  ]
  for (const testCase of invalid) {
    assert.throws(() => read(testCase), `${testCase} did not fail`)
  }
})