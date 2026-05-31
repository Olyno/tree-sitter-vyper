[
  (if_statement)
  (for_statement)
  (function_definition)
  (event_definition)
  (struct_definition)
  (enum_definition)
  (interface_definition)
  (import_from_statement)
  (parenthesized_expression)
  (binary_operator)
] @indent

(argument_list) @aligned_indent
(parameters) @aligned_indent

[
  ")"
  "]"
  (elif_clause)
  (else_clause)
] @branch

(string) @auto
