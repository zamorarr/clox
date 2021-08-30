#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

// initialize scanner
void initScanner(Scanner* scanner, const char* source) {
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
}

static bool isAtEnd(Scanner* scanner) {
  return *scanner->current == '\0';
}

// consume next character
static char advance(Scanner* scanner) {
  // increase current
  scanner->current++;
  // return previous character
  return scanner->current[-1];
}

// look at next character, but do not consume
static char peek(Scanner* scanner) {
  return *scanner->current;
}

// look two characters ahead, but do not consume either
static char peekNext(Scanner* scanner) {
  if (isAtEnd(scanner)) return '\0';
  return scanner->current[1];
}

// check if current value matches expected
static bool match(Scanner* scanner, char expected) {
  // if at end of file, no match
  if (isAtEnd(scanner)) return false;

  // if current val not equal to expected, no match
  if (*scanner->current != expected) return false;

  // move pointer up and return true
  scanner->current++;
  return true;
}

// create new token
static Token makeToken(Scanner* scanner, TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.length = (int) (scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}

static Token errorToken(Scanner* scanner, const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int) strlen(message);
  token.line = scanner->line;
  return token;
}

static void skipWhiteSpace(Scanner* scanner) {
  for (;;) {
    // get next character
    char c = peek(scanner);

    switch (c) {
      // consume whitespace
      case ' ':
      case '\r':
      case '\t':
        advance(scanner);
        break;
      // consume new line
      case '\n':
        scanner->line++;
        advance(scanner);
        break;
      // consume comments
      case '/':
        if (peekNext(scanner) == '/') {
          // comment goes until end of line
          while(peek(scanner) != '\n' && !isAtEnd(scanner)) advance(scanner);
        } else {
          // not a comment, probably division
          return;
        }
        break;
      default:
        return;
    }
  }
}

static Token string(Scanner* scanner) {
  // loop until we find terminating quotes
  while (peek(scanner) != '"' && !isAtEnd(scanner)) {
    if (peek(scanner) == '\n') scanner->line++;
    advance(scanner);
  }

  // found end of file before terminating quotes
  if (isAtEnd(scanner)) return errorToken(scanner, "Unterminated string.");

  // found terminating quotes
  advance(scanner);
  return makeToken(scanner, TOKEN_STRING);
}

// scan next token
Token scanToken(Scanner* scanner) {
  // skip whitespace
  skipWhiteSpace(scanner);

  // set scanner start to current location
  scanner->start = scanner->current;

  // if we are at end of file, make an EOF token
  if (isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);

  // get next character
  char c = advance(scanner);

  // make token
  switch (c) {
    case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case ';': return makeToken(scanner, TOKEN_SEMICOLON);
    case ',': return makeToken(scanner, TOKEN_COMMA);
    case '.': return makeToken(scanner, TOKEN_DOT);
    case '-': return makeToken(scanner, TOKEN_MINUS);
    case '+': return makeToken(scanner, TOKEN_PLUS);
    case '/': return makeToken(scanner, TOKEN_SLASH);
    case '*': return makeToken(scanner, TOKEN_STAR);
    case '!':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL: TOKEN_BANG);
    case '=':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL: TOKEN_EQUAL);
    case '<':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return string(scanner);
  }

  return errorToken(scanner, "Unexpected character.");
}