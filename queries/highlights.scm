; Vyper syntax highlighting queries

; ===== COMMENTS =====
(comment) @comment

; ===== STRINGS =====
(string) @string
(escape_sequence) @escape

; ===== NUMBERS =====
(integer) @number
(float) @number

; ===== CONSTANTS =====
[
  (none)
  (true)
  (false)
] @constant.builtin

((identifier) @constant.builtin
  (#match? @constant.builtin "^(ZERO_ADDRESS|EMPTY_BYTES32|MAX_INT128|MIN_INT128|MAX_INT256|MIN_INT256|MAX_UINT256)$"))

((identifier) @constant
  (#match? @constant "^[A-Z][A-Z_0-9]*$"))

; ===== KEYWORDS =====
[
  "assert"
  "break"
  "continue"
  "def"
  "elif"
  "else"
  "for"
  "from"
  "if"
  "import"
  "pass"
  "raise"
  "return"
  "as"
  "in"
  "is"
  "not"
  "and"
  "or"
  "event"
  "struct"
  "enum"
  "interface"
  "log"
  "extcall"
  "staticcall"
] @keyword

; ===== OPERATORS =====
[
  "-"
  "-="
  "!="
  "*"
  "**"
  "**="
  "*="
  "/"
  "//"
  "//="
  "/="
  "&"
  "&="
  "%"
  "%="
  "^"
  "^="
  "+"
  "->"
  "+="
  "<"
  "<<"
  "<<="
  "<="
  "="
  "=="
  ">"
  ">="
  ">>"
  ">>="
  "|"
  "|="
  "~"
] @operator

; ===== PUNCTUATION =====
[
  "."
  ","
  ":"
  ";"
] @punctuation.delimiter

[
  "("
  ")"
  "["
  "]"
] @punctuation.bracket

; ===== FUNCTIONS =====
(function_definition
  name: (identifier) @function)

((function_definition
  name: (identifier) @constructor)
  (#eq? @constructor "__init__"))

(call
  function: (identifier) @function)

(call
  function: (attribute attribute: (identifier) @function.method))

((call
  function: (identifier) @function.builtin)
  (#match? @function.builtin "^(convert|keccak256|sha256|slice|concat|len|create_minimal_proxy_to|create_copy_of|create_from_blueprint|selfdestruct|send|raw_call|raw_log|raw_revert|raw_create|abi_decode|abi_encode|method_id|shift|empty|as_wei_value|as_unitless_number|unsafe_add|unsafe_sub|unsafe_mul|unsafe_div|uint256_addmod|uint256_mulmod|pow_mod256|sqrt|isqrt|abs|ceil|epsilon|floor|max|min|max_value|min_value|ecrecover|ecadd|ecmul|extract32|uint2str|print|clear|pop|append)$"))

; ===== DECORATORS =====
(decorator) @attribute
((decorator (identifier) @attribute.builtin)
  (#match? @attribute.builtin "^(external|internal|public|private|view|pure|payable|nonpayable|nonreentrant|deploy)$"))

; ===== TYPES =====
(type (identifier) @type)
(generic_type (identifier) @type)

((identifier) @type.builtin
  (#match? @type.builtin "^(bool|address|decimal|String|Bytes|HashMap|DynArray|uint(8|16|24|32|40|48|56|64|72|80|88|96|104|112|120|128|136|144|152|160|168|176|184|192|200|208|216|224|232|240|248|256)|int(8|16|24|32|40|48|56|64|72|80|88|96|104|112|120|128|136|144|152|160|168|176|184|192|200|208|216|224|232|240|248|256)|bytes(1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|25|26|27|28|29|30|31|32)$"))

; ===== TYPE DEFINITIONS =====
(struct_definition name: (identifier) @type)
(event_definition name: (identifier) @type)
(enum_definition name: (identifier) @type)
(interface_definition name: (identifier) @type)

; ===== SPECIAL VARIABLES =====
((identifier) @variable.builtin
  (#match? @variable.builtin "^(self|msg|block|tx|chain)$"))

; ===== VARIABLES & PARAMETERS =====
(identifier) @variable
(parameter name: (identifier) @parameter)

; ===== ATTRIBUTE ACCESS =====
(attribute attribute: (identifier) @property)
