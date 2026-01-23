// Converted from scanner.cc to C for Zed compatibility
#include <tree_sitter/parser.h>
#include <wctype.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

enum TokenType {
  NEWLINE,
  INDENT,
  DEDENT,
  STRING_START,
  STRING_CONTENT,
  STRING_END,
  COMMENT,
  CLOSE_PAREN,
  CLOSE_BRACKET,
  CLOSE_BRACE,
};

#define DELIMITER_SINGLE_QUOTE (1 << 0)
#define DELIMITER_DOUBLE_QUOTE (1 << 1)
#define DELIMITER_BACK_QUOTE   (1 << 2)
#define DELIMITER_RAW          (1 << 3)
#define DELIMITER_FORMAT       (1 << 4)
#define DELIMITER_TRIPLE       (1 << 5)
#define DELIMITER_BYTES        (1 << 6)

typedef struct {
  char flags;
} Delimiter;

static bool delimiter_is_format(Delimiter *d) { return d->flags & DELIMITER_FORMAT; }
static bool delimiter_is_raw(Delimiter *d) { return d->flags & DELIMITER_RAW; }
static bool delimiter_is_triple(Delimiter *d) { return d->flags & DELIMITER_TRIPLE; }
static bool delimiter_is_bytes(Delimiter *d) { return d->flags & DELIMITER_BYTES; }

static int32_t delimiter_end_character(Delimiter *d) {
  if (d->flags & DELIMITER_SINGLE_QUOTE) return '\'';
  if (d->flags & DELIMITER_DOUBLE_QUOTE) return '"';
  if (d->flags & DELIMITER_BACK_QUOTE) return '`';
  return 0;
}

static void delimiter_set_format(Delimiter *d) { d->flags |= DELIMITER_FORMAT; }
static void delimiter_set_raw(Delimiter *d) { d->flags |= DELIMITER_RAW; }
static void delimiter_set_triple(Delimiter *d) { d->flags |= DELIMITER_TRIPLE; }
static void delimiter_set_bytes(Delimiter *d) { d->flags |= DELIMITER_BYTES; }

static void delimiter_set_end_character(Delimiter *d, int32_t character) {
  switch (character) {
    case '\'': d->flags |= DELIMITER_SINGLE_QUOTE; break;
    case '"':  d->flags |= DELIMITER_DOUBLE_QUOTE; break;
    case '`':  d->flags |= DELIMITER_BACK_QUOTE; break;
    default: assert(false);
  }
}

#define MAX_DELIMITER_STACK 256
#define MAX_INDENT_STACK 256

typedef struct {
  Delimiter delimiter_stack[MAX_DELIMITER_STACK];
  size_t delimiter_stack_size;
  uint16_t indent_length_stack[MAX_INDENT_STACK];
  size_t indent_length_stack_size;
} Scanner;

static void scanner_advance(TSLexer *lexer) {
  lexer->advance(lexer, false);
}

static void scanner_skip(TSLexer *lexer) {
  lexer->advance(lexer, true);
}

static unsigned scanner_serialize(Scanner *scanner, char *buffer) {
  size_t i = 0;

  size_t delimiter_count = scanner->delimiter_stack_size;
  if (delimiter_count > UINT8_MAX) delimiter_count = UINT8_MAX;
  buffer[i++] = (char)delimiter_count;

  if (delimiter_count > 0) {
    memcpy(&buffer[i], scanner->delimiter_stack, delimiter_count);
  }
  i += delimiter_count;

  for (size_t j = 1; j < scanner->indent_length_stack_size && i < TREE_SITTER_SERIALIZATION_BUFFER_SIZE; j++) {
    buffer[i++] = (char)scanner->indent_length_stack[j];
  }

  return (unsigned)i;
}

static void scanner_deserialize(Scanner *scanner, const char *buffer, unsigned length) {
  scanner->delimiter_stack_size = 0;
  scanner->indent_length_stack_size = 1;
  scanner->indent_length_stack[0] = 0;

  if (length > 0) {
    size_t i = 0;

    size_t delimiter_count = (uint8_t)buffer[i++];
    if (delimiter_count > MAX_DELIMITER_STACK) delimiter_count = MAX_DELIMITER_STACK;
    scanner->delimiter_stack_size = delimiter_count;
    if (delimiter_count > 0) {
      memcpy(scanner->delimiter_stack, &buffer[i], delimiter_count);
    }
    i += delimiter_count;

    for (; i < length && scanner->indent_length_stack_size < MAX_INDENT_STACK; i++) {
      scanner->indent_length_stack[scanner->indent_length_stack_size++] = (uint8_t)buffer[i];
    }
  }
}

