import fs from "node:fs";
import * as path from "node:path";
import { parse } from "comment-parser";

type MethodDocumentation = {
  name: string;
  description: string;
  example: string;
};

const result: Record<string, MethodDocumentation> = {};

const STD_DIR = path.join(__dirname, "..", "lib", "std");
const files = fs
  .readdirSync(STD_DIR)
  .filter((f) => f !== "index.ts" && !f.endsWith("test.ts"));

console.group(`Parsing docs for ${files.length} modules in 'lib/std'...`);
for (const file of files) {
  console.group(`Reading ${file}...`);
  const code = fs.readFileSync(path.join(STD_DIR, file), "utf-8");
  const jsdocs = code.match(/\/\*\*[\s\S]*?\*\//g);

  if (!jsdocs) {
    console.groupEnd();
    console.log(`Reading ${file} - done (no documentation found)`);
    continue;
  }

  jsdocs.forEach((doc) => {
    const [{ description, tags }] = parse(doc, { spacing: "preserve" });
    const methodDocumentation: MethodDocumentation = {
      description,
      example: "",
      name: "",
    };

    tags.forEach((tag) => {
      methodDocumentation[tag.tag] = (tag.name || tag.description).trim();
    });

    if (methodDocumentation.name) {
      result[methodDocumentation.name] = methodDocumentation;
    }
  });
  console.groupEnd();
  console.log(`Reading ${file} - done (${jsdocs.length} methods found)`);
}
console.groupEnd();
console.log(`Parsing docs - done`);

console.group(`Generating docs.json...`);
fs.writeFileSync(
  path.join(__dirname, "..", "docs", "docs.json"),
  // @ts-expect-error comes from runner
  JSON.stringify({ version: __VERSION__, functions: result }, null, 2),
);
console.groupEnd();
console.log(`Generating docs.json - done`);

console.group(`Generating docs.md...`);
// @ts-expect-error comes from runner
let buffer = `lifp - ${__VERSION__}\n---\n\n`;
for (const method of Object.values(result)) {
  buffer += `## ${method.name}\n\n${method.description}\n\n\`\`\`common lisp\n${method.example}\n\`\`\`\n\n\n\n`;
}

fs.writeFileSync(path.join(__dirname, "..", "docs", "docs.md"), buffer);
console.groupEnd();
console.log(`Generating docs.json - done`);
