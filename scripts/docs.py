import re
import sys
import os
import glob

def extract_api_docs(filename):
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
  return doc_block, api_docs

def generate_index_md(doc_file, src_files):
  version = os.getenv("VERSION") or "v0.0.0"
  sha = os.getenv("SHA") or "dev"
  with open(doc_file, "w") as out:
    out.write(f"lifp - {version} ({sha})\n---\n### Table of Contents\n")
    for filename in src_files:
      module, ext = os.path.splitext(os.path.basename(filename))
      out.write(f"  * [{module}](#{module})\n")
    out.write("\n")

    for filename in src_files:
      module, ext = os.path.splitext(os.path.basename(filename))
      doc_block, api_docs = extract_api_docs(filename)
      out.write(f"# {module}\n\n")
      for l in doc_block:
        out.write(l + "\n")
      for method in api_docs:
        out.write(f"### {method['name']}\n\n{method['desc']}\n\n```lisp\n{method['example']}\n```\n\n\n")

def generate_manpage(man_file, src_files):
  version = os.getenv("VERSION") or "v0.0.0"
  sha = os.getenv("SHA") or "dev"
  with open(man_file, "w") as mf:
    mf.write(f".TH lifp 1 \"{version}\" \"{sha}\" \"lifp manual\"\n")
    mf.write(".SH INTRODUCTION\n")
    mf.write(
      """
lifp is practical functional programming language belonging to the LISP family.
It features a REPL, file execution, a standard library, and modern conveniences.

Here's your first program:

  (io:stdout! \"Hello world!\") ; prints \"Hello World\"

This manual documents its standard library functions and usage examples.\n\n""")
    for filename in src_files:
      module, ext = os.path.splitext(os.path.basename(filename))
      _, api_docs = extract_api_docs(filename)
      mf.write(f".SH {module.upper()}\n")
      mf.write(f".SH\n")
      for method in api_docs:
        mf.write(f".SS {method['name']}\n")
        mf.write(f"{method['desc']}\n\n")
        if method['example']:
          mf.write(".nf\n")
          mf.write(f"{method['example']}\n")
          mf.write(".fi\n\n")
    github_url = "https://github.com/shikaan/lifp"
    mf.write(f".SH SEE ALSO\nFor more information, feedback, or bug reports {github_url}\n")

def generate_doc_header(repl_doc, src_files):
  count = 0
  with open(repl_doc, "w") as repl:
    repl.write("// Auto-generated documentation header\n")
    repl.write("// Do not modify manually\n\n")
    repl.write("typedef struct {\n")
    repl.write("  const char *name;\n")
    repl.write("  const char *description;\n")
    repl.write("  const char *example;\n")
    repl.write("} doc_record_t;\n\n")
    repl.write("static const doc_record_t DOCS[] = {\n")
    for filename in src_files:
      _, api_docs = extract_api_docs(filename)
      count += len(api_docs)
      for method in api_docs:
        name = method['name'].replace('"', '\\"')[:32]
        desc = method['desc'].replace('"', '\\"').replace('\n', ' ')[:128]
        example = method['example'].replace('"', '\\"').replace('\n', '\\n')[:128]
        repl.write(f'  {{"{name}", "{desc}", "{example}"}},\n')
    repl.write("};\n")
    repl.write(f"static const unsigned int DOCS_COUNT = {count};\n")

if __name__ == "__main__":
  doc_file = "docs/index.md"
  man_file = "artifacts/lifp.1"
  repl_doc = "artifacts/docs.h"

  src_files = glob.glob('lifp/std/*.c')
  src_files.append('lifp/specials.c')
  src_files = sorted(src_files)

  if len(sys.argv) < 2:
    print("Usage: docs.py [web|man|repl]")
    sys.exit(1)

  mode = sys.argv[1].lower()
  if mode == "web":
    generate_index_md(doc_file, src_files)
  elif mode == "man":
    generate_manpage(man_file, src_files)
  elif mode == "repl":
    generate_doc_header(repl_doc, src_files)
  else:
    print("Unknown mode. Use 'web', 'man', or 'repl'.")
    sys.exit(1)