static bool scanner_scan(Scanner *scanner, TSLexer *lexer, const bool *valid_symbols) {
  bool error_recovery_mode = valid_symbols[STRING_CONTENT] && valid_symbols[INDENT];
  bool within_brackets = valid_symbols[CLOSE_BRACE] || valid_symbols[CLOSE_PAREN] || valid_symbols[CLOSE_BRACKET];

  if (valid_symbols[STRING_CONTENT] && scanner->delimiter_stack_size > 0 && !error_recovery_mode) {
    Delimiter delimiter = scanner->delimiter_stack[scanner->delimiter_stack_size - 1];
    int32_t end_character = delimiter_end_character(&delimiter);
    bool has_content = false;
    while (lexer->lookahead) {
      if ((lexer->lookahead == '{' || lexer->lookahead == '}') && delimiter_is_format(&delimiter)) {
        lexer->mark_end(lexer);
        lexer->result_symbol = STRING_CONTENT;
        return has_content;
      } else if (lexer->lookahead == '\\') {
        if (delimiter_is_raw(&delimiter)) {
          lexer->advance(lexer, false);
        } else if (delimiter_is_bytes(&delimiter)) {
          lexer->mark_end(lexer);
          lexer->advance(lexer, false);
          if (lexer->lookahead == 'N' || lexer->lookahead == 'u' || lexer->lookahead == 'U') {
            lexer->advance(lexer, false);
          } else {
            lexer->result_symbol = STRING_CONTENT;
            return has_content;
          }
        } else {
          lexer->mark_end(lexer);
          lexer->result_symbol = STRING_CONTENT;
          return has_content;
        }
      } else if (lexer->lookahead == end_character) {
        if (delimiter_is_triple(&delimiter)) {
          lexer->mark_end(lexer);
          lexer->advance(lexer, false);
          if (lexer->lookahead == end_character) {
            lexer->advance(lexer, false);
            if (lexer->lookahead == end_character) {
              if (has_content) {
                lexer->result_symbol = STRING_CONTENT;
              } else {
                lexer->advance(lexer, false);
                lexer->mark_end(lexer);
                scanner->delimiter_stack_size--;
                lexer->result_symbol = STRING_END;
              }
              return true;
            } else {
              lexer->mark_end(lexer);
              lexer->result_symbol = STRING_CONTENT;
              return true;
            }
          } else {
            lexer->mark_end(lexer);
            lexer->result_symbol = STRING_CONTENT;
            return true;
          }
        } else {
          if (has_content) {
            lexer->result_symbol = STRING_CONTENT;
          } else {
            lexer->advance(lexer, false);
            scanner->delimiter_stack_size--;
            lexer->result_symbol = STRING_END;
          }
          lexer->mark_end(lexer);
          return true;
        }
      } else if (lexer->lookahead == '\n' && has_content && !delimiter_is_triple(&delimiter)) {
        return false;
      }
      scanner_advance(lexer);
      has_content = true;
    }
  }

  lexer->mark_end(lexer);

  bool found_end_of_line = false;
  uint32_t indent_length = 0;
  int32_t first_comment_indent_length = -1;
  for (;;) {
    if (lexer->lookahead == '\n') {
      found_end_of_line = true;
      indent_length = 0;
      scanner_skip(lexer);
    } else if (lexer->lookahead == ' ') {
      indent_length++;
      scanner_skip(lexer);
    } else if (lexer->lookahead == '\r') {
      indent_length = 0;
      scanner_skip(lexer);
    } else if (lexer->lookahead == '\t') {
      indent_length += 8;
      scanner_skip(lexer);
    } else if (lexer->lookahead == '#') {
      if (first_comment_indent_length == -1) {
        first_comment_indent_length = (int32_t)indent_length;
      }
      while (lexer->lookahead && lexer->lookahead != '\n') {
        scanner_skip(lexer);
      }
      scanner_skip(lexer);
      indent_length = 0;
    } else if (lexer->lookahead == '\\') {
      scanner_skip(lexer);
      if (lexer->lookahead == '\r') {
        scanner_skip(lexer);
      }
      if (lexer->lookahead == '\n') {
        scanner_skip(lexer);
      } else {
        return false;
      }
    } else if (lexer->lookahead == '\f') {
      indent_length = 0;
      scanner_skip(lexer);
    } else if (lexer->lookahead == 0) {
      indent_length = 0;
      found_end_of_line = true;
      break;
    } else {
      break;
    }
  }

  if (found_end_of_line) {
    if (scanner->indent_length_stack_size > 0) {
      uint16_t current_indent_length = scanner->indent_length_stack[scanner->indent_length_stack_size - 1];

      if (valid_symbols[INDENT] && indent_length > current_indent_length) {
        if (scanner->indent_length_stack_size < MAX_INDENT_STACK) {
          scanner->indent_length_stack[scanner->indent_length_stack_size++] = (uint16_t)indent_length;
        }
        lexer->result_symbol = INDENT;
        return true;
      }

      if ((valid_symbols[DEDENT] || (!valid_symbols[NEWLINE] && !within_brackets)) &&
          indent_length < current_indent_length &&
          first_comment_indent_length < (int32_t)current_indent_length) {
        scanner->indent_length_stack_size--;
        lexer->result_symbol = DEDENT;
        return true;
      }
    }

    if (valid_symbols[NEWLINE] && !error_recovery_mode) {
      lexer->result_symbol = NEWLINE;
      return true;
    }
  }

  if (first_comment_indent_length == -1 && valid_symbols[STRING_START]) {
    Delimiter delimiter = {0};

    bool has_flags = false;
    while (lexer->lookahead) {
      if (lexer->lookahead == 'f' || lexer->lookahead == 'F') {
        delimiter_set_format(&delimiter);
      } else if (lexer->lookahead == 'r' || lexer->lookahead == 'R') {
        delimiter_set_raw(&delimiter);
      } else if (lexer->lookahead == 'b' || lexer->lookahead == 'B') {
        delimiter_set_bytes(&delimiter);
      } else if (lexer->lookahead != 'u' && lexer->lookahead != 'U') {
        break;
      }
      has_flags = true;
      scanner_advance(lexer);
    }

    if (lexer->lookahead == '`') {
      delimiter_set_end_character(&delimiter, '`');
      scanner_advance(lexer);
      lexer->mark_end(lexer);
    } else if (lexer->lookahead == '\'') {
      delimiter_set_end_character(&delimiter, '\'');
      scanner_advance(lexer);
      lexer->mark_end(lexer);
      if (lexer->lookahead == '\'') {
        scanner_advance(lexer);
        if (lexer->lookahead == '\'') {
          scanner_advance(lexer);
          lexer->mark_end(lexer);
          delimiter_set_triple(&delimiter);
        }
      }
    } else if (lexer->lookahead == '"') {
      delimiter_set_end_character(&delimiter, '"');
      scanner_advance(lexer);
      lexer->mark_end(lexer);
      if (lexer->lookahead == '"') {
        scanner_advance(lexer);
        if (lexer->lookahead == '"') {
          scanner_advance(lexer);
          lexer->mark_end(lexer);
          delimiter_set_triple(&delimiter);
        }
      }
    }

    if (delimiter_end_character(&delimiter)) {
      if (scanner->delimiter_stack_size < MAX_DELIMITER_STACK) {
        scanner->delimiter_stack[scanner->delimiter_stack_size++] = delimiter;
      }
      lexer->result_symbol = STRING_START;
      return true;
    } else if (has_flags) {
      return false;
    }
  }

  return false;
}

void *tree_sitter_vyper_external_scanner_create() {
  Scanner *scanner = (Scanner *)malloc(sizeof(Scanner));
  scanner->delimiter_stack_size = 0;
  scanner->indent_length_stack_size = 1;
  scanner->indent_length_stack[0] = 0;
  return scanner;
}

bool tree_sitter_vyper_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
  Scanner *scanner = (Scanner *)payload;
  return scanner_scan(scanner, lexer, valid_symbols);
}

unsigned tree_sitter_vyper_external_scanner_serialize(void *payload, char *buffer) {
  Scanner *scanner = (Scanner *)payload;
  return scanner_serialize(scanner, buffer);
}

void tree_sitter_vyper_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
  Scanner *scanner = (Scanner *)payload;
  scanner_deserialize(scanner, buffer, length);
}

void tree_sitter_vyper_external_scanner_destroy(void *payload) {
  Scanner *scanner = (Scanner *)payload;
  free(scanner);
}
