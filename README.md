<p align="center">
  <img width="96" height="96" src="./docs/logo.png" alt="logo">
</p>

<h1 align="center">lifp</h1>

<p align="center">
A Lisp dialect and toolchain. All in one binary.
</p>

> [!WARNING]  
> This is very young and highly unstable. Don't take it too seriously.

## Quick start

### Installation

_MacOS and Linux_
```sh
sudo sh -c "curl -s https://shikaan.github.io/sup/install | REPO=shikaan/lifp sh -"
```

or

```sh
sudo sh -c "wget -q https://shikaan.github.io/sup/install -O- | REPO=shikaan/lifp sh -"
```

_Windows and manual instructions_

Head to the [releases](https://github.com/shikaan/lifp/releases) page and download the executable for your system and architecture.

### Usage

Launch the executable to start the REPL
```shell
lifp
```

Type `help` for more details.

Once you are ready, you can run your code (e.g.,  `script.lifp`) like this

```shell
lifp run ./script.lifp
```

Checkout the [examples](./examples) folder to see more. 

### API Docs

[API Docs](https://shikaan.github.io/lifp/)

### Syntax Highlighting

`.lifp` files can be edited with tools supporting lisps. Common Lisp's syntax highlighters seem to work particularly well.

Some examples:

 - in Vim you can just add the following modline at the end of your file

```
; vim: ft=lisp
```

 - in Visual Studio Code use the [Common Lisp VSCode](https://marketplace.visualstudio.com/items?itemName=ailisp.commonlisp-vscode) extension

## Development

To build the project from source, you need [Bun](https://bun.sh/) installed.

```bash
# Clone the repository
git clone https://github.com/shikaan/lifp.git
cd lifp

# Install dependencies and pre-commit hooks
bun install

# Run locally
bun start

# Build a development version for the current platform
bun run build
```

The binary will be created in the `build` directory.

## License

[MIT](./LICENSE)
