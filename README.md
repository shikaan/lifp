<p align="center">
  <img width="96" height="96" src="./docs/logo.png" alt="logo">
</p>

<h1 align="center">lifp</h1>

<p align="center">
A Lisp dialect and toolchain. All in one binary.
</p>

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

> [!WARNING]  
> This is very young and highly unstable. Don't take it too seriously.

Launch the executable to start the REPL 
```shell
lifp
```

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