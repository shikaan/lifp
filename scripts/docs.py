import re
import sys
import os
import glob

doc_file = "docs/index.md"
files = sorted(glob.glob('lifp/std/*.c'))

with open(doc_file, "w") as out:
  version = os.getenv("VERSION") or "v0.0.0"
  sha = os.getenv("SHA") or "dev"
  out.write(f"lifp - {version} ({sha})\n---\n# Table of Contents\n")

  for filename in files:
    module, ext = os.path.splitext(os.path.basename(filename))
    out.write(f"  * [{module}](#{module})\n")
  
  out.write("\n")


for filename in files:
  module, ext = os.path.splitext(os.path.basename(filename))

  with open(filename, "r") as f:
    lines = f.readlines()

  # Extract doc block before ___HEADER_END___
  doc_block = []
  for line in lines:
    if "___HEADER_END___" in line:
      break
    doc_block.append(line.rstrip("\n")[3:])

  # Find all JSDoc-style docblocks and their associated function/macro
  api_docs = []
  i = 0
  while i < len(lines):
    if lines[i].strip().startswith("/**"):
      # Parse docblock
      desc = []
      name = ""
      example = []
      in_example = False
      i += 1
      while i < len(lines) and not lines[i].strip().startswith("*/"):
        l = lines[i].strip()
        if "@name" in l:
          name = l.split("@name",1)[1].strip()
        elif "@example" in l:
          in_example = True
        elif in_example:
          if l.startswith("*"):
            l = l[1:].lstrip()
          if l == "":
            pass
          else:
            example.append(l)
        elif l.startswith("*"):
          l = l[1:].lstrip()
          if l and not l.startswith("@"):
            desc.append(l)
        i += 1
      api_docs.append({
        "name": name,
        "desc": " ".join(desc),
        "example": "\n".join(example),
      })
    i += 1

  # Sort api_docs alphabetically by 'name'
  api_docs = sorted(api_docs, key=lambda x: x["name"].lower())

  with open(doc_file, "a") as out:
    out.write(f"# {module}\n\n")
    for l in doc_block:
      out.write(l + "\n")

    for method in api_docs:
      out.write(f"### {method['name']}\n\n{method['desc']}\n\n```lisp\n{method['example']}\n```\n\n\n")
