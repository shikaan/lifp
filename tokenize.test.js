import {test} from 'node:test'
import assert from "node:assert";
import {Operator, tokenize, TokenType} from "./tokenize.js";

test('single item list', () => {
  const testCases = [
    ['(1)', [[TokenType.NUMBER, 1]], 'number'],
    ['(1.23)', [[TokenType.NUMBER, 1.23]], 'float'],
    ['(true)', [[TokenType.BOOLEAN, true]], 'boolean'],
    ['("string")', [[TokenType.STRING, "string"]], 'string'],
    ['(not)', [[TokenType.OPERATOR, Operator.NOT]], 'operator'],
  ]

  for (const [input, expected, name] of testCases) {
    const result = tokenize(input)
    assert.deepStrictEqual(result, expected, name)
  }
})

test('multi-item lists', () => {
  const testCases = [
    ['(not 1)', [[TokenType.OPERATOR, Operator.NOT], [TokenType.NUMBER, 1]], 'unary operator'],
    ['(+ 1 2)', [[TokenType.OPERATOR, Operator.PLUS], [TokenType.NUMBER, 1], [TokenType.NUMBER, 2]], 'binary operator'],
    ['(1\n2\t3 4\r)', [[TokenType.NUMBER, 1], [TokenType.NUMBER, 2], [TokenType.NUMBER, 3], [TokenType.NUMBER, 4]], 'whitespaces'],
  ]

  for (const [input, expected, name] of testCases) {
    const result = tokenize(input)
    assert.deepStrictEqual(result, expected, name)
  }
})

test('tokenizes strings', () => {
  const testCases = [
    [
      '(+ "a" "b")',
      [
        [TokenType.OPERATOR, Operator.PLUS],
        [TokenType.STRING, "a"],
        [TokenType.STRING, "b"]
      ],
      'single char with binary operator'
    ],
    [
      '("lol")',
      [
        [TokenType.STRING, "lol"],
      ],
      'multi-char list'
    ],
    [
      '("lol"\n\t "asdf")',
      [
        [TokenType.STRING, "lol"],
        [TokenType.STRING, "asdf"],
      ],
      'multi-char list with whitespaces'
    ]
  ]

  for (const [input, expected, name] of testCases) {
    const result = tokenize(input)
    assert.deepStrictEqual(result, expected, name)
  }
})

test('tokenizes sub lists', () => {
  const result = tokenize('(+ (1 2) (1))')
  assert.deepStrictEqual(result, [
    [TokenType.OPERATOR, Operator.PLUS],
    [[TokenType.NUMBER, 1], [TokenType.NUMBER, 2]],
    [[TokenType.NUMBER, 1]],
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
    assert.throws(() => tokenize(testCase), `${testCase} did not fail`)
  }
})