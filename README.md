lisp.js
---

A JavaScript lisp interpreter bundled in a single binary

## Warning!

This is very young and highly unstable. Don't take it too seriously.

## Binaries

Pre-built binaries are available for the following platforms:

- macOS (Intel and Apple Silicon)
- Linux (x64 and ARM64)
- Windows (x64)

You can download the latest binaries from the [Releases](https://github.com/shikaan/lisp-node/releases) page.

## Building from Source

To build the project from source, you need [Bun](https://bun.sh/) installed.

```bash
# Clone the repository
git clone https://github.com/shikaan/lisp-node.git
cd lisp-node

# Install dependencies
bun install

# Build for your current platform
bun run build
```

The binary will be created in the `build` directory.

## CI/CD

This project uses GitHub Actions to automatically build binaries for multiple platforms using Bun's cross-compiler. The workflow runs on every push to the main branch, pull request to main, and when a new release is created.

When a release is created, the binaries are automatically attached to the release.
