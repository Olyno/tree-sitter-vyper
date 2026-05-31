const path = require("path");
const fs = require("fs");

function loadLanguage() {
  const wasmPath = path.join(__dirname, "..", "..", "tree-sitter-vyper.wasm");
  if (!fs.existsSync(wasmPath)) {
    throw new Error(
      "tree-sitter-vyper: tree-sitter-vyper.wasm not found. " +
      "Run `npx tree-sitter build --wasm` to generate it."
    );
  }
  const Parser = require("tree-sitter");
  return Parser.Language.load(fs.readFileSync(wasmPath));
}

module.exports = loadLanguage();

try {
  module.exports.nodeTypeInfo = require("../../src/node-types.json");
} catch (_) {}
