import type { FilePointer } from "./types.js";

const CURRENT_FILE_POINTER: FilePointer = ["repl", 1];

export const updatePointer = (fp: FilePointer) => {
  CURRENT_FILE_POINTER[0] = fp[0];
  CURRENT_FILE_POINTER[1] = fp[1];
};

export const incrementLine = () => {
  CURRENT_FILE_POINTER[1]++;
};

export const getPointer = (): FilePointer => [
  CURRENT_FILE_POINTER[0],
  CURRENT_FILE_POINTER[1],
];

export const getLine = () => CURRENT_FILE_POINTER[1];
export const getFile = () => CURRENT_FILE_POINTER[0];
