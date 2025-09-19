import re
import sys
import os
import glob

doc_file = "docs/index.md"
man_file = "docs/lifp.man"
files = sorted(glob.glob('lifp/std/*.c'))

with open(doc_file, "w") as out:
  version = os.getenv("VERSION") or "v0.0.0"
  sha = os.getenv("SHA") or "dev"
  out.write(f"lifp - {version} ({sha})\n---\n### Table of Contents\n")

  for filename in files:
    module, ext = os.path.splitext(os.path.basename(filename))
    out.write(f"  * [{module}](#{module})\n")

  out.write("\n")

with open(man_file, "w") as mf:
  version = os.getenv("VERSION") or "v0.0.0"
  sha = os.getenv("SHA") or "dev"
  mf.write(f".TH lifp 1 \"{version}\" \"{sha}\" \"lifp manual\"\n")
  mf.write(".SH INTRODUCTION\n")

  mf.write(
    """
lifp is practical functional programming language belonging to the LISP family.
It features a REPL, file execution, a standard library, and modern conveniences.

Here's your first program:

    (io:stdout! \"Hello world!\") ; prints \"Hello World\"

This manual documents its standard library functions and usage examples.\n\n""");

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

  with open(man_file, "a") as mf:
    mf.write(f".SH {module.upper()}\n")
    mf.write(f".SH\n")
    for method in api_docs:
      mf.write(f".SS {method['name']}\n")  # Subsection for function/macro name
      mf.write(f"{method['desc']}\n\n")    # Description
      if method['example']:
        mf.write(".nf\n")                  # Disable filling for code block
        mf.write(f"{method['example']}\n") # Example code
        mf.write(".fi\n\n")                # Re-enable filling

with open(man_file, "a") as mf:
  github_url = "https://github.com/shikaan/lifp"
  mf.write(f".SH SEE ALSO\nFor more information, feedback, or bug reports {github_url}\n")
