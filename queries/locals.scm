;;; Program structure
(module) @scope

; Imports
(aliased_import
  alias: (identifier) @definition.import)
(import_statement
  name: (dotted_name ((identifier) @definition.import)))
(import_from_statement
  name: (dotted_name ((identifier) @definition.import)))

; Function parameters
(parameters
  (identifier) @definition.parameter)
(default_parameter
  (identifier) @definition.parameter)
(typed_parameter
  (identifier) @definition.parameter)
(typed_default_parameter
  (identifier) @definition.parameter)

; Function defines function and scope
((function_definition
  name: (identifier) @definition.function) @scope
 (#set! definition.function.scope "parent"))

;;; Loops
(for_statement
  left: (identifier) @definition.var)
(for_statement
  left: (typed_parameter (identifier) @definition.var))
(for_statement
  left: (pattern_list (identifier) @definition.var))
(for_statement
  left: (tuple_pattern (identifier) @definition.var))

;;; Assignments
(assignment
 left: (identifier) @definition.var)
(assignment
 left: (pattern_list (identifier) @definition.var))
(assignment
 left: (tuple_pattern (identifier) @definition.var))
(assignment
 left: (attribute (identifier) (identifier) @definition.field))

;;; REFERENCES
(identifier) @reference
