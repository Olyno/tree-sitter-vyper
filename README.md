# tree-sitter-vyper

[![CI](https://img.shields.io/github/actions/workflow/status/Olyno/tree-sitter-vyper/ci.yml?logo=github&label=CI)](https://github.com/Olyno/tree-sitter-vyper/actions/workflows/ci.yml)
[![npm](https://img.shields.io/npm/v/@olyno/tree-sitter-vyper?logo=npm&label=npm)](https://www.npmjs.com/package/@olyno/tree-sitter-vyper)

Vyper grammar for [tree-sitter](https://github.com/tree-sitter/tree-sitter).  
Forked from [tree-sitter-python](https://github.com/tree-sitter/tree-sitter-python), rewritten for the Vyper smart contract language.

## Supported constructs

- `event`, `struct`, `interface` with member definitions
- `enum` with member values
- `log` statement
- `extcall` and `staticcall` as assignable expressions
- `for i: uint256 in range(...)` with typed loop variables
- All standard Vyper syntax: functions, decorators, imports, type annotations, etc.

## Installation

### Node.js (WASM)

```bash
npm install @olyno/tree-sitter-vyper
```

```js
const Parser = require("tree-sitter");
const Vyper = require("@olyno/tree-sitter-vyper");

const parser = new Parser();
parser.setLanguage(Vyper);

const tree = parser.parse(`event Transfer:
    sender: indexed(address)
    receiver: indexed(address)
    value: uint256
`);
console.log(tree.rootNode.toString());
```

The Node binding is **WASM-only** — no native compilation, no `node-gyp`.  
Requires `tree-sitter ^0.25.0` as a peer dependency.

### Rust

Add to `Cargo.toml`:

```toml
[dependencies]
tree-sitter = "0.25"
tree-sitter-vyper = { git = "https://github.com/Olyno/tree-sitter-vyper" }
```

```rust
use tree_sitter::Parser;

let mut parser = Parser::new();
parser.set_language(&tree_sitter_vyper::LANGUAGE.into())?;

let tree = parser.parse("x: uint256 = 42", None).unwrap();
println!("{}", tree.root_node().to_sexp());
```

## Building from source

### Prerequisites

- Node.js ≥ 18
- [tree-sitter-cli](https://github.com/tree-sitter/tree-sitter) (`npm install -g tree-sitter-cli` or `npx tree-sitter`)

### Generate the parser

```bash
tree-sitter generate
```

### Run tests

```bash
tree-sitter test
```

### Build WASM

```bash
tree-sitter build --wasm
```

This produces `tree-sitter-vyper.wasm` — use it with `Parser.Language.load()`:

```js
const Vyper = await Parser.Language.load("./tree-sitter-vyper.wasm");
parser.setLanguage(Vyper);
```

### Rust tests

```bash
cargo test
```

## Queries

The grammar ships with:

| File | Purpose |
|---|---|
| `queries/highlights.scm` | Syntax highlighting |
| `queries/tags.scm` | Symbol tagging (outline, breadcrumbs) |
| `queries/indents.scm` | Indentation rules |
| `queries/folds.scm` | Code folding |

## References

- [Vyper Documentation](https://docs.vyperlang.org/)
- [Tree-sitter Documentation](https://tree-sitter.github.io/tree-sitter/)

## License

MIT — see [LICENSE](./LICENSE).
